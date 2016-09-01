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

namespace teaching {

using Eigen::Vector3d;
using Eigen::Matrix3d;
using Eigen::VectorXd;

enum NodeType {
  TYPE_VALUE = 0,
  TYPE_VECTOR = 1,
  TYPE_BIN_OPE = 2,
  TYPE_VARIABLE = 3,
  TYPE_FUNC = 4
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
  VAL_VECTOR = 1,
  VAL_VECTOR_6d = 2,
  VAL_MATRIX = 3
};
/////
class ValueNode;
class VectorConstNode;
class BinOpNode;
class VariableNode;
class FunCallNode;
typedef boost::shared_ptr<ValueNode> ValueNodeSp;
typedef boost::shared_ptr<VectorConstNode> VectorConstNodeSp;
typedef boost::shared_ptr<BinOpNode> BinOpNodeSp;
typedef boost::shared_ptr<VariableNode> VariableNodeSp;
typedef boost::shared_ptr<FunCallNode> FunCallNodeSp;
typedef boost::variant<ValueNodeSp, VectorConstNodeSp, BinOpNodeSp, VariableNodeSp, FunCallNodeSp> Node;

class ValueNode {
public:
  static Node create(double v) { return ValueNodeSp(new ValueNode(v)); }
private:
  ValueNode(double v):v_(v) {}
  double v_;
  friend class evaluator;
  friend class Calculator;
};

class VectorConstNode {
public:
  static Node create(Node const& x, Node const& y, Node const& z) { return VectorConstNodeSp(new VectorConstNode(x, y, z)); }
private:
  VectorConstNode(Node const&x, Node const& y, Node const& z):x_(x), y_(y), z_(z) {}
  Node x_;
  Node y_;
  Node z_;
  friend class evaluator;
  friend class Calculator;
};

class BinOpNode {
public:
  static Node create(std::string const& op, Node const& lhs, Node const& rhs) { return BinOpNodeSp(new BinOpNode(op, lhs, rhs)); }
private:
  BinOpNode(std::string const& op, Node const& lhs, Node const& rhs):op_(op), lhs_(lhs), rhs_(rhs) {}
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
  VariableNode(std::string const& nm):nm_(nm) {}
  std::string nm_;
  friend class evaluator;
  friend class Calculator;
};

class FunCallNode {
public:
  static Node create(Node const& fun, Node const& args) { return FunCallNodeSp(new FunCallNode(fun, args)); }
private:
  FunCallNode(Node const& fun, Node const& args):fun_(fun), args_(args) {}
  Node fun_;
  Node args_;
  friend class evaluator;
  friend class Calculator;
};

// parser
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

template<typename Iterator>
struct makeTree : qi::grammar<Iterator, Node(), qi::space_type>
{
  qi::rule<Iterator, Node(), qi::space_type> topexpr, expr, term, factor, variable, literal, args, vector_const;
makeTree() : makeTree::base_type(topexpr) {
    topexpr = expr > qi::eoi;
    expr = term [qi::_val = qi::_1] 
      > *(('+' > term [qi::_val = phx::bind(&BinOpNode::create, "+", qi::_val, qi::_1)])
          | ('-' > term [qi::_val = phx::bind(&BinOpNode::create, "-", qi::_val, qi::_1)]));
    term = factor [qi::_val = qi::_1] 
      > *(('*' > factor [qi::_val = phx::bind(&BinOpNode::create, "*", qi::_val, qi::_1)])
          |('/' > factor [qi::_val = phx::bind(&BinOpNode::create, "/", qi::_val, qi::_1)]));
    factor = literal [qi::_val = qi::_1]
      | vector_const [qi::_val = qi::_1]
      | ('(' > expr > ')') [qi::_val = qi::_1]
      | (variable >> '(' >  args > ')') [qi::_val = phx::bind(&FunCallNode::create, qi::_1, qi::_2)]
      | variable [qi::_val = qi::_1];
    args = expr [qi::_val = qi::_1]
      > *(',' > expr [qi::_val = phx::bind(&BinOpNode::create, ",", qi::_val, qi::_1)]);
    vector_const = ('[' > expr > ',' > expr > ',' > expr > ']')
      [qi::_val = phx::bind(&VectorConstNode::create, qi::_1, qi::_2, qi::_3)];
    //[qi::_val = phx::bind(&ValueNode::create, qi::_1)];
    variable = qi::as_string[qi::lexeme[(qi::alpha|qi::char_('_')) > *(qi::alnum|qi::char_('_'))]]
      [qi::_val = phx::bind(&VariableNode::create, qi::_1)];
    literal = qi::double_ [qi::_val = phx::bind(&ValueNode::create, qi::_1)];
    qi::on_error<qi::fail> (
      topexpr,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      expr,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      term,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      factor,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      args,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      vector_const,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
      variable,
      std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
      << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"") 
      << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
      << std::endl
      );
    qi::on_error<qi::fail> (
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
  std::string operator()(VectorConstNodeSp const& node) const;
  std::string operator()(BinOpNodeSp const& node) const;
  std::string operator()(VariableNodeSp const& node) const;
  std::string operator()(FunCallNodeSp const& node) const;

private:
  Node node_;

};
/////
class MemberParam {
public:
  MemberParam(NodeType type, std::string source, TaskModelParam* targetModel);
  ~MemberParam() {};

  inline CalcMode getCalcMode() const { return this->calcMode_; }
  inline ValueMode getValMode() const { return this->valMode_; }

  inline double getValueScalar() const { return this->valueScalar_; }
  inline Vector3d getValueVector() const { return this->valueVector_; }
  inline VectorXd getValueVector6d() const { return this->valueVector6d_; }
  inline Matrix3d getValueMatrix() const { return this->valueMatrix_; }

  bool doCalculate();

private:
  NodeType nodeType_;
  std::string source_;
  //
  std::string arg01_;
  std::string arg02_;
  std::string arg03_;
  int idxArg01_;
  int idxArg02_;
  int idxArg03_;
  /////
  ValueMode valMode_;
  double valueScalar_;
  Vector3d valueVector_;
  VectorXd valueVector6d_;
  Matrix3d valueMatrix_;
  TaskModelParam* targetModel_;

  CalcMode calcMode_;

  bool parseScalar();
  bool calcBinOpe(MemberParam* lhs, MemberParam* rhs);
  bool parseVector(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03);
  bool parseVariable();
  bool calcFunc(MemberParam* args);
  bool calcTranslation(VectorXd& arg);
  bool calcRotation(VectorXd& arg);
  bool calcRpy2mat(const Vector3d& rot);
  bool calcMat2rpy(const Matrix3d& matrix);

  friend class Calculator;
};

class Calculator {
public:
  Calculator() : resultScalar_(0.0), resultVector_(Vector3d::Zero()), valMode_(VAL_SCALAR) {};
  ~Calculator();

  inline double getResultScalar() const { return this->resultScalar_; }
  inline Vector3d getResultVector() const { return this->resultVector_; }
  inline Matrix3d getResultMatrix() const { return this->resultMatrix_; }

  inline ValueMode getValMode() const { return this->valMode_; }

  bool calculate(QString source, TaskModelParam* targetModel);

private:
  ValueMode valMode_;
  double resultScalar_;
  Vector3d resultVector_;
  Matrix3d resultMatrix_;

  TaskModelParam* targetModel_;
  std::vector<MemberParam*> memberList_;

  int extractNodeInfo(const Node& source);
};

struct ParameterParamComparatorByRName {
  QString rname_;
  ParameterParamComparatorByRName(QString value) {
    rname_ = value;
  }
  bool operator()(const ParameterParam* elem) const {
    return elem->getRName()==rname_;
  }
};

}
#endif
