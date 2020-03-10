#include "PythonWrapper.h"

#include <cnoid/PluginManager>
#include <pybind11/stl.h>

#include "TeachingUtil.h"
#include "LoggerUtil.h"

using namespace cnoid;

namespace teaching {

bool PythonWrapper::buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList) {
  TeachingUtil::setGlobalParam(taskParam);
  DDEBUG("PythonWrapper::buildArguments");
  parameterList.clear();

  //引数の組み立て
  vector<ArgumentParamPtr> argList = targetParam->getArgList();
  for (int idxArg = 0; idxArg < argList.size(); idxArg++) {
    DDEBUG_V("index : %d, %d", idxArg, argList.size());

		ArgumentParamPtr arg = argList[idxArg];
    QString valueDesc = arg->getValueDesc();
    DDEBUG_V("valueDesc : %s", valueDesc.toStdString().c_str());
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

    DDEBUG_V("argDef : %s %d", argDef->getType().c_str(), argDef->getLength());
    if (argDef->getType() == "double") {
      vector<double> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), argVal[0]);
        parameterList.push_back(argVal[0]);
      } else {
        if (this->execFunctionArray(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, [%f, %f, %f] %d", arg->getName().toStdString().c_str(), argVal[0], argVal[1], argVal[2], argVal.size());
        VectorXd argVec(argVal.size());
        for (unsigned int index = 0; index < argVal.size(); index++) {
          argVec[index] = argVal[index];
        }
        parameterList.push_back(argVec);
      }

    } else if (argDef->getType() == "int") {
      vector<int> argVal;
      if (argDef->getLength() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %d", arg->getName().toStdString().c_str(), argVal[0]);
        parameterList.push_back(argVal[0]);
      }

    } else if (argDef->getType() == "string") {
      vector<string> argVal;
      if (argVal.size() <= 1) {
        if (this->execFunction(valueDesc.toStdString(), argVal) == false) return false;
        DDEBUG_V("name : %s, %s", arg->getName().toStdString().c_str(), argVal[0].c_str());
        parameterList.push_back(argVal[0]);
      }
    }
  }
  DDEBUG("PythonWrapper::buildArguments End");
  return true;
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
