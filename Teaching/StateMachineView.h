#ifndef TEACHING_STATEMACHINE_VIEW_H_INCLUDED
#define TEACHING_STATEMACHINE_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "TeachingTypes.h"
#include "CommandDefTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class EditorView;
class StateMachineViewImpl;

static const double LINE_WIDTH = 3.0;
static const double ARRAW_WIDTH = 10.0;
static const double ARRAW_HEIGHT = 30.0;
static const double POS_DELTA = 2.0;

enum {
  TYPE_ELEMENT = 1,
  TYPE_CONNECTION
};

class EditorView : public QGraphicsView {
public:
  EditorView(QWidget* parent = 0);

  inline void setCntMode(bool mode) { this->modeCnt_ = mode; }
  inline ConnectionNode* getCurrentConnection() { return targetConnection_; }
  inline ElementNode* getCurrentNode() { return targetNode_; }

  void createStateMachine(TaskModelParam* param);
  void setTaskParam(TaskModelParam* param) { this->targetTask_ = param; }
  void removeAll();
  void deleteCurrent();

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event );
  void keyPressEvent(QKeyEvent* event);

  QGraphicsScene* scene_;

private:
  TaskModelParam* targetTask_;
  bool modeCnt_;
  int newStateNum;

  ElementNode* targetNode_;
  ConnectionNode* targetConnection_;
  vector<ElementNode*> elementList_;
  vector<ConnectionNode*> connectionList_;

  void deleteConnection(ConnectionNode* target);
  void deleteElement(ElementNode* target);
};

class ItemList : public QListWidget {
public:
  ItemList(QWidget* parent = 0);
protected:
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
private:
  QPoint startPos;
};

class StateMachineViewImpl : public QWidget {
  Q_OBJECT
public:
  StateMachineViewImpl(QWidget* parent = 0);

  void setTaskParam(TaskModelParam* param);
  void clearTaskParam();

private Q_SLOTS:
  void setClicked();
  void modeChanged();
  void deleteClicked();
  void editClicked();
  void runClicked();

private:
  QLabel* lblTarget;
  QPushButton* btnTrans;
  QListWidget* lstItem;

  QFrame* frmGuard;
  QRadioButton* rdTrue;
  QRadioButton* rdFalse;
  QPushButton* btnSet;
  QPushButton* btnDelete;
  QPushButton* btnEdit;
  QPushButton* btnRun;

  EditorView* grhStateMachine;
  TaskModelParam* targetTask_;
  vector<CommandDefParam*> commandList_;

  void createInitialNodeTarget();
  void createFinalNodeTarget();
  void createDecisionNodeTarget();
  void createForkNodeTarget();
  void createCommandNodeTarget(int id, QString name);

};

class StateMachineView : public cnoid::View {
public:
  StateMachineView();
  ~StateMachineView();
  void setTaskParam(TaskModelParam* param) { this->viewImpl->setTaskParam(param); }
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }

private:
  StateMachineViewImpl* viewImpl;
};

}
#endif
