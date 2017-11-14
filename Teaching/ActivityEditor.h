#ifndef TEACHING_ACTIVITY_EDITOR_H_INCLUDED
#define TEACHING_ACTIVITY_EDITOR_H_INCLUDED

#include "QtUtil.h"
#include "ActivityEditorBase.h"

using namespace std;

namespace teaching {

class StateMachineViewImpl;

class ActivityEditor : public ActivityEditorBase {
public:
  ActivityEditor(StateMachineViewImpl* stateView, QWidget* parent = 0);

//  void createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList);
  void setTaskParam(TaskModelParamPtr param) { this->targetTask_ = param; }

	void setBreakPoint(bool isBreak);

	void removeAll();

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent * event);

private:
  StateMachineViewImpl* stateView_;
	TaskModelParamPtr targetTask_;
};

}
#endif
