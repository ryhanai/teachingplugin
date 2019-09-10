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
  //AttachedModel* model = new AttachedModel();
  //model->object = object;
  //model->target = target;
  //modelList.push_back(model);

  //return handler_->attachModelItem(object, target);

  string strRobotName = SettingManager::getInstance().getRobotModelName();
  BodyItem* parentItem = ChoreonoidUtil::searchParentModel(strRobotName);
  Link* parentLink = handler_->getToolLink(target);

  AttachedItemsPtr attached_item = std::make_shared<AttachedItems>(parentItem, parentLink, object);
  attached_item->attachItems();

  AttachedModel* model = new AttachedModel();
  model->object = object;
  model->target = target;
  model->item = attached_item;
  modelList.push_back(model);

  return true;
}

bool TaskExecutor::detachModelItem(cnoid::BodyItemPtr object, int target) {
  //for (unsigned int index = 0; index < modelList.size(); index++) {
  //  AttachedModel* model = modelList[index];
  //  if (model->object == object && model->target == target) {
  //    this->modelList.erase(std::remove(this->modelList.begin(), this->modelList.end(), model), this->modelList.end());
  //    delete model;
  //    break;
  //  }
  //}
  //return handler_->detachModelItem(object, target);

  for (unsigned int index = 0; index < modelList.size(); index++) {
    AttachedModel* model = modelList[index];
    if (model->object == object && model->target == target) {
      model->item->detachItems();
      this->modelList.erase(std::remove(this->modelList.begin(), this->modelList.end(), model), this->modelList.end());
      delete model;
      break;
    }
  }
  return true;
}

bool TaskExecutor::detachAllModelItem() {
  for (unsigned int index = 0; index < modelList.size(); index++) {
    AttachedModel* model = modelList[index];
    if (handler_->detachModelItem(model->object, model->target) == false) return false;
  }
  return true;
}

bool TaskExecutor::attachModelItems(cnoid::BodyItemPtr parent, cnoid::BodyItemPtr child) {
  //AttachedItemsPtr attached_item = std::make_shared<AttachedItems>(parent, child);
  //attached_item->attachItems();
  //itemList_.push_back(attached_item);

  return true;
}

void TaskExecutor::detachModelItems() {
  //for (AttachedItemsPtr item : itemList_) {
  //  item->detachItems();
  //}
}
}
