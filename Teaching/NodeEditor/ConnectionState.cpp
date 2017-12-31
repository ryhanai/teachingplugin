#include "ConnectionState.hpp"

#include <iostream>

#include <QtCore/QPointF>

#include "FlowScene.hpp"
#include "Node.hpp"

#include "../LoggerUtil.h"

using QtNodes::ConnectionState;
using QtNodes::Node;
using namespace teaching;

ConnectionState::
~ConnectionState()
{
  resetLastHoveredNode();
}


void ConnectionState::interactWithNode(Node* node) {
	if (node) {
    _lastHoveredNode = node;
  } else {
    resetLastHoveredNode();
  }
}


void
ConnectionState::
setLastHoveredNode(Node* node)
{
  _lastHoveredNode = node;
}


void ConnectionState::resetLastHoveredNode() {
  if (_lastHoveredNode)
    _lastHoveredNode->resetReactionToConnection();

  _lastHoveredNode = nullptr;
}
