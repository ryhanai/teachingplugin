#include "StateMachineView.h"
#include "TeachingTypes.h"

#include "LoggerUtil.h"

namespace teaching {

ConnectionNode::ConnectionNode(double sourceX, double sourceY, double targetX, double targetY) : condition("") {
  body = new QGraphicsLineItem(sourceX, sourceY, targetX, targetY);
  this->addToGroup(body);
  lineRight = new QGraphicsLineItem(sourceX, sourceY, targetX, targetY);
  this->addToGroup(lineRight);
  lineLeft = new QGraphicsLineItem(sourceX, sourceY, targetX, targetY);
  this->addToGroup(lineLeft);
  text = new QGraphicsSimpleTextItem("");
  text->setPos(sourceX, sourceY);
  QFont font( "Arial", 12, QFont::Bold);
  text->setFont(font);
  addToGroup(text);
}

void ConnectionNode::setLine(double sourceX, double sourceY, double targetX, double targetY) {
  body->setLine( sourceX, sourceY, targetX, targetY);
}

void ConnectionNode::setPen(QPen target) {
  body->setPen(target);
  lineRight->setPen(target);
  lineLeft->setPen(target);
}

void ConnectionNode::setText(QString target) {
  text->setText("[ " + target + " ]");
  condition = target;
}

void ConnectionNode::reDrawConnection() {
  double sourceX = this->source->pos().x();
  double sourceY = this->source->pos().y();
  double targetX = this->target->pos().x();
  double targetY = this->target->pos().y();

  body->setLine( sourceX, sourceY, targetX, targetY);
  //
  double deltaX = targetX - sourceX;
  double deltaY = targetY - sourceY;
  double length = sqrtf(deltaX*deltaX + deltaY*deltaY);
  double Ux = deltaX/length;
  double Uy = deltaY/length;

  double rightX = targetX - Uy*ARRAW_WIDTH - Ux*ARRAW_HEIGHT; 
  double rightY = targetY + Ux*ARRAW_WIDTH - Uy*ARRAW_HEIGHT; 
  lineRight->setLine(targetX-Ux*5.0, targetY-Uy*5.0, rightX, rightY);

  double leftX = targetX + Uy*ARRAW_WIDTH - Ux*ARRAW_HEIGHT;
  double leftY = targetY - Ux*ARRAW_WIDTH - Uy*ARRAW_HEIGHT;
  lineLeft->setLine(targetX-Ux*5.0, targetY-Uy*5.0, leftX, leftY);

  QRectF size = text->boundingRect();
  double textX = 0.0;
  double textY = 0.0;
  if( fabsf(sourceY - targetY) < 30.0) {
      textX = sourceX + deltaX / 2.0 - size.width()/2.0 + 5.0;
      if(sourceY<targetY) {
        textY = sourceY + deltaY / 2.0 - size.height();
      } else {
        textY = sourceY + deltaY / 2.0 + 5.0;
      }
  } else {
    if(sourceX<targetX) {
      textX = sourceX + deltaX / 2.0 + 10.0;
    } else {
      textX = sourceX + deltaX / 2.0 - size.width() - 10.0;
    }
    textY = sourceY + deltaY / 2.0;
  }
  text->setPos(textX, textY);
}

///
ElementNode::ElementNode(QString target) : isBreak_(false), isActive_(false) {
  if(target == "Start") {
    createStartNode();
  } else if(target == "Final") {
    createFinalNode();
  } else if(target == "Decision/Merge") {
    createDecisionNode();
  } else if(target == "Fork/Join") {
    createForkNode();
  } else {
    createCommandNode(target);
  }
}

ElementNode::ElementNode(int type, QString cmdName) : isBreak_(false), isActive_(false) {
  switch(type) {
    case ELEMENT_START:
      createStartNode();
      break;
    case ELEMENT_FINAL:
      createFinalNode();
      break;
    case ELEMENT_DECISION:
      createDecisionNode();
      break;
    case ELEMENT_FORK:
      createForkNode();
      break;
		case ELEMENT_POINT:
			createPoint();
			break;
		default:
      createCommandNode(cmdName);
      break;
  }
}

void ElementNode::createStartNode() {
  type_ = ELEMENT_START;
  item_ = new QGraphicsEllipseItem(-15, -15, 30, 30);
  item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  item_->setParentItem(this);

  addToGroup(item_);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::createFinalNode() {
  type_ = ELEMENT_FINAL;
  item_ = new QGraphicsEllipseItem(-15, -15, 30, 30);
  item_->setPen(QPen(Qt::black, 3.0));
  item_->setBrush(QBrush(Qt::white, Qt::SolidPattern));
  item_->setParentItem(this);
  item_->setData(Qt::UserRole, TYPE_ELEMENT);

  QGraphicsEllipseItem* itemUpper = new QGraphicsEllipseItem(-9, -9, 18, 18);
  itemUpper->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  itemUpper->setParentItem(this);
  itemUpper->setData(Qt::UserRole, TYPE_ELEMENT);

  addToGroup(itemUpper);
  addToGroup(item_);

  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::createDecisionNode() {
  type_ = ELEMENT_DECISION;
  QPolygon polygon;
  polygon << QPoint(0.0, 10.0) << QPoint(-15.0, 0.0)
            << QPoint(0.0, -10.0) << QPoint(15.0, 0.0) << QPoint(0.0, 10.0);
  item_ = new QGraphicsPolygonItem(polygon);
  item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  addToGroup(item_);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::createForkNode() {
  type_ = ELEMENT_FORK;
  item_ = new QGraphicsRectItem(-40, -4, 80, 8);
  item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  addToGroup(item_);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::createPoint() {
	type_ = ELEMENT_POINT;
	item_ = new QGraphicsEllipseItem(-5, -5, 10, 10);
	item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
	item_->setParentItem(this);

	addToGroup(item_);
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::createCommandNode(QString name) {
  type_ = ELEMENT_COMMAND;
  item_ = new QGraphicsEllipseItem(-10, -10, 20, 20);
  item_->setPen(QPen(Qt::black, 3.0));
  item_->setBrush(QBrush(Qt::white, Qt::SolidPattern));
  item_->setParentItem(this);
  addToGroup(item_);

	itemText_ = new QGraphicsSimpleTextItem(name);
	itemText_->setParentItem(this);
	itemText_->setPos(15, -9);
  QFont font( "Arial", 12, QFont::Bold);
	itemText_->setFont(font);
	addToGroup(itemText_);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
}

void ElementNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  QStyleOptionGraphicsItem myoption(*option);
  myoption.state &= !QStyle::State_Selected;
  QGraphicsItemGroup::paint(painter, &myoption, widget);
}

void ElementNode::updatePosition(double x, double y) {
  this->setPos(x, y);
  //
	//DDEBUG_V("updatePosition: %d", lineList_.size());
	for(int index=0; index<lineList_.size(); index++) {
    lineList_[index]->reDrawConnection();
  }
	//
	parentElem_->setPosX(x);
	parentElem_->setPosY(y);
	parentElem_->setUpdate();
}

}
