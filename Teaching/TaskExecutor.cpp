#ifdef _WIN32
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif

#include "ControllerManager.h"
#include "TaskExecutor.h"
#include "TeachingUtil.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

TaskExecutor* TaskExecutor::instance() {
  static TaskExecutor* taskExecutor = new TaskExecutor();
  return taskExecutor;
}

TaskExecutor::TaskExecutor() {
  handler_ = ControllerManager::instance()->getController(SettingManager::getInstance().getController());
}

TaskExecutor::~TaskExecutor() {
}

CommandDefParam* TaskExecutor::getCommandDef(const std::string& commandName) {
  CommandDefParam* result = 0;

  std::vector<CommandDefParam*> cmdDefList = handler_->getCommandDefList();
  for (int index = 0; index < cmdDefList.size(); index++) {
    CommandDefParam* cmd = cmdDefList[index];
    if (cmd->getName().toStdString() == commandName) {
      result = cmd;
      break;
    }
  }

  return result;
}

CommandDefParam* TaskExecutor::getCommandDef(const int commandId) {
  CommandDefParam* result = 0;

  std::vector<CommandDefParam*> cmdDefList = handler_->getCommandDefList();
  for (int index = 0; index < cmdDefList.size(); index++) {
    CommandDefParam* cmd = cmdDefList[index];
    if (cmd->getId() == commandId) {
      result = cmd;
      break;
    }
  }

  return result;
}

std::vector<CommandDefParam*> TaskExecutor::getCommandDefList() {
  return handler_->getCommandDefList();
}

void TaskExecutor::setRootName(std::string value) {
  handler_->setRootName(value);
}

bool TaskExecutor::executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params) {
  try {
    return handler_->executeCommand(commandName, params,SettingManager::getInstance().getIsReal());
  }
  catch (...) {
    detachAllModelItem();
    return false;
  }
}

bool TaskExecutor::attachModelItem(cnoid::BodyItemPtr object, int target) {
  AttachedModel* model = new AttachedModel();
  model->object = object;
  model->target = target;
  modelList.push_back(model);

  return handler_->attachModelItem(object, target);
}

bool TaskExecutor::detachModelItem(cnoid::BodyItemPtr object, int target) {
  for (unsigned int index = 0; index < modelList.size(); index++) {
    AttachedModel* model = modelList[index];
    if (model->object == object && model->target == target) {
      this->modelList.erase(std::remove(this->modelList.begin(), this->modelList.end(), model), this->modelList.end());
      delete model;
      break;
    }
  }
  return handler_->detachModelItem(object, target);
}

bool TaskExecutor::detachAllModelItem() {
  for (unsigned int index = 0; index < modelList.size(); index++) {
    AttachedModel* model = modelList[index];
    if (handler_->detachModelItem(model->object, model->target) == false) return false;
  }
  return true;
}

}
