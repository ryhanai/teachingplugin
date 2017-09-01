#include <cnoid/InfoBar>

#include "TaskExecuteManager.h"
#include "StateMachineView.h"
#include "TaskInstanceView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

TaskExecuteManager::TaskExecuteManager()
  : currParam_(0), currentTask_(0), prevTask_(0),
  taskInstView(0), statemachineView_(0), parameterView_(0), metadataView(0),
  isBreak_(false), isAbort_(false){
}

void TaskExecuteManager::setButtonEnableMode(bool isEnable) {
  taskInstView->setButtonEnableMode(isEnable);
  flowView_->setButtonEnableMode(isEnable);
  statemachineView_->setButtonEnableMode(isEnable);
}

void TaskExecuteManager::abortOperation() {
  isAbort_ = true;
}

void TaskExecuteManager::runFlow(FlowParam* targetFlow) {
  if (!targetFlow) {
    QMessageBox::warning(0, _("Run Flow"), _("Select Target Flow"));
    return;
  }
  this->taskInstView->unloadCurrentModel();
  if (currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }

  parameterView_->setInputValues();

  if (targetFlow->checkAndOrderStateMachine()) {
    QMessageBox::warning(0, _("Run Flow"), targetFlow->getErrStr());
    return;
  }
  vector<ElementStmParam*> stateList = targetFlow->getStmElementList();
  for (int index = 0; index < stateList.size(); index++) {
    ElementStmParam* targetState = stateList[index];
    if (targetState->getType() != ELEMENT_COMMAND) continue;
    //
    TaskModelParam* targetTask = targetState->getTaskParam();
    if (targetTask->IsModelLoaded()) {
      ChoreonoidUtil::unLoadTaskModelItem(targetTask);
    }
    if (targetState->getMode() == DB_MODE_DELETE || targetState->getMode() == DB_MODE_IGNORE) {
      continue;
    }
    if (targetTask->IsLoaded() == false) {
      TeachingUtil::loadTaskDetailData(targetTask);
    }
    if (targetTask->checkAndOrderStateMachine()) {
      QMessageBox::warning(0, _("Run Flow"), currentTask_->getErrStr());
      return;
    }
    targetTask->setNextTask(targetState->getNextElem()->getTaskParam());
  }
  //
  setButtonEnableMode(false);
  isAbort_ = false;
  InfoBar::instance()->showMessage(_("Running Flow :") + targetFlow->getName(), MESSAGE_PERIOD);
  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  //
  ElementStmParam* currTask = targetFlow->getStartParam();
  ElementStmParam* nextTask = 0;
  while (true) {
    if (currTask->getType() == ELEMENT_COMMAND) {
      currentTask_ = currTask->getTaskParam();
      break;
    }
    currTask = currTask->getNextElem();
  }

  currentTask_ = currTask->getTaskParam();
  currTask->updateActive(true);
  this->flowView_->repaint();
  if (doFlowOperation() == ExecResult::EXEC_BREAK) {
    setButtonEnableMode(true);
    statemachineView_->setStepStatus(true);
    return;
  }
  setButtonEnableMode(true);

  //while (true) {
  //	if (currTask->getType() == ELEMENT_COMMAND) {
  //		prevTask_ = currentTask_;
  //		currentTask_ = currTask->getTaskParam();
  //		if (doFlowOperation() == ExecResult::EXEC_BREAK) {
  //			statemachineView_->setStepStatus(true);
  //			return;
  //		}
  //	}
  //	nextTask = currTask->getNextElem();

  //	if (!nextTask) {
  //		InfoBar::instance()->showMessage("");
  //		DDEBUG_V("currTask : %d, nextTask:NOT EXIST.", currTask->getType());
  //		detachAllModelItem();
  //		return;
  //	}
  //	/////
  //	currTask->updateActive(false);
  //	nextTask->updateActive(true);
  //	this->flowView_->repaint();

  //	currTask = nextTask;
  //	if (nextTask->getType() == ELEMENT_FINAL) {
  //		break;
  //	}
  //}
  InfoBar::instance()->showMessage(_("Finished Flow :") + targetFlow->getName(), MESSAGE_PERIOD);
}

bool TaskExecuteManager::runSingleCommand() {
  bool isReal = SettingManager::getInstance().getIsReal();

  //引数計算モジュールの初期化
  createArgEstimator(currentTask_);
  //引数の組み立て
  std::vector<CompositeParamType> parameterList;
  if (argHandler_->buildArguments(currentTask_, currParam_, parameterList) == false) {
    deleteArgEstimator();
    return false;
  }
  isAbort_ = false;
  InfoBar::instance()->showMessage(_("Running Command :") + currParam_->getCmdName());
  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  bool cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
  //out引数の設定
  setOutArgument(parameterList);

  deleteArgEstimator();
  InfoBar::instance()->showMessage(_("Finished Command :") + currParam_->getCmdName(), MESSAGE_PERIOD);

  return cmdRet;
}

void TaskExecuteManager::runSingleTask() {
  DDEBUG("TaskExecuteManager::runSingleTask");
  parameterView_->setInputValues();
  statemachineView_->setStepStatus(false);
  if (currParam_) {
    currParam_->updateActive(false);
    statemachineView_->repaint();
  }
  if (!currentTask_) {
    QMessageBox::warning(0, _("Run Task"), _("Please select target TASK"));
    return;
  }
  //
  if (currentTask_->checkAndOrderStateMachine()) {
    QMessageBox::warning(0, _("Run Task"), currentTask_->getErrStr());
    return;
  }
  //
  setButtonEnableMode(false);
  isAbort_ = false;
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();
  QString taskName = currentTask_->getName();
  InfoBar::instance()->showMessage(_("Running Task :") + taskName, MESSAGE_PERIOD);

  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  ExecResult ret = doFlowOperation(true);
  setButtonEnableMode(true);
  if (ret == ExecResult::EXEC_BREAK) {
    statemachineView_->setStepStatus(true);
    InfoBar::instance()->notify(_("Paused Task :") + taskName);
    return;
  } else if (ret == ExecResult::EXEC_ERROR) {
    InfoBar::instance()->notify(_("Failed Task :") + taskName);
    return;
  }
  InfoBar::instance()->notify(_("Finished Task :") + taskName);
}

ExecResult TaskExecuteManager::doFlowOperation(bool isSingle) {
  DDEBUG("TaskExecuteManager::doFlowOperation");
  prevTask_ = 0;
  if (currentTask_) prepareTask();
  while (true) {
    if (currentTask_ == 0) break;

    ExecResult ret = doTaskOperation(!isSingle); // R.Hanai
    if (ret != ExecResult::EXEC_FINISHED) return ret;
    if (isSingle || isAbort_) return ExecResult::EXEC_FINISHED;
  }
  if (prevTask_ && prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
  this->flowView_->repaint();
  return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doFlowOperationCont() {
  while (true) {
    if (currentTask_ == 0) break;

    ExecResult ret = doTaskOperation();
    if (ret != ExecResult::EXEC_FINISHED) return ret;
  }
  if (prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
  this->flowView_->repaint();
  return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doTaskOperation(bool updateCurrentTask) {
  bool isReal = SettingManager::getInstance().getIsReal();

  //モデル情報の設定
  parseModelInfo();

  ElementStmParam* nextParam;
  bool cmdRet = false;

  //引数計算モジュールの初期化
  createArgEstimator(currentTask_);

  std::vector<CompositeParamType> parameterList;
  while (true) {
    if (currParam_->getType() == ELEMENT_COMMAND) {
      if (currParam_->isBreak()) return ExecResult::EXEC_BREAK;
      if (isAbort_) {
        detachAllModelItem();
        deleteArgEstimator();
        return ExecResult::EXEC_FINISHED;
      }
      //引数の組み立て
      if (argHandler_->buildArguments(currentTask_, currParam_, parameterList) == false) {
        detachAllModelItem();
        deleteArgEstimator();
        return ExecResult::EXEC_ERROR;
      }
      //コマンド実行
      cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
      //out引数の設定
      setOutArgument(parameterList);

      //モデルアクション実行
      if (doModelAction() == false) {
        detachAllModelItem();
        deleteArgEstimator();
        return ExecResult::EXEC_ERROR;
      }
      //コマンドの実行結果がFalseで次の要素がデシジョンではない場合は終了
      if (cmdRet == false) {
        ElementStmParam* checkNext = currParam_->getNextElem();
        if (checkNext == 0 || checkNext->getType() != ELEMENT_DECISION) {
          InfoBar::instance()->showMessage("");
          DDEBUG("NextParam is NOT Decision.");
          detachAllModelItem();
          deleteArgEstimator();
          return ExecResult::EXEC_ERROR;
        }
      }
    }
    //
    if (currParam_->getType() == ELEMENT_DECISION) {
      QString strCond = currParam_->getCondition();
      string strTarget = "";
      if (0<strCond.length()) {
        strTarget = strCond.toStdString();
      }
      bool checkCond = argHandler_->checkCondition(cmdRet, strTarget);
      if (checkCond) {
        nextParam = currParam_->getTrueElem();
      } else {
        nextParam = currParam_->getFalseElem();
      }
    } else {
      nextParam = currParam_->getNextElem();
    }
    if (!nextParam) {
      InfoBar::instance()->showMessage("");
      DDEBUG_V("currParam : %d, nextParam:NOT EXIST.", currParam_->getType());
      detachAllModelItem();
      deleteArgEstimator();
      return ExecResult::EXEC_ERROR;
    }
    /////
    currParam_->updateActive(false);
    nextParam->updateActive(true);
    this->statemachineView_->repaint();
    DDEBUG_V("currParam : %d, nextParam : %d", currParam_->getType(), nextParam->getType());

    currParam_ = nextParam;
    if (nextParam->getType() == ELEMENT_FINAL) {

      // R.Hanai
      if (!updateCurrentTask) { break; }
      // R.Hanai

      prevTask_ = currentTask_;
      currentTask_ = currentTask_->getNextTask();
      //
      if (currentTask_) {
        prepareTask();
      }
      break;
    }
  }
  deleteArgEstimator();
  return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doTaskOperationStep() {
  bool isReal = SettingManager::getInstance().getIsReal();
  parameterView_->setInputValues();

  //モデル情報の設定
  parseModelInfo();

  ElementStmParam* nextParam;
  bool cmdRet = false;
  std::vector<CompositeParamType> parameterList;

  //引数の組み立て
  if (argHandler_->buildArguments(currentTask_, currParam_, parameterList) == false) {
    detachAllModelItem();
    return ExecResult::EXEC_ERROR;
  }
  //コマンド実行
  cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
  //out引数の設定
  setOutArgument(parameterList);
  //モデルアクション実行
  if (doModelAction() == false) {
    detachAllModelItem();
    return ExecResult::EXEC_ERROR;
  }
  //コマンドの実行結果がFalseで次の要素がデシジョンではない場合は終了
  if (cmdRet == false) {
    ElementStmParam* checkNext = currParam_->getNextElem();
    if (checkNext == 0 || checkNext->getType() != ELEMENT_DECISION) {
      InfoBar::instance()->showMessage("");
      DDEBUG("NextParam is NOT Decision.");
      detachAllModelItem();
      deleteArgEstimator();
      return ExecResult::EXEC_ERROR;
    }
  }
  //
  while (true) {
    if (currParam_->getType() == ELEMENT_DECISION) {
      QString strCond = currParam_->getCondition();
      string strTarget = "";
      if (0<strCond.length()) {
        strTarget = strCond.toStdString();
      }
      bool checkCond = argHandler_->checkCondition(cmdRet, strCond.toStdString());
      if (checkCond) {
        nextParam = currParam_->getTrueElem();
      } else {
        nextParam = currParam_->getFalseElem();
      }
    } else {
      nextParam = currParam_->getNextElem();
    }
    if (!nextParam) {
      InfoBar::instance()->showMessage("");
      DDEBUG_V("currParam : %d, nextParam:NOT EXIST.", currParam_->getType());
      detachAllModelItem();
      deleteArgEstimator();
      return ExecResult::EXEC_ERROR;
    }
    /////
    currParam_->updateActive(false);
    nextParam->updateActive(true);
    this->statemachineView_->repaint();
    DDEBUG_V("currParam : %d, nextParam : %d", currParam_->getType(), nextParam->getType());

    currParam_ = nextParam;
    if (nextParam->getType() == ELEMENT_FINAL) {
      prevTask_ = currentTask_;
      currentTask_ = currentTask_->getNextTask();
      //
      if (currentTask_) {
        prepareTask();
        return ExecResult::EXEC_BREAK;
      }
      break;
    }
    if (currParam_->getType() == ELEMENT_COMMAND) return ExecResult::EXEC_BREAK;
  }
  return ExecResult::EXEC_FINISHED;
}
//////////
void TaskExecuteManager::parseModelInfo() {
  vector<ModelParam*> modelList = currentTask_->getModelList();
  vector<ElementStmParam*> stateList = currentTask_->getStmElementList();
  for (int index = 0; index < stateList.size(); index++) {
    ElementStmParam* state = stateList[index];
    vector<ElementStmActionParam*> actionList = state->getActionList();
    for (int idxAction = 0; idxAction < actionList.size(); idxAction++) {
      ElementStmActionParam* action = actionList[idxAction];
      std::vector<ModelParam*>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparatorByRName(action->getModel()));
      if (model == modelList.end()) continue;
      action->setModelParam(*model);
    }
  }
}

bool TaskExecuteManager::doModelAction() {
  vector<ElementStmActionParam*> actionList = currParam_->getActionList();
  for (int index = 0; index < actionList.size(); index++) {
    ElementStmActionParam* action = actionList[index];
    DDEBUG_V("Action : %s = %s, %s, %s", currParam_->getCmdName().toStdString().c_str(), action->getAction().toStdString().c_str(), action->getModel().toStdString().c_str(), action->getTarget().toStdString().c_str());
    //
    if (action->getAction() == "attach" || action->getAction() == "detach") {
      vector<ParameterParam*> paramList = currentTask_->getParameterList();
      vector<ParameterParam*>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(action->getTarget()));
      QString strVal;
      int intTarget = -1;
      if (targetParam != paramList.end()) {
        strVal = QString::fromStdString((*targetParam)->getValues(0));
        intTarget = strVal.toInt();
      }
      //
      if (action->getAction() == "attach") {
        bool ret = TaskExecutor::instance()->attachModelItem(action->getModelParam()->getModelItem(), intTarget);
        if (ret == false) {
          QMessageBox::warning(0, _("Run Task"), _("Model Attach Failed."));
          return false;
        }

      } else if (action->getAction() == "detach") {
        bool ret = TaskExecutor::instance()->detachModelItem(action->getModelParam()->getModelItem(), intTarget);
        if (ret == false) {
          QMessageBox::warning(0, _("Run Task"), _("Model Detach Failed."));
          return false;
        }
      }
    }
  }
  return true;
}

void TaskExecuteManager::prepareTask() {
  this->taskInstView->unloadCurrentModel();
  if (prevTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(prevTask_);
    if (prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
    if (currentTask_ && currentTask_->getStateParam()) currentTask_->getStateParam()->updateActive(true);
    this->flowView_->repaint();
  }
  bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);
  this->metadataView->setTaskParam(currentTask_);
  this->statemachineView_->setTaskParam(currentTask_);
  this->parameterView_->setTaskParam(currentTask_);
  if (isUpdateTree) {
    ChoreonoidUtil::showAllModelItem();
  }
  //
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();
  ChoreonoidUtil::updateScene();
  parameterView_->setInputValues();
}

void TaskExecuteManager::setOutArgument(std::vector<CompositeParamType>& parameterList) {
  for (int idxArg = 0; idxArg < currParam_->getArgList().size(); idxArg++) {
    ArgumentDefParam* argDef = currParam_->getCommadDefParam()->getArgList()[idxArg];
    if (argDef->getDirection() != 1) continue;

    QString targetStr = currParam_->getArgList()[idxArg]->getValueDesc();
    ParameterParam* targetParam = NULL;
    for (int idxParam = 0; idxParam < currentTask_->getParameterList().size(); idxParam++) {
      ParameterParam* parmParm = currentTask_->getParameterList()[idxParam];
      if (parmParm->getRName() == targetStr) {
        targetParam = parmParm;
        break;
      }
    }

    if (argDef->getType() == "double") {
      if (argDef->getLength() == 1) {
        double result = boost::get<double>(parameterList[idxArg]);
        DDEBUG_V("result : %f, %s", result, targetStr.toStdString().c_str());
        targetParam->setValues(0, QString::number(result));
      } else {
        VectorX result = boost::get<VectorX>(parameterList[idxArg]);
        DDEBUG_V("result size: %d, %s", result.size(), targetStr.toStdString().c_str());
        for (int index = 0; index < result.size(); index++) {
          targetParam->setValues(index, QString::number(result[index]));
        }
      }

    } else if (argDef->getType() == "int") {
      int result = boost::get<int>(parameterList[idxArg]);
      DDEBUG_V("result : %d, %s", result, targetStr.toStdString().c_str());
      targetParam->setValues(0, QString::number(result));

    } else if (argDef->getType() == "string") {
      string result = boost::get<string>(parameterList[idxArg]);
      DDEBUG_V("result : %s, %s", result.c_str(), targetStr.toStdString().c_str());
      targetParam->setValues(0, QString::fromStdString(result));
    }
  }
}

bool TaskExecuteManager::detachAllModelItem() {
  return TaskExecutor::instance()->detachAllModelItem();
}

void TaskExecuteManager::createArgEstimator(TaskModelParam* targetParam) {
  argHandler_ = EstimatorFactory::getInstance().createArgEstimator(targetParam);
}

void TaskExecuteManager::deleteArgEstimator() {
  EstimatorFactory::getInstance().deleteArgEstimator(argHandler_);
}

}
