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

  void initialize(TaskModelParamPtr targetParam = NULL);
  void finalize();

  bool buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList);
  bool checkSyntax(FlowParamPtr flowParam, TaskModelParamPtr taskParam, QString script, string& errStr);
  bool checkCondition(bool cmdRet, string script);
  bool checkFlowCondition(FlowParamPtr flowParam, string script, bool lastRet);

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
