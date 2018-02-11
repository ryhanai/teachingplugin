#ifndef TEACHING_CALCULATOR_H_INCLUDED
#define TEACHING_CALCULATOR_H_INCLUDED

#include <boost/config/warning_disable.hpp>

#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <boost/spirit/include/support_utree.hpp>

#include <vector>
#include <QString>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "TeachingTypes.h"

#include "ArgumentEstimator.h"
#include "TaskExecutor.h"

namespace teaching {

using Eigen::Vector3d;
using Eigen::Matrix3d;
using Eigen::VectorXd;

enum NodeType {
  TYPE_VALUE = 0,
  TYPE_VECTOR = 1,
  TYPE_BIN_OPE = 2,
  TYPE_VARIABLE = 3,
  TYPE_FUNC = 4,
	TYPE_VECTOR6 = 5
};

enum CalcMode {
  CALC_NOTHING = 0,
  CALC_ADDING = 1,
  CALC_SUBSTRACTION = 2,
  CALC_MULTIPLICATION = 3,
  CALC_DIVISION = 4
};

enum ValueMode {
  VAL_SCALAR = 0,
  VAL_VECTOR_3D = 1,
  VAL_VECTOR_6D = 2,
  VAL_MATRIX = 3
};
/////
class ValueNode;
class Vector3dConstNode;
class Vector6dConstNode;
class BinOpNode;
class VariableNode;
class FunCallNode;
typedef boost::shared_ptr<ValueNode> ValueNodeSp;
typedef boost::shared_ptr<Vector3dConstNode> Vector3dConstNodeSp;
typedef boost::shared_ptr<Vector6dConstNode> Vector6dConstNodeSp;
typedef boost::shared_ptr<BinOpNode> BinOpNodeSp;
typedef boost::shared_ptr<VariableNode> VariableNodeSp;
typedef boost::shared_ptr<FunCallNode> FunCallNodeSp;
typedef boost::variant<ValueNodeSp, Vector3dConstNodeSp, Vector6dConstNodeSp, BinOpNodeSp, VariableNodeSp, FunCallNodeSp> Node;

class ValueNode {
public:
  static Node create(double v) { return ValueNodeSp(new ValueNode(v)); }
private:
  ValueNode(double v) :v_(v) {}
  double v_;
  friend class evaluator;
  friend class Calculator;
};

class Vector3dConstNode {
public:
  static Node create(Node const& x, Node const& y, Node const& z) { return Vector3dConstNodeSp(new Vector3dConstNode(x, y, z)); }
private:
  Vector3dConstNode(Node const&x, Node const& y, Node const& z) :x_(x), y_(y), z_(z) {}
  Node x_;
  Node y_;
  Node z_;
  friend class evaluator;
  friend class Calculator;
};

class Vector6dConstNode {
public:
	static Node create(Node const& x, Node const& y, Node const& z, Node const& rx, Node const& ry, Node const& rz) { return Vector6dConstNodeSp(new Vector6dConstNode(x, y, z, rx, ry, rz)); }
private:
	Vector6dConstNode(Node const& x, Node const& y, Node const& z, Node const& rx, Node const& ry, Node const& rz) :x_(x), y_(y), z_(z), Rx_(rx), Ry_(ry), Rz_(rz) {}
	Node x_;
	Node y_;
	Node z_;
	Node Rx_;
	Node Ry_;
	Node Rz_;
	friend class evaluator;
	friend class Calculator;
};

class BinOpNode {
public:
  static Node create(std::string const& op, Node const& lhs, Node const& rhs) { return BinOpNodeSp(new BinOpNode(op, lhs, rhs)); }
private:
  BinOpNode(std::string const& op, Node const& lhs, Node const& rhs) :op_(op), lhs_(lhs), rhs_(rhs) {}
  std::string op_;
  Node lhs_;
  Node rhs_;
  friend class evaluator;
  friend class Calculator;
};

class VariableNode {
public:
  static Node create(std::string const& nm) { return VariableNodeSp(new VariableNode(nm)); }
private:
  VariableNode(std::string const& nm) :nm_(nm) {}
  std::string nm_;
  friend class evaluator;
  friend class Calculator;
};

class FunCallNode {
public:
  static Node create(Node const& fun, Node const& args) { return FunCallNodeSp(new FunCallNode(fun, args)); }
private:
  FunCallNode(Node const& fun, Node const& args) :fun_(fun), args_(args) {}
  Node fun_;
  Node args_;
  friend class evaluator;
  friend class Calculator;
};

// parser
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

template<typename Iterator>
struct makeTree : qi::grammar<Iterator, Node(), qi::space_type> {
  qi::rule<Iterator, Node(), qi::space_type> topexpr, expr, term, factor, variable, literal, args, vector3d_const, vector6d_const;
  makeTree() : makeTree::base_type(topexpr) {
    topexpr = expr > qi::eoi;
#if _MSC_VER >= 1900
	const char* str_plus = "+";
	const char* str_minus = "-";
	const char* str_astarisc = "*";
	const char* str_slash = "/";
	const char* str_comma = ",";

	expr = term[qi::_val = qi::_1]
	> *(('+' > term[qi::_val = phx::bind(&BinOpNode::create, str_plus, qi::_val, qi::_1)])
		| ('-' > term[qi::_val = phx::bind(&BinOpNode::create, str_minus, qi::_val, qi::_1)]));
	term = factor[qi::_val = qi::_1]
	> *(('*' > factor[qi::_val = phx::bind(&BinOpNode::create, str_astarisc, qi::_val, qi::_1)])
		| ('/' > factor[qi::_val = phx::bind(&BinOpNode::create, str_slash, qi::_val, qi::_1)]));
	factor = literal[qi::_val = qi::_1]
		| vector3d_const[qi::_val = qi::_1]
		| vector6d_const[qi::_val = qi::_1]
		| ('(' > expr > ')')[qi::_val = qi::_1]
		| (variable >> '(' > args > ')')[qi::_val = phx::bind(&FunCallNode::create, qi::_1, qi::_2)]
		| variable[qi::_val = qi::_1];
	args = expr[qi::_val = qi::_1]
	> *(',' > expr[qi::_val = phx::bind(&BinOpNode::create, str_comma, qi::_val, qi::_1)]);
#else
	expr = term[qi::_val = qi::_1]
	> *(('+' > term[qi::_val = phx::bind(&BinOpNode::create, "+", qi::_val, qi::_1)])
		| ('-' > term[qi::_val = phx::bind(&BinOpNode::create, "-", qi::_val, qi::_1)]));
	term = factor[qi::_val = qi::_1]
	> *(('*' > factor[qi::_val = phx::bind(&BinOpNode::create, "*", qi::_val, qi::_1)])
		| ('/' > factor[qi::_val = phx::bind(&BinOpNode::create, "/", qi::_val, qi::_1)]));
	factor = literal[qi::_val = qi::_1]
		| vector3d_const[qi::_val = qi::_1]
		| vector6d_const[qi::_val = qi::_1]
		| ('(' > expr > ')')[qi::_val = qi::_1]
		| (variable >> '(' > args > ')')[qi::_val = phx::bind(&FunCallNode::create, qi::_1, qi::_2)]
		| variable[qi::_val = qi::_1];
	args = expr[qi::_val = qi::_1]
	> *(',' > expr[qi::_val = phx::bind(&BinOpNode::create, ",", qi::_val, qi::_1)]);
#endif
	vector3d_const = ('[' > expr > ',' > expr > ',' > expr > ']')
      [qi::_val = phx::bind(&Vector3dConstNode::create, qi::_1, qi::_2, qi::_3)];
	vector6d_const = ('[' > expr > ',' > expr > ',' > expr > ',' > expr > ',' > expr > ',' > expr > ']')
		[qi::_val = phx::bind(&Vector6dConstNode::create, qi::_1, qi::_2, qi::_3, qi::_4, qi::_5, qi::_6)];
	variable = qi::as_string[qi::lexeme[(qi::alpha | qi::char_('_') | qi::char_('.')) > *(qi::alnum | qi::char_('_') | qi::char_('.'))]]
      [qi::_val = phx::bind(&VariableNode::create, qi::_1)];
    literal = qi::double_[qi::_val = phx::bind(&ValueNode::create, qi::_1)];
    qi::on_error<qi::fail>(
      topexpr,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      expr,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      term,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      factor,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      args,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      vector3d_const,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
		qi::on_error<qi::fail>(
			vector6d_const,
			std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
			<< phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
			<< phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
			<< std::endl
			);
		qi::on_error<qi::fail>(
      variable,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail>(
      literal,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
  }
};

// evaluator
class evaluator : public boost::static_visitor<std::string> {
public:
  std::string operator()(ValueNodeSp const& node) const;
  std::string operator()(Vector3dConstNodeSp const& node) const;
	std::string operator()(Vector6dConstNodeSp const& node) const;
	std::string operator()(BinOpNodeSp const& node) const;
  std::string operator()(VariableNodeSp const& node) const;
  std::string operator()(FunCallNodeSp const& node) const;

private:
  Node node_;

};
/////
class MemberParam {
public:
  MemberParam(NodeType type, std::string source, TaskModelParamPtr targetModel);
  ~MemberParam() {};

  inline CalcMode getCalcMode() const { return this->calcMode_; }
  inline ValueMode getValMode() const { return this->valMode_; }

  inline double getValueScalar() const { return this->valueScalar_; }
  inline Vector3d getValueVector3d() const { return this->valueVector3d_; }
  inline VectorXd getValueVector6d() const { return this->valueVector6d_; }
  inline Matrix3d getValueMatrix() const { return this->valueMatrix_; }

private:
  NodeType nodeType_;
  std::string source_;
  //
  std::string arg01_;
  std::string arg02_;
  std::string arg03_;
	std::string arg04_;
	std::string arg05_;
	std::string arg06_;
	int idxArg01_;
  int idxArg02_;
  int idxArg03_;
	int idxArg04_;
	int idxArg05_;
	int idxArg06_;
	/////
  ValueMode valMode_;
  double valueScalar_;
  Vector3d valueVector3d_;
  VectorXd valueVector6d_;
  Matrix3d valueMatrix_;
	TaskModelParamPtr targetModel_;

  CalcMode calcMode_;

  bool parseScalar();
  bool calcBinOpe(MemberParam* lhs, MemberParam* rhs);
	bool parseVector3d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03);
	bool parseVector6d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03, MemberParam* elem04, MemberParam* elem05, MemberParam* elem06);
  bool parseVariable(bool isSub);
  bool calcFunc(MemberParam* args);
  bool calcTranslation(VectorXd& arg);
  bool calcRotation(VectorXd& arg);
  bool calcRpy2mat(const Vector3d& rot);
  bool calcMat2rpy(const Matrix3d& matrix);

  friend class Calculator;
};

class Calculator : public ArgumentEstimator {
public:
  Calculator() : resultScalar_(0.0), resultVector3d_(Vector3d::Zero()), resultVector6d_(6), valMode_(VAL_SCALAR) {};
  ~Calculator();

  void initialize(TaskModelParamPtr targetParam = NULL);
  void finalize();

  bool buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList);
  bool checkSyntax(TaskModelParamPtr taskParam, QString script, string& errStr);
  bool checkCondition(bool cmdRet, string script);

private:
  ValueMode valMode_;
  double resultScalar_;
  Vector3d resultVector3d_;
	VectorXd resultVector6d_;
	Matrix3d resultMatrix_;

	TaskModelParamPtr targetModel_;
  std::vector<MemberParam*> memberList_;

  inline double getResultScalar() const { return this->resultScalar_; }
  inline Vector3d getResultVector3d() const { return this->resultVector3d_; }
	inline VectorXd getResultVector6d() const { return this->resultVector6d_; }
	inline Matrix3d getResultMatrix() const { return this->resultMatrix_; }

  inline ValueMode getValMode() const { return this->valMode_; }

  bool calculate(QString source, TaskModelParamPtr targetModel, bool isSub = false);

  int extractNodeInfo(const Node& source);

	friend class MemberParam;
};

}
#endif
