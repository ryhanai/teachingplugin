#ifndef TEACHING_PYTHON_WAPPER_H_INCLUDED
#define TEACHING_PYTHON_WAPPER_H_INCLUDED

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif
#include <string>
#include <vector>
#include <sstream>

#include "ArgumentEstimator.h"

using namespace std;

namespace teaching {

class PythonWrapper : public ArgumentEstimator {
public:
  PythonWrapper() : funcNum_(1), usePython_(false) {};
  ~PythonWrapper() {};

  void initialize(TaskModelParam* targetParam = NULL);
  void finalize();

  bool buildArguments(TaskModelParam* taskParam, ElementStmParam* targetParam, std::vector<CompositeParamType>& parameterList);
  bool checkSyntax(TaskModelParam* taskParam, QString script, string& errStr);
  bool checkCondition(bool cmdRet, string script);

private:
  bool usePython_;
  PyThreadState *mainState_;
  PyThreadState *subState_;

  PyObject *global_;
  PyObject *ans_;
  int funcNum_;

  int setImport(string target);
  int setGlobalParam(string paramName, vector<double> value);
  int getGlobalParam(string paramName, vector<double>& result);

  int execFunction(string script, vector<double>& result);
  int execFunction(string script, vector<int>& result);
  int execFunction(string script, vector<string>& result);

  string checkError();
  int doFunction(string script);
  int getGlobalParamScalar(string paramName, vector<double>& result);
  int getGlobalParamList(string paramName, vector<double>& result);
};

}
#endif
