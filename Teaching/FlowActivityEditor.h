#ifndef TEACHING_FLOW_ACTIVITY_EDITOR_H_INCLUDED
#define TEACHING_FLOW_ACTIVITY_EDITOR_H_INCLUDED

#include "QtUtil.h"
#include "ActivityEditorBase.h"

using namespace std;

namespace teaching {

class FlowViewImpl;

class FlowActivityEditor : public ActivityEditorBase {
public:
  FlowActivityEditor(FlowViewImpl* flowView, QWidget* parent = 0);

//  void createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList);
  void setFlowParam(FlowParamPtr param) { this->targetFlow_ = param; }

  void removeAll();

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent * event);

private:
  FlowViewImpl* flowView_;
  FlowParamPtr targetFlow_;
};

}
#endif
