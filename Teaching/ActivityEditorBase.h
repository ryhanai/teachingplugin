#ifndef TEACHING_ACTIVITY_EDITOR_BASE_H_INCLUDED
#define TEACHING_ACTIVITY_EDITOR_BASE_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace std;

namespace teaching {

static const double LINE_WIDTH = 3.0;
static const double ARRAW_WIDTH = 10.0;
static const double ARRAW_HEIGHT = 30.0;
static const double POS_DELTA = 2.0;

enum {
  TYPE_ELEMENT = 1,
  TYPE_CONNECTION
};

enum MenuMode {
  MODE_NONE = 0,
  MODE_LINE,
  MODE_POINT
};

class QGraphicsSceneWithMenu : public QGraphicsScene {
Q_OBJECT
public:
  QMenu ContextMenu_;
  QGraphicsSceneWithMenu(QObject *parent = 0);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);

  inline void setMode(MenuMode mode) { this->mode_ = mode; }
  inline void setConnection(ConnectionNode* conn) { this->targetConnection_ = conn; }
  inline void setElement(ElementStmParam* param) { this->targetElem_ = param; }

private Q_SLOTS:
  void addPoint();
  void removePoint();

private:
  MenuMode mode_;
  ConnectionNode* targetConnection_;
  ElementStmParam* targetElem_;
};

class ItemList : public QListWidget {
public:
  ItemList(QString elemType, QWidget* parent = 0);

  void createInitialNodeTarget();
  void createFinalNodeTarget();
  void createDecisionNodeTarget();
  void createForkNodeTarget();
protected:
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
private:
  QPoint startPos;
  QString elemType_;
};

}
#endif
