#ifndef TEACHING_TASK_EXECUTE_MANAGER_H_INCLUDED
#define TEACHING_TASK_EXECUTE_MANAGER_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ParameterView.h"
#include "MetaDataView.h"
#include "ArgumentEstimator.h"

#include "ControllerBase.h"

namespace teaching {

using namespace cnoid;

class FlowView;
class StateMachineView;
class TaskInstanceView;

class TaskExecuteManager {
public:
  TaskExecuteManager();
  ~TaskExecuteManager();

  inline void setFlowView(FlowView* view) { this->flowView_ = view; }
  inline void setTaskInstanceView(TaskInstanceView* view) { this->taskInstView = view; }
  inline void setStateMachineView(StateMachineView* view) { this->statemachineView_ = view; }
  inline void setParameterView(ParameterView* view) { this->parameterView_ = view; }
  inline void setMetadataView(MetaDataView* view) { this->metadataView = view; }

  inline void setCurrentTask(TaskModelParamPtr param) { this->currentTask_ = param; }
  inline void setCurrentElement(ElementStmParamPtr param) { this->currParam_ = param; }

  inline void setBreak(bool value) { this->isBreak_ = value; }
  inline bool isBreak() const { return this->isBreak_; }

  inline TaskModelParamPtr getCurrentTask() const {
    if (this->currentTask_) return this->currentTask_;
    return this->prevTask_;
  }

  void runSingleTask();
  bool runSingleCommand();
  void runFlow(FlowParamPtr targetFlow);
  bool detachAllModelItem();
  void abortOperation();

  ExecResult doFlowOperation(bool isSingle = false);
  ExecResult doFlowOperationCont();
  ExecResult doTaskOperation(bool updateCurrentTask = true);
  ExecResult doTaskOperationStep();
  ExecResult doFlowSingleOperation();

private:
  bool isBreak_;
  bool isAbort_;
  bool lastResult_;

  FlowParamPtr currentFlow_;
  TaskModelParamPtr currentTask_;
	TaskModelParamPtr prevTask_;
	ElementStmParamPtr currParam_;
	ElementStmParamPtr currFlowParam_;

  ArgumentEstimator* argHandler_;

  TaskInstanceView* taskInstView;
  FlowView* flowView_;
  StateMachineView* statemachineView_;
  ParameterView* parameterView_;
  MetaDataView* metadataView;

  void parseModelInfo();
  bool doModelAction();
  void prepareTask(bool isFlow);
  void setOutArgument(std::vector<CompositeParamType>& parameterList);

  void createArgEstimator(TaskModelParamPtr targetParam = NULL);
  void createArgEstimator(FlowParamPtr targetParam = NULL);
  void deleteArgEstimator();

  TaskModelParamPtr getNextTask(ElementStmParamPtr target);
};

}
#endif
