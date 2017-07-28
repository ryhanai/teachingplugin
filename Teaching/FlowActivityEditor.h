#ifndef TEACHING_FLOW_ACTIVITY_EDITOR_H_INCLUDED
#define TEACHING_FLOW_ACTIVITY_EDITOR_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ActivityEditorBase.h"

using namespace std;

namespace teaching {

class FlowViewImpl;

class FlowActivityEditor : public QGraphicsView {
public:
  FlowActivityEditor(FlowViewImpl* flowView, QWidget* parent = 0);

  inline void setCntMode(bool mode) { this->modeCnt_ = mode; }
  inline ConnectionNode* getCurrentConnection() { return targetConnection_; }
  inline ElementNode* getCurrentNode() { return targetNode_; }

  void createStateMachine(FlowParam* param);
  void setFlowParam(FlowParam* param) { this->targetFlow_ = param; }
  void removeAll();
  void deleteCurrent();

  void addChildConnection(ConnectionStmParam* target, ElementNode* sourceChild, ElementNode* targetChild);

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void mouseDoubleClickEvent(QMouseEvent * event);

  QGraphicsSceneWithMenu* scene_;

private:
  FlowViewImpl* flowView_;

  FlowParam* targetFlow_;

  bool modeCnt_;
  int newStateNum;
  bool selectionMode_;
  QPointF selStartPnt_;
  QGraphicsRectItem* selRect_;
  QPointF elemStartPnt_;

  ElementNode* targetNode_;
  ConnectionNode* targetConnection_;
  vector<ElementNode*> selectedNode_;

  void deleteConnection(ConnectionNode* target);
  void deleteElement();
};

}
#endif
