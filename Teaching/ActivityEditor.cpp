#include "ActivityEditor.h"
#include "TaskExecutor.h"
#include "StateMachineView.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;

namespace teaching {

ActivityEditor::ActivityEditor(StateMachineViewImpl* stateView, QWidget* parent)
	: stateView_(stateView), targetTask_(0), ActivityEditorBase(parent) {
}

void ActivityEditor::dragEnterEvent(QDragEnterEvent* event) {
  DDEBUG("ActivityEditor::dragEnterEvent");
  if (event->mimeData()->hasFormat("application/StateMachineItem")) {
    event->acceptProposedAction();
  }
}

void ActivityEditor::dragMoveEvent(QDragMoveEvent *event) {
  DDEBUG("ActivityEditor::dragMoveEvent");
  if (event->mimeData()->hasFormat("application/StateMachineItem")) {
    event->acceptProposedAction();
  }
}

void ActivityEditor::dropEvent(QDropEvent* event) {
  if (targetTask_ == 0) return;
  if (event->mimeData()->hasFormat("application/StateMachineItem") == false) return;
  DDEBUG("ActivityEditor::dropEvent");

  QString strDispName = event->mimeData()->text();
  QString strName = "";
  QVariant varData = event->mimeData()->property("CommandId");
  int id = varData.toInt();
  DDEBUG_V("ActivityEditor::dropEvent id=%d, strDispName=%s", id, strDispName.toStdString().c_str());
  CommandDefParam* cmdParam = TaskExecutor::instance()->getCommandDef(id);
  if (cmdParam) {
    strName = cmdParam->getName();
    DDEBUG("ActivityEditor::dropEvent cmdParam Exists");
  } else {
    DDEBUG("ActivityEditor::dropEvent cmdParam NOT Exists");
  }
  //
  ElementNode* node = new ElementNode(strDispName);
  scene_->addItem(node);
  QPointF position = mapToScene(event->pos());
  node->setPos(position.x(), position.y());
  newStateNum--;
  DDEBUG_V("ActivityEditor::dropEvent newStateNum=%d, ElementType=%d", newStateNum, node->getElementType());
	ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newStateNum, node->getElementType(), strName, strDispName, position.x(), position.y(), "");
  newParam->setOrgId(newStateNum);
  newParam->setCommadDefParam(cmdParam);
  newParam->setRealElem(node);
  newParam->setNew();
  node->setElemParam(newParam);
  targetTask_->addStmElement(newParam);
  node->setData(Qt::UserRole, TYPE_ELEMENT);

  targetNode_ = 0;
  if (targetConnection_) {
    targetConnection_->setPen(QPen(Qt::black, LINE_WIDTH));
    targetConnection_ = 0;
  }
  DDEBUG("ActivityEditor::dropEvent End");
}

void ActivityEditor::mousePressEvent(QMouseEvent* event) {
  DDEBUG("ActivityEditor::mousePressEvent");

	QPointF pos = mapToScene(event->pos());
	QTransform trans;
	QGraphicsItem* gItem = scene_->itemAt(pos, trans);

	if (event->button() == Qt::LeftButton) {
		selectionMode_ = false;

		if (targetNode_) {
			targetNode_->updateSelect(false);
		}
		if (targetConnection_) {
			targetConnection_->setPen(QPen(Qt::black, LINE_WIDTH));
			targetConnection_ = 0;
		}
		targetNode_ = 0;
		stateView_->setBPStatus(false, false);
		//
		if (gItem) {
			QGraphicsItem* parentItem = gItem->parentItem();
			if (parentItem) {
				int type = parentItem->data(Qt::UserRole).toInt();
				DDEBUG_V("ActivityEditor::mousePressEvent type %d", type);
				if (type == TYPE_ELEMENT) {
					elemStartPnt_ = pos;
					targetNode_ = (ElementNode*)gItem->parentItem();
					DDEBUG_V("ActivityEditor::mousePressEvent targetNode_ %d", targetNode_->getElementType());

					std::vector<ElementNode*>::iterator elem = std::find(selectedNode_.begin(), selectedNode_.end(), targetNode_);
					if (elem == selectedNode_.end()) {
						for (unsigned int index = 0; index < selectedNode_.size(); index++) {
							selectedNode_[index]->updateSelect(false);
						}
						selectedNode_.clear();
						targetNode_->updateSelect(true);
					}
					if (targetNode_->getElementType() == ElementType::ELEMENT_COMMAND) {
						stateView_->setBPStatus(true, targetNode_->isBreak());
					}

					if (modeCnt_) {
						DDEBUG("ActivityEditor::mousePressEvent CntMode");
						ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
						item->setSource(targetNode_);
						item->setPen(QPen(Qt::gray));
						item->setZValue(-10);
						targetNode_->addToGroup(item);
						targetNode_->addConnection(item);
					} else {
						DDEBUG("ActivityEditor::mousePressEvent NormalMode");
					}

				} else if (type == TYPE_CONNECTION) {
					ConnectionNode* targetConn = (ConnectionNode*)gItem->parentItem();
					targetConn->setPen(QPen(Qt::red, LINE_WIDTH));
					targetConnection_ = targetConn;
				}
			}
		} else {
			for (unsigned int index = 0; index < selectedNode_.size(); index++) {
				selectedNode_[index]->updateSelect(false);
			}
			selectedNode_.clear();
			//
			selectionMode_ = true;
			selStartPnt_ = pos;
			selRect_ = scene_->addRect(0, 0, 0, 0);
			selRect_->setPen(QPen(QColor(0, 255, 0, 128)));
			selRect_->setBrush(QBrush(QColor(0, 255, 0, 128), Qt::SolidPattern));
		}

	} else if (event->button() == Qt::RightButton) {
		if (gItem == 0) return;
		QGraphicsItem* parentItem = gItem->parentItem();
		if (parentItem == 0) return;
		int type = parentItem->data(Qt::UserRole).toInt();
		if (type == TYPE_ELEMENT) {
			if (targetNode_ == 0) return;
			if (targetNode_->getElementType() == ELEMENT_POINT) {
				menuManager.setNewPopupMenu(this);
				menuManager.addItem(_("Remove Point"))
					->sigTriggered().connect(std::bind(&ActivityEditorBase::removePoint, this));
				menuManager.popupMenu()->popup(event->globalPos());
			}

		} else if (type == TYPE_CONNECTION) {
			if (targetConnection_ == 0) return;
			menuManager.setNewPopupMenu(this);
			menuManager.addItem(_("Add Point"))
				->sigTriggered().connect(std::bind(&ActivityEditorBase::addPoint, this));
			menuManager.popupMenu()->popup(event->globalPos());
		}

	}
	QWidget::mousePressEvent(event);
}

void ActivityEditor::setBreakPoint(bool isBreak) {
  DDEBUG("ActivityEditor::setBreakPoint");
  if (targetNode_) {
    targetNode_->setBreak(isBreak);
    if (targetNode_->getElemParam()) {
      targetNode_->getElemParam()->setBreak(isBreak);
    }
  }
}

void ActivityEditor::mouseReleaseEvent(QMouseEvent* event) {
  if (targetTask_ == 0) return;
  DDEBUG("ActivityEditor::mouseReleaseEvent");
  if (modeCnt_ && targetNode_) {
    QPointF pos = mapToScene(event->pos());
    QTransform trans;
    QGraphicsItem* gItem = scene_->itemAt(pos, trans);
    if (gItem) {
      DDEBUG("ActivityEditor::mouseReleaseEvent Item Exists");
      ConnectionNode* item = targetNode_->getCurrentConnection();
      ElementNode* currNode = (ElementNode*)gItem->parentItem();
      if (currNode && item) {
        DDEBUG_V("ActivityEditor::mouseReleaseEvent currNode %d", currNode->getElementType());
        if (item->getSource()->getElemParam()->getId() == currNode->getElemParam()->getId()) {
          DDEBUG("ActivityEditor::mouseReleaseEvent Source and Target are SAME");
          ConnectionNode* item = targetNode_->getCurrentConnection();
          item->setPen(QPen(Qt::white));
          item->setVisible(false);
          targetNode_->removeCurrentConnection();
          QWidget::mouseReleaseEvent(event);
          return;
        }
        if (currNode->getElementType() < ELEMENT_START || ELEMENT_POINT < currNode->getElementType()) {
          DDEBUG("ActivityEditor::mouseReleaseEvent ElementType INVALID");
          ConnectionNode* item = targetNode_->getCurrentConnection();
          item->setPen(QPen(Qt::white));
          item->setVisible(false);
          targetNode_->removeCurrentConnection();
          QWidget::mouseReleaseEvent(event);
          return;
        }

        item->setTarget(currNode);
        item->setPen(QPen(Qt::black, LINE_WIDTH));
        item->setData(Qt::UserRole, TYPE_CONNECTION);
        item->reDrawConnection();
        currNode->addConnection(item);
        DDEBUG("ActivityEditor::mouseReleaseEvent ConnectionStmParam");
				ConnectionStmParamPtr newConn = std::make_shared<ConnectionStmParam>(NULL_ID, item->getSource()->getElemParam()->getId(), item->getTarget()->getElemParam()->getId(), "");
        newConn->setNew();
        item->setConnParam(newConn);
        newConn->setSourceId(item->getSource()->getElemParam()->getId());
        newConn->setTargetId(item->getTarget()->getElemParam()->getId());
        targetTask_->addStmConnection(newConn);
        targetNode_->removeFromGroup(item);
        //
        DDEBUG("ActivityEditor::mouseReleaseEvent ConnectionStmParam Via");
				ConnectionStmParamPtr newParam = std::make_shared<ConnectionStmParam>(NULL_ID, NULL_ID, NULL_ID, "");
        newParam->setNew();
        targetTask_->getStmConnectionList().push_back(newParam);

        if (item->getSource()->getElementType() == ELEMENT_DECISION) {
          item->setText("false");
        }
      }

    } else {
      DDEBUG("ActivityEditor::mouseReleaseEvent Item NOT Exists");
      ConnectionNode* item = targetNode_->getCurrentConnection();
      item->setPen(QPen(Qt::white));
      targetNode_->removeCurrentConnection();
    }
  }
  DDEBUG("ActivityEditor::mouseReleaseEvent Edit End");
  //
  if (selectionMode_) {
    DDEBUG("ActivityEditor::mouseReleaseEvent selectionMode");
    selectionMode_ = false;
    scene_->removeItem(selRect_);
    //
    QPointF pos = mapToScene(event->pos());
    double baseXm = pos.x();
    double baseXp = selStartPnt_.x();
    double baseYm = pos.y();
    double baseYp = selStartPnt_.y();
    if (selStartPnt_.x() < pos.x()) {
      baseXm = selStartPnt_.x();
      baseXp = pos.x();
    }
    if (selStartPnt_.y() < pos.y()) {
      baseYm = selStartPnt_.y();
      baseYp = pos.y();
    }

    vector<ElementStmParamPtr> stateList = targetTask_->getStmElementList();
    for (unsigned int index = 0; index < stateList.size(); index++) {
			ElementStmParamPtr state = stateList[index];
      double posX = state->getPosX();
      double posY = state->getPosY();
      if (baseXm <= posX && posX <= baseXp && baseYm <= posY && posY <= baseYp) {
        state->getRealElem()->updateSelect(true);
        selectedNode_.push_back(state->getRealElem());
      }
    }
    vector<ConnectionStmParamPtr> connList = targetTask_->getStmConnectionList();
    for (unsigned int index = 0; index < connList.size(); index++) {
			ConnectionStmParamPtr conn = connList[index];
      vector<ElementStmParamPtr> childList = conn->getChildList();
      for (unsigned int idxChild = 0; idxChild < childList.size(); idxChild++) {
				ElementStmParamPtr state = childList[idxChild];
        double posX = state->getPosX();
        double posY = state->getPosY();
        if (baseXm <= posX && posX <= baseXp && baseYm <= posY && posY <= baseYp) {
          state->getRealElem()->updateSelect(true);
          selectedNode_.push_back(state->getRealElem());
        }
      }
    }
  }
	setDragMode(DragMode::NoDrag);
	setCursor(Qt::ArrowCursor);
	DDEBUG("ActivityEditor::mouseReleaseEvent End");
  QWidget::mouseReleaseEvent(event);
}

void ActivityEditor::mouseDoubleClickEvent(QMouseEvent * event) {
  DDEBUG("ActivityEditor::mouseDoubleClickEvent");

  if (event->button() != Qt::LeftButton) return;
  selectionMode_ = false;

  QPointF pos = mapToScene(event->pos());
  QTransform trans;
  QGraphicsItem* gItem = scene_->itemAt(pos, trans);
  if (gItem) {
    QGraphicsItem* parentItem = gItem->parentItem();
    if (parentItem) {
      int type = parentItem->data(Qt::UserRole).toInt();
      if (type == TYPE_ELEMENT) {
        targetNode_ = (ElementNode*)gItem->parentItem();
        if (targetNode_) {
          stateView_->editClicked();
        }
      }
    }
  }
}

void ActivityEditor::removeAll() {
  DDEBUG("ActivityEditor::removeAll");
	QList<QGraphicsItem*> itemsList = scene_->items();
	QList<QGraphicsItem*>::iterator iter = itemsList.begin();
	QList<QGraphicsItem*>::iterator end = itemsList.end();
	while (iter != end) {
		QGraphicsItem* item = (*iter);
		scene_->removeItem(item);
		iter++;
	}
	//scene_->clear();
}

//void ActivityEditor::createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList) {
//  DDEBUG("ActivityEditor::createStateMachine");
//  //DDEBUG_V("createStateMachine : %d", param->getStmConnectionList().size());
//  targetNode_ = 0;
//	targetConnection_ = 0;
//
//	removeAll();
//	//
//	for (int index = 0; index < elemList.size(); index++) {
//		ElementStmParamPtr target = elemList[index];
//		if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;
//    //DDEBUG_V("state : id=%d", target->getId());
//
//    ElementNode* node = new ElementNode(target->getType(), target->getCmdDspName());
//    scene_->addItem(node);
//    node->setPos(target->getPosX(), target->getPosY());
//    target->setRealElem(node);
//    node->setElemParam(target);
//    node->setData(Qt::UserRole, TYPE_ELEMENT);
//    node->setBreak(target->isBreak());
//  }
//  //
//  for (int index = 0; index < connList.size(); index++) {
//		ConnectionStmParamPtr target = connList[index];
//    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;
//
//    //DDEBUG_V("connection : Source=%d, Target=%d", target->getSourceId(), target->getTargetId());
//    vector<ElementStmParamPtr>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
//    if (sourceElem == elemList.end()) continue;
//    vector<ElementStmParamPtr>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
//    if (targetElem == elemList.end()) continue;
//    //
//    ElementNode* sourceNode = (*sourceElem)->getRealElem();
//    ElementNode* targetNode = (*targetElem)->getRealElem();
//
//    vector<ElementStmParamPtr> childNodeList = target->getChildList();
//    int pointNum = 0;
//    for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
//			ElementStmParamPtr childParam = childNodeList[idxChild];
//      if (childParam->getMode() == DB_MODE_DELETE || childParam->getMode() == DB_MODE_IGNORE) continue;
//      pointNum++;
//    }
//    if (pointNum == 0) {
//      ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
//      item->setSource(sourceNode);
//      item->setTarget(targetNode);
//      item->setPen(QPen(Qt::black, LINE_WIDTH));
//      item->setData(Qt::UserRole, TYPE_CONNECTION);
//      item->setZValue(-10);
//      targetNode->addToGroup(item);
//      sourceNode->addConnection(item);
//      targetNode->addConnection(item);
//      item->reDrawConnection();
//      item->setConnParam(target);
//      sourceNode->removeFromGroup(item);
//      if (item->getSource()->getElementType() == ELEMENT_DECISION) {
//        item->setText(target->getCondition());
//      }
//    } else {
//      ElementNode* sourceChild;
//      ElementNode* targetChild;
//      for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
//				ElementStmParamPtr childParam = childNodeList[idxChild];
//        if (childParam->getMode() == DB_MODE_DELETE || childParam->getMode() == DB_MODE_IGNORE) continue;
//
//        ElementNode* child = new ElementNode(ELEMENT_POINT, "");
//        scene_->addItem(child);
//        child->setPos(childParam->getPosX(), childParam->getPosY());
//        childParam->setRealElem(child);
//        child->setElemParam(childParam);
//        child->setData(Qt::UserRole, TYPE_ELEMENT);
//
//        if (idxChild == 0) {
//          sourceChild = sourceNode;
//          targetChild = child;
//        } else {
//					ElementStmParamPtr preElem = childNodeList[idxChild - 1];
//          sourceChild = preElem->getRealElem();
//          targetChild = child;
//        }
//        addChildConnection(target, sourceChild, targetChild);
//      }
//      sourceChild = childNodeList[childNodeList.size() - 1]->getRealElem();
//      targetChild = targetNode;
//      addChildConnection(target, sourceChild, targetChild);
//    }
//  }
//}

}
