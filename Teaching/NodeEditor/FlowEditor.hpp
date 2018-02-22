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

  void createStateMachine(FlowParamPtr target);
	bool updateTargetFlowParam();
	void updatingParamInfo(TaskModelParamPtr targetTask, ElementStmParamPtr targetState);

public Q_SLOTS:
  void deleteSelectedNodes();

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent* event);

	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent * event);

private:
	FlowViewImpl* flowView_;
};
}
