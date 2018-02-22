#include "FlowEditor.hpp"

#include "StyleCollection.hpp"

#include "../TeachingEventHandler.h"
#include "../TeachingDataHolder.h"
#include "../FlowView.h"
#include "models.hpp"

#include "../LoggerUtil.h"

using QtNodes::FlowEditor;
using QtNodes::FlowScene;
using namespace teaching;


FlowEditor::FlowEditor(FlowScene *scene, FlowViewImpl* flowView)
  : ActivityEditorBase(scene), flowView_(flowView) {
  setDragMode(QGraphicsView::ScrollHandDrag);
  setRenderHint(QPainter::Antialiasing);

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  setBackgroundBrush(flowViewStyle.BackgroundColor);

  //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  //setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  setCacheMode(QGraphicsView::CacheBackground);

  //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

  // setup actions
  _clearSelectionAction = new QAction(QStringLiteral("Clear Selection"), this);
  _clearSelectionAction->setShortcut(Qt::Key_Escape);
  connect(_clearSelectionAction, &QAction::triggered, _scene, &QGraphicsScene::clearSelection);
  addAction(_clearSelectionAction);

  _deleteSelectionAction = new QAction(QStringLiteral("Delete Selection"), this);
  _deleteSelectionAction->setShortcut(Qt::Key_Delete);
  connect(_deleteSelectionAction, &QAction::triggered, this, &FlowEditor::deleteSelectedNodes);
  addAction(_deleteSelectionAction);
}

void FlowEditor::contextMenuEvent(QContextMenuEvent *event) {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  if (itemAt(event->pos())) {
    QGraphicsView::contextMenuEvent(event);
    return;
  }

  QMenu modelMenu;
  auto skipText = QStringLiteral("skip me");

  //Add filterbox to the context menu
  auto *txtBox = new QLineEdit(&modelMenu);
  txtBox->setPlaceholderText(QStringLiteral("Filter"));
  txtBox->setClearButtonEnabled(true);

  auto *txtBoxAction = new QWidgetAction(&modelMenu);
  txtBoxAction->setDefaultWidget(txtBox);
  modelMenu.addAction(txtBoxAction);

  //Add result treeview to the context menu
  auto *treeView = new QTreeWidget(&modelMenu);
  treeView->header()->close();

  auto *treeViewAction = new QWidgetAction(&modelMenu);
  treeViewAction->setDefaultWidget(treeView);

  modelMenu.addAction(treeViewAction);

  QMap<QString, QTreeWidgetItem*> topLevelItems;
  for (auto const &cat : _scene->registry().categories()) {
		if (cat == "Tasks") continue;
		auto item = new QTreeWidgetItem(treeView);
    item->setText(0, cat);
    item->setData(0, Qt::UserRole, skipText);
    topLevelItems[cat] = item;
  }

  for (auto const &assoc : _scene->registry().registeredModelsCategoryAssociation()) {
    if (assoc.first == "Task") continue;
    auto parent = topLevelItems[assoc.second];
    auto item   = new QTreeWidgetItem(parent);
    item->setText(0, assoc.first);
    item->setData(0, Qt::UserRole, assoc.first);
  }

  treeView->expandAll();

  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int) {
    QString modelName = item->data(0, Qt::UserRole).toString();
    DDEBUG_V("modelName=%s", modelName.toStdString().c_str());

    if (modelName == skipText) {
      return;
    }

    auto type = _scene->registry().create(modelName);

    if (type) {
      auto& node = _scene->createNode(std::move(type));
      QPoint pos = event->pos();
      QPointF posView = this->mapToScene(pos);
      node.nodeGraphicsObject().setPos(posView);

      if (modelName == "Model Param") {
        ((TransformDataModel*)(node.nodeDataModel()))->initialize();

        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        int newId = flowParam->getMaxModelId();
        FlowModelParamPtr fmParam = std::make_shared<FlowModelParam>(newId, NULL_ID, NULL_ID);
        fmParam->setRealElem(&node);
        fmParam->setNew();
        node.setParamId(fmParam->getId());
        flowParam->addModel(fmParam);

      } else if (modelName == "Flow Param") {
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        int newId = flowParam->getMaxParamId();
        FlowParameterParamPtr fmParam = std::make_shared<FlowParameterParam>(newId, "", "");
        fmParam->setRealElem(&node);
        fmParam->setNew();
        node.setParamId(fmParam->getId());
        flowParam->addFlowParam(fmParam);

      } else {
        ElementType typeId = ELEMENT_COMMAND;
        if (modelName == "Initial") {
          typeId = ELEMENT_START;
        } else if (modelName == "Final") {
          typeId = ELEMENT_FINAL;
        } else if (modelName == "Decision") {
          typeId = ELEMENT_DECISION;
        } else if (modelName == "Merge") {
          typeId = ELEMENT_MERGE;
        }
        int newId = targetParam_->getMaxStateId();
        ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newId, typeId, modelName, modelName, pos.x(), pos.y(), "");
        newParam->setRealElem(&node);
        newParam->setNew();
        node.setParamId(newParam->getId());
        targetParam_->addStmElement(newParam);
      }

    } else {
      DDEBUG("Etc Node");
      qDebug() << "Model not found";
    }

    modelMenu.close();
  });

  //Setup filtering
  connect(txtBox, &QLineEdit::textChanged, [&](const QString &text) {
    for (auto& topLvlItem : topLevelItems) {
      for (int i = 0; i < topLvlItem->childCount(); ++i) {
        auto child = topLvlItem->child(i);
        auto modelName = child->data(0, Qt::UserRole).toString();
        if (modelName.contains(text, Qt::CaseInsensitive)) {
          child->setHidden(false);
        } else {
          child->setHidden(true);
        }
      }
    }
  });

  // make sure the text box gets focus so the user doesn't have to click on it
  txtBox->setFocus();

  modelMenu.exec(event->globalPos());
}
///////////
void FlowEditor::dragEnterEvent(QDragEnterEvent* event) {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  if (event->mimeData()->hasFormat("application/TaskInstanceItem")) {
		event->acceptProposedAction();
	}
}

void FlowEditor::dragMoveEvent(QDragMoveEvent *event) {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  if (event->mimeData()->hasFormat("application/TaskInstanceItem")) {
		event->acceptProposedAction();
	}
}

void FlowEditor::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	ElementStmParamPtr target = getCurrentNode();
	if (target) {
		if (target->getType() == ElementType::ELEMENT_COMMAND) {
			flowView_->flowSelectionChanged(target->getTaskParam());
		}
	}
}

void FlowEditor::mouseDoubleClickEvent(QMouseEvent * event) {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  if (event->button() != Qt::LeftButton) return;
  DDEBUG("FlowActivityEditor::mouseDoubleClickEvent");

	QPointF pos = mapToScene(event->pos());
	QTransform trans;
	QGraphicsItem* item = _scene->itemAt(pos, trans);
	if (item) {
		if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
			//QGraphicsView::mouseReleaseEvent(event);
			int targetId = n->node().getParamId();
			std::vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
			vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
			if (targetElem == stateList.end()) return;
			flowView_->editClicked();
		}
	}
}

void FlowEditor::dropEvent(QDropEvent* event) {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
	if (event->mimeData()->hasFormat("application/TaskInstanceItem") == false) return;
	DDEBUG("FlowActivityEditor::dropEvent");

	QString strDispName = event->mimeData()->text();
	QString strName = "";
	QVariant varData = event->mimeData()->property("TaskInstanceItemId");
	/////
	auto type = _scene->registry().create("Task");

	if (type) {
		type->setTaskName(strDispName); // R.Hanai
		int id = varData.toInt();
		teaching::TaskModelParamPtr targetTask = teaching::TeachingDataHolder::instance()->getTaskInstanceById(id);
		if (targetTask) {
			std::vector<teaching::ParameterParamPtr> paramList = targetTask->getActiveParameterList();
			vector<PortInfo> portList;
			for (int index = 0; index < paramList.size(); index++) {
				teaching::ParameterParamPtr param = paramList[index];
				if (param->getHide()) continue;
				PortInfo info(param->getId(), param->getName());
				portList.push_back(info);
			}
			type->portNames = portList;
		}
		auto& node = _scene->createNode(std::move(type));
		QPoint pos = event->pos();
		QPointF posView = this->mapToScene(pos);
		node.nodeGraphicsObject().setPos(posView);
		//
		int newId = targetParam_->getMaxStateId();
		ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newId, ELEMENT_COMMAND, strName, strDispName, pos.x(), pos.y(), "");
		if (targetTask) {
			TaskModelParamPtr newTaskParam(new TaskModelParam(targetTask.get()));
			newTaskParam->setNewForce();
			newParam->setTaskParam(newTaskParam);
			newTaskParam->setStateParam(newParam);
		}
		newParam->setRealElem(&node);
		newParam->setNew();
		node.setParamId(newParam->getId());
		targetParam_->addStmElement(newParam);

	} else {
		qDebug() << "Model not found";
	}
}

void FlowEditor::createStateMachine(FlowParamPtr target) {
  removeAll();
  //
  vector<ElementStmParamPtr> elemList = target->getActiveStateList();
  for (int index = 0; index < elemList.size(); index++) {
    ElementStmParamPtr target = elemList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

    QString typeName = "";
    int typeId = target->getType();
    if (typeId == ELEMENT_START) {
      typeName = "Initial";
    } else if (typeId == ELEMENT_FINAL) {
      typeName = "Final";
    } else if (typeId == ELEMENT_DECISION) {
      typeName = "Decision";
    } else if (typeId == ELEMENT_MERGE) {
      typeName = "Merge";
    } else {
      typeName = "Task";
    }
    auto type = _scene->registry().create(typeName);
    if (type) {
      type->setTaskName(target->getCmdDspName());
      if (typeId == ELEMENT_COMMAND) {
        int id = target->getTaskParam()->getId();
        teaching::TaskModelParamPtr targetTask = teaching::TeachingDataHolder::instance()->getFlowTaskInstanceById(id);
        if (targetTask) {
          std::vector<teaching::ParameterParamPtr> paramList = targetTask->getActiveParameterList();
          vector<PortInfo> portList;
          for (int index = 0; index < paramList.size(); index++) {
            teaching::ParameterParamPtr param = paramList[index];
            if (param->getHide()) continue;
            PortInfo info(param->getId(), param->getName());
            portList.push_back(info);
          }
          type->portNames = portList;
        }
      }

      auto& node = _scene->createNode(std::move(type));
      node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
      target->setRealElem(&node);
      node.setParamId(target->getId());
      node.setBreak(target->isBreak());

    } else {
      qDebug() << "Model not found";
    }
  }
  /////
  vector<FlowModelParamPtr> modelList = target->getModelList();
  for (int index = 0; index < modelList.size(); index++) {
    FlowModelParamPtr target = modelList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

    QString typeName = "Model Param";
    auto type = _scene->registry().create(typeName);
    if (type) {
      auto& node = _scene->createNode(std::move(type));
      node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
      ((TransformDataModel*)(node.nodeDataModel()))->initialize();
      ((TransformDataModel*)(node.nodeDataModel()))->setMasterInfo(target->getMasterId(), target->getMasterParamId());
      target->setRealElem(&node);
      node.setParamId(target->getId());
    }
  }
  /////
  vector<FlowParameterParamPtr> paramList = target->getFlowParamList();
  for (int index = 0; index < paramList.size(); index++) {
    FlowParameterParamPtr target = paramList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

    QString typeName = "Flow Param";
    auto type = _scene->registry().create(typeName);
    if (type) {
      auto& node = _scene->createNode(std::move(type));
      node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
      ((ParamDataModel*)(node.nodeDataModel()))->setParamInfo(target->getName(), target->getValue());
      target->setRealElem(&node);
      node.setParamId(target->getId());
    }
  }
  /////
  vector<ConnectionStmParamPtr> connList = target->getActiveTransitionList();
  for (int index = 0; index < connList.size(); index++) {
    ConnectionStmParamPtr target = connList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

    Node* sourceNode;
    if (target->getType() == TYPE_MODEL_PARAM) {
      vector<FlowModelParamPtr>::iterator sourceElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(target->getSourceId()));
      if (sourceElem == modelList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();

    } else if (target->getType() == TYPE_FLOW_PARAM) {
      vector<FlowParameterParamPtr>::iterator sourceElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(target->getSourceId()));
      if (sourceElem == paramList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();

    } else {
      vector<ElementStmParamPtr>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
      if (sourceElem == elemList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();
    }

    vector<ElementStmParamPtr>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
    if (targetElem == elemList.end()) continue;
    Node* targetNode = (*targetElem)->getRealElem();
    _scene->createConnection(*targetNode, target->getTargetIndex(), *sourceNode, target->getSourceIndex());
  }
}

bool FlowEditor::updateTargetFlowParam() {
  DDEBUG("FlowEditor::updateTargetParam");
  if (targetParam_ == 0) return true;

	vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
	for (int index = 0; index < stateList.size(); index++) {
		ElementStmParamPtr target = stateList[index];
		target->updatePos();
	}
  FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
  //flowParam
  vector<FlowModelParamPtr> modelList = flowParam->getModelList();
  for (int index = 0; index < modelList.size(); index++) {
    FlowModelParamPtr target = modelList[index];
    target->updatePos();
  }
  vector<FlowParameterParamPtr> paramList = flowParam->getFlowParamList();
  for (int index = 0; index < paramList.size(); index++) {
    FlowParameterParamPtr target = paramList[index];
    target->updatePos();
  }
  //
	targetParam_->clearTransitionList();
	int connId = 1;
  unordered_map<QUuid, shared_ptr<Connection> > connMap = _scene->connections();
	for (auto it = connMap.begin(); it != connMap.end(); ++it) {
		shared_ptr<Connection> target = it->second;

		Node* sourceNode = target->getNode(PortType::Out);
		int sourceId = sourceNode->getParamId();
		int sourceIndex = target->getPortIndex(PortType::Out);
		//
		Node* targetNode = target->getNode(PortType::In);
		int targetId = targetNode->getParamId();
    int targetIndex = target->getPortIndex(PortType::In);
    //
    int type = TYPE_TRANSITION;
    if (sourceNode->nodeDataModel()->name() == "Model Param") {
      type = TYPE_MODEL_PARAM;
    } else if (sourceNode->nodeDataModel()->name() == "Flow Param") {
      type = TYPE_FLOW_PARAM;
    }
    ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(connId, type, sourceId, sourceIndex, targetId, targetIndex);
    connId++;
    connParam->setNew();
    targetParam_->addStmConnection(connParam);
    ////////
    flowParam->updateExecParam();
    if (type == TYPE_MODEL_PARAM) {
      //FlowModelParameterÇÃåüçı
      vector<FlowModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(sourceId));
      if (modelElem == modelList.end()) continue;
      //TaskParameterÇÃåüçı
      vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
      if (targetElem == stateList.end()) continue;

      Node* orgNode = (*targetElem)->getRealElem();
      int targetId = orgNode->nodeDataModel()->portNames.at(targetIndex - 1).id_;
      ParameterParamPtr targetParam = (*targetElem)->getTaskParam()->getParameterById(targetId);
      targetParam->setExecParamId((*modelElem)->getMasterId(), (*modelElem)->getMasterParamId());

    } else if (type == TYPE_FLOW_PARAM) {
      //FlowParameterÇÃåüçı
      vector<FlowParameterParamPtr>::iterator paramElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(sourceId));
      if (paramElem == paramList.end()) continue;
      //TaskParameterÇÃåüçı
      vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
      if (targetElem == stateList.end()) continue;

      Node* orgNode = (*targetElem)->getRealElem();
      int targetId = orgNode->nodeDataModel()->portNames.at(targetIndex - 1).id_;
      ParameterParamPtr targetParam = (*targetElem)->getTaskParam()->getParameterById(targetId);

      QString paramValue = (*paramElem)->getValue();
      if (paramValue.size() == 0) return false;
      QStringList valList = paramValue.split(",");
      int size = targetParam->getElemNum();
      if (valList.size() != size) return false;
      targetParam->setFlowValues(paramValue);
      DDEBUG_V("param Value: %s", paramValue.toStdString().c_str());
    }
	}
  return true;
}

void FlowEditor::updatingParamInfo(TaskModelParamPtr targetTask, ElementStmParamPtr targetState) {
	DDEBUG("FlowEditor::updatingParamInfo");

	std::vector<teaching::ParameterParamPtr> paramList = targetTask->getActiveParameterList();
	vector<PortInfo> portList;
	for (int index = 0; index < paramList.size(); index++) {
		teaching::ParameterParamPtr param = paramList[index];
		if (param->getHide()) continue;
		PortInfo info(param->getId(), param->getName());
		portList.push_back(info);
	}

	auto type = _scene->registry().create("Task");
	type->setTaskName(targetState->getCmdDspName());
	type->portNames = portList;

	auto& node = _scene->createNode(std::move(type));
	node.nodeGraphicsObject().setPos(targetState->getPosX(), targetState->getPosY());
	node.setParamId(targetState->getId());
	node.setBreak(targetState->isBreak());

	Node* orgNode = targetState->getRealElem();

	std::unordered_map<QUuid, Connection*> outMap = orgNode->nodeState().connections(PortType::Out, 0);
	for (auto it = outMap.begin(); it != outMap.end(); ++it) {
		Connection* target = it->second;
		_scene->deleteConnection(*target);
		Node* inNode = target->getNode(PortType::In);
		_scene->createConnection(*inNode, 0, node, 0);
	}
	std::unordered_map<QUuid, Connection*> inMap = orgNode->nodeState().connections(PortType::In, 0);
	for (auto it = inMap.begin(); it != inMap.end(); ++it) {
		Connection* target = it->second;
		Node* oppNode = target->getNode(PortType::Out);
		_scene->createConnection(node, 0, *oppNode, 0);
	}
	//
	int inNum = orgNode->nodeDataModel()->nPorts(PortType::In);
	for (int index = 1; index < inNum; index++) {
		int targetId = orgNode->nodeDataModel()->portNames.at(index - 1).id_;
		ParameterParamPtr targetParam = targetTask->getParameterById(targetId);
		if (targetParam->getHide()) {
			continue;
		}
		int targetIndex = -1;
		for (int idxInfo = 0; idxInfo < portList.size(); idxInfo++) {
			PortInfo info = portList[idxInfo];
			if (info.id_ == targetId) {
				targetIndex = idxInfo;
				break;
			}
		}
		//DDEBUG_V("index %d=%s", index, orgNode->nodeDataModel()->portNames.at(index - 1).toStdString().c_str());
		std::unordered_map<QUuid, Connection*> inMap = orgNode->nodeState().connections(PortType::In, index);
		for (auto it = inMap.begin(); it != inMap.end(); ++it) {
			Connection* target = it->second;
			Node* oppNode = target->getNode(PortType::Out);
			_scene->createConnection(node, targetIndex + 1, *oppNode, 0);
		}
	}
	_scene->removeNode(*orgNode);
	targetState->setRealElem(&node);
}

void FlowEditor::deleteSelectedNodes() {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  // delete the nodes, this will delete many of the connections
  for (QGraphicsItem * item : _scene->selectedItems()) {
    if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
      int targetId = n->node().getParamId();

      if (n->node().nodeDataModel()->name() == "Model Param") {
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        vector<FlowModelParamPtr> modelList = flowParam->getModelList();
        vector<FlowModelParamPtr>::iterator targetElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(targetId));
        if (targetElem != modelList.end()) {
          DDEBUG_V("DELETE Flow Model Param:%d", targetId);
          (*targetElem)->setDelete();
        }

      } else if (n->node().nodeDataModel()->name() == "Flow Param") {
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        vector<FlowParameterParamPtr> paramList = flowParam->getFlowParamList();
        vector<FlowParameterParamPtr>::iterator targetElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(targetId));
        if (targetElem != paramList.end()) {
          DDEBUG_V("DELETE Flow Param:%d", targetId);
          (*targetElem)->setDelete();
        }

      } else {
        std::vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
        vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
        if (targetElem != stateList.end()) {
          (*targetElem)->setDelete();
        }
      }
      _scene->removeNode(n->node());
    }
  }

  for (QGraphicsItem * item : _scene->selectedItems()) {
    if (auto c = qgraphicsitem_cast<ConnectionGraphicsObject*>(item))
      _scene->deleteConnection(c->connection());
  }
}

void FlowEditor::keyReleaseEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_Shift:
      setDragMode(QGraphicsView::ScrollHandDrag);
      break;

    case Qt::Key_Delete:
      deleteSelectedNodes();
      break;

    default:
      break;
  }
  QGraphicsView::keyReleaseEvent(event);
}
