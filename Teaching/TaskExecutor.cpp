#ifdef _WIN32
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif

#include "ControllerManager.h"
#include "TaskExecutor.h"

#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
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
    if (cmd->getCmdName().toStdString() == commandName) {
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
  string strRobotName = SettingManager::getInstance().getRobotModelName();
  BodyItem* parentItem = ChoreonoidUtil::searchParentModel(strRobotName);
  Link* parentLink = handler_->getToolLink(target);

  AttachedItemsPtr attached_item = std::make_shared<AttachedItems>(parentItem, parentLink, object);
  attached_item->attachItems();

  AttachedModelPtr model = std::make_shared<AttachedModel>();
  model->object = object;
  model->target = target;
  model->item = attached_item;
  modelList.push_back(model);

  return true;
}

bool TaskExecutor::detachModelItem(cnoid::BodyItemPtr object, int target) {
  for (AttachedModelPtr model : modelList) {
    if (model->object == object && model->target == target) {
      model->item->detachItems();
      this->modelList.erase(std::remove(this->modelList.begin(), this->modelList.end(), model), this->modelList.end());
      break;
    }
  }
  return true;
}

bool TaskExecutor::detachAllModelItem() {
  for (AttachedModelPtr model : modelList) {
    model->item->detachItems();
  }
  modelList.clear();
  return true;
}

}
