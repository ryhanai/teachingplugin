#ifndef TEACHING_TASK_EXECUTE_MANAGER_H_INCLUDED
#define TEACHING_TASK_EXECUTE_MANAGER_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ParameterView.h"
#include "MetaDataView.h"

#include "ControllerBase.h"

namespace teaching {

using namespace cnoid;

class FlowView;
class StateMachineView;
class TaskInstanceView;

class TaskExecuteManager {
public:
	TaskExecuteManager();
	inline void setFlowView(FlowView* view) { this->flowView_ = view; }
	inline void setTaskInstanceView(TaskInstanceView* view) { this->taskInstView = view; }
	inline void setStateMachineView(StateMachineView* view) { this->statemachineView_ = view; }
  inline void setParameterView(ParameterView* view) { this->parameterView_ = view; }
	inline void setMetadataView(MetaDataView* view) { this->metadataView = view; }

	inline void setCurrentTask(TaskModelParam* param) { this->currentTask_ = param; }
	inline void setCurrentElement(ElementStmParam* param) { this->currParam_ = param; }

	inline void setBreak(bool value) { this->isBreak_ = value; }
	inline bool isBreak() const { return this->isBreak_; }

	inline TaskModelParam* getCurrentTask() const {
		if (this->currentTask_) return this->currentTask_;
		return this->prevTask_;
	}

	void runSingleTask();
	void runFlow(FlowParam* targetFlow);
	bool detachAllModelItem();

	ExecResult doFlowOperation();
	ExecResult doFlowOperationCont();
	ExecResult doTaskOperation();
	ExecResult doTaskOperationStep();

private:
	bool isBreak_;

	TaskModelParam* currentTask_;
	TaskModelParam* prevTask_;
	ElementStmParam* currParam_;

	TaskInstanceView* taskInstView;
	FlowView* flowView_;
	StateMachineView* statemachineView_;
	ParameterView* parameterView_;
	MetaDataView* metadataView;

	void parseModelInfo();
	bool buildArguments(std::vector<CompositeParamType>& parameterList);
	bool doModelAction();
	void prepareTask();
};

}
#endif
