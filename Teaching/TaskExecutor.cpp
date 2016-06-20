#ifdef _WIN32
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif
#include "TaskExecutor.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

TaskExecutor* TaskExecutor::instance() {
  static TaskExecutor* taskExecutor = new TaskExecutor();
  return taskExecutor;
}

TaskExecutor::TaskExecutor() {
  handler_ = SampleHiroController::instance();
}

TaskExecutor::~TaskExecutor() {
}

CommandDefParam* TaskExecutor::getCommandDef(const std::string& commandName) {
  CommandDefParam* result = 0;

  std::vector<CommandDefParam*> cmdDefList = handler_->getCommandDefList();
  for(int index=0; index<cmdDefList.size(); index++) {
    CommandDefParam* cmd = cmdDefList[index];
    if(cmd->getName().toStdString() == commandName) {
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

bool TaskExecutor::executeCommand (const std::string& commandName, const std::vector<CompositeParamType>& params, bool simulation) throw (CommandParseErrorException) {
  try {
    return handler_->executeCommand(commandName, params, simulation);
  } catch (...) {
    return false;
  }
}

bool TaskExecutor::attachModelItem(cnoid::BodyItemPtr object, int target) {
    return handler_->attachModelItem(object, target);
}

bool TaskExecutor::detachModelItem(cnoid::BodyItemPtr object, int target) {
    return handler_->detachModelItem(object, target);
}

}