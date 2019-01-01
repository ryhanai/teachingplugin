#ifndef TEACHING_CALCULATOR_H_INCLUDED
#define TEACHING_CALCULATOR_H_INCLUDED

#include <boost/config/warning_disable.hpp>

#include <iostream>
#include <string>

#include "Parser.h"

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
  MemberParam(NodeType type, std::string source, TaskModelParamPtr targetModel, FlowParamPtr targetFlow);
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

  TaskModelParamPtr targetTask_;
  FlowParamPtr targetFlow_;

  CalcMode calcMode_;

  bool parseScalar();
  bool calcBinOpe(MemberParam* lhs, MemberParam* rhs);
	bool parseVector3d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03);
	bool parseVector6d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03, MemberParam* elem04, MemberParam* elem05, MemberParam* elem06);
  bool parseVariable(bool isSub, bool lastRet);
  bool calcFunc(MemberParam* args);
  bool calcTranslation(VectorXd& arg);
  bool calcRotation(VectorXd& arg);
  bool calcRpy2mat(const Vector3d& rot);
  bool calcMat2rpy(const Matrix3d& matrix);

  friend class Calculator;
};

class Calculator : public ArgumentEstimator {
public:
  Calculator();
  Calculator(TaskModelParamPtr targetModel);
  ~Calculator();

  void initialize(TaskModelParamPtr targetParam = NULL);
  void finalize();

  inline void setFlowParam(FlowParamPtr targetFlow) {
    this->targetFlow_ = targetFlow;
    this->targetTask_ = 0;
  }
  inline void setTaskModelParam(TaskModelParamPtr targetModel) {
    this->targetFlow_ = 0;
    this->targetTask_ = targetModel;
  }

  bool buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList);
  bool checkSyntax(FlowParamPtr flowParam, TaskModelParamPtr taskParam, QString script, string& errStr);
  bool checkCondition(bool cmdRet, string script);
  bool checkFlowCondition(FlowParamPtr flowParam, string script, bool lastRet);

private:
  ValueMode valMode_;
  double resultScalar_;
  Vector3d resultVector3d_;
	VectorXd resultVector6d_;
	Matrix3d resultMatrix_;
  bool lastRet_;

	TaskModelParamPtr targetTask_;
  FlowParamPtr targetFlow_;
  std::vector<MemberParam*> memberList_;

  inline double getResultScalar() const { return this->resultScalar_; }
  inline Vector3d getResultVector3d() const { return this->resultVector3d_; }
	inline VectorXd getResultVector6d() const { return this->resultVector6d_; }
	inline Matrix3d getResultMatrix() const { return this->resultMatrix_; }

  inline ValueMode getValMode() const { return this->valMode_; }

  bool calculate(QString source, bool isSub = false);

  int extractNodeInfo(const Node& source);

	friend class MemberParam;
};

}
#endif
