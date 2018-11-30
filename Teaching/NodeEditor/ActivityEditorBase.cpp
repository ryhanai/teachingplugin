#include "ActivityEditorBase.hpp"

#include <memory>
#include <QTouchEvent>
#include <QScrollBar>
#include "StyleCollection.hpp"

#include "../TeachingEventHandler.h"
#include "../TeachingUtil.h"
#include "../LoggerUtil.h"

using namespace std;

namespace QtNodes {

ActivityEditorBase::ActivityEditorBase(FlowScene* scene)
	: QGraphicsView(scene), _clickPos(QPointF()), _scene(scene),
		targetParam_(0)
#ifdef MODE_TABLET
  , totalScaleFactor(1)
#endif
{
#ifdef MODE_TABLET
  viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    setDragMode(ScrollHandDrag);
#endif
}

QAction* ActivityEditorBase::clearSelectionAction() const {
	return _clearSelectionAction;
}

QAction* ActivityEditorBase::deleteSelectionAction() const {
	return _deleteSelectionAction;
}

#ifdef MODE_TABLET
bool ActivityEditorBase::viewportEvent(QEvent *event) {
  DDEBUG("ActivityEditorBase::viewportEvent");
  switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
      QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
      QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
      if (touchPoints.count() == 2) {
        // determine scale factor
        const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
        const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
        qreal currentScaleFactor =
          QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
          / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
        if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
          // if one of the fingers is released, remember the current scale
          // factor so that adding another finger later will continue zooming
          // by adding new scale factor to the existing remembered value.
          totalScaleFactor *= currentScaleFactor;
          currentScaleFactor = 1;
        }
        setTransform(QTransform().scale(totalScaleFactor * currentScaleFactor,
          totalScaleFactor * currentScaleFactor));
      }
      return true;
    }
    default:
      break;
  }
  return QGraphicsView::viewportEvent(event);
}
#else

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
#endif

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
  _scene->clearScene();
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

void ActivityEditorBase::setEditMode(bool canEdit) {
  for (QGraphicsItem* item : _scene->items()) {
    ((NodeGraphicsObject*)item)->lock(!canEdit);
  }
}

}
