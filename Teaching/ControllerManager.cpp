#include "ControllerManager.h"
#include "TeachingUtil.h"
#include "LoggerUtil.h"

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
  DDEBUG_V("ControllerManager::registController: %s", controllerName.c_str());
  ControllerInfo param;
  param.controllerName = controllerName;
  param.controller = controller;
  controllerList_.push_back(param);

  if(SettingManager::getInstance().getController()==controllerName) {
    controller->initialize(); // R.Hanai
    stateView_->createStateCommands();
    taskView_->loadTaskInfo();
  }
}

void ControllerManager::initialize() {
}

vector<string> ControllerManager::getControllerNameList() {
  vector<string> result;

  for(ControllerInfo target : controllerList_) {
    result.push_back(target.controllerName);
  }

  return result;
}

ControllerBase* ControllerManager::getController(string controllerName) {
  for(ControllerInfo target : controllerList_) {
    if(target.controllerName == controllerName) {
      return target.controller;
    }
  }
  return controllerList_[0].controller;
}

void ControllerManager::setStateMachineView(StateMachineViewImpl* view) {
  stateView_ = view;
}

void ControllerManager::setTaskInstanceView(TaskInstanceViewImpl* view) {
  taskView_ = view;
}

bool ControllerManager::isExistController() {
  return 0 < controllerList_.size();
}

}