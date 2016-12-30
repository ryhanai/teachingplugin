#ifndef TEACHING_ACTIVITY_EDITOR_H_INCLUDED
#define TEACHING_ACTIVITY_EDITOR_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ActivityEditorBase.h"

using namespace std;

namespace teaching {

class StateMachineViewImpl;

class ActivityEditor : public QGraphicsView {
public:
	ActivityEditor(StateMachineViewImpl* stateView, QWidget* parent = 0);

	inline void setCntMode(bool mode) { this->modeCnt_ = mode; }
	inline ConnectionNode* getCurrentConnection() { return targetConnection_; }
	inline ElementNode* getCurrentNode() { return targetNode_; }

	void createStateMachine(TaskModelParam* param);
	void setTaskParam(TaskModelParam* param) { this->targetTask_ = param; }
	void removeAll();
	void deleteCurrent();

	void setBreakPoint(bool isBreak);
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

	QGraphicsSceneWithMenu* scene_;

private:
	StateMachineViewImpl* stateView_;

	TaskModelParam* targetTask_;
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
