#ifndef TEACHING_PYTHON_WAPPER_H_INCLUDED
#define TEACHING_PYTHON_WAPPER_H_INCLUDED

#include <cnoid/PythonExecutor>

#include "ArgumentEstimator.h"

using namespace std;

namespace teaching {

class PythonWrapper : public ArgumentEstimator {
public:
  PythonWrapper() {};
  ~PythonWrapper() {};

  void initialize(TaskModelParamPtr targetParam = NULL);

  bool buildArguments(TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, std::vector<CompositeParamType>& parameterList);
  bool checkSyntax(FlowParamPtr flowParam, TaskModelParamPtr taskParam, ArgumentDefParam* argDef, QString script, string& errStr);
  bool checkCondition(bool cmdRet, string script);
  bool checkFlowCondition(FlowParamPtr flowParam, string script, bool lastRet);

private:
  PythonExecutor executor_;
  string errMsg_;

  void setGlobalParam(TaskModelParamPtr targetParam);

  bool execFunction(string script, vector<double>& result);
  bool execFunctionArray(string script, vector<double>& result);
  bool execFunction(string script, vector<int>& result);
  bool execFunction(string script, vector<string>& result);
};

}
#endif
