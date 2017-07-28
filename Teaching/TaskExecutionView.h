#ifndef TEACHING_TASK_EXECUTION_VIEW_H_INCLUDED
#define TEACHING_TASK_EXECUTION_VIEW_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ParameterView.h"
#include "ControllerBase.h"
#include "TaskExecuteManager.h"

namespace teaching {

using namespace cnoid;

class StateMachineView;

class TaskExecutionView : public QWidget {
public:
  TaskExecutionView(QWidget* parent = 0);
  inline void setStateMachineView(StateMachineView* view) { this->statemachineView_ = view; }
  inline void setParameterView(ParameterView* view) { this->parameterView_ = view; }
  inline void setTaskExecutor(TaskExecuteManager* executor) { this->executor_ = executor; }

  void unloadCurrentModel();

protected:
  void runSingleTask();
  bool runSingleCommand(ElementStmParam* targetStm);
  void runFlow(FlowParam* targetFlow);
  void abortOperation();

  ExecResult doTaskOperation();
  ExecResult doOperationStep();
  ExecResult doOperationCont();

  bool checkPaused();
  bool isSkipCheck_;

  TaskModelParam* currentTask_;
  ElementStmParam* currParam_;

  StateMachineView* statemachineView_;
  ParameterView* parameterView_;
  TaskExecuteManager* executor_;
};

}
#endif
