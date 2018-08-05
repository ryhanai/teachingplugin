#include "Calculator.h"
#include <QStringList>

#include "TeachingUtil.h"

#include "LoggerUtil.h"

using namespace std;

namespace teaching {

std::string evaluator::operator()(ValueNodeSp const& node) const {
  std::string result = boost::lexical_cast<std::string>(node->v_);
  return result;
}
std::string evaluator::operator()(Vector3dConstNodeSp const& node) const {
  std::string result = (boost::format("[%s,%s,%s]")
    % boost::apply_visitor(*this, node->x_)
    % boost::apply_visitor(*this, node->y_)
    % boost::apply_visitor(*this, node->z_)).str();
  return result;
}
std::string evaluator::operator()(Vector6dConstNodeSp const& node) const {
	std::string result = (boost::format("[%s,%s,%s,%s,%s,%s]")
		% boost::apply_visitor(*this, node->x_)
		% boost::apply_visitor(*this, node->y_)
		% boost::apply_visitor(*this, node->z_)
		% boost::apply_visitor(*this, node->Rx_)
		% boost::apply_visitor(*this, node->Ry_)
		% boost::apply_visitor(*this, node->Rz_) ).str();
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
MemberParam::MemberParam(NodeType type, std::string source, TaskModelParamPtr targetModel, FlowParamPtr targetFlow)
  : nodeType_(type), source_(source), valueVector6d_(6) {
  targetFlow_ = targetFlow;
  targetModel_ = targetModel;
}

bool MemberParam::parseScalar() {
  valueScalar_ = QString::fromLatin1(source_.c_str()).toDouble();
  valMode_ = VAL_SCALAR;
  return true;
}

bool MemberParam::calcBinOpe(MemberParam* lhs, MemberParam* rhs) {
	ValueMode leftNode = lhs->valMode_;
  ValueMode rightNode = rhs->valMode_;
	DDEBUG_V("MemberParam::calcBinOpe %s %d %d", arg03_.c_str(), leftNode, rightNode);
  //
  if (arg03_ == "+") {
    if (leftNode == VAL_SCALAR && rightNode == VAL_SCALAR) {
      valueScalar_ = lhs->getValueScalar() + rhs->getValueScalar();
      valMode_ = VAL_SCALAR;
      return true;

    } else if (leftNode == VAL_VECTOR_3D && rightNode == VAL_VECTOR_3D) {
      valueVector3d_ = lhs->getValueVector3d() + rhs->getValueVector3d();
      valMode_ = VAL_VECTOR_3D;
      return true;

		} else if (leftNode == VAL_VECTOR_6D && rightNode == VAL_VECTOR_6D) {
			valueVector6d_ = lhs->getValueVector6d() + rhs->getValueVector6d();
      DDEBUG_V("MemberParam::calcBinOpe lhs : %f %f %f %f %f %f", lhs->getValueVector6d()[0], lhs->getValueVector6d()[1], lhs->getValueVector6d()[2], lhs->getValueVector6d()[3], lhs->getValueVector6d()[4], lhs->getValueVector6d()[5]);
      DDEBUG_V("MemberParam::calcBinOpe rhs : %f %f %f %f %f %f", rhs->getValueVector6d()[0], rhs->getValueVector6d()[1], rhs->getValueVector6d()[2], rhs->getValueVector6d()[3], rhs->getValueVector6d()[4], rhs->getValueVector6d()[5]);
      DDEBUG_V("MemberParam::calcBinOpe 6d : %f %f %f %f %f %f", valueVector6d_[0], valueVector6d_[1], valueVector6d_[2], valueVector6d_[3], valueVector6d_[4], valueVector6d_[5]);
      valMode_ = VAL_VECTOR_6D;
			return true;
		}

  } else if (arg03_ == "-") {
    if (leftNode == VAL_SCALAR && rightNode == VAL_SCALAR) {
      valueScalar_ = lhs->getValueScalar() - rhs->getValueScalar();
      valMode_ = VAL_SCALAR;
      return true;

    } else if (leftNode == VAL_VECTOR_3D && rightNode == VAL_VECTOR_3D) {
      valueVector3d_ = lhs->getValueVector3d() - rhs->getValueVector3d();
      valMode_ = VAL_VECTOR_3D;
      return true;

		} else if (leftNode == VAL_VECTOR_6D && rightNode == VAL_VECTOR_6D) {
			valueVector6d_ = lhs->getValueVector6d() - rhs->getValueVector6d();
			valMode_ = VAL_VECTOR_6D;
			return true;
		}

  } else if (arg03_ == "*") {
    if (leftNode == VAL_SCALAR) {
      if (rightNode == VAL_SCALAR) {
        valueScalar_ = lhs->getValueScalar() * rhs->getValueScalar();
        valMode_ = VAL_SCALAR;
        return true;
      } else if (rightNode == VAL_VECTOR_3D) {
        valueVector3d_ = lhs->getValueScalar() * rhs->getValueVector3d();
        valMode_ = VAL_VECTOR_3D;
        return true;
			} else if (rightNode == VAL_VECTOR_6D) {
				valueVector6d_ = lhs->getValueScalar() * rhs->getValueVector6d();
				valMode_ = VAL_VECTOR_6D;
				return true;
			}

    } else if (leftNode == VAL_VECTOR_3D) {
      if (rightNode == VAL_SCALAR) {
        valueVector3d_ = lhs->getValueVector3d() * rhs->getValueScalar();
        valMode_ = VAL_VECTOR_3D;
        return true;
      }

		} else if (leftNode == VAL_VECTOR_6D) {
			if (rightNode == VAL_SCALAR) {
				valueVector6d_ = lhs->getValueVector6d() * rhs->getValueScalar();
				valMode_ = VAL_VECTOR_6D;
				return true;
			}

		} else if (leftNode == VAL_MATRIX) {
      if (rightNode == VAL_VECTOR_3D) {
        valueVector3d_ = lhs->getValueMatrix() * rhs->getValueVector3d();
        valMode_ = VAL_VECTOR_3D;
        return true;

      } else if (rightNode == VAL_MATRIX) {
        valueMatrix_ = lhs->getValueMatrix() * rhs->getValueMatrix();
        valMode_ = VAL_MATRIX;
        return true;
      }
    }

  } else if (arg03_ == "/") {
    if (leftNode == VAL_SCALAR) {
      if (rightNode == VAL_SCALAR) {
        valueScalar_ = lhs->getValueScalar() / rhs->getValueScalar();
        valMode_ = VAL_SCALAR;
        return true;
      }
    } else if (leftNode == VAL_VECTOR_3D) {
      if (rightNode == VAL_SCALAR) {
        valueVector3d_ = lhs->getValueVector3d() / rhs->getValueScalar();
        valMode_ = VAL_VECTOR_3D;
        return true;
      }

		} else if (leftNode == VAL_VECTOR_6D) {
			if (rightNode == VAL_SCALAR) {
				valueVector6d_ = lhs->getValueVector6d() / rhs->getValueScalar();
				valMode_ = VAL_VECTOR_6D;
				return true;
			}
		}

  } else if (arg03_ == "||" || arg03_ == "&&" || arg03_ == "==" || arg03_ == "!=" 
          || arg03_ == "<=" || arg03_ == ">=" || arg03_ == "<" || arg03_ == ">") {
    if (leftNode != VAL_SCALAR || leftNode != VAL_SCALAR) return false;
    valMode_ = VAL_SCALAR;
    if (     arg03_ == "||") valueScalar_ = (lhs->getValueScalar() || rhs->getValueScalar());
    else if (arg03_ == "&&") valueScalar_ = (lhs->getValueScalar() && rhs->getValueScalar());
    else if (arg03_ == "==") valueScalar_ = dbl_eq(lhs->getValueScalar(),rhs->getValueScalar());
    else if (arg03_ == "!=") valueScalar_ = !dbl_eq(lhs->getValueScalar(), rhs->getValueScalar());
    else if (arg03_ == "<=") valueScalar_ = (lhs->getValueScalar() <= rhs->getValueScalar());
    else if (arg03_ == ">=") valueScalar_ = (lhs->getValueScalar() >= rhs->getValueScalar());
    else if (arg03_ == "<")  valueScalar_ = (lhs->getValueScalar() <  rhs->getValueScalar());
    else if (arg03_ == ">")  valueScalar_ = (lhs->getValueScalar() >  rhs->getValueScalar());

    DDEBUG_V("left:%f, %s, right:%f, Result:%f", lhs->getValueScalar(), arg03_.c_str(), rhs->getValueScalar(), valueScalar_);
    return true;
  }

  return false;
}

bool MemberParam::parseVector3d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03) {
	DDEBUG("MemberParam::parseVector3d");
	valueVector3d_[0] = elem01->getValueScalar();
  valueVector3d_[1] = elem02->getValueScalar();
  valueVector3d_[2] = elem03->getValueScalar();
	valMode_ = VAL_VECTOR_3D;

  return true;
}

bool MemberParam::parseVector6d(MemberParam* elem01, MemberParam* elem02, MemberParam* elem03, MemberParam* elem04, MemberParam* elem05, MemberParam* elem06) {
	DDEBUG("MemberParam::parseVector6d");
	valueVector6d_[0] = elem01->getValueScalar();
	valueVector6d_[1] = elem02->getValueScalar();
	valueVector6d_[2] = elem03->getValueScalar();
	valueVector6d_[3] = elem04->getValueScalar();
	valueVector6d_[4] = elem05->getValueScalar();
	valueVector6d_[5] = elem06->getValueScalar();
	valMode_ = VAL_VECTOR_6D;

	return true;
}

bool MemberParam::parseVariable(bool isSub) {
	DDEBUG_V("MemberParam::parseVariable source:%s", source_.c_str());
	QString paramName = QString::fromStdString(source_);
  DDEBUG("MemberParam::parseVariable : Param");
  if (this->targetFlow_) {
    vector<FlowParameterParamPtr> paramList = targetFlow_->getFlowParamList();
    vector<FlowParameterParamPtr>::iterator targetParam = find_if(paramList.begin(), paramList.end(), FlowParameterParamByNameComparator(paramName));
    if (targetParam == paramList.end()) return false;
    //
    QString strValue = (*targetParam)->getValue();
    QStringList valList = strValue.split(",");
    if (valList.size() == 1) {
      valueScalar_ = strValue.toDouble();
      valMode_ = VAL_SCALAR;
      return true;
    } else {
      return false;
    }
  }
  //モデルの検索
  QString featureName;
  vector<ModelParamPtr> modelList = targetModel_->getActiveModelList();
  std::vector<ModelParamPtr>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparatorByRName(paramName));
  if (model != modelList.end()) {
    DDEBUG("MemberParam::parseVariable : ModelParam");
    valueVector6d_[0] = (*model)->getPosX();
    valueVector6d_[1] = (*model)->getPosY();
    valueVector6d_[2] = (*model)->getPosZ();
    valueVector6d_[3] = (*model)->getRotRx();
    valueVector6d_[4] = (*model)->getRotRy();
    valueVector6d_[5] = (*model)->getRotRz();
    valMode_ = VAL_VECTOR_6D;
    return true;
  }
  //パラメータの検索
  vector<ParameterParamPtr> paramList = targetModel_->getActiveParameterList();
  vector<ParameterParamPtr>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(paramName));
  if (targetParam == paramList.end()) return false;
  DDEBUG_V("type:%d, model_param_id:%d", (*targetParam)->getType(), (*targetParam)->getModelParamId());
  //
  if ((*targetParam)->getParamType() == PARAM_TYPE_FRAME) {
    if ((*targetParam)->getType()==PARAM_KIND_NORMAL || (*targetParam)->getModelParamId() == NULL_ID) {
      valueVector6d_[0] = (*targetParam)->getNumValuesForCalc(0);
      valueVector6d_[1] = (*targetParam)->getNumValuesForCalc(1);
      valueVector6d_[2] = (*targetParam)->getNumValuesForCalc(2);
      valueVector6d_[3] = (*targetParam)->getNumValuesForCalc(3);
      valueVector6d_[4] = (*targetParam)->getNumValuesForCalc(4);
      valueVector6d_[5] = (*targetParam)->getNumValuesForCalc(5);

    } else {
      int model_id = (*targetParam)->getModelId();
      vector<ModelParamPtr> modelList = targetModel_->getActiveModelList();
      std::vector<ModelParamPtr>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparator(model_id));
      if (model == modelList.end()) return false;

      int feature_id = (*targetParam)->getModelParamId();

      ModelMasterParamPtr master = (*model)->getModelMaster();
      if(!master) return false;
      vector<ModelParameterParamPtr> masterParamList = master->getModelParameterList();
      vector<ModelParameterParamPtr>::iterator masterParamItr = find_if(masterParamList.begin(), masterParamList.end(), ModelMasterParamComparator(feature_id));
      if (masterParamItr == masterParamList.end()) return false;
      QString desc = (*masterParamItr)->getValueDesc();
      DDEBUG_V("MemberParam::parseVariable : Model Param=%s", desc.toStdString().c_str());
      desc = desc.replace("origin", (*model)->getRName());
      DDEBUG_V("MemberParam::parseVariable : Model Param Rep=%s", desc.toStdString().c_str());
      Calculator* calc = new Calculator();
      //再計算しないようにisSubをTrueに設定
      calc->setTaskModelParam(targetModel_);
      if (calc->calculate(desc, true) == false) {
        DDEBUG("MemberParam::parseVariable : Calc Error");
        return false;
      }
      valueVector6d_ = calc->getResultVector6d();
      delete calc;
      DDEBUG_V("MemberParam::parseVariable : Calc End %f, %f, %f, %f, %f, %f", valueVector6d_[0], valueVector6d_[1], valueVector6d_[2], valueVector6d_[3], valueVector6d_[4], valueVector6d_[5]);
    }
    valMode_ = VAL_VECTOR_6D;

  } else {
    valueScalar_ = (*targetParam)->getNumValuesForCalc(0);
    DDEBUG_V("MemberParam::parseVariable : SCALA : %f", valueScalar_);
    valMode_ = VAL_SCALAR;
  }

  return true;
}

bool MemberParam::calcFunc(MemberParam* args) {
	DDEBUG("MemberParam::calcFunc");
	
	ValueMode argsMode = args->valMode_;
  if (arg01_ == "xyz") {
    if (argsMode != VAL_VECTOR_6D) return false;
    VectorXd argVec6d = args->getValueVector6d();
    if (calcTranslation(argVec6d) == false) return false;
    valMode_ = VAL_VECTOR_3D;

  } else if (arg01_ == "rpy") {
    if (argsMode != VAL_VECTOR_6D) return false;
    VectorXd argVec6d = args->getValueVector6d();
    if (calcRotation(argVec6d) == false) return false;
    valMode_ = VAL_VECTOR_3D;

  } else if (arg01_ == "rotFromRpy") {
    if (argsMode != VAL_VECTOR_3D) return false;
    Vector3d argVec3d = args->getValueVector3d();
    if (calcRpy2mat(argVec3d) == false) return false;
    valMode_ = VAL_MATRIX;

  } else if (arg01_ == "rpyFromRot") {
    if (argsMode != VAL_MATRIX) return false;
    Matrix3d argMtx3d = args->getValueMatrix();
    if (calcMat2rpy(argMtx3d) == false) return false;
    valMode_ = VAL_VECTOR_3D;

  } else {
    return false;
  }

  return true;
}

bool MemberParam::calcTranslation(VectorXd& arg) {
	DDEBUG_V("MemberParam::calcTranslation: %f, %f, %f", arg[0], arg[1], arg[2]);
	valueVector3d_[0] = arg[0];
  valueVector3d_[1] = arg[1];
  valueVector3d_[2] = arg[2];

  return true;
}

bool MemberParam::calcRotation(VectorXd& arg) {
  valueVector3d_[0] = arg[3];
  valueVector3d_[1] = arg[4];
  valueVector3d_[2] = arg[5];

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

  valueVector3d_[0] = Yaw * 180.0 / PI;
  valueVector3d_[1] = Pitch * 180.0 / PI;
  valueVector3d_[2] = Roll * 180.0 / PI;

  return true;
}
/////
Calculator::Calculator()
  : resultScalar_(0.0), resultVector3d_(Vector3d::Zero()), resultVector6d_(6),
    valMode_(VAL_SCALAR), targetModel_(0), targetFlow_(0) {
}

Calculator::~Calculator() {
  std::vector<MemberParam*>::iterator itStart = memberList_.begin();
  while (itStart != memberList_.end()) {
    delete *itStart;
    ++itStart;
  }
  memberList_.clear();
}

void Calculator::initialize(TaskModelParamPtr targetParam) {
}

void Calculator::finalize() {
  DDEBUG(" Calculator::finalize");
}

bool Calculator::buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList) {
  DDEBUG(" Calculator::buildArguments");
  parameterList.clear();

  //引数の組み立て
  for (int idxArg = 0; idxArg < targetParam->getArgList().size(); idxArg++) {
		ArgumentParamPtr arg = targetParam->getArgList()[idxArg];
    QString valueDesc = arg->getValueDesc();
    DDEBUG_V("index:%d, desc:%s", idxArg, valueDesc.toStdString().c_str());
    //
    if (targetParam->getCommadDefParam() == 0) return false;

    ArgumentDefParam* argDef = targetParam->getCommadDefParam()->getArgList()[idxArg];
    DDEBUG_V("type:%s", argDef->getType().c_str());
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
      this->setTaskModelParam(taskParam);
      bool ret = this->calculate(valueDesc);
      if (ret == false) return false;
      if (this->getValMode() == VAL_SCALAR) {
        DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), this->getResultScalar());
        parameterList.push_back(this->getResultScalar());
      } else {
        DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), this->getResultVector3d()[0], this->getResultVector3d()[1], this->getResultVector3d()[2]);
        VectorXd argVec(3);
        argVec[0] = this->getResultVector3d()[0];
        argVec[1] = this->getResultVector3d()[1];
        argVec[2] = this->getResultVector3d()[2];
        parameterList.push_back(argVec);
      }

    } else {
      vector<ParameterParamPtr> paramList = taskParam->getActiveParameterList();
      vector<ParameterParamPtr>::iterator param = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(valueDesc));
      QString strVal;
      if (param != paramList.end()) {
        strVal = QString::fromStdString((*param)->getValues(0));
      } else {
        strVal = valueDesc;
      }
      DDEBUG_V("strVal:%s",strVal.toStdString().c_str());
      if (argDef->getType() == "int") {
        parameterList.push_back(strVal.toInt());
      } else {
        parameterList.push_back(strVal.toStdString());
      }
    }
  }
  DDEBUG(" Calculator::buildArguments End");
  return true;
}

bool Calculator::checkSyntax(FlowParamPtr flowParam, TaskModelParamPtr taskParam, QString script, string& errStr) {
  if(flowParam) {
    this->setFlowParam(flowParam);
  } else {
    this->setTaskModelParam(taskParam);
  }
  return calculate(script);
}

bool Calculator::checkCondition(bool cmdRet, string script) {
  return cmdRet;
}

bool Calculator::checkFlowCondition(FlowParamPtr flowParam, string script) {
  DDEBUG_V("Calculator::checkFlowCondition : %s", script.c_str());
  this->setFlowParam(flowParam);
  if (calculate(QString::fromStdString(script)) == false) return false;
  return this->getResultScalar();
}

int Calculator::extractNodeInfo(const Node& source) {
  int ret = -1;
  int type = source.which();
  switch (type) {
    case 0:
    {
      ValueNodeSp val = boost::get<ValueNodeSp>(source);
      std::string strVal = boost::lexical_cast<std::string>(val->v_);
      memberList_.push_back(new MemberParam(TYPE_VALUE, strVal, targetModel_, targetFlow_));
      ret = memberList_.size() - 1;
      break;
    }

    case 1:
    {
      Vector3dConstNodeSp val = boost::get<Vector3dConstNodeSp>(source);
      Node nodeX = val->x_;
      std::string strX = boost::apply_visitor(evaluator(), nodeX);
      Node nodeY = val->y_;
      std::string strY = boost::apply_visitor(evaluator(), nodeY);
      Node nodeZ = val->z_;
      std::string strZ = boost::apply_visitor(evaluator(), nodeZ);
      MemberParam* member = new MemberParam(TYPE_VECTOR, "[" + strX + "," + strY + "," + strZ + "]", targetModel_, targetFlow_);
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
			Vector6dConstNodeSp val = boost::get<Vector6dConstNodeSp>(source);
			Node nodeX = val->x_;
			std::string strX = boost::apply_visitor(evaluator(), nodeX);
			Node nodeY = val->y_;
			std::string strY = boost::apply_visitor(evaluator(), nodeY);
			Node nodeZ = val->z_;
			std::string strZ = boost::apply_visitor(evaluator(), nodeZ);
			Node nodeRX = val->Rx_;
			std::string strRX = boost::apply_visitor(evaluator(), nodeRX);
			Node nodeRY = val->Ry_;
			std::string strRY = boost::apply_visitor(evaluator(), nodeRY);
			Node nodeRZ = val->Rz_;
			std::string strRZ = boost::apply_visitor(evaluator(), nodeRZ);

			MemberParam* member = new MemberParam(TYPE_VECTOR6, "[" + strX + "," + strY + "," + strZ + "," + strRX + "," + strRY + "," + strRZ + "]", targetModel_, targetFlow_);
			member->arg01_ = strX;
			member->arg02_ = strY;
			member->arg03_ = strZ;
			member->arg04_ = strRX;
			member->arg05_ = strRY;
			member->arg06_ = strRZ;
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

			int idx04 = extractNodeInfo(nodeRX);
			if (idx04 < 0) return false;
			member->idxArg04_ = idx04;

			int idx05 = extractNodeInfo(nodeRY);
			if (idx05 < 0) return false;
			member->idxArg05_ = idx05;

			int idx06 = extractNodeInfo(nodeRZ);
			if (idx06 < 0) return false;
			member->idxArg06_ = idx06;
			break;
		}

		case 3:
    {
      BinOpNodeSp val = boost::get<BinOpNodeSp>(source);
      Node lhs = val->lhs_;
      std::string strLhs = boost::apply_visitor(evaluator(), lhs);
      Node rhs = val->rhs_;
      std::string strRhs = boost::apply_visitor(evaluator(), rhs);
      std::string strOpe = val->op_;
      MemberParam* member = new MemberParam(TYPE_BIN_OPE, strLhs + strOpe + strRhs, targetModel_, targetFlow_);
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

    case 4:
    {
      VariableNodeSp val = boost::get<VariableNodeSp>(source);
      std::string strVal = boost::lexical_cast<std::string>(val->nm_);
      memberList_.push_back(new MemberParam(TYPE_VARIABLE, strVal, targetModel_, targetFlow_));
      ret = memberList_.size() - 1;
      break;
    }

    case 5:
    {
      FunCallNodeSp val = boost::get<FunCallNodeSp>(source);
      Node func = val->fun_;
      std::string strFunc = boost::apply_visitor(evaluator(), func);
      Node args = val->args_;
      std::string strArgs = boost::apply_visitor(evaluator(), args);
      MemberParam* member = new MemberParam(TYPE_FUNC, strFunc + "(" + strArgs + ")", targetModel_, targetFlow_);
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

bool Calculator::calculate(QString source, bool isSub) {
	DDEBUG_V("Calculator::calculate: %s", source.toStdString().c_str());
  CalcMode mode = CALC_NOTHING;
  valMode_ = VAL_SCALAR;
  std::vector<MemberParam*>::iterator itStart = memberList_.begin();
  while (itStart != memberList_.end()) {
    delete *itStart;
    ++itStart;
  }
  memberList_.clear();

  std::string str = source.toStdString();
  makeTree<std::string::iterator> mt;
  Node result;
  std::string::iterator it = str.begin();
	//std::vector<Node> nodeList;
  DDEBUG("qi::phrase_parse");
  if (qi::phrase_parse(it, str.end(), mt, qi::space, result) == false) {
    DDEBUG_V("Error:%", mt.getErrorMessage().c_str());
    mt.clearErrorMessage();
    return false;
  }
  DDEBUG("qi::phrase_parse OK");
  int ret = extractNodeInfo(result);
	if (ret < 0) return false;
  if (memberList_.size() == 0) return false;
  //
  for (int index = memberList_.size() - 1; 0 <= index; index--) {
    MemberParam* member = memberList_[index];
		DDEBUG_V("member %d, %s", member->nodeType_, member->source_.c_str());
    switch (member->nodeType_) {
      case TYPE_VALUE:
      {
        if (member->parseScalar() == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_VALUE");
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_VECTOR:
      {
				if (member->parseVector3d(
								memberList_[member->idxArg01_],
								memberList_[member->idxArg02_],
								memberList_[member->idxArg03_]) == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_VECTOR");
          return false;
				}
				valMode_ = member->valMode_;
				break;
      }

			case TYPE_VECTOR6:
			{
				if (member->parseVector6d(
					memberList_[member->idxArg01_],
					memberList_[member->idxArg02_],
					memberList_[member->idxArg03_],
					memberList_[member->idxArg04_],
					memberList_[member->idxArg05_],
					memberList_[member->idxArg06_]) == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_VECTOR6");
          return false;
				}
				DDEBUG("Calculator::calculate Vector6");
				valMode_ = member->valMode_;
				break;
			}

			case TYPE_BIN_OPE:
      {
        if (member->calcBinOpe(memberList_[member->idxArg01_],
          memberList_[member->idxArg02_]) == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_BIN_OPE");
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_VARIABLE:
      {
        if (member->parseVariable(isSub) == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_VARIABLE");
          return false;
        }
        valMode_ = member->valMode_;
        break;
      }

      case TYPE_FUNC:
      {
        if (member->calcFunc(memberList_[member->idxArg01_]) == false) {
          DDEBUG("Calculator::calculate ERROR TYPE_FUNC");
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
  } else if (valMode_ == VAL_VECTOR_3D) {
    resultVector3d_ = memberList_[0]->getValueVector3d();
	} else if (valMode_ == VAL_VECTOR_6D) {
		resultVector6d_ = memberList_[0]->getValueVector6d();
	} else if (valMode_ == VAL_MATRIX) {
    resultMatrix_ = memberList_[0]->getValueMatrix();
  }
  return true;
}

}
