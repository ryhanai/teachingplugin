#include "PythonWrapper.h"

#include <cnoid/PluginManager>

#include "LoggerUtil.h"

#define	RETURN_OK         0
#define	ERROR_NO_FUNC     1
#define	ERROR_FUNC_CALL   2
#define	ERROR_SYNTAX      3
#define	ERROR_INVALID_ARG 4

using namespace cnoid;

namespace teaching {

vector<string> split(const string &s, char delim) {
  vector<string> elems;
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    if (!item.empty()) {
      elems.push_back(item);
    }
  }
  return elems;
}

string trim(const string& str) {
  const char* trimCharacterList = " \t\v\r\n";
  size_t first = str.find_first_not_of(trimCharacterList);
  if (string::npos == first) {
    return "";
  }
  size_t last = str.find_last_not_of(trimCharacterList);
  return str.substr(first, (last - first + 1));
}

void PythonWrapper::initialize(TaskModelParam* targetParam) {
  Plugin* target = PluginManager::instance()->findPlugin("Python");
  if (target == NULL) {
    usePython_ = false;
  } else {
    usePython_ = true;
  }

  if (Py_IsInitialized() == 0) {
    Py_Initialize();
  }
  if (PyEval_ThreadsInitialized() == 0) {
    PyEval_InitThreads();
  }
  if (usePython_) {
    PyEval_AcquireLock();
  }
  subState_ = Py_NewInterpreter();
  PyThreadState_Swap(subState_);
  //
  global_ = PyImport_AddModule("__main__");
  /////
  if (targetParam != NULL) {
    setImport(targetParam->getExecEnv().toStdString());
    //
    //タスクパラメータの設定
    for (int idxParam = 0; idxParam < targetParam->getParameterList().size(); idxParam++) {
      ParameterParam* param = targetParam->getParameterList()[idxParam];
      QString paramName = param->getRName();
      int paramNum = param->getElemNum();
      DDEBUG_V("name : %s, %d", paramName.toStdString().c_str(), paramNum);
      vector<double> paramList;
      for (int idxElem = 0; idxElem < paramNum; idxElem++) {
        QString each = QString::fromStdString(param->getValues(idxElem));
        paramList.push_back(each.toDouble());
      }
      setGlobalParam(paramName.toStdString(), paramList);
    }
  }
}

void PythonWrapper::finalize() {
  PyThreadState_Swap(NULL);

  PyThreadState_Clear(subState_);
  PyThreadState_Delete(subState_);
  if (usePython_) {
    PyEval_ReleaseLock();
  }
}

bool PythonWrapper::buildArguments(TaskModelParam* taskParam, ElementStmParam* targetParam, std::vector<CompositeParamType>& parameterList) {
  parameterList.clear();

  //引数の組み立て
  for (int idxArg = 0; idxArg < targetParam->getArgList().size(); idxArg++) {
    ArgumentParam* arg = targetParam->getArgList()[idxArg];
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

    if (argDef->getType() == "double") {
      vector<double> argVal;
      int ret = this->execFunction(valueDesc.toStdString(), argVal);
      DDEBUG_V("ret : %d", ret);
      if (ret != 0) return false;

      if (argVal.size() == 1) {
        DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), argVal[0]);
        parameterList.push_back(argVal[0]);
      } else {
        DDEBUG_V("name : %s, [%f, %f, %f] %d", arg->getName().toStdString().c_str(), argVal[0], argVal[1], argVal[2], argVal.size());
        VectorXd argVec(argVal.size());
        for (unsigned int index = 0; index < argVal.size(); index++) {
          argVec[index] = argVal[index];
        }
        parameterList.push_back(argVec);
      }

    } else if (argDef->getType() == "int") {
      vector<int> argVal;
      int ret = this->execFunction(valueDesc.toStdString(), argVal);
      DDEBUG_V("ret : %d", ret);
      if (ret != 0) return false;
      if (argVal.size() == 1) {
        DDEBUG_V("name : %s, %d", arg->getName().toStdString().c_str(), argVal[0]);
        parameterList.push_back(argVal[0]);
      }

    } else if (argDef->getType() == "string") {
      vector<string> argVal;
      int ret = this->execFunction(valueDesc.toStdString(), argVal);
      DDEBUG_V("ret : %d", ret);
      if (ret != 0) return false;
      if (argVal.size() == 1) {
        DDEBUG_V("name : %s, %s", arg->getName().toStdString().c_str(), argVal[0].c_str());
        parameterList.push_back(argVal[0]);
      }
    }
  }
  return true;
}

bool PythonWrapper::checkSyntax(TaskModelParam* taskParam, QString script, string& errStr) {
  std::stringstream strNum;
  strNum << funcNum_;

  std::stringstream contents;
  contents << "def userFunc" << strNum.str() << "():" << std::endl;

  vector<string> eachLine = split(script.toStdString(), '\n');
  for (unsigned int index = 0; index < eachLine.size(); index++) {
    contents << "  " << eachLine.at(index) << std::endl;
  }

  PyObject* pCodeObj = Py_CompileString(contents.str().c_str(), "", Py_file_input);
  if (PyErr_Occurred()) {
    PyObject* excType, *excValue, *excTraceback;
    PyErr_Fetch(&excType, &excValue, &excTraceback);
    PyObject *pystr = PyObject_Str(excValue);
    errStr = PyString_AsString(pystr);
    return false;
  }
  Py_DECREF(pCodeObj);
  return true;
}

bool PythonWrapper::checkCondition(bool cmdRet, string script) {
  //条件判断用のスクリプトが設定されていない場合は，前コマンドの実行結果をそのまま使用
  if (script.length() == 0) {
    return cmdRet;
  }
  /////
  vector<int> calcResult;
  int ret = execFunction(script, calcResult);
  if (ret != RETURN_OK) return false;

  return calcResult.at(0) == 1 ? true : false;
}

int PythonWrapper::execFunction(string script, vector<double>& result) {
  int ret = doFunction(script);
  if (ret != RETURN_OK) return ret;
  /////
  result.clear();
  int retSize = PyList_Size(ans_);
  //
  if (0 <= retSize) {
    for (unsigned int index = 0; index < retSize; index++) {
      PyObject *each = PyList_GetItem(ans_, index);
      result.push_back(PyFloat_AsDouble(each));
      Py_DECREF(each);
    }
  } else {
    PyErr_Clear();
    double val = PyFloat_AsDouble(ans_);
    if (PyErr_Occurred()) {
      Py_DECREF(ans_);
      return ERROR_FUNC_CALL;
    }
    result.push_back(val);
  }
  Py_DECREF(ans_);

  return RETURN_OK;
}

int PythonWrapper::execFunction(string script, vector<int>& result) {
  int ret = doFunction(script);
  if (ret != RETURN_OK) return ret;
  /////
  result.clear();
  int retSize = PyList_Size(ans_);
  //
  if (0 <= retSize) {
    for (unsigned int index = 0; index < retSize; index++) {
      PyObject *each = PyList_GetItem(ans_, index);
      result.push_back(PyLong_AsLong(each));
      Py_DECREF(each);
    }
  } else {
    PyErr_Clear();
    long val = PyLong_AsLong(ans_);
    if (PyErr_Occurred()) {
      Py_DECREF(ans_);
      return ERROR_FUNC_CALL;
    }
    result.push_back(val);
  }
  Py_DECREF(ans_);

  return RETURN_OK;
}

int PythonWrapper::execFunction(string script, vector<string>& result) {
  int ret = doFunction(script);
  if (ret != RETURN_OK) return ret;
  /////
  result.clear();
  int retSize = PyList_Size(ans_);
  //
  if (0 <= retSize) {
    for (unsigned int index = 0; index < retSize; index++) {
      PyObject *each = PyList_GetItem(ans_, index);
      result.push_back(PyString_AsString(each));
      Py_DECREF(each);
    }
  } else {
    PyErr_Clear();
    string val = PyString_AsString(ans_);
    if (PyErr_Occurred()) {
      Py_DECREF(ans_);
      return ERROR_FUNC_CALL;
    }
    result.push_back(val);
  }
  Py_DECREF(ans_);

  return RETURN_OK;
}

int PythonWrapper::setImport(string target) {
  std::stringstream contents;
  vector<string> eachLine = split(target, '\n');
  for (unsigned int index = 0; index < eachLine.size(); index++) {
    contents << eachLine.at(index) << std::endl;
  }
  string strCon = contents.str();
  PyRun_SimpleString(contents.str().c_str());

  return RETURN_OK;
}

int PythonWrapper::setGlobalParam(string paramName, vector<double> value) {
  if (value.size() == 0) return ERROR_INVALID_ARG;

  std::stringstream script;
  script << "def set" << paramName << "():" << std::endl;
  script << "  global " << paramName << std::endl;
  if (value.size() == 1) {
    script << "  " << paramName << " = " << value.at(0);
  } else {
    script << "  " << paramName << " = [";
    for (unsigned int index = 0; index < value.size(); index++) {
      script << value.at(index) << ",";
    }
    script << "]" << std::endl;
  }
  string strCon = script.str();
  PyRun_SimpleString(script.str().c_str());

  string funcName = "set" + paramName;
  PyObject *func = PyObject_GetAttrString(global_, funcName.c_str());
  if (func == NULL) return ERROR_NO_FUNC;

  PyObject *obj = PyObject_CallObject(func, NULL);
  Py_DECREF(obj);
  Py_DECREF(func);

  return RETURN_OK;
}

int PythonWrapper::getGlobalParam(string paramName, vector<double>& result) {
  int ret = getGlobalParamList(paramName, result);
  if (ret != RETURN_OK) {
    ret = getGlobalParamScalar(paramName, result);
  }
  return ret;
}

string PythonWrapper::checkError() {
  string result;
  if (PyErr_Occurred()) {
    PyObject* excType, *excValue, *excTraceback;
    PyErr_Fetch(&excType, &excValue, &excTraceback);
    PyObject *pystr = PyObject_Str(excValue);
    result = PyString_AsString(pystr);
  }
  return result;
}

int PythonWrapper::doFunction(string script) {
  std::stringstream strNum;
  strNum << funcNum_;

  std::stringstream contents;
  contents << "def userFunc" << strNum.str() << "():" << std::endl;
  vector<string> eachLine = split(script, '\n');
  //空白行の除去
  vector<string> contLines;
  for (unsigned int index = 0; index < eachLine.size(); index++) {
    if (0 < trim(eachLine.at(index)).length()) {
      contLines.push_back(eachLine.at(index));
    }
  }
  //関数本体の組み立て
  for (unsigned int index = 0; index < contLines.size(); index++) {
    if (index == contLines.size() - 1) {
      contents << "  return " << contLines.at(index) << std::endl;
    } else {
      contents << "  " << contLines.at(index) << std::endl;
    }
  }
  string strCon = contents.str();
  int ret = PyRun_SimpleString(contents.str().c_str());
  //
  string funcName = "userFunc" + strNum.str();
  PyObject *func = PyObject_GetAttrString(global_, funcName.c_str());
  if (func == NULL) return ERROR_NO_FUNC;
  funcNum_++;

  ans_ = PyObject_CallObject(func, NULL);
  if (ans_ == NULL) {
    Py_DECREF(func);
    return ERROR_FUNC_CALL;
  }
  Py_DECREF(func);

  return RETURN_OK;
}

int PythonWrapper::getGlobalParamScalar(string paramName, vector<double>& result) {
  std::stringstream script;
  script << "def get" << paramName << "Scalar():" << std::endl;
  script << "  global " << paramName << std::endl;
  script << "  return " << paramName << "" << std::endl;
  string strCon = script.str();
  PyRun_SimpleString(script.str().c_str());

  string funcName = "get" + paramName + "Scalar";
  PyObject *func = PyObject_GetAttrString(global_, funcName.c_str());
  if (func == NULL) return ERROR_NO_FUNC;

  PyObject *ans = PyObject_CallObject(func, NULL);
  if (ans == NULL) {
    Py_DECREF(func);
    return ERROR_FUNC_CALL;
  }

  result.clear();
  double val = PyFloat_AsDouble(ans);
  if (PyErr_Occurred()) {
    Py_DECREF(ans);
    return ERROR_FUNC_CALL;
  }
  result.push_back(val);

  Py_DECREF(ans);
  Py_DECREF(func);

  return RETURN_OK;
}

int PythonWrapper::getGlobalParamList(string paramName, vector<double>& result) {
  std::stringstream script;
  script << "def get" << paramName << "List():" << std::endl;
  script << "  global " << paramName << std::endl;
  script << "  return list(" << paramName << ")" << std::endl;
  string strCon = script.str();
  PyRun_SimpleString(script.str().c_str());

  string funcName = "get" + paramName + "List";
  PyObject *func = PyObject_GetAttrString(global_, funcName.c_str());
  if (func == NULL) return ERROR_NO_FUNC;

  PyObject *ans = PyObject_CallObject(func, NULL);
  if (ans == NULL) {
    PyErr_Clear();
    Py_DECREF(func);
    return ERROR_FUNC_CALL;
  }

  result.clear();
  int retSize = PyList_Size(ans);
  //
  if (0 <= retSize) {
    for (unsigned int index = 0; index < retSize; index++) {
      PyObject *each = PyList_GetItem(ans, index);
      result.push_back(PyFloat_AsDouble(each));
      Py_DECREF(each);
    }
  } else {
    PyErr_Clear();
    return ERROR_FUNC_CALL;
  }

  Py_DECREF(ans);
  Py_DECREF(func);

  return RETURN_OK;
}

}
