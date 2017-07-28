#include "ActivityEditor.h"
#include "TaskExecutor.h"
#include "StateMachineView.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;

namespace teaching {

ActivityEditor::ActivityEditor(StateMachineViewImpl* stateView, QWidget* parent)
  : stateView_(stateView), QGraphicsView(parent), selectionMode_(false),
  targetTask_(0), modeCnt_(false), newStateNum(0),
  targetNode_(0), targetConnection_(0) {
  scene_ = new QGraphicsSceneWithMenu(this);
  setScene(scene_);
  //
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setAcceptDrops(true);
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
  ElementStmParam* newParam = new ElementStmParam(newStateNum, node->getElementType(), strName, strDispName, position.x(), position.y(), "");
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

  if (event->button() == Qt::LeftButton) {
    scene_->setMode(MODE_NONE);
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
    QPointF pos = mapToScene(event->pos());
    QTransform trans;
    QGraphicsItem* gItem = scene_->itemAt(pos, trans);
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
            ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
            item->setSource(targetNode_);
            item->setPen(QPen(Qt::gray));
            item->setZValue(-10);
            targetNode_->addToGroup(item);
            targetNode_->addConnection(item);
          }
          if (targetNode_->getElementType() == ELEMENT_POINT) {
            scene_->setMode(MODE_POINT);
            scene_->setElement(targetNode_->getElemParam());
          }

        } else if (type == TYPE_CONNECTION) {
          ConnectionNode* targetConn = (ConnectionNode*)gItem->parentItem();
          targetConn->setPen(QPen(Qt::red, LINE_WIDTH));
          targetConnection_ = targetConn;
          scene_->setMode(MODE_LINE);
          scene_->setConnection(targetConnection_);
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

void ActivityEditor::mouseMoveEvent(QMouseEvent* event) {
  DDEBUG("ActivityEditor::mouseMoveEvent");
  QPointF pos = mapToScene(event->pos());
  if (targetNode_) {
    if (modeCnt_) {
      ConnectionNode* item = targetNode_->getCurrentConnection();
      if (item) {
        item->setLine(targetNode_->pos().x(), targetNode_->pos().y(), pos.x() + POS_DELTA, pos.y() + POS_DELTA);
      }

    } else {
      if (0 < selectedNode_.size()) {
        double deltaX = elemStartPnt_.x() - pos.x();
        double deltaY = elemStartPnt_.y() - pos.y();
        elemStartPnt_ = pos;
        for (unsigned int index = 0; index < selectedNode_.size(); index++) {
          ElementNode* sel = selectedNode_[index];
          double newX = sel->getElemParam()->getPosX() - deltaX;
          double newY = sel->getElemParam()->getPosY() - deltaY;
          sel->updatePosition(newX, newY);
        }

      } else {
        if (targetNode_->getElemParam()) {
          targetNode_->updatePosition(pos.x(), pos.y());
        }
      }
    }
  }
  //
  if (selectionMode_) {
    double width = fabs(selStartPnt_.x() - pos.x());
    double height = fabs(selStartPnt_.y() - pos.y());
    double startX = pos.x();
    double startY = pos.y();
    if (selStartPnt_.x() < pos.x()) startX = selStartPnt_.x();
    if (selStartPnt_.y() < pos.y()) startY = selStartPnt_.y();
    selRect_->setRect(startX, startY, width, height);
  }
  QWidget::mouseMoveEvent(event);
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
        ConnectionStmParam* newConn = new ConnectionStmParam(NULL_ID, item->getSource()->getElemParam()->getId(), item->getTarget()->getElemParam()->getId(), "");
        newConn->setNew();
        item->setConnParam(newConn);
        newConn->setSourceId(item->getSource()->getElemParam()->getId());
        newConn->setTargetId(item->getTarget()->getElemParam()->getId());
        targetTask_->addStmConnection(newConn);
        targetNode_->removeFromGroup(item);
        //
        DDEBUG("ActivityEditor::mouseReleaseEvent ConnectionStmParam Via");
        ConnectionStmParam* newParam = new ConnectionStmParam(NULL_ID, NULL_ID, NULL_ID, "");
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

    vector<ElementStmParam*> stateList = targetTask_->getStmElementList();
    for (unsigned int index = 0; index < stateList.size(); index++) {
      ElementStmParam* state = stateList[index];
      double posX = state->getPosX();
      double posY = state->getPosY();
      if (baseXm <= posX && posX <= baseXp && baseYm <= posY && posY <= baseYp) {
        state->getRealElem()->updateSelect(true);
        selectedNode_.push_back(state->getRealElem());
      }
    }
    vector<ConnectionStmParam*> connList = targetTask_->getStmConnectionList();
    for (unsigned int index = 0; index < connList.size(); index++) {
      ConnectionStmParam* conn = connList[index];
      vector<ElementStmParam*> childList = conn->getChildList();
      for (unsigned int idxChild = 0; idxChild < childList.size(); idxChild++) {
        ElementStmParam* state = childList[idxChild];
        double posX = state->getPosX();
        double posY = state->getPosY();
        if (baseXm <= posX && posX <= baseXp && baseYm <= posY && posY <= baseYp) {
          state->getRealElem()->updateSelect(true);
          selectedNode_.push_back(state->getRealElem());
        }
      }
    }
  }
  DDEBUG("ActivityEditor::mouseReleaseEvent End");
  QWidget::mouseReleaseEvent(event);
}

void ActivityEditor::wheelEvent(QWheelEvent* event) {
  DDEBUG("ActivityEditor::wheelEvent");
  double dSteps = (double)event->delta() / 120.0;
  double scaleVal = 1.0;
  scaleVal -= (dSteps / 20.0);
  scale(scaleVal, scaleVal);
}

void ActivityEditor::keyPressEvent(QKeyEvent* event) {
  DDEBUG("ActivityEditor::keyPressEvent");
  if (event->key() == Qt::Key_Delete) {
    deleteCurrent();
  }
  QWidget::keyPressEvent(event);
}

void ActivityEditor::mouseDoubleClickEvent(QMouseEvent * event) {
  DDEBUG("ActivityEditor::mouseDoubleClickEvent");

  if (event->button() != Qt::LeftButton) return;
  scene_->setMode(MODE_NONE);
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

void ActivityEditor::deleteCurrent() {
  DDEBUG("ActivityEditor::deleteCurrent");
  if (targetConnection_) {
    deleteConnection(targetConnection_);
    targetConnection_ = 0;
  } else if (targetNode_ || 0 < selectedNode_.size()) {
    deleteElement();
    targetNode_ = 0;
    selectedNode_.clear();
  }
}

void ActivityEditor::deleteConnection(ConnectionNode* target) {
  if (target == 0) return;
  ConnectionStmParam* connParam = target->getConnParam();
  if (connParam == 0) return;

  DDEBUG("ActivityEditor::deleteConnection");
  connParam->setDelete();
  vector<ElementStmParam*> childNodeList = connParam->getChildList();

  if (childNodeList.size() == 0) {
    target->getSource()->removeTargetConnection(target);
    target->getTarget()->removeTargetConnection(target);
    target->setVisible(false);

  } else {
    for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
      ElementNode* childNode = childNodeList[idxChild]->getRealElem();
      vector<ConnectionNode*> conns = childNode->getConnectionList();
      vector<ConnectionNode*>::iterator itConn = conns.begin();
      while (itConn != conns.end()) {
        (*itConn)->getConnParam()->setDelete();
        (*itConn)->getSource()->removeTargetConnection((*itConn));
        (*itConn)->getTarget()->removeTargetConnection((*itConn));
        (*itConn)->setVisible(false);
        ++itConn;
      }
      //
      childNode->getElemParam()->setDelete();
      childNode->setVisible(false);
    }
  }
}

void ActivityEditor::deleteElement() {
  DDEBUG("ActivityEditor::deleteElement");
  vector<ElementNode*> removeList;
  if (0 < selectedNode_.size()) {
    removeList = selectedNode_;
  } else {
    if (targetNode_) {
      removeList.push_back(targetNode_);
    }
  }
  //
  for (unsigned int index = 0; index < removeList.size(); index++) {
    ElementNode* target = removeList[index];
    vector<ConnectionNode*> connList = target->getConnectionList();
    if (target->getElementType() == ELEMENT_POINT) {
      if (target->getElemParam() == 0) return;
      ConnectionStmParam* parentConn = target->getElemParam()->getParentConn();
      if (parentConn == 0) return;

      ConnectionNode* inFlow = 0;
      ConnectionNode* outFlow = 0;
      for (unsigned int index = 0; index < connList.size(); index++) {
        ConnectionNode* conn = connList[index];
        if (conn->getSource() == target) {
          outFlow = conn;
        } else if (conn->getTarget() == target) {
          inFlow = conn;
        }
      }
      if (inFlow == 0 || outFlow == 0) return;
      //
      ElementNode* newTarget = outFlow->getTarget();
      inFlow->setTarget(newTarget);
      newTarget->addToGroup(inFlow);
      newTarget->addConnection(inFlow);
      inFlow->reDrawConnection();
      outFlow->setVisible(false);
      parentConn->removeChildNode(target->getElemParam());

    } else {
      vector<ConnectionNode*>::iterator itConn = connList.begin();
      while (itConn != connList.end()) {
        deleteConnection((*itConn));
        ++itConn;
      }
      target->getElemParam()->setDelete();
    }
    target->setVisible(false);
  }
}

void ActivityEditor::removeAll() {
  DDEBUG("ActivityEditor::removeAll");
  scene_->clear();
}

void ActivityEditor::createStateMachine(TaskModelParam* param) {
  DDEBUG("ActivityEditor::createStateMachine");
  //DDEBUG_V("createStateMachine : %d", param->getStmConnectionList().size());
  targetNode_ = 0;
  targetConnection_ = 0;
  scene_->clear();
  //
  vector<ElementStmParam*> elemList = param->getStmElementList();
  for (int index = 0; index < elemList.size(); index++) {
    ElementStmParam* target = elemList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;
    //DDEBUG_V("state : id=%d", target->getId());

    QString strName = target->getCmdDspName();
    ElementNode* node = new ElementNode(target->getType(), strName);
    scene_->addItem(node);
    node->setPos(target->getPosX(), target->getPosY());
    target->setRealElem(node);
    node->setElemParam(target);
    node->setData(Qt::UserRole, TYPE_ELEMENT);
    node->setBreak(target->isBreak());
  }
  //
  vector<ConnectionStmParam*> connList = param->getStmConnectionList();
  for (int index = 0; index < connList.size(); index++) {
    ConnectionStmParam* target = connList[index];
    if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

    //DDEBUG_V("connection : Source=%d, Target=%d", target->getSourceId(), target->getTargetId());
    vector<ElementStmParam*>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
    if (sourceElem == elemList.end()) continue;
    vector<ElementStmParam*>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
    if (targetElem == elemList.end()) continue;
    //
    ElementNode* sourceNode = (*sourceElem)->getRealElem();
    ElementNode* targetNode = (*targetElem)->getRealElem();

    vector<ElementStmParam*> childNodeList = target->getChildList();
    int pointNum = 0;
    for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
      ElementStmParam* childParam = childNodeList[idxChild];
      if (childParam->getMode() == DB_MODE_DELETE || childParam->getMode() == DB_MODE_IGNORE) continue;
      pointNum++;
    }
    if (pointNum == 0) {
      ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
      item->setSource(sourceNode);
      item->setTarget(targetNode);
      item->setPen(QPen(Qt::black, LINE_WIDTH));
      item->setData(Qt::UserRole, TYPE_CONNECTION);
      item->setZValue(-10);
      targetNode->addToGroup(item);
      sourceNode->addConnection(item);
      targetNode->addConnection(item);
      item->reDrawConnection();
      item->setConnParam(target);
      sourceNode->removeFromGroup(item);
      if (item->getSource()->getElementType() == ELEMENT_DECISION) {
        item->setText(target->getCondition());
      }
    } else {
      ElementNode* sourceChild;
      ElementNode* targetChild;
      for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
        ElementStmParam* childParam = childNodeList[idxChild];
        if (childParam->getMode() == DB_MODE_DELETE || childParam->getMode() == DB_MODE_IGNORE) continue;

        ElementNode* child = new ElementNode(ELEMENT_POINT, "");
        scene_->addItem(child);
        child->setPos(childParam->getPosX(), childParam->getPosY());
        childParam->setRealElem(child);
        child->setElemParam(childParam);
        child->setData(Qt::UserRole, TYPE_ELEMENT);

        if (idxChild == 0) {
          sourceChild = sourceNode;
          targetChild = child;
        } else {
          ElementStmParam* preElem = childNodeList[idxChild - 1];
          sourceChild = preElem->getRealElem();
          targetChild = child;
        }
        addChildConnection(target, sourceChild, targetChild);
      }
      sourceChild = childNodeList[childNodeList.size() - 1]->getRealElem();
      targetChild = targetNode;
      addChildConnection(target, sourceChild, targetChild);
    }
  }
}
void ActivityEditor::addChildConnection(ConnectionStmParam* target, ElementNode* sourceChild, ElementNode* targetChild) {
  DDEBUG("ActivityEditor::addChildConnection");
  ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
  item->setConnParam(target);
  //
  item->setSource(sourceChild);
  item->setTarget(targetChild);
  item->setPen(QPen(Qt::black, LINE_WIDTH));
  item->setData(Qt::UserRole, TYPE_CONNECTION);

  item->setZValue(-10);
  targetChild->addToGroup(item);
  sourceChild->addConnection(item);
  targetChild->addConnection(item);
  item->reDrawConnection();
  sourceChild->removeFromGroup(item);
  if (item->getSource()->getElementType() == ELEMENT_DECISION) {
    item->setText(target->getCondition());
  }
}

}
