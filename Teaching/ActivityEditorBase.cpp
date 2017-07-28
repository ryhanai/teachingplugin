#include "ActivityEditorBase.h"
#include "TaskExecutor.h"
#include "StateMachineView.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;

namespace teaching {

QGraphicsSceneWithMenu::QGraphicsSceneWithMenu(QObject *parent)
  : QGraphicsScene(parent), mode_(MODE_NONE), targetConnection_(0) {
}

void QGraphicsSceneWithMenu::contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent) {
  ContextMenu_.clear();

  if (mode_ == MODE_NONE) return;

  if (mode_ == MODE_LINE) {
    QAction *addAction = ContextMenu_.addAction(_("Add Point"));
    connect(addAction, SIGNAL(triggered()), this, SLOT(addPoint()));

  } else if (mode_ == MODE_POINT) {
    QAction *removeAction = ContextMenu_.addAction(_("Remove Point"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removePoint()));
  }

  ContextMenu_.exec(contextMenuEvent->screenPos());
}

void QGraphicsSceneWithMenu::addPoint() {
  if (targetConnection_ == 0) return;
  DDEBUG("QGraphicsSceneWithMenu::addPoint");
  //
  ElementNode* node = new ElementNode(ELEMENT_POINT, "");
  addItem(node);

  ElementNode* source = targetConnection_->getSource();
  ElementNode* target = targetConnection_->getTarget();
  double posX = source->pos().x() + (target->pos().x() - source->pos().x()) / 2.0;
  double posY = source->pos().y() + (target->pos().y() - source->pos().y()) / 2.0;
  //DDEBUG_V("addPoint:targetX=%f, targetY=%f, sourcetX=%f, sourcetY=%f, X=%f, Y=%f",
  //	target->pos().x(), target->pos().y(), source->pos().x(), source->pos().y(), posX, posY);
  node->setPos(posX, posY);
  ElementStmParam* newParam = new ElementStmParam(NULL_ID, node->getElementType(), "", "", posX, posY, "");
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

void QGraphicsSceneWithMenu::removePoint() {
  if (targetElem_ == 0) return;
  ConnectionStmParam* parentConn = targetElem_->getParentConn();
  if (parentConn == 0) return;
  DDEBUG("QGraphicsSceneWithMenu::removePoint");
  //
  ElementNode* realElem = targetElem_->getRealElem();
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
  parentConn->removeChildNode(targetElem_);
  realElem->setVisible(false);
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
