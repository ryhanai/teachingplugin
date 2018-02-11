#include "FlowEditor.hpp"

#include "StyleCollection.hpp"

#include "../TeachingEventHandler.h"
#include "../TeachingDataHolder.h"
#include "../FlowView.h"

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
		if (assoc.first == "Task" || assoc.first == "3D Model") continue;
    auto parent = topLevelItems[assoc.second];
    auto item   = new QTreeWidgetItem(parent);
    item->setText(0, assoc.first);
    item->setData(0, Qt::UserRole, assoc.first);
  }

  auto parentModel = topLevelItems["3D Models"];
  std::vector<ElementStmParamPtr> taskList = targetParam_->getActiveStateList();
  for (int index = 0; index < taskList.size(); index++) {
    TaskModelParamPtr task = taskList[index]->getTaskParam();
    if (task == 0) continue;
    vector<ModelParamPtr> modelList = task->getActiveModelList();
    if (modelList.size() == 0) continue;
    auto taskItem = new QTreeWidgetItem(parentModel);
    taskItem->setText(0, task->getName());
    for (int idxMode = 0; idxMode < modelList.size(); idxMode++) {
      ModelParamPtr model = modelList[idxMode];
      if (model->getRName().length() == 0) continue;
      auto modelItem = new QTreeWidgetItem(taskItem);
      modelItem->setText(0, model->getRName());
      modelItem->setIcon(0, QIcon(QPixmap::fromImage(model->getModelMaster()->getImage())));
      modelItem->setData(0, Qt::UserRole, "3D Model");
      modelItem->setData(1, Qt::UserRole, task->getId());
      modelItem->setData(2, Qt::UserRole, model->getId());
    }
  }
  treeView->expandAll();

  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int) {
    QString modelName = item->data(0, Qt::UserRole).toString();
    DDEBUG_V("modelName %s", modelName.toStdString().c_str());

    if (modelName == skipText) {
      return;
    }

    auto type = _scene->registry().create(modelName);

    if (type) {
      if (modelName == "3D Model") {
        int taskId = item->data(1, Qt::UserRole).toInt();
        int modelId = item->data(2, Qt::UserRole).toInt();
        DDEBUG_V("taskId %d, modelId %d", taskId, modelId);

        ModelParamPtr targetModel;
        bool isHit = false;
        std::vector<ElementStmParamPtr> taskList = targetParam_->getActiveStateList();
        ElementStmParamPtr targetNode;
        TaskModelParamPtr targetTask;
        for (int index = 0; index < taskList.size(); index++) {
          targetTask = taskList[index]->getTaskParam();
          if (targetTask == 0) continue;
          if (targetTask->getId() != taskId) continue;
          vector<ModelParamPtr> modelList = targetTask->getActiveModelList();
          for (int idxMode = 0; idxMode < modelList.size(); idxMode++) {
            ModelParamPtr model = modelList[idxMode];
            if (model->getId() == modelId) {
              targetModel = model;
              targetNode = taskList[index];
              isHit = true;
              break;
            }
          }
          if (isHit) break;
        }
        if (isHit == false) return;
        //
        ModelMasterParamPtr master = targetModel->getModelMaster();
        DDEBUG_V("Master Name %s", master->getName().toStdString().c_str());
        type->setTaskName(targetModel->getRName() + "::" + master->getName());
        //
        vector<ModelParameterParamPtr> modelParamList =  master->getActiveParamList();
        ModelParameterParamPtr targetModelParam;
        vector<PortInfo> portList;
        PortInfo originPort(0, "origin");
        portList.push_back(originPort);
        for (int index = 0; index < modelParamList.size(); index++) {
          teaching::ModelParameterParamPtr param = modelParamList[index];
          PortInfo info(param->getId(), param->getName());
          portList.push_back(info);
        }
        type->portNames = portList;

        auto& node = _scene->createNode(std::move(type));
        QPoint pos = event->pos();
        QPointF posView = this->mapToScene(pos);
        node.nodeGraphicsObject().setPos(posView);
        //
        FlowModelParamPtr fmParam = std::make_shared<FlowModelParam>(NULL_ID, master->getId());
        fmParam->setRealElem(&node);
        fmParam->setNew();
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        flowParam->addModel(fmParam);
        /////
        //Port間の接続
        Node* taskNode = targetNode->getRealElem();
        std::vector<ParameterParamPtr> paramList = targetTask->getActiveParameterList();
        DDEBUG_V("paramList:%d", paramList.size());
        for (int index = 0; index < paramList.size(); index++) {
          ParameterParamPtr param = paramList[index];
          if (param->getType() == PARAM_KIND_NORMAL) continue;
          if (param->getHide() == 1) continue;
          DDEBUG_V("ModelId:%d ModelParamId:%d", param->getModelId(), param->getModelParamId());
          if (param->getModelId() == targetModel->getId()) {
            //タスク側の対象ポートの検索
            int portNum = taskNode->nodeDataModel()->nPorts(PortType::In);
            int taskPortIdx = 0;
            int modelPortIdx = 0;
            for (int idxPort = 0; idxPort < portNum; idxPort++) {
              if (taskNode->nodeDataModel()->portCaption(PortType::In, idxPort) == param->getName()) {
                taskPortIdx = idxPort;
                break;
              }
            }
            //モデル側対象ポートの検索
            if (0 < param->getModelParamId()) {
              for (int index = 0; index < modelParamList.size(); index++) {
                teaching::ModelParameterParamPtr master = modelParamList[index];
                if (master->getId() == param->getModelParamId()) {
                  modelPortIdx = index + 1;
                  break;
                }
              }
            }
            _scene->createConnection(*taskNode, taskPortIdx, node, modelPortIdx);
          }
        }

      } else {
        auto& node = _scene->createNode(std::move(type));
        QPoint pos = event->pos();
        QPointF posView = this->mapToScene(pos);
        node.nodeGraphicsObject().setPos(posView);

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

void FlowEditor::createStateMachine(std::vector<ElementStmParamPtr>& elemList, std::vector<ConnectionStmParamPtr>& connList) {
	removeAll();

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
	for (int index = 0; index < connList.size(); index++) {
		ConnectionStmParamPtr target = connList[index];
		if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

		vector<ElementStmParamPtr>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
		if (sourceElem == elemList.end()) continue;
		vector<ElementStmParamPtr>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
		if (targetElem == elemList.end()) continue;
		//
		Node* sourceNode = (*sourceElem)->getRealElem();
		Node* targetNode = (*targetElem)->getRealElem();

    _scene->createConnection(*targetNode, target->getTargetIndex(), *sourceNode, target->getSourceIndex());
  }
}

void FlowEditor::updateTargetParam() {
	if (targetParam_ == 0) return;

	vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
	for (int index = 0; index < stateList.size(); index++) {
		ElementStmParamPtr target = stateList[index];
		target->updatePos();
	}
  FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
  vector<FlowModelParamPtr> modelList = flowParam->getModelList();
  for (int index = 0; index < modelList.size(); index++) {
    FlowModelParamPtr target = modelList[index];
    target->updatePos();
  }
	//
	//TODO Flow Parameter対応
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
		ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(connId, sourceId, sourceIndex, targetId, targetIndex);
		connId++;
		connParam->setNew();
		targetParam_->addStmConnection(connParam);
	}
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
