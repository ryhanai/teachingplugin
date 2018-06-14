#pragma once

#include "Export.hpp"
#include "ActivityEditorBase.hpp"
#include <cnoid/LazyCaller>

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
	void paramInfoUpdated(TaskModelParamPtr targetTask, ElementStmParamPtr targetState);
  void modelParamUpdated(int flowModelId, ModelMasterParamPtr masterParam);
  void dispSetting();

public Q_SLOTS:
  void deleteSelectedNodes();
  void hideSelectedNodes();

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

  cnoid::LazyCaller updateNodesLater_;
  Node* removingNode_;

  void createFlowTaskNode(ElementStmParamPtr target);
  void createFlowExtNode(int typeId, ElementStmParamPtr target);
  void createFlowParamNode(FlowParameterParamPtr target);
  void createFlowModelNode(FlowModelParamPtr target);
  void removeNode();
};
}
