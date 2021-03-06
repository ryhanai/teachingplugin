#pragma once

#include "Export.hpp"
#include "ActivityEditorBase.hpp"
#include "Node.hpp"
#include <cnoid/LazyCaller>

namespace teaching {
	class FlowViewImpl;
}

namespace QtNodes {

class FlowScene;

struct ConnectionInfo {
  //Node* sourceNode;
  int sourcePortIndex;
  Node* targetNode;
  int targetPortIndex;
};

class NODE_EDITOR_PUBLIC FlowEditor : public ActivityEditorBase {
public:
	FlowEditor(FlowScene *scene, FlowViewImpl* flowView);

	FlowEditor(const FlowEditor&) = delete;
	FlowEditor operator=(const FlowEditor&) = delete;

  void createStateMachine(FlowParamPtr target);
	bool updateTargetFlowParam(QString& errMessage);
	void paramInfoUpdated(ElementStmParamPtr targetState);
  void modelParamUpdated(int flowModelId, ModelMasterParamPtr masterParam);
  void dispSetting();
  void portDispSetting();

  bool checkOutConnection(int nodeId, int portIndex);

  bool renameNode(QString currentName, QString newName);
  Node* getNodeByName(QString name);
  bool connectNodes(QString from, QString fromPort, QString to, QString toPort);
  bool connectModelToTask(Node* fromNode, QString fromPort, Node* toNode, QString toPort);
  bool createFlowNode(QString modelName, QPoint posView);
  void clearFlowScene();
  bool deleteConnection(Node* toNode, QString toPort);

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
  void removeModelNodeLater();
  void createPortInfo(TaskModelParamPtr targetTask, std::vector<PortInfo>& portList);
};
}
