#include "FlowEditor.hpp"

#include "StyleCollection.hpp"

#include "../TeachingEventHandler.h"
#include "../TeachingDataHolder.h"
#include "../FlowView.h"
#include "../TeachingUtil.h"
#include "models.hpp"

#include "../ChoreonoidUtil.h"
#include "../LoggerUtil.h"
#include "../gettext.h"

using QtNodes::FlowEditor;
using QtNodes::FlowScene;
using namespace teaching;

FlowEditor::FlowEditor(FlowScene *scene, FlowViewImpl* flowView)
  : ActivityEditorBase(scene), updateNodesLater_([&]() { removeModelNodeLater(); }),
  flowView_(flowView) {
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
    auto item = new QTreeWidgetItem(parent);
    item->setText(0, assoc.first);
    item->setData(0, Qt::UserRole, assoc.first);
  }

  treeView->expandAll();

  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int) {
    QString modelName = item->data(0, Qt::UserRole).toString();
    DDEBUG_V("modelName=%s", modelName.toStdString().c_str());
    if (modelName == skipText) return;

    QPoint pos = event->pos();
    createFlowNodeAux(modelName, pos);

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

bool FlowEditor::renameNode(QString currentName, QString newName) {
  Node* node = getNodeByName(currentName);
  if (node->nodeDataModel()->name() == "Model Param") {
    ((TransformDataModel*)(node->nodeDataModel()))->setName(newName);
    return true;
  }

  return false;
}

Node* FlowEditor::getNodeByName(QString name) {
  Node* node = NULL;
  this->_scene->iterateOverNodes(
    [&](Node* nd) {
    // name() of Task node returns just "Task" instead of the name of the task
    // std::cout << nd->nodeDataModel()->caption().toStdString() << std::endl;
    if ((nd->nodeDataModel()->name() == "Model Param" && ((TransformDataModel*)nd->nodeDataModel())->getName() == name)
      || nd->nodeDataModel()->caption() == name) {
      node = nd;
    }
  }
  );

  if (node) {
    std::cout << "NODE " << name.toStdString() << " found." << std::endl;
  } else {
    std::cout << "NODE " << name.toStdString() << " not found." << std::endl;
  }
  return node;
}

bool FlowEditor::connectNodes(QString from, QString fromPort, QString to, QString toPort) {
  //bool FlowEditor::connectNodes(QString from, PortIndex fromIdx, QString to, PortIndex toIdx) {
  Node* fromNode = getNodeByName(from);
  Node* toNode = getNodeByName(to);

  PortIndex fromIdx = 0;
  PortIndex toIdx = 0;
  if (from == "Decision" && fromPort == "false") { fromIdx = 1; }
  if (to == "Merge" && toPort == "2nd") { toIdx = 1; }
  // "Task"
  if (toNode->nodeDataModel()->name() == "Task") {
    std::cout << "TASK:" << std::endl;
    auto portNames = toNode->nodeDataModel()->portNames;
    for (auto p : portNames) {
      std::cout << "P: " << p.name_.toStdString() << std::endl;
    }
    auto result = std::find_if(portNames.begin(), portNames.end(), [&](PortInfo info) { return info.name_ == toPort; });
    if (result != portNames.end()) {
      toIdx = std::distance(portNames.begin(), result) + 1;
    }
    // control flow
  }
  // "Model"
  if (fromNode->nodeDataModel()->name() == "Model Param") {
    auto portNames = fromNode->nodeDataModel()->portNames;
    auto result = std::find_if(portNames.begin(), portNames.end(), [&](PortInfo info) { return info.name_ == fromPort; });
    if (result != portNames.end()) {
      fromIdx = std::distance(portNames.begin(), result);
    }
  }

  if (fromNode && toNode) {
    this->_scene->createConnection(*toNode, toIdx, *fromNode, fromIdx);
    return true;
  }

  return false;
}

bool FlowEditor::connectModelToTask(Node* fromNode, QString fromPort, Node* toNode, QString toPort) {
  PortIndex fromIdx = 0;
  PortIndex toIdx = 0;

  if (fromNode->nodeDataModel()->name() != "Model Param") return false;
  if (toNode->nodeDataModel()->name() != "Task") return false;

  {
    auto portNames = fromNode->nodeDataModel()->portNames;
    auto result = std::find_if(portNames.begin(), portNames.end(), [&](PortInfo info) { return info.name_ == fromPort; });
    if (result != portNames.end()) {
      fromIdx = std::distance(portNames.begin(), result);
    }
  }
  {
    auto portNames = toNode->nodeDataModel()->portNames;
    auto result = std::find_if(portNames.begin(), portNames.end(), [&](PortInfo info) { return info.name_ == toPort; });
    if (result != portNames.end()) {
      toIdx = std::distance(portNames.begin(), result) + 1;
    }
  }

  if (fromIdx == 0 || toIdx == 0) return false;
  std::shared_ptr<QtNodes::Connection> connection = this->_scene->createConnection(*toNode, toIdx, *fromNode, fromIdx);
  TeachingEventHandler::instance()->flv_Connected(*connection);

  return true;
}

bool FlowEditor::createFlowNodeAux(QString modelName, QPoint pos) {
  QPointF posView = this->mapToScene(pos);

  if (modelName == "Model Param") {
    FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
    int newId = flowParam->getMaxModelId();
    int masterId = NULL_ID;
    vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
    if (0 < modelMasterList.size()) {
      masterId = modelMasterList[0]->getId();
    }
    FlowModelParamPtr fmParam = std::make_shared<FlowModelParam>(newId, masterId, "New Model Param");
    fmParam->setPosX(posView.x());
    fmParam->setPosY(posView.y());
    fmParam->setNew();
    flowParam->addModel(fmParam);
    createFlowModelNode(fmParam);
    return true;

  } else if (modelName.startsWith("Flow Param")) {
    FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
    int newId = flowParam->getMaxParamId();
    int type = 0;
    FlowParameterParamPtr fmParam;
    if (modelName == "Flow Param (Integer)") {
      type = PARAM_TYPE_INTEGER;
      fmParam = std::make_shared<FlowParameterParam>(newId, type, "ParamName", "0");

    } else if (modelName == "Flow Param (Double)") {
      type = PARAM_TYPE_DOUBLE;
      fmParam = std::make_shared<FlowParameterParam>(newId, type, "ParamName", "0.0");

    } else if (modelName == "Flow Param (String)") {
      type = PARAM_TYPE_STRING;
      fmParam = std::make_shared<FlowParameterParam>(newId, type, "ParamName", "");

    } else if (modelName == "Flow Param (Frame)") {
      type = PARAM_TYPE_FRAME;
      fmParam = std::make_shared<FlowParameterParam>(newId, type, "ParamName", "0.0");
    }
    fmParam->setPosX(posView.x());
    fmParam->setPosY(posView.y());
    fmParam->setNew();
    flowParam->addFlowParam(fmParam);
    createFlowParamNode(fmParam);
    return true;

  } else if (modelName == "Initial" || modelName == "Final" || modelName == "Decision" || modelName == "Merge") {
    ElementType typeId = ELEMENT_START;
    if (modelName == "Initial") {
      typeId = ELEMENT_START;
    } else if (modelName == "Final") {
      typeId = ELEMENT_FINAL;
    } else if (modelName == "Decision") {
      typeId = ELEMENT_DECISION;
    } else if (modelName == "Merge") {
      typeId = ELEMENT_MERGE;
    } else {
      DDEBUG("Etc Node");
      qDebug() << "Model not found";
    }

    int newId = targetParam_->getMaxStateId();
    ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newId, typeId, modelName, modelName, posView.x(), posView.y(), "");
    newParam->setNew();
    targetParam_->addStmElement(newParam);
    createFlowExtNode(typeId, newParam);
    return true;

  } else {
    //if (TeachingEventHandler::instance()->canEdit() == false) return;
    QString strDispName = modelName;
    QString strName = "";

    auto type = _scene->registry().create("Task");
    if (type) {
      type->setTaskName(strDispName);
      auto taskList = teaching::TeachingDataHolder::instance()->getTaskList();
      auto result = std::find_if(taskList.begin(), taskList.end(),
        [&](teaching::TaskModelParamPtr task) { return task->getName() == modelName; });
      if (result == taskList.end()) { return false; }

      teaching::TaskModelParamPtr targetTask = *result;
      vector<PortInfo> portList;
      createPortInfo(targetTask, portList);
      type->portNames = portList;

      auto& node = _scene->createNode(std::move(type));
      QPointF posView = this->mapToScene(pos);
      node.nodeGraphicsObject().setPos(posView);
      //
      int newId = targetParam_->getMaxStateId();
      ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newId, ELEMENT_COMMAND, strName, strDispName, pos.x(), pos.y(), "");
      TaskModelParamPtr newTaskParam(new TaskModelParam(targetTask.get()));
      newTaskParam->setNewForce();
      newParam->setTaskParam(newTaskParam);
      newTaskParam->setStateParam(newParam);

      newParam->setRealElem(&node);
      newParam->setNew();
      node.setParamId(newParam->getId());
      targetParam_->addStmElement(newParam);

      return true;
    }
  }

  return false;
}

void FlowEditor::clearFlowScene() {
  this->_scene->clearScene();
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
  DDEBUG("FlowActivityEditor::mouseReleaseEvent");
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
      int targetId = n->node().getParamId();
      std::vector<ElementStmParamPtr> stateList = targetParam_->getActiveStateList();
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
      vector<PortInfo> portList;
      createPortInfo(targetTask, portList);
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
  for (ElementStmParamPtr target : elemList) {
    int typeId = target->getType();
    if (typeId == ELEMENT_COMMAND) {
      createFlowTaskNode(target);
    } else {
      createFlowExtNode(typeId, target);
    }
  }
  vector<FlowModelParamPtr> modelList = target->getActiveModelList();
  for (FlowModelParamPtr target : modelList) {
    createFlowModelNode(target);
  }
  vector<FlowParameterParamPtr> paramList = target->getActiveFlowParamList();
  for (FlowParameterParamPtr target : paramList) {
    createFlowParamNode(target);
  }
  /////
  vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
  for (ConnectionStmParamPtr targetCon : target->getActiveTransitionList()) {
    vector<ElementStmParamPtr>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(targetCon->getTargetId()));
    if (targetElem == elemList.end()) continue;
    Node* targetNode = (*targetElem)->getRealElem();

    Node* sourceNode;
    if (targetCon->getType() == TYPE_MODEL_PARAM) {
      vector<FlowModelParamPtr>::iterator sourceElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(targetCon->getSourceId()));
      if (sourceElem == modelList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();
      //
      //モデルの差し替え
      int targetId = targetNode->getParamId();
      int id = targetNode->nodeDataModel()->portNames[targetCon->getTargetIndex() - 1].id_;
      int sourceId = sourceNode->getParamId();

      TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();
      ModelParamPtr model = taskParam->getModelParamById(id);
      int masterId = (*sourceElem)->getMasterId();
      vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
      if (masterParamItr != modelMasterList.end()) {
        model->updateModelMaster(*masterParamItr);
      }

    } else if (targetCon->getType() == TYPE_FLOW_PARAM) {
      vector<FlowParameterParamPtr>::iterator sourceElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(targetCon->getSourceId()));
      if (sourceElem == paramList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();

    } else {
      vector<ElementStmParamPtr>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(targetCon->getSourceId()));
      if (sourceElem == elemList.end()) continue;
      sourceNode = (*sourceElem)->getRealElem();
    }

    _scene->createConnection(*targetNode, targetCon->getTargetIndex(), *sourceNode, targetCon->getSourceIndex());
  }
}

void FlowEditor::createFlowTaskNode(ElementStmParamPtr target) {
  auto type = _scene->registry().create("Task");
  type->setTaskName(target->getCmdDspName());

  teaching::TaskModelParamPtr targetTask = target->getTaskParam();
  if (targetTask) {
    vector<PortInfo> portList;
    createPortInfo(targetTask, portList);
    type->portNames = portList;
  }

  auto& node = _scene->createNode(std::move(type));
  node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
  target->setRealElem(&node);
  node.setParamId(target->getId());
  node.setBreak(target->isBreak());
}

void FlowEditor::createFlowExtNode(int typeId, ElementStmParamPtr target) {
  QString typeName = "";
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
    auto& node = _scene->createNode(std::move(type));
    node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
    target->setRealElem(&node);
    node.setParamId(target->getId());
    node.setBreak(target->isBreak());
  } else {
    qDebug() << "Model not found";
  }
}

void FlowEditor::createFlowParamNode(FlowParameterParamPtr target) {
  QString typeName = "";
  switch (target->getType()) {
    case PARAM_TYPE_INTEGER:  typeName = "Flow Param (Integer)"; break;
    case PARAM_TYPE_DOUBLE:   typeName = "Flow Param (Double)";  break;
    case PARAM_TYPE_STRING:   typeName = "Flow Param (String)";  break;
    case PARAM_TYPE_FRAME:    typeName = "Flow Param (Frame)";  break;
    default: return;
  }
  auto type = _scene->registry().create(typeName);
  if (type) {
    auto& node = _scene->createNode(std::move(type));
    node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
    if (target->getType() == PARAM_TYPE_FRAME) {
      ((FrameParamDataModel*)(node.nodeDataModel()))->setParamInfo(target->getName(), target->getValue());
    } else {
      ((ParamDataModel*)(node.nodeDataModel()))->setParamInfo(target->getName(), target->getValue());
    }
    target->setRealElem(&node);
    node.setParamId(target->getId());
  }
}

void FlowEditor::createFlowModelNode(FlowModelParamPtr target) {
  auto type = _scene->registry().create("Model Param");
  if (type) {
    vector<PortInfo> portList;
    PortInfo modelPort(0, "", 1);
    portList.push_back(modelPort);
    PortInfo orgPort(0, "origin", 0);
    portList.push_back(orgPort);

    int masterId = target->getMasterId();
    vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
    vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
    if (masterParamItr != modelMasterList.end()) {
      vector<ModelParameterParamPtr> paramList = (*masterParamItr)->getActiveModelParamList();
      for (int index = 0; index < paramList.size(); index++) {
        ModelParameterParamPtr param = paramList[index];
        PortInfo infoParam(index + 1, param->getName(), 0);
        portList.push_back(infoParam);
      }
    }
    type->portNames = portList;

    auto& node = _scene->createNode(std::move(type));
    node.nodeGraphicsObject().setPos(target->getPosX(), target->getPosY());
    ((TransformDataModel*)(node.nodeDataModel()))->initialize();
    ((TransformDataModel*)(node.nodeDataModel()))->setMasterInfo(target->getMasterId());
    ((TransformDataModel*)(node.nodeDataModel()))->setName(target->getName());
    target->setRealElem(&node);
    node.setParamId(target->getId());
    ((TransformDataModel*)(node.nodeDataModel()))->setFlowModelParamId(target->getId());
  }
}

bool FlowEditor::updateTargetFlowParam(QString& errMessage) {
  DDEBUG("FlowEditor::updateTargetParam");
  if (targetParam_ == 0) return true;

  //flowParam
  FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
  ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(flowParam);
  for (ElementStmParamPtr state : flowParam->getActiveStateList()) {
    if (state->getType() != ELEMENT_DECISION) continue;
    QString cond = state->getCondition();
    if (cond.length() == 0) {
      errMessage = _("Condition is not set for Decision node.");
      delete handler;
      return false;
    }
    string errmsg;
    if (handler->checkSyntax(flowParam, 0, cond, errmsg) == false) {
      errMessage = _("Decision node condition is INVALID.") + QString::fromStdString(errmsg);
      delete handler;
      return false;
    }
  }
  delete handler;

  QStringList checkList;
  for (FlowParameterParamPtr target : flowParam->getActiveFlowParamList()) {
    if (target->updateParamInfo() == false) {
      errMessage = _("Parameter setting value is invalid.") + QString::fromStdString(" ") + target->getName();
      return false;
    }
    if (checkList.contains(target->getName())) {
      errMessage = _("Duplicate flow parameter names.");
      return false;
    }
    checkList.append(target->getName());
  }

  vector<FlowModelParamPtr> modelList = flowParam->getActiveModelList();
  for (FlowModelParamPtr target : modelList) {
    target->updatePos();
  }

  vector<ElementStmParamPtr> stateList = targetParam_->getActiveStateList();
  for (ElementStmParamPtr target : stateList) {
    target->updatePos();
  }
  //
  DDEBUG("FlowEditor::updateTargetParam Transition");
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
    } else if (sourceNode->nodeDataModel()->name().startsWith("Flow Param")) {
      type = TYPE_FLOW_PARAM;
    }
    ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(connId, type, sourceId, sourceIndex, targetId, targetIndex);
    connId++;
    connParam->setNew();
    targetParam_->addStmConnection(connParam);
  }
  DDEBUG("FlowEditor::updateTargetParam End");
  return true;
}

void FlowEditor::paramInfoUpdated(ElementStmParamPtr targetState) {
  DDEBUG("FlowEditor::paramInfoUpdated");

  TaskModelParamPtr targetTask = targetState->getTaskParam();
  if (!targetTask) return;

  vector<PortInfo> portList;
  createPortInfo(targetTask, portList);

  auto type = _scene->registry().create("Task");
  type->setTaskName(targetState->getCmdDspName());
  type->portNames = portList;

  Node* orgNode = targetState->getRealElem();
  auto& node = _scene->createNode(std::move(type));
  node.nodeGraphicsObject().setPos(orgNode->nodeGraphicsObject().pos().x(), orgNode->nodeGraphicsObject().pos().y());
  node.setParamId(targetState->getId());
  node.setBreak(targetState->isBreak());

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
    int targetType = orgNode->nodeDataModel()->portNames.at(index - 1).type_;
    int targetId = orgNode->nodeDataModel()->portNames.at(index - 1).id_;
    DDEBUG_V("index:%d, targetType:%d, targetId:%d", index, targetType, targetId);

    if (targetType == 0 || targetType == 2) {
      ParameterParamPtr targetParam = targetTask->getParameterById(targetId);
      if (targetParam->getHide()) {
        continue;
      }
    } else {
      ModelParamPtr targetParam = targetTask->getModelParamById(targetId);
      if (targetParam->getHide()) {
        continue;
      }
    }

    int targetIndex = -1;
    for (int idxInfo = 0; idxInfo < portList.size(); idxInfo++) {
      PortInfo info = portList[idxInfo];
      if (info.id_ == targetId && info.type_ == targetType) {
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
  DDEBUG("FlowEditor::deleteSelectedNodes");
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  // delete the nodes, this will delete many of the connections
  for (QGraphicsItem * item : _scene->selectedItems()) {
    if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
      int targetId = n->node().getParamId();

      if (n->node().nodeDataModel()->name() == "Model Param") {
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        vector<FlowModelParamPtr> modelList = flowParam->getActiveModelList();
        vector<FlowModelParamPtr>::iterator targetElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(targetId));
        if (targetElem != modelList.end()) {
          DDEBUG_V("DELETE Flow Model Param:%d", targetId);
          (*targetElem)->setDelete();
        }

      } else if (n->node().nodeDataModel()->name().startsWith("Flow Param")) {
        FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
        vector<FlowParameterParamPtr> paramList = flowParam->getActiveFlowParamList();
        vector<FlowParameterParamPtr>::iterator targetElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(targetId));
        if (targetElem != paramList.end()) {
          DDEBUG_V("DELETE Flow Param:%d", targetId);
          (*targetElem)->setDelete();
        }

      } else {
        std::vector<ElementStmParamPtr> stateList = targetParam_->getActiveStateList();
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

void FlowEditor::hideSelectedNodes() {
  for (QGraphicsItem * item : _scene->selectedItems()) {
    if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
      n->node().setHide(true);
      QWidget* widget = n->node().nodeDataModel()->embeddedWidget();
      if (widget) {
        widget->setVisible(false);
      }
      n->node().nodeGraphicsObject().update();
    }
  }
  _scene->update();
}

void FlowEditor::dispSetting() {
  QString errMessage;
  updateTargetFlowParam(errMessage);
  FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
  NodeDispDialog dialog(flowParam, this);
  dialog.exec();
  //
  for (ElementStmParamPtr node : targetParam_->getActiveStateList()) {
    QtNodes::Node* target = node->getRealElem();
    if (target) {
      target->nodeGraphicsObject().update();
    }
  }
  //
  for (FlowModelParamPtr node : flowParam->getActiveModelList()) {
    QtNodes::Node* target = node->getRealElem();
    if (target) {
      QWidget* widget = target->nodeDataModel()->embeddedWidget();
      if (widget) {
        widget->setVisible(!target->isHide());
      }
      target->nodeGraphicsObject().update();
    }
  }
  //
  for (FlowParameterParamPtr node : flowParam->getActiveFlowParamList()) {
    QtNodes::Node* target = node->getRealElem();
    if (target) {
      QWidget* widget = target->nodeDataModel()->embeddedWidget();
      if (widget) {
        widget->setVisible(!target->isHide());
      }
      target->nodeGraphicsObject().update();
    }
  }

  _scene->update();
}

void FlowEditor::portDispSetting() {
  bool isActive = true;
  ElementStmParamPtr target = getCurrentNode();
  if (!target || target->getType() != ElementType::ELEMENT_COMMAND) {
    isActive = false;
  }
  TeachingEventHandler::instance()->flv_PortDispSetting(isActive);
}

void FlowEditor::modelParamUpdated(int flowModelId, ModelMasterParamPtr masterParam) {
  DDEBUG_V("FlowEditor::modelParamUpdated : %d, %d", flowModelId, masterParam->getId());
  //対象ノードの検索
  FlowParamPtr flowParam = std::dynamic_pointer_cast<FlowParam>(targetParam_);
  vector<FlowModelParamPtr> modelList = flowParam->getActiveModelList();
  vector<FlowModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(flowModelId));
  if (modelElem == modelList.end()) {
    DDEBUG("FlowModelParam NOT FOUND");
    return;
  }

  if ((*modelElem)->getMasterId() == masterParam->getId()) return;

  Node* targetNode = (*modelElem)->getRealElem();

  std::vector<ConnectionInfo> connectionList;
  std::unordered_map<QUuid, Connection*> outKeepMap = targetNode->nodeState().connections(PortType::Out, 0);
  for (auto it = outKeepMap.begin(); it != outKeepMap.end(); ++it) {
    Connection* target = it->second;
    ConnectionInfo info;
    info.node = target->getNode(PortType::In);
    info.portindex = target->getPortIndex(PortType::In);
    connectionList.push_back(info);
  }

  int outNum = targetNode->nodeDataModel()->nPorts(PortType::Out);
  for (int index = 0; index < outNum; index++) {
    std::unordered_map<QUuid, Connection*> outMap = targetNode->nodeState().connections(PortType::Out, index);
    for (auto it = outMap.begin(); it != outMap.end(); ++it) {
      Connection* target = it->second;
      _scene->deleteConnection(*target);
    }
  }

  (*modelElem)->setName(((TransformDataModel*)targetNode->nodeDataModel())->getName());
  (*modelElem)->setMasterId(masterParam->getId());
  (*modelElem)->setPosX(targetNode->nodeGraphicsObject().pos().x());
  (*modelElem)->setPosY(targetNode->nodeGraphicsObject().pos().y());
  createFlowModelNode(*modelElem);

  vector<ElementStmParamPtr> stateList = flowParam->getActiveStateList();
  for (ConnectionInfo info : connectionList) {
    _scene->createConnection(*info.node, info.portindex, *(*modelElem)->getRealElem(), 0);

    int targetId = info.node->getParamId();
    int id = info.node->nodeDataModel()->portNames[info.portindex - 1].id_;
    vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
    if (targetElem == stateList.end()) return;
    TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();

    ModelParamPtr model = taskParam->getModelParamById(id);
    ChoreonoidUtil::replaceMaster(model, masterParam);
  }

  removingNode_ = targetNode;
  updateNodesLater_();
  DDEBUG("FlowEditor::modelParamUpdated End");
}

void FlowEditor::removeModelNodeLater() {
  DDEBUG("FlowEditor::removeModelNodeLater");
  _scene->removeNode(*removingNode_);
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

bool FlowEditor::checkOutConnection(int nodeId, int portIndex) {
  int count = 0;
  unordered_map<QUuid, shared_ptr<Connection> > connMap = _scene->connections();
  for (auto it = connMap.begin(); it != connMap.end(); ++it) {
    shared_ptr<Connection> target = it->second;

    Node* sourceNode = target->getNode(PortType::Out);
    int sourceId = sourceNode->getParamId();
    int sourceIndex = target->getPortIndex(PortType::Out);

    if (sourceId == nodeId && sourceIndex == portIndex) count++;
  }
  if (count == 1) return false;
  return true;
}

void FlowEditor::createPortInfo(TaskModelParamPtr targetTask, vector<PortInfo>& portList) {
  for (ModelParamPtr model : targetTask->getVisibleModelList()) {
    PortInfo info(model->getId(), model->getRName(), 1);
    portList.push_back(info);
  }
  //
  for (ParameterParamPtr param : targetTask->getVisibleParameterList()) {
    int type = 0;
    QString typeName;
    if (param->getType() == PARAM_KIND_MODEL) {
      type = 2;
      typeName = ":Mdl";
    } else {
      switch (param->getParamType()) {
        case 1:
          typeName = ":Int"; break;
        case 2:
          typeName = ":Dbl"; break;
        case 3:
          typeName = ":Str"; break;
        case 4:
          typeName = ":Frm"; break;
      }
    }
    PortInfo info(param->getId(), param->getName() + typeName, type);
    portList.push_back(info);
  }
}
