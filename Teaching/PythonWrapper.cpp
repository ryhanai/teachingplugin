#include "PythonWrapper.h"

#include <cnoid/PluginManager>
#include <pybind11/stl.h>

#include "TeachingUtil.h"
#include "LoggerUtil.h"

using namespace cnoid;

namespace teaching {

bool PythonWrapper::setOutArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam) {
  DDEBUG("PythonWrapper::setOutArguments");
  for (int idxArg = 0; idxArg < targetParam->getArgList().size(); idxArg++) {
    ArgumentDefParam* argDef = targetParam->getCommadDefParam()->getArgList()[idxArg];
    if (argDef->getDirection() == 0) continue;

		ArgumentParamPtr arg = targetParam->getArgList()[idxArg];
    QString valueDesc = arg->getValueDesc();
    DDEBUG_V("targetStr : %s", valueDesc.toStdString().c_str());
    ParameterParamPtr targetParam = NULL;
    for( ParameterParamPtr parmParm : taskParam->getActiveParameterList()) {
      if (parmParm->getRName() == valueDesc) {
        targetParam = parmParm;
        break;
      }
    }
    /////
    DDEBUG_V("argType : %s, length : %d", argDef->getType().c_str(), argDef->getLength());
    if (argDef->getType() == "double") {
      vector<double> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), argVal[0]);
        targetParam->setOutValues(0, QString::number(argVal[0]));
      } else {
        if (this->execFunctionArray(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, [%f, %f, %f] %d", arg->getName().toStdString().c_str(), argVal[0], argVal[1], argVal[2], argVal.size());
        for (unsigned int index = 0; index < argVal.size(); index++) {
          targetParam->setOutValues(index, QString::number(argVal[index]));
        }
      }

    } else if (argDef->getType() == "int") {
      vector<int> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %d", arg->getName().toStdString().c_str(), argVal[0]);
        targetParam->setOutValues(0, QString::number(argVal[0]));
      }

    } else if (argDef->getType() == "string") {
      vector<string> argVal;
      if (argVal.size() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %s", arg->getName().toStdString().c_str(), argVal[0].c_str());
        targetParam->setOutValues(0, QString::fromStdString(argVal[0]));
      }

    } else if (argDef->getType() == "Frame") {
      vector<double> argVal;
      if (this->execFunctionFrame(valueDesc.toStdString(), argVal) == false) return false;
      DDEBUG_V("name : %s, [%f, %f, %f, %f, %f, %f] %d", 
                  arg->getName().toStdString().c_str(),
                  argVal[0], argVal[1], argVal[2], argVal[3], argVal[4], argVal[5], 
                  argVal.size());
      for (unsigned int index = 0; index < argVal.size(); index++) {
        targetParam->setOutValues(index, QString::number(argVal[index]));
      }
    }
    targetParam->updateOutValues();
  }
}

bool PythonWrapper::checkSyntax(FlowParamPtr flowParam, TaskModelParamPtr taskParam, ArgumentDefParam* argDef, QString script, string& errStr) {
  if(taskParam) TeachingUtil::setGlobalParam(taskParam);
  if(flowParam) setGlobalParamFlow(flowParam);
  errMsg_ = "";

  if(argDef) {
    if (argDef->getType() == "double") {
      vector<double> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(script.toStdString(), argVal) == false) {
          errStr = errMsg_;
          return false;
        }
      } else {
        if (this->execFunctionArray(script.toStdString(), argVal) == false) {
          errStr = errMsg_;
          return false;
        }
      }

    } else if (argDef->getType() == "int") {
      vector<int> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(script.toStdString(), argVal) == false) {
          errStr = errMsg_;
          return false;
        }
      } else {
        return false;
      }
    } else if (argDef->getType() == "string") {
      vector<string> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(script.toStdString(), argVal) == false) {
          errStr = errMsg_;
          return false;
        }
      } else {
        return false;
      }

    } else if (argDef->getType() == "Frame") {
      vector<double> argVal;
      if (this->execFunctionFrame(script.toStdString(), argVal) == false) {
          errStr = errMsg_;
        return false;
      }
    }
  } else {
      //argDefが設定されていないのは，条件分岐の場合
      vector<int> argVal;
      if (this->execFunction(script.toStdString(), argVal) == false) {
        errStr = errMsg_;
        return false;
      }
  }
  return true;
}

bool PythonWrapper::checkCondition(TaskModelParamPtr targetParam, string script, bool lastRet) {
  //条件判断用のスクリプトが設定されていない場合は，前コマンドの実行結果をそのまま使用
  if (script.length() == 0) {
    return lastRet;
  }
  /////
  TeachingUtil::setGlobalParam(targetParam);
  vector<int> calcResult;
  int ret = execFunction(script, calcResult);
  if (ret == false) return false;

  return calcResult.at(0) == 1 ? true : false;
}

bool PythonWrapper::checkFlowCondition(FlowParamPtr flowParam, string script, bool lastRet) {
  setGlobalParamFlow(flowParam);
  vector<int> calcResult;
  int ret = execFunction(script, calcResult);
  if (ret == false) return false;

  return calcResult.at(0) == 1 ? true : false;
}

bool PythonWrapper::execFunction(string script, vector<double>& result) {
  DDEBUG("PythonWrapper::execFunction double");

  if (executor_.eval(script) == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunction(double) eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    double returnValue = executor_.returnValue().cast<double>();
    result.clear();
    result.push_back(returnValue);
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(double) %s", script.c_str());
    return false;
  }

  return true;
}

bool PythonWrapper::execFunctionArray(string script, vector<double>& result) {
  DDEBUG("PythonWrapper::execFunctionArray double");
  
  if (executor_.eval(script + ".tolist()") == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunctionArray eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    result = executor_.returnValue().cast<vector<double>>();
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(vector<double>) %s", script.c_str());
    return false;
  }

  return true;
}

bool PythonWrapper::execFunctionFrame(string script, vector<double>& result) {
  DDEBUG("PythonWrapper::execFunctionFrame");
  
  if (executor_.eval(script + ".to_tp().tolist()") == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunctionFrame eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    result = executor_.returnValue().cast<vector<double>>();
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(vector<double>) %s", script.c_str());
    return false;
  }

  return true;
}

bool PythonWrapper::execFunction(string script, vector<int>& result) {
  DDEBUG("PythonWrapper::execFunction int");

  if (executor_.eval(script) == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunction(int) eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    int returnValue = executor_.returnValue().cast<int>();
    result.clear();
    result.push_back(returnValue);
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(int) %s", script.c_str());
    return false;
  }

  DDEBUG("PythonWrapper::execFunction int End");
  return true;
}

bool PythonWrapper::execFunctionArray(string script, vector<int>& result) {
  DDEBUG("PythonWrapper::execFunctionArray int");
  
  if (executor_.eval(script) == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunctionArray(int) eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    result = executor_.returnValue().cast<vector<int>>();
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(vector<int>) %s", script.c_str());
    return false;
  }

  return true;
}

bool PythonWrapper::execFunction(string script, vector<string>& result) {
  DDEBUG("PythonWrapper::execFunction string");

  if (executor_.eval(script) == false) {
    errMsg_ = executor_.exceptionText();
    DDEBUG_V("PythonWrapper::execFunction(string) eval Error: %s", errMsg_.c_str());
    return false;
  }
  try {
    string returnValue = executor_.returnValue().cast<string>();
    result.clear();
    result.push_back(returnValue);
  } catch(pybind11::cast_error) {
    DDEBUG_V("cast_error(string) %s", script.c_str());
    return false;
  }

  return true;
}

void PythonWrapper::setGlobalParamFlow(FlowParamPtr targetParam) {
  if (targetParam == NULL) return;
  DDEBUG("PythonWrapper::setGlobalParamFlow");

  std::stringstream script;
  script << "def setGlobalParam():" << std::endl;

  for (FlowParameterParamPtr param : targetParam->getActiveFlowParamList()) {
    QString paramName = param->getName();
    QString strValue = param->getValue();
    QStringList valList = strValue.split(",");
    vector<double> paramList;
    for (QString each : valList) {
      paramList.push_back(each.toDouble());
    }
    if (paramList.size() == 0) continue;

    script << "  global " << paramName.toStdString() << std::endl;
    if (paramList.size() == 1) {
      script << "  " << paramName.toStdString() << " = " << paramList.at(0) << std::endl;
    } else {
      script << "  " << paramName.toStdString() << " = Frame(xyzRPY=[";
      for (unsigned int index = 0; index < paramList.size(); index++) {
        script << paramList.at(index) << ",";
      }
      script << "])" << std::endl;
    }
  }
  script << "setGlobalParam()";

  string strCon = script.str();
  DDEBUG_V("%s", strCon.c_str());
  if (executor_.execCode(strCon)==false) {
    DDEBUG("PythonWrapper::setGlobalParamFlow Error");
    return;
  }
}

}
