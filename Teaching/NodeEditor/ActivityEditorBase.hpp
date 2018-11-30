#pragma once

//#define MODE_TABLET

#include <QtWidgets/QGraphicsView>
#include "FlowScene.hpp"

#include "../TeachingTypes.h"

using namespace teaching;

namespace QtNodes {

class FlowScene;

class ActivityEditorBase : public QGraphicsView {
	Q_OBJECT

public:
	ActivityEditorBase(FlowScene* scene = 0);

	QAction* clearSelectionAction() const;
	QAction* deleteSelectionAction() const;

	void setTargetParam(ActivityParamPtr param) { this->targetParam_ = param; }
	void removeAll();
  void setEditMode(bool canEdit);

	ElementStmParamPtr getCurrentNode();

#ifndef MODE_TABLET
public Q_SLOTS:
	void scaleUp();
	void scaleDown();
#endif

protected:
	void keyPressEvent(QKeyEvent *event) override;

	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
#ifdef MODE_TABLET
  bool viewportEvent(QEvent *event);
#else
  void wheelEvent(QWheelEvent *event) override;
#endif

	void drawBackground(QPainter* painter, const QRectF& r) override;
	void showEvent(QShowEvent *event) override;

	QAction* _clearSelectionAction;
	QAction* _deleteSelectionAction;

	QPointF _clickPos;
	FlowScene * _scene;

	FlowScene * scene();

	ActivityParamPtr targetParam_;

#ifdef MODE_TABLET
  qreal totalScaleFactor;
#endif
};

}
