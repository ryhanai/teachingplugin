#include "StateMachineEditor.hpp"

#include "StyleCollection.hpp"
//
#include "../TaskExecutor.h"
#include "../StateMachineView.h"

#include "../LoggerUtil.h"

using QtNodes::StateMachineEditor;
using QtNodes::FlowScene;
using QtNodes::Node;

StateMachineEditor::StateMachineEditor(FlowScene *scene, teaching::StateMachineViewImpl* stateView)
  : ActivityEditorBase(scene), stateView_(stateView) {

  setDragMode(QGraphicsView::ScrollHandDrag);
  setRenderHint(QPainter::Antialiasing);

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  setBackgroundBrush(flowViewStyle.BackgroundColor);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  setCacheMode(QGraphicsView::CacheBackground);

  // setup actions
  _clearSelectionAction = new QAction(QStringLiteral("Clear Selection"), this);
  _clearSelectionAction->setShortcut(Qt::Key_Escape);
  connect(_clearSelectionAction, &QAction::triggered, _scene, &QGraphicsScene::clearSelection);
  addAction(_clearSelectionAction);

  _deleteSelectionAction = new QAction(QStringLiteral("Delete Selection"), this);
  _deleteSelectionAction->setShortcut(Qt::Key_Delete);
  connect(_deleteSelectionAction, &QAction::triggered, this, &StateMachineEditor::deleteSelectedNodes);
  addAction(_deleteSelectionAction);
}

//void StateMachineEditor::contextMenuEvent(QContextMenuEvent *event) {
//	if (itemAt(event->pos())) {
//    QGraphicsView::contextMenuEvent(event);
//    return;
//  }
//
//  QMenu modelMenu;
//
//  auto skipText = QStringLiteral("skip me");
//
//  //Add filterbox to the context menu
//  auto *txtBox = new QLineEdit(&modelMenu);
//
//  txtBox->setPlaceholderText(QStringLiteral("Filter"));
//  txtBox->setClearButtonEnabled(true);
//
//  auto *txtBoxAction = new QWidgetAction(&modelMenu);
//  txtBoxAction->setDefaultWidget(txtBox);
//
//  modelMenu.addAction(txtBoxAction);
//
//  //Add result treeview to the context menu
//  auto *treeView = new QTreeWidget(&modelMenu);
//  treeView->header()->close();
//
//  auto *treeViewAction = new QWidgetAction(&modelMenu);
//  treeViewAction->setDefaultWidget(treeView);
//
//  modelMenu.addAction(treeViewAction);
//
//  QMap<QString, QTreeWidgetItem*> topLevelItems;
//  for (auto const &cat : _scene->registry().categories())
//  {
//    auto item = new QTreeWidgetItem(treeView);
//    item->setText(0, cat);
//    item->setData(0, Qt::UserRole, skipText);
//    topLevelItems[cat] = item;
//  }
//
//  std::vector<QString> taskNames = {"go initial", "pick", "place", "screw", "recognize"};
//
//  for (auto const &assoc : _scene->registry().registeredModelsCategoryAssociation())
//  {
//    if (assoc.second == "Tasks") {
//      auto parent = topLevelItems[assoc.second];
//      for (auto taskname : taskNames) {
//        auto item   = new QTreeWidgetItem(parent);
//        item->setText(0, taskname);
//        item->setData(0, Qt::UserRole, taskname);
//      }
//    } else {
//      auto parent = topLevelItems[assoc.second];
//      auto item   = new QTreeWidgetItem(parent);
//      item->setText(0, assoc.first);
//      item->setData(0, Qt::UserRole, assoc.first);
//    }
//  }
//
//  treeView->expandAll();
//
//  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int)
//  {
//    QString modelName = item->data(0, Qt::UserRole).toString();
//		DDEBUG_V("modelName:%s", modelName.toStdString().c_str());
//
//    if (modelName == skipText)
//    {
//      return;
//    }
//
//    QString taskName("");
//    auto result = std::find(taskNames.begin(), taskNames.end(), modelName);
//    if (result != taskNames.end()) {
//      taskName = modelName;
//      modelName = "Task";
//    }
//
//    auto type = _scene->registry().create(modelName);
//
//    if (type) {
//      type->setTaskName(taskName); // R.Hanai
//
//      auto& node = _scene->createNode(std::move(type));
//
//      QPoint pos = event->pos();
//
//      QPointF posView = this->mapToScene(pos);
//
//      node.nodeGraphicsObject().setPos(posView);
//    }
//    else
//    {
//      qDebug() << "Model not found";
//    }
//
//    modelMenu.close();
//  });
//
//  //Setup filtering
//  connect(txtBox, &QLineEdit::textChanged, [&](const QString &text)
//  {
//    for (auto& topLvlItem : topLevelItems)
//    {
//      for (int i = 0; i < topLvlItem->childCount(); ++i)
//      {
//        auto child = topLvlItem->child(i);
//        auto modelName = child->data(0, Qt::UserRole).toString();
//        if (modelName.contains(text, Qt::CaseInsensitive))
//        {
//          child->setHidden(false);
//        }
//        else
//        {
//          child->setHidden(true);
//        }
//      }
//    }
//  });
//
//  // make sure the text box gets focus so the user doesn't have to click on it
//  txtBox->setFocus();
//
//  modelMenu.exec(event->globalPos());
//}

///////////
void StateMachineEditor::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasFormat("application/StateMachineItem")) {
		event->acceptProposedAction();
	}
}

void StateMachineEditor::dragMoveEvent(QDragMoveEvent *event) {
	if (event->mimeData()->hasFormat("application/StateMachineItem")) {
		event->acceptProposedAction();
	}
}

void StateMachineEditor::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	stateView_->setBPStatus(false, false);
	ElementStmParamPtr target = getCurrentNode();
	if (target) {
		if (target->getType() == ElementType::ELEMENT_COMMAND) {
			stateView_->setBPStatus(true, target->isBreak());
		}
	}
}

void StateMachineEditor::mouseDoubleClickEvent(QMouseEvent * event) {
	if (event->button() != Qt::LeftButton) return;

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
			stateView_->editClicked();
		}
	}
}

void StateMachineEditor::dropEvent(QDropEvent* event) {
	if (targetParam_ == 0) return;

	if (event->mimeData()->hasFormat("application/StateMachineItem") == false) return;
	DDEBUG("FlowActivityEditor::dropEvent");

	QString strDispName = event->mimeData()->text();
	DDEBUG_V("strDispName:%s", strDispName.toStdString().c_str());
	QString strName = "";
	QVariant varData = event->mimeData()->property("CommandId");
	int id = varData.toInt();
	CommandDefParam* cmdParam = TaskExecutor::instance()->getCommandDef(id);
	if (cmdParam) {
		strName = cmdParam->getName();
		DDEBUG("ActivityEditor::dropEvent cmdParam Exists");
	} else {
		DDEBUG("ActivityEditor::dropEvent cmdParam NOT Exists");
	}
	/////
	QString typeName = "Task";
	ElementType typeId = ELEMENT_COMMAND;
	if (strDispName == "Start") {
		typeName = "Initial";
		typeId = ELEMENT_START;
	} else if (strDispName == "Final") {
		typeName = "Final";
		typeId = ELEMENT_FINAL;
	} else if (strDispName == "Decision/Merge") {
		typeName = "Decision";
		typeId = ELEMENT_DECISION;
	}
	auto type = _scene->registry().create(typeName);

	if (type) {
		type->setTaskName(strDispName);
		auto& node = _scene->createNode(std::move(type));
		QPoint pos = event->pos();
		QPointF posView = this->mapToScene(pos);
		node.nodeGraphicsObject().setPos(posView);
		//
		int newId = targetParam_->getMaxStateId();
		ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newId, typeId, strName, strDispName, pos.x(), pos.y(), "");
		newParam->setCommadDefParam(cmdParam);
		newParam->setRealElem(&node);
		newParam->setNew();
		node.setParamId(newParam->getId());
		targetParam_->addStmElement(newParam);

	} else {
		qDebug() << "Model not found";
	}
}

void StateMachineEditor::createStateMachine(std::vector<ElementStmParamPtr>& elemList, std::vector<ConnectionStmParamPtr>& connList) {
	DDEBUG_V("StateMachineEditor::createStateMachine %d, %d", elemList.size(), connList.size());
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

		_scene->createConnection(*targetNode, 0, *sourceNode, target->getSourceIndex());
	}
}

void StateMachineEditor::updateTargetParam() {
	if (targetParam_ == 0) return;

	vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
	for (int index = 0; index < stateList.size(); index++) {
		ElementStmParamPtr target = stateList[index];
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

		ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(connId, sourceId, targetId, sourceIndex);
		connId++;
		connParam->setNew();
		targetParam_->addStmConnection(connParam);
	}
}

void StateMachineEditor::setBreakPoint(bool isBreak) {
	ElementStmParamPtr target = getCurrentNode();
	if (target) {
		target->setBreak(isBreak);
		if (target->getRealElem()) {
			target->getRealElem()->setBreak(isBreak);
		}
	}
}
