#pragma once

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

	ElementStmParamPtr getCurrentNode();

public Q_SLOTS:
	void scaleUp();
	void scaleDown();
	void deleteSelectedNodes();

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;

	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

	void drawBackground(QPainter* painter, const QRectF& r) override;
	void showEvent(QShowEvent *event) override;

	QAction* _clearSelectionAction;
	QAction* _deleteSelectionAction;

	QPointF _clickPos;
	FlowScene * _scene;

	FlowScene * scene();

	ActivityParamPtr targetParam_;
};

}
