#include "TeachingEventHandler.h"

using namespace std;

namespace teaching {

  bool TeachingEventHandler::flv_ConnectNodes(QString from, QString fromPort, QString to, QString toPort) {
    return this->flv_->connectNodes(from, fromPort, to, toPort);
  }

  bool TeachingEventHandler::tst_DeleteAllFlows() {
    vector<string> condList;
    bool isOr = false;
    condList.push_back("");
    TeachingDataHolder::instance()->searchFlow(condList, isOr);
    for (auto f : TeachingDataHolder::instance()->getFlowList()) {
      if (!TeachingDataHolder::instance()->deleteFlow(f->getId())) {
        return false;
      }
    }
    return true;
  }

  bool TeachingEventHandler::tst_DeleteAllTasks() {
    for (auto tmi : TeachingDataHolder::instance()->getTaskList()) {
      if (!TeachingDataHolder::instance()->deleteTaskModel(tmi->getId())) {
        return false;
      }
    }
    return true;
  }

  void TeachingEventHandler::tst_ClearFlowScene() {
    this->flv_->clearFlowScene();
  }

  bool TeachingEventHandler::flv_RenameNode(QString currentName, QString newName) {
    return this->flv_->renameNode(currentName, newName);
  }

  bool TeachingEventHandler::flv_CreateNode(QString modelName, QPoint pos) {
    return this->flv_->createNode(modelName, pos);
  }

}
