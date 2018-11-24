#include <cnoid/InfoBar>

#include "TaskExecuteManager.h"
#include "StateMachineView.h"
#include "TaskInstanceView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"
#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

TaskExecuteManager::TaskExecuteManager()
  : currParam_(0), currentTask_(0), prevTask_(0),
  taskInstView(0), statemachineView_(0), parameterView_(0), metadataView(0),
  isBreak_(false), isAbort_(false), lastResult_(false) {
}

TaskExecuteManager::~TaskExecuteManager() {
  DDEBUG("TaskExecuteManager Destructor");
}

void TaskExecuteManager::abortOperation() {
  isAbort_ = true;
}

void TaskExecuteManager::runFlow(FlowParamPtr targetFlow) {
  if (!targetFlow) {
    QMessageBox::warning(0, _("Run Flow"), _("Select Target Flow"));
    return;
  }
  currentFlow_ = targetFlow;
  if (currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }

	TeachingEventHandler::instance()->prv_SetInputValues();

  DDEBUG("FlowParam checkAndOrderStateMachine");
  if (targetFlow->checkAndOrderStateMachine()) {
    QMessageBox::warning(0, _("Run Flow"), targetFlow->getErrStr());
    return;
  }
  vector<ElementStmParamPtr> stateList = targetFlow->getActiveStateList();
  TaskModelParamPtr currentTask;
  for (int index = 0; index < stateList.size(); index++) {
		ElementStmParamPtr targetState = stateList[index];
    if (targetState->getType() != ELEMENT_COMMAND) continue;
    //
    currentTask = targetState->getTaskParam();
    ChoreonoidUtil::unLoadTaskModelItem(currentTask);

    TeachingUtil::loadTaskDetailData(currentTask);
    DDEBUG("TaskParam checkAndOrderStateMachine");
    if (currentTask->checkAndOrderStateMachine()) {
      QMessageBox::warning(0, _("Flow"), currentTask->getErrStr() + " [" + currentTask->getName() + "]");
      return;
    }

    if (targetState->getNextElem()->getType() == ELEMENT_DECISION) {
      ElementStmParamPtr decisionElem = targetState->getNextElem();
      currentTask->setFlowCondition(decisionElem->getCondition());

      currentTask->setTrueTask(getNextTask(decisionElem->getTrueElem()));
      currentTask->setFalseTask(getNextTask(decisionElem->getFalseElem()));

    } else {
      currentTask->setNextTask(getNextTask(targetState->getNextElem()));
    }
  }
  //
  isAbort_ = false;
	currFlowParam_ = targetFlow->getStartParam();
  currFlowParam_->updateActive(true);
  this->flowView_->repaint();
  InfoBar::instance()->showMessage(_("Running Flow :") + targetFlow->getName(), MESSAGE_PERIOD);

  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  ExecResult ret = doFlowSingleOperation();
  if (ret == ExecResult::EXEC_BREAK) {
    statemachineView_->setStepStatus(true);
    return;
  } else if (ret == ExecResult::EXEC_ERROR) {
    InfoBar::instance()->showMessage(_("Failed Flow :") + targetFlow->getName(), MESSAGE_PERIOD);
    return;
  }
  InfoBar::instance()->showMessage(_("Finished Flow :") + targetFlow->getName(), MESSAGE_PERIOD);
	DDEBUG("TaskExecuteManager::runFlow End");
}

TaskModelParamPtr TaskExecuteManager::getNextTask(ElementStmParamPtr target) {
  if (target->getType() == ELEMENT_MERGE) {
    return target->getNextElem()->getTaskParam();
  }
  return target->getTaskParam();
}


bool TaskExecuteManager::runSingleCommand() {
	DDEBUG("TaskExecuteManager::runSingleCommand");

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
  bool cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList);
  //out引数の設定
  setOutArgument(parameterList);

  deleteArgEstimator();
  InfoBar::instance()->showMessage(_("Finished Command :") + currParam_->getCmdName(), MESSAGE_PERIOD);

  return cmdRet;
}

void TaskExecuteManager::runSingleTask() {
  DDEBUG("TaskExecuteManager::runSingleTask");
	TeachingEventHandler::instance()->prv_SetInputValues();

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
  isAbort_ = false;
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();
  QString taskName = currentTask_->getName();
  InfoBar::instance()->showMessage(_("Running Task :") + taskName, MESSAGE_PERIOD);

  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  ExecResult ret = doFlowOperation(true);
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
  if (currentTask_) prepareTask(!isSingle);
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

ExecResult TaskExecuteManager::doFlowSingleOperation() {
  for(ElementStmParamPtr state : currentFlow_->getActiveStateList() ) {
    state->updateActive(false);
  }
	ElementStmParamPtr nextParam;
  prevTask_ = 0;

	DDEBUG("Start Execution");
  while (true) {
    if (currFlowParam_->getType() == ELEMENT_COMMAND) {
      prevTask_ = currentTask_;
      currentTask_ = currFlowParam_->getTaskParam();
      prepareTask(true);
      ExecResult ret = doTaskOperation(false); // R.Hanai
      if (ret != ExecResult::EXEC_FINISHED) {
        return ret;
      }
      if (isAbort_) {
        return ExecResult::EXEC_FINISHED;
      }
      nextParam = currFlowParam_->getNextElem();

    } else if (currFlowParam_->getType() == ELEMENT_DECISION) {
        QString cond = currFlowParam_->getCondition();
        DDEBUG_V("FlowCondition : %s",cond.toStdString().c_str());
        if(cond.length()==0) {
        	return ExecResult::EXEC_ERROR;
        }
        createArgEstimator(currentFlow_);
        if (argHandler_->checkFlowCondition(currentFlow_, cond.toStdString().c_str(), lastResult_)) {
          DDEBUG("TaskExecuteManager::doFlowSingleOperation TRUE");
          nextParam = currFlowParam_->getTrueElem();
        } else {
          DDEBUG("TaskExecuteManager::doFlowSingleOperation FALSE");
          nextParam = currFlowParam_->getFalseElem();
        }

    } else if (currFlowParam_->getType() == ELEMENT_MERGE) {
      nextParam = currFlowParam_->getNextElem();

    } else if (currFlowParam_->getType() == ELEMENT_FINAL) {
      break;

    } else if (currFlowParam_->getType() == ELEMENT_START) {
      nextParam = currFlowParam_->getNextElem();
    }
    //
    currFlowParam_->updateActive(false);
    nextParam->updateActive(true);
    this->flowView_->repaint();
    currFlowParam_ = nextParam;

  }
	return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doFlowOperationCont() {
	DDEBUG("TaskExecuteManager::doFlowOperationCont");
	while (true) {
    if (currentTask_ == 0) break;

    ExecResult ret = doTaskOperation();
    if (ret != ExecResult::EXEC_FINISHED) return ret;
		if (isAbort_) return ExecResult::EXEC_FINISHED;
	}
  if (prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
  this->flowView_->repaint();
  return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doTaskOperation(bool updateCurrentTask) {
	DDEBUG("TaskExecuteManager::doTaskOperation");

  //モデル情報の設定
  parseModelInfo();

	ElementStmParamPtr nextParam;
  lastResult_ = false;

  //引数計算モジュールの初期化
  createArgEstimator(currentTask_);

	DDEBUG("Start Execution");
  std::vector<CompositeParamType> parameterList;
  while (true) {
    if (currParam_->getType() == ELEMENT_COMMAND) {
      if (currParam_->isBreak()) return ExecResult::EXEC_BREAK;
      if (isAbort_) {
        detachAllModelItem();
        deleteArgEstimator();
				DDEBUG("TaskExecuteManager::doTaskOperation EXEC_FINISHED(Abort)");
				return ExecResult::EXEC_FINISHED;
      }
      //引数の組み立て
      if (argHandler_->buildArguments(currentTask_, currParam_, parameterList) == false) {
        detachAllModelItem();
        deleteArgEstimator();
				DDEBUG("TaskExecuteManager::doTaskOperation EXEC_ERROR(Arg)");
				return ExecResult::EXEC_ERROR;
      }
      //コマンド実行
      lastResult_ = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList);
      //out引数の設定
      setOutArgument(parameterList);

      //モデルアクション実行
      if (doModelAction() == false) {
        detachAllModelItem();
        deleteArgEstimator();
				DDEBUG("TaskExecuteManager::doTaskOperation EXEC_ERROR(Model Action)");
				return ExecResult::EXEC_ERROR;
      }
      //コマンドの実行結果がFalseで次の要素がデシジョンではない場合は終了
      if (lastResult_ == false) {
				ElementStmParamPtr checkNext = currParam_->getNextElem();
        if (checkNext == 0 || checkNext->getType() != ELEMENT_DECISION) {
          InfoBar::instance()->showMessage("");
          DDEBUG("NextParam is NOT Decision.");
          detachAllModelItem();
          deleteArgEstimator();
					DDEBUG("TaskExecuteManager::doTaskOperation EXEC_ERROR(Dec)");
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
      bool checkCond = argHandler_->checkCondition(lastResult_, strTarget);
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
			DDEBUG("TaskExecuteManager::doTaskOperation EXEC_ERROR");
			return ExecResult::EXEC_ERROR;
    }
    /////
    currParam_->updateActive(false);
    nextParam->updateActive(true);
    this->statemachineView_->repaint();
    DDEBUG_V("currParam : %d, nextParam : %d", currParam_->getType(), nextParam->getType());

    currParam_ = nextParam;
    if (nextParam->getType() == ELEMENT_FINAL) {
			DDEBUG("TaskExecuteManager::doTaskOperation ELEMENT_FINAL");

      // R.Hanai
      if (!updateCurrentTask) { break; }
      // R.Hanai

      prevTask_ = currentTask_;
      if (currentTask_->getNextTask()) {
        currentTask_ = currentTask_->getNextTask();
      } else {
        QString cond = currentTask_->getFlowCondition();
        DDEBUG_V("FlowCondition : %s", cond.toStdString().c_str());
        if (argHandler_->checkFlowCondition(currentFlow_, cond.toStdString().c_str(), lastResult_)) {
          DDEBUG("TaskExecuteManager::doTaskOperation TRUE");
          currentTask_ = currentTask_->getTrueTask();
        } else {
          DDEBUG("TaskExecuteManager::doTaskOperation FALSE");
          currentTask_ = currentTask_->getFalseTask();
        }
      }
      //
      if (currentTask_) {
        prepareTask(false);
      }
      break;
    }
  }
  deleteArgEstimator();
	DDEBUG("TaskExecuteManager::doTaskOperation EXEC_FINISHED");
	return ExecResult::EXEC_FINISHED;
}

ExecResult TaskExecuteManager::doTaskOperationStep() {
	DDEBUG("TaskExecuteManager::doTaskOperationStep");
  if(isBreak_==false) return ExecResult::EXEC_FINISHED;

  ElementStmParamPtr nextParam;

  if(currParam_->getType() == ELEMENT_START) {
    nextParam = currParam_->getNextElem();

    currParam_->updateActive(false);
    nextParam->updateActive(true);
    this->statemachineView_->repaint();

    currParam_ = nextParam;

    return ExecResult::EXEC_BREAK;
  }
	TeachingEventHandler::instance()->prv_SetInputValues();

  //モデル情報の設定
  parseModelInfo();

  lastResult_ = false;
  std::vector<CompositeParamType> parameterList;

  //引数の組み立て
  if (argHandler_->buildArguments(currentTask_, currParam_, parameterList) == false) {
    detachAllModelItem();
    return ExecResult::EXEC_ERROR;
  }
  //コマンド実行
  lastResult_ = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList);
  //out引数の設定
  setOutArgument(parameterList);
  //モデルアクション実行
  if (doModelAction() == false) {
    detachAllModelItem();
    return ExecResult::EXEC_ERROR;
  }
  //コマンドの実行結果がFalseで次の要素がデシジョンではない場合は終了
  if (lastResult_ == false) {
		ElementStmParamPtr checkNext = currParam_->getNextElem();
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
      bool checkCond = argHandler_->checkCondition(lastResult_, strCond.toStdString());
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
      if (currentTask_->getNextTask()) {
        currentTask_ = currentTask_->getNextTask();
      } else {
        QString cond = currentTask_->getFlowCondition();
        DDEBUG_V("FlowCondition : %s", cond.toStdString().c_str());
        if (0 == cond.length()) {
          currentTask_ = 0;
        } else {
          if (argHandler_->checkFlowCondition(currentFlow_, cond.toStdString().c_str(), lastResult_)) {
            DDEBUG("TaskExecuteManager::doTaskOperation TRUE");
            currentTask_ = currentTask_->getTrueTask();
          } else {
            DDEBUG("TaskExecuteManager::doTaskOperation FALSE");
            currentTask_ = currentTask_->getFalseTask();
          }
        }
      }
      //
      if (currentTask_) {
        prepareTask(false);
        TeachingEventHandler::instance()->setComCurrentTask(currentTask_);
        DDEBUG("TaskExecuteManager::doTaskOperation EXEC_BREAK");
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
  DDEBUG("TaskExecuteManager::parseModelInfo");
  vector<ModelParamPtr> modelList = currentTask_->getActiveModelList();
  vector<ElementStmParamPtr> stateList = currentTask_->getActiveStateList();
  for (ElementStmParamPtr state : currentTask_->getActiveStateList()) {
    for (ElementStmActionParamPtr action : state->getActiveStateActionList()) {
      std::vector<ModelParamPtr>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparatorByRName(action->getModel()));
      if (model == modelList.end()) continue;
      action->setModelParam(*model);
    }
  }
  DDEBUG("TaskExecuteManager::parseModelInfo End");
}

bool TaskExecuteManager::doModelAction() {
  for (ElementStmActionParamPtr action : currParam_->getActiveStateActionList()) {
    DDEBUG_V("Action : %s = %s, %s, %s", currParam_->getCmdName().toStdString().c_str(), action->getAction().toStdString().c_str(), action->getModel().toStdString().c_str(), action->getTarget().toStdString().c_str());
    //
    if (action->getAction() == "attach" || action->getAction() == "detach") {
      vector<ParameterParamPtr> paramList = currentTask_->getActiveParameterList();
      vector<ParameterParamPtr>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(action->getTarget()));
      QString strVal;
      int intTarget = -1;
      if (targetParam != paramList.end()) {
        strVal = QString::fromStdString((*targetParam)->getValues(0));
        intTarget = strVal.toInt();
      }
      //
      if (action->getAction() == "attach") {
        bool ret = false;
        if(action->getModelParam()->getModelMaster()) {
          ret = TaskExecutor::instance()->attachModelItem(action->getModelParam()->getModelMaster()->getModelItem(), intTarget);
        }
        if (ret == false) {
          QMessageBox::warning(0, _("Run Task"), _("Model Attach Failed."));
          return false;
        }

      } else if (action->getAction() == "detach") {
        bool ret = false;
        if(action->getModelParam()->getModelMaster()) {
          ret = TaskExecutor::instance()->detachModelItem(action->getModelParam()->getModelMaster()->getModelItem(), intTarget);
        }
        if (ret == false) {
          QMessageBox::warning(0, _("Run Task"), _("Model Detach Failed."));
          return false;
        }
      }
    }
  }
  return true;
}

void TaskExecuteManager::prepareTask(bool isFlow) {
	DDEBUG("TaskExecuteManager::prepareTask");

	if (prevTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(prevTask_);
    if (prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
    if (currentTask_ && currentTask_->getStateParam()) currentTask_->getStateParam()->updateActive(true);
    this->flowView_->repaint();
  }
  bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);

	this->metadataView->setTaskParam(currentTask_);
	this->statemachineView_->setTaskParam(currentTask_);
	this->parameterView_->setTaskParam(currentTask_, isFlow);
  if (isUpdateTree) {
    ChoreonoidUtil::showAllModelItem();
  }
  //
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();
  ChoreonoidUtil::updateScene();
}

void TaskExecuteManager::setOutArgument(std::vector<CompositeParamType>& parameterList) {
  DDEBUG("TaskExecuteManager::setOutArgument");
  for (int idxArg = 0; idxArg < currParam_->getArgList().size(); idxArg++) {
    ArgumentDefParam* argDef = currParam_->getCommadDefParam()->getArgList()[idxArg];
    if (argDef->getDirection() != 1) continue;

    QString targetStr = currParam_->getArgList()[idxArg]->getValueDesc();
    DDEBUG_V("targetStr : %s", targetStr.toStdString().c_str());
    ParameterParamPtr targetParam = NULL;
    for( ParameterParamPtr parmParm : currentTask_->getActiveParameterList()) {
      if (parmParm->getRName() == targetStr) {
        targetParam = parmParm;
        break;
      }
    }

    if (argDef->getType() == "double") {
      if (argDef->getLength() == 1) {
        double result = boost::get<double>(parameterList[idxArg]);
        DDEBUG_V("result : %f, %s", result, targetStr.toStdString().c_str());
        targetParam->setOutValues(0, QString::number(result));
      } else {
        VectorX result = boost::get<VectorX>(parameterList[idxArg]);
        DDEBUG_V("result size: %d, %s", result.size(), targetStr.toStdString().c_str());
        for (int index = 0; index < result.size(); index++) {
          targetParam->setOutValues(index, QString::number(result[index]));
        }
      }

    } else if (argDef->getType() == "int") {
      int result = boost::get<int>(parameterList[idxArg]);
      DDEBUG_V("result : %d, %s", result, targetStr.toStdString().c_str());
      targetParam->setOutValues(0, QString::number(result));

    } else if (argDef->getType() == "string") {
      string result = boost::get<string>(parameterList[idxArg]);
      DDEBUG_V("result : %s, %s", result.c_str(), targetStr.toStdString().c_str());
      targetParam->setOutValues(0, QString::fromStdString(result));
    }
    targetParam->updateOutValues();
  }
}

bool TaskExecuteManager::detachAllModelItem() {
  return TaskExecutor::instance()->detachAllModelItem();
}

void TaskExecuteManager::createArgEstimator(TaskModelParamPtr targetParam) {
  argHandler_ = EstimatorFactory::getInstance().createArgEstimator(targetParam);
}

void TaskExecuteManager::createArgEstimator(FlowParamPtr targetParam) {
  argHandler_ = EstimatorFactory::getInstance().createArgEstimator(targetParam);
}

void TaskExecuteManager::deleteArgEstimator() {
  DDEBUG("TaskExecuteManager::deleteArgEstimator");
  EstimatorFactory::getInstance().deleteArgEstimator(argHandler_);
}

}
