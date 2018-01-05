#pragma once

#include "Export.hpp"
#include "ActivityEditorBase.hpp"

namespace teaching {
	class StateMachineViewImpl;
}

namespace QtNodes {

class FlowScene;

class NODE_EDITOR_EXPORT StateMachineEditor : public ActivityEditorBase {
public:

	StateMachineEditor(FlowScene *scene, StateMachineViewImpl* stateView);

	StateMachineEditor(const StateMachineEditor&) = delete;
	StateMachineEditor operator=(const StateMachineEditor&) = delete;

	void createStateMachine(std::vector<ElementStmParamPtr>& elemList, std::vector<ConnectionStmParamPtr>& connList);
	void updateTargetParam();

	void setBreakPoint(bool isBreak);

protected:
  //void contextMenuEvent(QContextMenuEvent *event) override;

	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent* event);

	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent * event);

private:
	StateMachineViewImpl* stateView_;
};

}