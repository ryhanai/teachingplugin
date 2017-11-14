#include "ActivityEditorBase.h"
#include "TaskExecutor.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;

namespace teaching {

ActivityEditorBase::ActivityEditorBase(QWidget* parent)
	: QGraphicsView(parent), targetNode_(0), targetConnection_(0),
		selectionMode_(false), modeCnt_(false), newStateNum(0) {
	scene_ = new QGraphicsScene(this);
	setScene(scene_);
	//
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setAcceptDrops(true);
}

void ActivityEditorBase::mouseMoveEvent(QMouseEvent* event) {
	DDEBUG("ActivityEditorBase::mouseMoveEvent");
	QPointF pos = mapToScene(event->pos());
	if (targetNode_) {
		if (modeCnt_) {
			DDEBUG("ActivityEditorBase::mouseMoveEvent CntMode");
			ConnectionNode* item = targetNode_->getCurrentConnection();
			if (item) {
				item->setLine(targetNode_->pos().x(), targetNode_->pos().y(), pos.x() + POS_DELTA, pos.y() + POS_DELTA);
			}

		} else {
			DDEBUG("ActivityEditorBase::mouseMoveEvent NormalMode");
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

void ActivityEditorBase::keyPressEvent(QKeyEvent* event) {
	DDEBUG("ActivityEditorBase::keyPressEvent");
	if (event->key() == Qt::Key_Delete) {
		deleteCurrent();
	}
	QWidget::keyPressEvent(event);
}

void ActivityEditorBase::wheelEvent(QWheelEvent* event) {
	DDEBUG("ActivityEditorBase::wheelEvent");
	double dSteps = (double)event->delta() / 120.0;
	double scaleVal = 1.0;
	scaleVal -= (dSteps / 20.0);
	scale(scaleVal, scaleVal);
}

void ActivityEditorBase::addChildConnection(ConnectionStmParamPtr target, ElementNode* sourceChild, ElementNode* targetChild) {
	DDEBUG("ActivityEditorBase::addChildConnection");
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

void ActivityEditorBase::deleteCurrent() {
	DDEBUG("ActivityEditorBase::deleteCurrent");
	if (targetConnection_) {
		deleteConnection(targetConnection_);
		targetConnection_ = 0;
	} else if (targetNode_ || 0 < selectedNode_.size()) {
		deleteElement();
		targetNode_ = 0;
		selectedNode_.clear();
	}
}

void ActivityEditorBase::deleteElement() {
	DDEBUG("ActivityEditorBase::deleteElement");
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
			ConnectionStmParamPtr parentConn = target->getElemParam()->getParentConn();
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

void ActivityEditorBase::deleteConnection(ConnectionNode* target) {
	if (target == 0) return;
	ConnectionStmParamPtr connParam = target->getConnParam();
	if (connParam == 0) return;

	DDEBUG("ActivityEditorBase::deleteConnection");
	connParam->setDelete();
	vector<ElementStmParamPtr> childNodeList = connParam->getChildList();

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

void ActivityEditorBase::addPoint() {
	if (targetConnection_ == 0) return;
	DDEBUG("ActivityEditorBase::addPoint");
	//
	ElementNode* node = new ElementNode(ELEMENT_POINT, "");
	scene_->addItem(node);

	ElementNode* source = targetConnection_->getSource();
	ElementNode* target = targetConnection_->getTarget();
	double posX = source->pos().x() + (target->pos().x() - source->pos().x()) / 2.0;
	double posY = source->pos().y() + (target->pos().y() - source->pos().y()) / 2.0;
	//DDEBUG_V("addPoint:targetX=%f, targetY=%f, sourcetX=%f, sourcetY=%f, X=%f, Y=%f",
	//	target->pos().x(), target->pos().y(), source->pos().x(), source->pos().y(), posX, posY);
	node->setPos(posX, posY);
	ElementStmParamPtr newParam = std::make_shared<ElementStmParam>(NULL_ID, node->getElementType(), "", "", posX, posY, "");
	newParam->setRealElem(node);
	newParam->setParentConn(targetConnection_->getConnParam());
	node->setElemParam(newParam);
	node->setData(Qt::UserRole, TYPE_ELEMENT);
	//
	targetConnection_->setTarget(node);
	target->removeTargetConnection(targetConnection_);
	target->removeFromGroup(targetConnection_);
	node->addConnection(targetConnection_);
	targetConnection_->reDrawConnection();

	targetConnection_->getConnParam()->addChildNode(source->getElemParam(), newParam);

	ConnectionNode* newConnection = new ConnectionNode(0, 0, 0, 0);
	newConnection->setConnParam(targetConnection_->getConnParam());

	newConnection->setSource(node);
	newConnection->setTarget(target);
	newConnection->setPen(QPen(Qt::black, LINE_WIDTH));
	newConnection->setData(Qt::UserRole, TYPE_CONNECTION);

	newConnection->setZValue(-10);
	target->addToGroup(newConnection);
	node->addConnection(newConnection);
	target->addConnection(newConnection);
	newConnection->reDrawConnection();
	node->removeFromGroup(newConnection);
}

void ActivityEditorBase::removePoint() {
	ElementStmParamPtr targetElem = targetNode_->getElemParam();
	if (targetElem == 0) return;

	ConnectionStmParamPtr parentConn = targetElem->getParentConn();
	if (parentConn == 0) return;
	DDEBUG("ActivityEditor::removePoint");
	//
	ElementNode* realElem = targetElem->getRealElem();
	vector<ConnectionNode*> connList = realElem->getConnectionList();
	ConnectionNode* inFlow = 0;
	ConnectionNode* outFlow = 0;
	for (unsigned int index = 0; index < connList.size(); index++) {
		ConnectionNode* conn = connList[index];
		if (conn->getSource() == realElem) {
			outFlow = conn;
		} else if (conn->getTarget() == realElem) {
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
	//
	parentConn->removeChildNode(targetElem);
	realElem->setVisible(false);
}

void ActivityEditorBase::createStateMachine(vector<ElementStmParamPtr>& elemList, vector<ConnectionStmParamPtr>& connList) {
	targetNode_ = 0;
	targetConnection_ = 0;
	removeAll();
	//
	for (int index = 0; index < elemList.size(); index++) {
		ElementStmParamPtr target = elemList[index];
		//if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;
		//DDEBUG_V("state : %s %d %f %f", target->getCmdDspName().toStdString().c_str(), target->getType(), target->getPosX(), target->getPosY());

		ElementNode* node = new ElementNode(target->getType(), target->getCmdDspName());
		scene_->addItem(node);
		node->setPos(target->getPosX(), target->getPosY());
		target->setRealElem(node);
		node->setElemParam(target);
		node->setData(Qt::UserRole, TYPE_ELEMENT);
		node->setBreak(target->isBreak());
	}
	//
	for (int index = 0; index < connList.size(); index++) {
		ConnectionStmParamPtr target = connList[index];
		//if (target->getMode() == DB_MODE_DELETE || target->getMode() == DB_MODE_IGNORE) continue;

		//DDEBUG_V("connection : Source=%d, Target=%d", target->getSourceId(), target->getTargetId());
		vector<ElementStmParamPtr>::iterator sourceElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
		if (sourceElem == elemList.end()) continue;
		vector<ElementStmParamPtr>::iterator targetElem = find_if(elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
		if (targetElem == elemList.end()) continue;
		//
		ElementNode* sourceNode = (*sourceElem)->getRealElem();
		ElementNode* targetNode = (*targetElem)->getRealElem();

		vector<ElementStmParamPtr> childNodeList = target->getChildList();
		int pointNum = 0;
		for (unsigned int idxChild = 0; idxChild < childNodeList.size(); idxChild++) {
			ElementStmParamPtr childParam = childNodeList[idxChild];
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
				ElementStmParamPtr childParam = childNodeList[idxChild];
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
					ElementStmParamPtr preElem = childNodeList[idxChild - 1];
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
//////////
ItemList::ItemList(QString elemType, QWidget* parent)
  : elemType_(elemType), QListWidget(parent) {
}
void ItemList::mousePressEvent(QMouseEvent* event) {
  DDEBUG("ItemList::mousePressEvent");
  if (event->button() == Qt::LeftButton) {
    startPos = event->pos();
  }
  QListWidget::mousePressEvent(event);
}

void ItemList::mouseMoveEvent(QMouseEvent* event) {
  DDEBUG("ItemList::mouseMoveEvent");
  if (event->buttons() == Qt::LeftButton) {
    int distance = (event->pos() - startPos).manhattanLength();
    if (QApplication::startDragDistance() <= distance) {
      QModelIndexList indexes = selectionModel()->selection().indexes();
      if (0 < indexes.size()) {
        QModelIndex selected = indexes.at(0);
        if (0 <= selected.row()) {
          QByteArray itemData;
          QMimeData* mimeData = new QMimeData;
          mimeData->setData(elemType_, itemData);
          mimeData->setText(item(selected.row())->text());
          mimeData->setProperty("CommandId", item(selected.row())->data(Qt::UserRole));
          QDrag* drag = new QDrag(this);
          drag->setMimeData(mimeData);
          drag->exec(Qt::CopyAction);
        }
      }
    }
  }
  QListWidget::mouseMoveEvent(event);
}

void ItemList::createInitialNodeTarget() {
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  painter->drawEllipse(5, 5, 20, 20);

  this->setIconSize(QSize(30, 30));
  QListWidgetItem* item = new QListWidgetItem("Start", this);
  item->setIcon(QIcon(*pix));
}

void ItemList::createFinalNodeTarget() {
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
  painter->setPen(QPen(Qt::black));
  painter->drawEllipse(5, 5, 20, 20);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  painter->drawEllipse(10, 10, 10, 10);

  this->setIconSize(QSize(30, 30));
  QListWidgetItem* item = new QListWidgetItem("Final", this);
  item->setIcon(QIcon(*pix));
}

void ItemList::createDecisionNodeTarget() {
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  QPolygon polygon;
  polygon << QPoint(15.0, 5.0) << QPoint(0.0, 15.0)
    << QPoint(15.0, 25.0) << QPoint(30.0, 15.0) << QPoint(15.0, 5.0);
  painter->drawPolygon(polygon);
  this->setIconSize(QSize(30, 30));
  QListWidgetItem* item = new QListWidgetItem("Decision/Merge", this);
  item->setIcon(QIcon(*pix));
}

void ItemList::createForkNodeTarget() {
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawLine(0, 15, 30, 15);
  this->setIconSize(QSize(30, 30));
  QListWidgetItem* item = new QListWidgetItem("Fork/Join", this);
  item->setIcon(QIcon(*pix));
}

}
