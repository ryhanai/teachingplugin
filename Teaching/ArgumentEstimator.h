#ifndef TEACHING_TEACHING_ARGUMENT_ESTIMATOR_H_INCLUDED
#define TEACHING_TEACHING_ARGUMENT_ESTIMATOR_H_INCLUDED

#include <string>

#include "TeachingTypes.h"
#include "ControllerBase.h"

using namespace std;

namespace teaching {

class ArgumentEstimator {
public:
	ArgumentEstimator() {};
	virtual ~ArgumentEstimator() {};

	virtual void initialize(TaskModelParam* targetParam = NULL) = 0;
	virtual void finalize() = 0;

	virtual bool buildArguments(TaskModelParam* taskParam, ElementStmParam* targetParam, std::vector<CompositeParamType>& parameterList) = 0;
	virtual bool checkSyntax(TaskModelParam* taskParam, QString script, string& errStr) = 0;
	virtual bool checkCondition(bool cmdRet, string script) = 0;
};

}
#endif
