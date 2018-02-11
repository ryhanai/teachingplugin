#include "ActivityEditorBase.hpp"

#include <memory>
#include "StyleCollection.hpp"

#include "../TeachingEventHandler.h"
#include "../LoggerUtil.h"

using namespace std;

namespace QtNodes {

ActivityEditorBase::ActivityEditorBase(FlowScene* scene)
	: QGraphicsView(scene), _clickPos(QPointF()), _scene(scene),
		targetParam_(0) {
}

QAction* ActivityEditorBase::clearSelectionAction() const {
	return _clearSelectionAction;
}

QAction* ActivityEditorBase::deleteSelectionAction() const {
	return _deleteSelectionAction;
}

void ActivityEditorBase::deleteSelectedNodes() {
  if (TeachingEventHandler::instance()->canEdit() == false) return;
  // delete the nodes, this will delete many of the connections
	for (QGraphicsItem * item : _scene->selectedItems()) {
		if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
			int targetId = n->node().getParamId();
			std::vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
			vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
			if (targetElem != stateList.end()) {
				(*targetElem)->setDelete();
			}

			_scene->removeNode(n->node());
		}
	}

	for (QGraphicsItem * item : _scene->selectedItems()) {
		if (auto c = qgraphicsitem_cast<ConnectionGraphicsObject*>(item))
			_scene->deleteConnection(c->connection());
	}
}

void ActivityEditorBase::wheelEvent(QWheelEvent *event) {
	QPoint delta = event->angleDelta();

	if (delta.y() == 0) {
		event->ignore();
		return;
	}

	double const d = delta.y() / std::abs(delta.y());

	if (d > 0.0)
		scaleUp();
	else
		scaleDown();
}

void ActivityEditorBase::scaleUp() {
	double const step = 1.2;
	double const factor = std::pow(step, 1.0);

	QTransform t = transform();

	if (t.m11() > 2.0)
		return;

	scale(factor, factor);
}


void ActivityEditorBase::scaleDown() {
	double const step = 1.2;
	double const factor = std::pow(step, -1.0);

	scale(factor, factor);
}

void ActivityEditorBase::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
	case Qt::Key_Shift:
		setDragMode(QGraphicsView::RubberBandDrag);
		break;

	default:
		break;
	}

	QGraphicsView::keyPressEvent(event);
}

void ActivityEditorBase::keyReleaseEvent(QKeyEvent *event) {
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

void ActivityEditorBase::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);
	if (event->button() == Qt::LeftButton) {
		_clickPos = mapToScene(event->pos());
	}
}

void ActivityEditorBase::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
	if (scene()->mouseGrabberItem() == nullptr && event->buttons() == Qt::LeftButton) {
		// Make sure shift is not being pressed
		if ((event->modifiers() & Qt::ShiftModifier) == 0) {
			QPointF difference = _clickPos - mapToScene(event->pos());
			setSceneRect(sceneRect().translated(difference.x(), difference.y()));
		}
	}
	//
	//_scene().loca->locateNode
}

void ActivityEditorBase::drawBackground(QPainter* painter, const QRectF& r) {
	QGraphicsView::drawBackground(painter, r);

	auto drawGrid =
		[&](double gridStep) {
		QRect   windowRect = rect();
		QPointF tl = mapToScene(windowRect.topLeft());
		QPointF br = mapToScene(windowRect.bottomRight());

		double left = std::floor(tl.x() / gridStep - 0.5);
		double right = std::floor(br.x() / gridStep + 1.0);
		double bottom = std::floor(tl.y() / gridStep - 0.5);
		double top = std::floor(br.y() / gridStep + 1.0);

		// vertical lines
		for (int xi = int(left); xi <= int(right); ++xi) {
			QLineF line(xi * gridStep, bottom * gridStep,
				xi * gridStep, top * gridStep);

			painter->drawLine(line);
		}

		// horizontal lines
		for (int yi = int(bottom); yi <= int(top); ++yi) {
			QLineF line(left * gridStep, yi * gridStep,
				right * gridStep, yi * gridStep);
			painter->drawLine(line);
		}
	};

	auto const &flowViewStyle = StyleCollection::flowViewStyle();

	QBrush bBrush = backgroundBrush();

	QPen pfine(flowViewStyle.FineGridColor, 1.0);

	painter->setPen(pfine);
	drawGrid(15);

	QPen p(flowViewStyle.CoarseGridColor, 1.0);

	painter->setPen(p);
	drawGrid(150);
}

void ActivityEditorBase::showEvent(QShowEvent *event) {
	_scene->setSceneRect(this->rect());
	QGraphicsView::showEvent(event);
}

FlowScene* ActivityEditorBase::scene() {
	return _scene;
}
//////////
void ActivityEditorBase::removeAll() {
	unordered_map<QUuid, shared_ptr<Connection> > connMap = _scene->connections();
	for (auto it = connMap.begin(); it != connMap.end(); ++it) {
		shared_ptr<Connection> target = it->second;
		_scene->deleteConnection(*target);
	}
	//
	QList<QGraphicsItem*> itemsList = _scene->items();
	QList<QGraphicsItem*>::iterator iter = itemsList.begin();
	while (iter != itemsList.end()) {
		QGraphicsItem* item = (*iter);
		_scene->removeItem(item);
		iter++;
	}
}

ElementStmParamPtr ActivityEditorBase::getCurrentNode() {
	ElementStmParamPtr result = 0;
	for (QGraphicsItem * item : _scene->selectedItems()) {
		if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
			int targetId = n->node().getParamId();
			std::vector<ElementStmParamPtr> stateList = targetParam_->getStmElementList();
			vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
			if (targetElem == stateList.end()) break;
			result = *targetElem;
			break;
		}
	}
	return result;
}

}
