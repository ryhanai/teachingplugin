#ifndef TEACHING_ACTIVITY_EDITOR_BASE_H_INCLUDED
#define TEACHING_ACTIVITY_EDITOR_BASE_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include <cnoid/MenuManager>

using namespace std;
using namespace cnoid;

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

class ActivityEditorBase : public QGraphicsView {
public:
	ActivityEditorBase(QWidget* parent = 0);

	inline void setCntMode(bool mode) { this->modeCnt_ = mode; }

	inline ElementNode* getCurrentNode() { return targetNode_; }
	inline ConnectionNode* getCurrentConnection() { return targetConnection_; }

	void deleteCurrent();
	void addChildConnection(ConnectionStmParamPtr target, ElementNode* sourceChild, ElementNode* targetChild);

	void addPoint();
	void removePoint();

	void createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList);
	virtual void removeAll() {};

protected:
	bool modeCnt_;
	int newStateNum;
	bool selectionMode_;
	QPointF selStartPnt_;
	QGraphicsRectItem* selRect_;
	QPointF elemStartPnt_;

	ElementNode* targetNode_;
	ConnectionNode* targetConnection_;
	vector<ElementNode*> selectedNode_;

	QGraphicsScene* scene_;
	MenuManager menuManager;

	void mouseMoveEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void wheelEvent(QWheelEvent* event);

	void deleteConnection(ConnectionNode* target);
	void deleteElement();
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
