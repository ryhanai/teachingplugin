#include "Calculator.h"
#include <QStringList>

#include "LoggerUtil.h"

using namespace std;

namespace teaching {

std::string evaluator::operator()(ValueNodeSp const& node) const {
  std::string result = boost::lexical_cast<std::string>(node->v_);
  return result;
}
std::string evaluator::operator()(VectorConstNodeSp const& node) const {
  std::string result = (boost::format("[%s,%s,%s]")
    % boost::apply_visitor(*this, node->x_)
    % boost::apply_visitor(*this, node->y_)
    % boost::apply_visitor(*this, node->z_)).str();
  return result;
}
std::string evaluator::operator()(BinOpNodeSp const& node) const {
  std::string result = (boost::format("(%1%%2%%3%)")
    % boost::apply_visitor(*this, node->lhs_)
    % node->op_
    % boost::apply_visitor(*this, node->rhs_)).str();
  return result;
}
std::string evaluator::operator()(VariableNodeSp const& node) const {
  std::string result = boost::lexical_cast<std::string>(node->nm_);
  return result;
}
std::string evaluator::operator()(FunCallNodeSp const& node) const {
  std::string result = (boost::format("%s(%s)")
    % boost::apply_visitor(*this, node->fun_)
    % boost::apply_visitor(*this, node->args_)).str();
  return result;
}

/////
MemberParam::MemberParam(NodeType type, std::string source, TaskModelParam* targetModel)
  : nodeType_(type), source_(source), valueVector6d_(6) {
  targetModel_ = targetModel;
}

bool MemberParam::parseScalar() {
  valueScalar_ = QString::fromLatin1(source_.c_str()).toDouble();
  valMode_ = VAL_SCALAR;
  return true;
}

bool MemberParam::calcBinOpe(MemberParam* lhs, MemberParam* rhs) {
  ValueMode leftMode = lhs->valMode_;
  ValueMode rightMode = rhs->valMode_;
  //
  if (arg03_ == "+") {
    if (leftMode == VAL_SCALAR && rightMode == VAL_SCALAR) {
      valueScalar_ = lhs->getValueScalar() + rhs->getValueScalar();
      valMode_ = VAL_SCALAR;
      return true;

    } else if (leftMode == VAL_VECTOR && rightMode == VAL_VECTOR) {
      valueVector_ = lhs->getValueVector() + rhs->getValueVector();
      valMode_ = VAL_VECTOR;
      return true;
    }

  } else if (arg03_ == "-") {
    if (leftMode == VAL_SCALAR && rightMode == VAL_SCALAR) {
      valueScalar_ = lhs->getValueScalar() - rhs->getValueScalar();
      valMode_ = VAL_SCALAR;
      return true;

    } else if (leftMode == VAL_VECTOR && rightMode == VAL_VECTOR) {
      valueVector_ = lhs->getValueVector() - rhs->getValueVector();
      valMode_ = VAL_VECTOR;
      return true;
    }

  } else if (arg03_ == "*") {
    if (leftMode == VAL_SCALAR) {
      if (rightMode == VAL_SCALAR) {
        valueScalar_ = lhs->getValueScalar() * rhs->getValueScalar();
        valMode_ = VAL_SCALAR;
        return true;
      } else if (rightMode == VAL_VECTOR) {
        valueVector_ = lhs->getValueScalar() * rhs->getValueVector();
        valMode_ = VAL_VECTOR;
        return true;
      }
    } else if (leftMode == VAL_VECTOR) {
      if (rightMode == VAL_SCALAR) {
        valueVector_ = lhs->getValueVector() * rhs->getValueScalar();
        valMode_ = VAL_VECTOR;
        return true;
      }

    } else if (leftMode == VAL_MATRIX) {
      if (rightMode == VAL_VECTOR) {
        valueVector_ = lhs->getValueMatrix() * rhs->getValueVector();
        valMode_ = VAL_VECTOR;
        return true;

      } else if (rightMode == VAL_MATRIX) {
        valueMatrix_ = lhs->getValueMatrix() * rhs->getValueMatrix();
        valMode_ = VAL_MATRIX;
        return true;
      }
    }

  } else if (arg03_ == "/") {
    if (leftMode == VAL_SCALAR) {
      if (rightMode == VAL_SCALAR) {
        valueScalar_ = lhs->getValueScalar() / rhs->getValueScalar();
        valMode_ = VAL_SCALAR;
        return true;
      }
    } else if (leftMode == VAL_VECTOR) {
      if (rightMode == VAL_SCALAR) {
        valueVector_ = lhs->getValueVector() / rhs->getValueScalar();
        valMode_ = VAL_VECTOR;
        return true;
      }
    }
  }

  return false;
}

bool MemberParam::parseVector(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03) {
  valueVector_[0] = elem01->getValueScalar();
  valueVector_[1] = elem02->getValueScalar();
  valueVector_[2] = elem03->getValueScalar();
  valMode_ = VAL_VECTOR;

  return true;
}

bool MemberParam::parseVariable() {
  vector<ParameterParam*> paramList = targetModel_->getParameterList();
  vector<ParameterParam*>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(QString::fromLatin1(source_.c_str())));
  if (targetParam == paramList.end()) return false;
  //
  if ((*targetParam)->getElemNum() == 1) {
    valueScalar_ = (*targetParam)->getNumValues(0);
    valMode_ = VAL_SCALAR;

  } else if ((*targetParam)->getElemNum() == 3) {
    valueVector_[0] = (*targetParam)->getNumValues(0);
    valueVector_[1] = (*targetParam)->getNumValues(1);
    valueVector_[2] = (*targetParam)->getNumValues(2);
    valMode_ = VAL_VECTOR;

  } else if ((*targetParam)->getElemNum() == 6) {
    valueVector6d_[0] = (*targetParam)->getNumValues(0);
    valueVector6d_[1] = (*targetParam)->getNumValues(1);
    valueVector6d_[2] = (*targetParam)->getNumValues(2);
    valueVector6d_[3] = (*targetParam)->getNumValues(3);
    valueVector6d_[4] = (*targetParam)->getNumValues(4);
    valueVector6d_[5] = (*targetParam)->getNumValues(5);
    valMode_ = VAL_VECTOR_6d;
  }

  return true;
}

bool MemberParam::calcFunc(MemberParam* args) {
  ValueMode argsMode = args->valMode_;
  if (arg01_ == "xyz") {
    if (argsMode != VAL_VECTOR_6d) return false;
    VectorXd argVec6d = args->getValueVector6d();
    if (calcTranslation(argVec6d) == false) return false;
    valMode_ = VAL_VECTOR;

  } else if (arg01_ == "rpy") {
    if (argsMode != VAL_VECTOR_6d) return false;
    VectorXd argVec6d = args->getValueVector6d();
    if (calcRotation(argVec6d) == false) return false;
    valMode_ = VAL_VECTOR;

  } else if (arg01_ == "rotFromRpy") {
    if (argsMode != VAL_VECTOR) return false;
    Vector3d argVec3d = args->getValueVector();
    if (calcRpy2mat(argVec3d) == false) return false;
    valMode_ = VAL_MATRIX;

  } else if (arg01_ == "rpyFromRot") {
    if (argsMode != VAL_MATRIX) return false;
    Matrix3d argMtx3d = args->getValueMatrix();
    if (calcMat2rpy(argMtx3d) == false) return false;
    valMode_ = VAL_VECTOR;

  } else {
    return false;
  }

  return true;
}

bool MemberParam::calcTranslation(VectorXd& arg) {
  valueVector_[0] = arg[0];
  valueVector_[1] = arg[1];
  valueVector_[2] = arg[2];

  return true;
}

bool MemberParam::calcRotation(VectorXd& arg) {
  valueVector_[0] = arg[3];
  valueVector_[1] = arg[4];
  valueVector_[2] = arg[5];

  return true;
}

bool MemberParam::calcRpy2mat(const Vector3d& rot) {
  double rx = rot.x() * PI / 180.0;
  double ry = rot.y() * PI / 180.0;
  double rz = rot.z() * PI / 180.0;

  double cZ, cY, cX, sZ, sY, sX;
  sZ = sin(rz);
  sY = sin(ry);
  sX = sin(rx);
  cZ = cos(rz);
  cY = cos(ry);
  cX = cos(rx);
  //
  valueMatrix_(0, 0) = (float)(cZ*cY);
  valueMatrix_(0, 1) = (float)(cZ*sY*sX - sZ*cX);
  valueMatrix_(0, 2) = (float)(cZ*sY*cX + sZ*sX);

  valueMatrix_(1, 0) = (float)(sZ*cY);
  valueMatrix_(1, 1) = (float)(sZ*sY*sX + cZ*cX);
  valueMatrix_(1, 2) = (float)(sZ*sY*cX - cZ*sX);

  valueMatrix_(2, 0) = (float)(-sY);
  valueMatrix_(2, 1) = (float)(cY*sX);
  valueMatrix_(2, 2) = (float)(cY*cX);

  return true;
}

bool MemberParam::calcMat2rpy(const Matrix3d& matrix) {
  double tol_15 = 1e-10;
  double cb, sb;
  double Yaw_Roll, Yaw, Roll, Pitch;

  cb = sqrt(matrix(0, 0)*matrix(0, 0) + matrix(1, 0)*matrix(1, 0));
  sb = -matrix(2, 0);
  Pitch = atan2(sb, cb);
  if (abs(Pitch) <= tol_15) {
    Pitch = 0;
  }

  sb = sin(Pitch);
  cb = cos(Pitch);

  double diff1 = fabs(sb - 1);
  double diff2 = fabs(sb + 1);

  if (diff1 <= tol_15) {
    Yaw_Roll = atan2(matrix(0, 1), matrix(1, 1));
    Roll = 0.000;
    Pitch = M_PI / 2.000;
    Yaw = Yaw_Roll - Roll;

  } else if (diff2 <= tol_15) {
    Yaw_Roll = -atan2(matrix(0, 1), matrix(1, 1));
    Roll = 0.000;
    Pitch = -M_PI / 2.000;
    Yaw = Yaw_Roll - Roll;

  } else {
    Roll = atan2(matrix(1, 0), matrix(0, 0));
    Yaw = atan2(matrix(2, 1), matrix(2, 2));
  }

  valueVector_[0] = Yaw * 180.0 / PI;
  valueVector_[1] = Pitch * 180.0 / PI;
  valueVector_[2] = Roll * 180.0 / PI;

  return true;
}
/////
Calculator::~Calculator() {
  std::vector<MemberParam*>::iterator itStart = memberList_.begin();
  while (itStart != memberList_.end()) {
    delete *itStart;
    ++itStart;
  }
  memberList_.clear();
}

void Calculator::initialize(TaskModelParam* targetParam) {
}

void Calculator::finalize() {
}

bool Calculator::buildArguments(TaskModelParam* taskParam, ElementStmParam* targetParam, std::vector<CompositeParamType>& parameterList) {
  parameterList.clear();

  //à¯êîÇÃëgÇ›óßÇƒ
  for (int idxArg = 0; idxArg < targetParam->getArgList().size(); idxArg++) {
    ArgumentParam* arg = targetParam->getArgList()[idxArg];
    QString valueDesc = arg->getValueDesc();
    //
    if (targetParam->getCommadDefParam() == 0) return false;

    ArgumentDefParam* argDef = targetParam->getCommadDefParam()->getArgList()[idxArg];
    if (argDef->getDirection() == 1) {
      if (argDef->getType() == "double") {
        if (argDef->getLength() == 1) {
          double argRet;
          parameterList.push_back(argRet);
        } else {
          VectorXd argRet;
          parameterList.push_back(argRet);
        }
      } else if (argDef->getType() == "int") {
        int argRet;
        parameterList.push_back(argRet);
      } else if (argDef->getType() == "string") {
        string argRet;
        parameterList.push_back(argRet);
      }
      continue;
    }

    if (argDef->getType() == "double") {
      bool ret = this->calculate(valueDesc, taskParam);
      if (this->getValMode() == VAL_SCALAR) {
        DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), this->getResultScalar());
        parameterList.push_back(this->getResultScalar());
      } else {
        DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), this->getResultVector()[0], this->getResultVector()[1], this->getResultVector()[2]);
        VectorXd argVec(3);
        argVec[0] = this->getResultVector()[0];
        argVec[1] = this->getResultVector()[1];
        argVec[2] = this->getResultVector()[2];
        parameterList.push_back(argVec);
      }

    } else {
      vector<ParameterParam*> paramList = taskParam->getParameterList();
      vector<ParameterParam*>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(valueDesc));
      QString strVal;
      if (targetParam != paramList.end()) {
        strVal = QString::fromStdString((*targetParam)->getValues(0));
      } else {
        strVal = valueDesc;
      }
      if (argDef->getType() == "int") {
        parameterList.push_back(strVal.toInt());
      } else {
        parameterList.push_back(strVal.toStdString());
      }
    }
  }
  return true;
}

bool Calculator::checkSyntax(TaskModelParam* taskParam, QString script, string& errStr) {
  return calculate(script, taskParam);
}

bool Calculator::checkCondition(bool cmdRet, string script) {
  return cmdRet;
}

int Calculator::extractNodeInfo(const Node& source) {
  int ret = -1;
  int type = source.which();
  switch (type) {
    case 0:
    {
      ValueNodeSp val = boost::get<ValueNodeSp>(source);
      std::string strVal = boost::lexical_cast<std::string>(val->v_);
      memberList_.push_back(new MemberParam(TYPE_VALUE, strVal, targetModel_));
      ret = memberList_.size() - 1;
      break;
    }

    case 1:
    {
      VectorConstNodeSp val = boost::get<VectorConstNodeSp>(source);
      Node nodeX = val->x_;
      std::string strX = boost::apply_visitor(evaluator(), nodeX);
      Node nodeY = val->y_;
      std::string strY = boost::apply_visitor(evaluator(), nodeY);
      Node nodeZ = val->z_;
      std::string strZ = boost::apply_visitor(evaluator(), nodeZ);
      MemberParam* member = new MemberParam(TYPE_VECTOR, "[" + strX + "," + strY + "," + strZ + "]", targetModel_);
      member->arg01_ = strX;
      member->arg02_ = strY;
      member->arg03_ = strZ;
      memberList_.push_back(member);
      ret = memberList_.size() - 1;
      //
      int idx01 = extractNodeInfo(nodeX);
      if (idx01 < 0) return false;
      member->idxArg01_ = idx01;
      int idx02 = extractNodeInfo(nodeY);
      if (idx02 < 0) return false;
      member->idxArg02_ = idx02;
      int idx03 = extractNodeInfo(nodeZ);
      if (idx03 < 0) return false;
      member->idxArg03_ = idx03;
      break;
    }

    case 2:
    {
      BinOpNodeSp val = boost::get<BinOpNodeSp>(source);
      Node lhs = val->lhs_;
      std::string strLhs = boost::apply_visitor(evaluator(), lhs);
      Node rhs = val->rhs_;
      std::string strRhs = boost::apply_visitor(evaluator(), rhs);
      std::string strOpe = val->op_;
      MemberParam* member = new MemberParam(TYPE_BIN_OPE, strLhs + strOpe + strRhs, targetModel_);
      member->arg01_ = strLhs;
      member->arg02_ = strRhs;
      member->arg03_ = strOpe;
      memberList_.push_back(member);
      ret = memberList_.size() - 1;
      //
      int idx01 = extractNodeInfo(lhs);
      if (idx01 < 0) return false;
      member->idxArg01_ = idx01;
      int idx02 = extractNodeInfo(rhs);
      if (idx02 < 0) return false;
      member->idxArg02_ = idx02;
      break;
    }

    case 3:
    {
      VariableNodeSp val = boost::get<VariableNodeSp>(source);
      std::string strVal = boost::lexical_cast<std::string>(val->nm_);
      memberList_.push_back(new MemberParam(TYPE_VARIABLE, strVal, targetModel_));
      ret = memberList_.size() - 1;
      break;
    }

    case 4:
    {
      FunCallNodeSp val = boost::get<FunCallNodeSp>(source);
      Node func = val->fun_;
      std::string strFunc = boost::apply_visitor(evaluator(), func);
      Node args = val->args_;
      std::string strArgs = boost::apply_visitor(evaluator(), args);
      MemberParam* member = new MemberParam(TYPE_FUNC, strFunc + "(" + strArgs + ")", targetModel_);
      member->arg01_ = strFunc;
      member->arg02_ = strArgs;
      memberList_.push_back(member);
      ret = memberList_.size() - 1;
      //
      int idx01 = extractNodeInfo(args);
      if (idx01 < 0) return false;
      member->idxArg01_ = idx01;
      break;
    }
  }
  return ret;
}

bool Calculator::calculate(QString source, TaskModelParam* targetModel) {
  QString target;
  CalcMode mode = CALC_NOTHING;
  valMode_ = VAL_SCALAR;
  std::vector<MemberParam*>::iterator itStart = memberList_.begin();
  while (itStart != memberList_.end()) {
    delete *itStart;
    ++itStart;
  }
  memberList_.clear();
  targetModel_ = targetModel;

  //std::string str = source.toAscii();
  std::string str = source.toLocal8Bit().constData();
  makeTree<std::string::iterator> mt;
  Node result;
  std::string::iterator it = str.begin();
  std::vector<Node> nodeList;
  if (qi::phrase_parse(it, str.end(), mt, qi::space, result) == false) return false;
  int ret = extractNodeInfo(result);
  if (ret < 0) return false;
  if (memberList_.size() == 0) return false;
  //
  for (int index = memberList_.size() - 1; 0 <= index; index--) {
    MemberParam* member = memberList_[index];
    switch (member->nodeType_) {
      case TYPE_VALUE:
      {
        if (member->parseScalar() == false) {
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_VECTOR:
      {
        if (member->parseVector(memberList_[member->idxArg01_],
          memberList_[member->idxArg02_],
          memberList_[member->idxArg03_]) == false) {
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_BIN_OPE:
      {
        if (member->calcBinOpe(memberList_[member->idxArg01_],
          memberList_[member->idxArg02_]) == false) {
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_VARIABLE:
      {
        if (member->parseVariable() == false) {
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_FUNC:
      {
        if (member->calcFunc(memberList_[member->idxArg01_]) == false) {
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }
    }
  }
  //
  if (valMode_ == VAL_SCALAR) {
    resultScalar_ = memberList_[0]->getValueScalar();
  } else if (valMode_ == VAL_VECTOR) {
    resultVector_ = memberList_[0]->getValueVector();
  } else if (valMode_ == VAL_MATRIX) {
    resultMatrix_ = memberList_[0]->getValueMatrix();
  }
  return true;
}

}
