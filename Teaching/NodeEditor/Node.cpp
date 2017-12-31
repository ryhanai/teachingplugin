#include "Node.hpp"

#include <QtCore/QObject>

#include <iostream>

#include "FlowScene.hpp"

#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

#include "../LoggerUtil.h"

using QtNodes::Node;
using QtNodes::NodeGeometry;
using QtNodes::NodeState;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeGraphicsObject;
using QtNodes::PortIndex;
using QtNodes::PortType;

using namespace teaching;

Node::Node(std::unique_ptr<NodeDataModel> && dataModel)
  : _id(QUuid::createUuid())
  , _nodeDataModel(std::move(dataModel))
  , _nodeState(_nodeDataModel)
  , _nodeGeometry(_nodeDataModel)
  , _nodeGraphicsObject(nullptr)
	, isBreak_(false), isActive_(false)
{
  _nodeGeometry.recalculateSize();

  // propagate data: model => node
  connect(_nodeDataModel.get(), &NodeDataModel::dataUpdated,
          this, &Node::onDataUpdated);
}

Node::~Node() {}

QJsonObject Node::save() const {
  QJsonObject nodeJson;

  nodeJson["id"] = _id.toString();

  nodeJson["model"] = _nodeDataModel->save();

  QJsonObject obj;
  obj["x"] = _nodeGraphicsObject->pos().x();
  obj["y"] = _nodeGraphicsObject->pos().y();
  nodeJson["position"] = obj;

  return nodeJson;
}

void Node::restore(QJsonObject const& json) {
  _id = QUuid(json["id"].toString());

  QJsonObject positionJson = json["position"].toObject();
  QPointF     point(positionJson["x"].toDouble(),
                    positionJson["y"].toDouble());
  _nodeGraphicsObject->setPos(point);

  _nodeDataModel->restore(json["model"].toObject());
}


QUuid Node::id() const {
  return _id;
}

void Node::reactToPossibleConnection(PortType reactingPortType,
																		 NodeDataType reactingDataType,
																		 QPointF const &scenePoint) {
	QTransform const t = _nodeGraphicsObject->sceneTransform();
  QPointF p = t.inverted().map(scenePoint);
  _nodeGeometry.setDraggingPosition(p);
  _nodeGraphicsObject->update();
  _nodeState.setReaction(NodeState::REACTING, reactingPortType, reactingDataType);
}


void Node::resetReactionToConnection() {
	DDEBUG("Node::resetReactionToConnection");

	_nodeState.setReaction(NodeState::NOT_REACTING);
  _nodeGraphicsObject->update();
}


NodeGraphicsObject const & Node::nodeGraphicsObject() const {
  return *_nodeGraphicsObject.get();
}

NodeGraphicsObject & Node::nodeGraphicsObject() {
  return *_nodeGraphicsObject.get();
}

void Node::setGraphicsObject(std::unique_ptr<NodeGraphicsObject>&& graphics) {
  _nodeGraphicsObject = std::move(graphics);

  _nodeGeometry.recalculateSize();
}

NodeGeometry& Node::nodeGeometry() {
  return _nodeGeometry;
}

NodeGeometry const& Node::nodeGeometry() const {
  return _nodeGeometry;
}

NodeState const & Node::nodeState() const {
  return _nodeState;
}

NodeState & Node::nodeState() {
  return _nodeState;
}

NodeDataModel* Node::nodeDataModel() const {
  return _nodeDataModel.get();
}

void Node::propagateData(std::shared_ptr<NodeData> nodeData,
              PortIndex inPortIndex) const {
  _nodeDataModel->setInData(nodeData, inPortIndex);

  //Recalculate the nodes visuals. A data change can result in the node taking more space than before, so this forces a recalculate+repaint on the affected node
  _nodeGraphicsObject->setGeometryChanged();
  _nodeGeometry.recalculateSize();
  _nodeGraphicsObject->update();
  _nodeGraphicsObject->moveConnections();
}

void Node::onDataUpdated(PortIndex index) {
  auto nodeData = _nodeDataModel->outData(index);

  auto connections =
    _nodeState.connections(PortType::Out, index);

  for (auto const & c : connections)
    c.second->propagateData(nodeData);
}

void Node::updateActive(bool isActive) {
	this->isActive_ = isActive;
	updateDisp();
}

void Node::setBreak(bool isBreak) {
	this->isBreak_ = isBreak;
	updateDisp();
}

void Node::updateDisp() {
	NodeStyle style = _nodeDataModel->nodeStyle();
	if (isBreak_) {
		if (isActive_) {
			style.NormalBoundaryColor = Qt::magenta;
		} else {
			style.NormalBoundaryColor = Qt::green;
		}
		style.SelectedBoundaryColor = Qt::blue;

	} else {
		if (isActive_) {
			style.NormalBoundaryColor = Qt::red;
		} else {
			style.NormalBoundaryColor = Qt::darkGray;
		}
		style.SelectedBoundaryColor = Qt::cyan;
	}
	_nodeDataModel->setNodeStyle(style);
	_nodeGraphicsObject->update();
}
