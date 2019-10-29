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

void TaskExecutor::updateController() {
  DDEBUG_V("TaskExecutor::updateController %s", SettingManager::getInstance().getController().c_str());

  handler_ = ControllerManager::instance()->getController(SettingManager::getInstance().getController());
  handler_->initialize();
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

bool TaskExecutor::attachModelItem(cnoid::BodyItemPtr parent, cnoid::BodyItemPtr child, int target) {
  DDEBUG("TaskExecutor::attachModelItem");
  BodyItem* parentItem;
  Link* parentLink;
  if (!parent) {
    string strRobotName = SettingManager::getInstance().getRobotModelName();
    parentItem = ChoreonoidUtil::searchParentModel(strRobotName);
    parentLink = handler_->getToolLink(target);
  } else {
    parentItem = parent;
    parentLink = parent->body()->link(0);
  }

  if(!parentItem || !parentLink) {
    DDEBUG("TaskExecutor::attachModelItem Error");
    return false;
  }

  AttachedItemsPtr attached_item = std::make_shared<AttachedItems>(parentItem, parentLink, child);
  attached_item->attachItems();

  AttachedModelPtr model = std::make_shared<AttachedModel>();
  model->parent = parent;
  model->child = child;
  model->target = target;
  model->item = attached_item;
  modelList.push_back(model);

  return true;
}

bool TaskExecutor::detachModelItem(cnoid::BodyItemPtr parent, cnoid::BodyItemPtr child, int target) {
  for (AttachedModelPtr model : modelList) {
    if (model->parent == parent && model->child == child && model->target == target) {
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
