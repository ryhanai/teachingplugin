#include "ControllerManager.h"

namespace teaching {

ControllerManager* ControllerManager::instance() {
  static ControllerManager* handler = new ControllerManager();
  return handler;
}

ControllerManager::ControllerManager() {
}

ControllerManager::~ControllerManager() {
}

void ControllerManager::registController(string controllerName, ControllerBase* controller) {
  controllerList_.clear();
  ControllerInfo param;
  param.controllerName = controllerName;
  param.controller = controller;
  controllerList_.push_back(param);

  stateView_->createStateCommands();
  taskView_->loadTaskInfo();
}

ControllerBase* ControllerManager::getController(string controllerName) {
  return controllerList_[0].controller;
}

void ControllerManager::setStateMachineView(StateMachineViewImpl* view) {
  stateView_ = view;
}

void ControllerManager::setTaskInstanceView(TaskInstanceViewImpl* view) {
  taskView_ = view;
}

bool ControllerManager::isExistController() {
  return 0<controllerList_.size();
}

}