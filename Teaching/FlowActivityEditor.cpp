#include "FlowActivityEditor.h"
#include "TaskExecutor.h"
#include "FlowView.h"
#include "TeachingDataHolder.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;

namespace teaching {

FlowActivityEditor::FlowActivityEditor(FlowViewImpl* flowView, QWidget* parent)
	: flowView_(flowView), targetFlow_(0), ActivityEditorBase(parent) {
}

void FlowActivityEditor::dragEnterEvent(QDragEnterEvent* event) {
  DDEBUG("FlowActivityEditor::dragEnterEvent");
  if (event->mimeData()->hasFormat("application/TaskInstanceItem")) {
    event->acceptProposedAction();
  }
}

void FlowActivityEditor::dragMoveEvent(QDragMoveEvent *event) {
  DDEBUG("FlowActivityEditor::dragMoveEvent");
  if (event->mimeData()->hasFormat("application/TaskInstanceItem")) {
    event->acceptProposedAction();
  }
}

void FlowActivityEditor::dropEvent(QDropEvent* event) {
  if (targetFlow_ == 0) return;
  if (event->mimeData()->hasFormat("application/TaskInstanceItem") == false) return;
  DDEBUG("FlowActivityEditor::dropEvent");

  QString strDispName = event->mimeData()->text();
  QString strName = "";
  QVariant varData = event->mimeData()->property("TaskInstanceItemId");

  TaskModelParamPtr param = 0;
  ElementNode* node = new ElementNode(strDispName);
  DDEBUG_V("strDispName:%s", strDispName.toStdString().c_str());
  if (node->getElementType() == ELEMENT_COMMAND) {
    int id = varData.toInt();
    DDEBUG_V("id:%d", id);
		param = TeachingDataHolder::instance()->getTaskInstanceById(id);
	}
  //
  scene_->addItem(node);
  QPointF position = mapToScene(event->pos());
  node->setPos(position.x(), position.y());
  newStateNum--;
	ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(newStateNum, node->getElementType(), strName, strDispName, position.x(), position.y(), "");
  newParam->setOrgId(newStateNum);
  if (param) {
		TaskModelParamPtr newTaskParam( new TaskModelParam(param.get()));
    newTaskParam->setNewForce();
    newParam->setTaskParam(newTaskParam);
    newTaskParam->setStateParam(newParam);
  }
  newParam->setRealElem(node);
  newParam->setNew();
  node->setElemParam(newParam);
  targetFlow_->addStmElement(newParam);
  node->setData(Qt::UserRole, TYPE_ELEMENT);

  targetNode_ = 0;
  if (targetConnection_) {
    targetConnection_->setPen(QPen(Qt::black, LINE_WIDTH));
    targetConnection_ = 0;
  }
}

void FlowActivityEditor::mousePressEvent(QMouseEvent* event) {
	DDEBUG("FlowActivityEditor::mousePressEvent");

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
		//
		if (gItem) {
			QGraphicsItem* parentItem = gItem->parentItem();
			if (parentItem) {
				int type = parentItem->data(Qt::UserRole).toInt();
				if (type == TYPE_ELEMENT) {
					elemStartPnt_ = pos;
					targetNode_ = (ElementNode*)gItem->parentItem();
					std::vector<ElementNode*>::iterator elem = std::find(selectedNode_.begin(), selectedNode_.end(), targetNode_);
					if (elem == selectedNode_.end()) {
						for (unsigned int index = 0; index < selectedNode_.size(); index++) {
							selectedNode_[index]->updateSelect(false);
						}
						selectedNode_.clear();
						targetNode_->updateSelect(true);
						flowView_->flowSelectionChanged(targetNode_->getElemParam()->getTaskParam());
					}

					if (modeCnt_) {
						ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
						item->setSource(targetNode_);
						item->setPen(QPen(Qt::gray));
						item->setZValue(-10);
						targetNode_->addToGroup(item);
						targetNode_->addConnection(item);
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

void FlowActivityEditor::mouseReleaseEvent(QMouseEvent* event) {
  if (targetFlow_ == 0) return;
  if (modeCnt_ && targetNode_) {
    QPointF pos = mapToScene(event->pos());
    QTransform trans;
    QGraphicsItem* gItem = scene_->itemAt(pos, trans);
    if (gItem) {
      ConnectionNode* item = targetNode_->getCurrentConnection();
      ElementNode* currNode = (ElementNode*)gItem->parentItem();
      if (currNode && item) {
        if (item->getSource()->getElemParam()->getId() == currNode->getElemParam()->getId()) {
          ConnectionNode* item = targetNode_->getCurrentConnection();
          item->setPen(QPen(Qt::white));
          item->setVisible(false);
          targetNode_->removeCurrentConnection();
          QWidget::mouseReleaseEvent(event);
          return;
        }
        if (currNode->getElementType() < ELEMENT_START || ELEMENT_POINT < currNode->getElementType()) {
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
				ConnectionStmParamPtr newConn = std::make_shared<ConnectionStmParam>(NULL_ID, item->getSource()->getElemParam()->getId(), item->getTarget()->getElemParam()->getId(), "");
        newConn->setNew();
        item->setConnParam(newConn);
        newConn->setSourceId(item->getSource()->getElemParam()->getId());
        newConn->setTargetId(item->getTarget()->getElemParam()->getId());
        targetFlow_->addStmConnection(newConn);
        targetNode_->removeFromGroup(item);
        //
				ConnectionStmParamPtr newParam = std::make_shared<ConnectionStmParam>(NULL_ID, NULL_ID, NULL_ID, "");
        newParam->setNew();
        targetFlow_->getStmConnectionList().push_back(newParam);

        if (item->getSource()->getElementType() == ELEMENT_DECISION) {
          item->setText("false");
        }
      }

    } else {
      ConnectionNode* item = targetNode_->getCurrentConnection();
      item->setPen(QPen(Qt::white));
      targetNode_->removeCurrentConnection();
    }
  }
  //
  if (selectionMode_) {
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

    vector<ElementStmParamPtr> stateList = targetFlow_->getStmElementList();
    for (unsigned int index = 0; index < stateList.size(); index++) {
			ElementStmParamPtr state = stateList[index];
      double posX = state->getPosX();
      double posY = state->getPosY();
      if (baseXm <= posX && posX <= baseXp && baseYm <= posY && posY <= baseYp) {
        state->getRealElem()->updateSelect(true);
        selectedNode_.push_back(state->getRealElem());
      }
    }
    vector<ConnectionStmParamPtr> connList = targetFlow_->getStmConnectionList();
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
  QWidget::mouseReleaseEvent(event);
}

void FlowActivityEditor::mouseDoubleClickEvent(QMouseEvent * event) {
  DDEBUG("FlowActivityEditor::mouseDoubleClickEvent");

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
          flowView_->editClicked();
        }
      }
    }
  }
}

void FlowActivityEditor::removeAll() {
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

//void FlowActivityEditor::createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList) {
//  targetNode_ = 0;
//  targetConnection_ = 0;
//	removeAll();
//	//
//  //vector<ElementStmParamPtr> elemList = param->getStmElementList();
//  for (int index = 0; index < elemList.size(); index++) {
//		ElementStmParamPtr target = elemList[index];
//    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;
//    //DDEBUG_V("state : %s %d %f %f", target->getCmdDspName().toStdString().c_str(), target->getType(), target->getPosX(), target->getPosY());
//
//    ElementNode* node = new ElementNode(target->getType(), target->getCmdDspName());
//    scene_->addItem(node);
//    node->setPos(target->getPosX(), target->getPosY());
//    target->setRealElem(node);
//    node->setElemParam(target);
//    node->setData(Qt::UserRole, TYPE_ELEMENT);
//  }
//  //
//  //vector<ConnectionStmParamPtr> connList = param->getStmConnectionList();
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
