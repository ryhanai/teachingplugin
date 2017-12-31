#pragma once

#include "Export.hpp"
#include "ActivityEditorBase.hpp"

namespace teaching {
	class FlowViewImpl;
}

namespace QtNodes {

class FlowScene;

class NODE_EDITOR_PUBLIC FlowEditor : public ActivityEditorBase {
public:

	FlowEditor(FlowScene *scene, FlowViewImpl* flowView);

	FlowEditor(const FlowEditor&) = delete;
	FlowEditor operator=(const FlowEditor&) = delete;

	void createStateMachine(std::vector<ElementStmParamPtr>& elemList, std::vector<ConnectionStmParamPtr>& connList);
	void updateTargetParam();

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;

	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent* event);

	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	FlowViewImpl* flowView_;
};
}
