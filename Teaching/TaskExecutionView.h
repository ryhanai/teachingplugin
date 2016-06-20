#ifndef TEACHING_TASK_EXECUTION_VIEW_H_INCLUDED
#define TEACHING_TASK_EXECUTION_VIEW_H_INCLUDED

#include <QtGui>
#include "TeachingTypes.h"

#include "StateMachineView.h"
#include "ParameterView.h"

namespace teaching {

using namespace cnoid;

class TaskExecutionView : public QWidget {
public:
  TaskExecutionView(QWidget* parent = 0);
  inline void setStateMachineView(StateMachineView* view) { this->statemachineView_ = view; }
  inline void setParameterView(ParameterView* view) { this->parameterView_ = view; }

  void unloadCurrentModel();

protected:
  bool doTaskOperation(TaskModelParam* targetTask, bool isReal);
  void runSingleTask(bool isReal);

  TaskModelParam* currentTask_;
  ElementStmParam* currParam_;

  StateMachineView* statemachineView_;
  ParameterView* parameterView_;
private:
};

}
#endif
