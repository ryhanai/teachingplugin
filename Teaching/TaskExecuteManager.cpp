#include <cnoid/InfoBar>

#include "TaskExecuteManager.h"
#include "StateMachineView.h"
#include "TaskInstanceView.h"
#include "TeachingUtil.h"
#include "Calculator.h"
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
		isBreak_(false) {
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
	for (int index = 0; index<stateList.size(); index++) {
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
	InfoBar::instance()->showMessage(_("Running Flow :") + targetFlow->getName());
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
		statemachineView_->setStepStatus(true);
		return;
	}

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
	InfoBar::instance()->showMessage(_("Finished Flow :") + targetFlow->getName());
}

void TaskExecuteManager::runSingleTask() {
  parameterView_->setInputValues();
	statemachineView_->setStepStatus(false);
	if (currParam_) {
    currParam_->updateActive(false);
    statemachineView_->repaint();
  }
  if( !currentTask_ ) {
    QMessageBox::warning(0, _("Run Task"), _("Please select target TASK"));
    return;
  }
  //
  if( currentTask_->checkAndOrderStateMachine()) {
    QMessageBox::warning(0, _("Run Task"), currentTask_->getErrStr());
    return;
  }
  //
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();
	QString taskName = currentTask_->getName();
	InfoBar::instance()->showMessage(_("Running Task :") + taskName);

  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
	if (doFlowOperation() == ExecResult::EXEC_BREAK) {
		statemachineView_->setStepStatus(true);
		return;
	}
	InfoBar::instance()->notify(_("Finished Task :") + taskName);
}

ExecResult TaskExecuteManager::doFlowOperation() {
	prevTask_ = 0;
	if (currentTask_) prepareTask();
	while (true) {
		if (currentTask_ == 0) break;
		
		ExecResult ret = doTaskOperation();
		if (ret != ExecResult::EXEC_FINISHED) return ret;
	}
	if (prevTask_->getStateParam()) prevTask_->getStateParam()->updateActive(false);
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

ExecResult TaskExecuteManager::doTaskOperation() {
  bool isReal = SettingManager::getInstance().getIsReal();

	//モデル情報の設定
	parseModelInfo();

  ElementStmParam* nextParam;
  bool cmdRet = false;
  std::vector<CompositeParamType> parameterList;

  while(true) {
		if (currParam_->getType() == ELEMENT_COMMAND) {
			if (currParam_->isBreak()) return ExecResult::EXEC_BREAK;
			//引数の組み立て
			if (buildArguments(parameterList) == false) {
				detachAllModelItem();
				return ExecResult::EXEC_ERROR;
			}
			//コマンド実行
      cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
      //モデルアクション実行
			if (doModelAction() == false) {
				detachAllModelItem();
				return ExecResult::EXEC_ERROR;
			}
    }
    //
    if(currParam_->getType()==ELEMENT_DECISION) {
      if(cmdRet) {
        nextParam = currParam_->getTrueElem();
      } else {
        nextParam = currParam_->getFalseElem();
      }
    } else {
      nextParam = currParam_->getNextElem();
    }
    if(!nextParam) {
      InfoBar::instance()->showMessage("");
      DDEBUG_V("currParam : %d, nextParam:NOT EXIST.", currParam_->getType());
			detachAllModelItem();
			return ExecResult::EXEC_ERROR;
    }
    if(currParam_->getType()==ELEMENT_COMMAND && nextParam->getType()!=ELEMENT_DECISION) {
      if( cmdRet==false ) {
        break;
      }
    }
    /////
    currParam_->updateActive(false);
    nextParam->updateActive(true);
    this->statemachineView_->repaint();
    DDEBUG_V("currParam : %d, nextParam : %d", currParam_->getType(), nextParam->getType());

    currParam_ = nextParam;
    if(nextParam->getType()==ELEMENT_FINAL) {
			prevTask_ = currentTask_;
			currentTask_ = currentTask_->getNextTask();
			//
			if (currentTask_) {
				prepareTask();
			}
			break;
    }
  }
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
	if (buildArguments(parameterList) == false) {
		detachAllModelItem();
		return ExecResult::EXEC_ERROR;
	}
	//コマンド実行
	cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
	//モデルアクション実行
	if (doModelAction() == false) {
		detachAllModelItem();
		return ExecResult::EXEC_ERROR;
	}
	//
	while (true) {
		if (currParam_->getType() == ELEMENT_DECISION) {
			if (cmdRet) {
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
			return ExecResult::EXEC_ERROR;
		}
		if (currParam_->getType() == ELEMENT_COMMAND && nextParam->getType() != ELEMENT_DECISION) {
			if (cmdRet == false) {
				break;
			}
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
	for (int index = 0; index<stateList.size(); index++) {
		ElementStmParam* state = stateList[index];
		vector<ElementStmActionParam*> actionList = state->getActionList();
		for (int idxAction = 0; idxAction<actionList.size(); idxAction++) {
			ElementStmActionParam* action = actionList[idxAction];
			std::vector<ModelParam*>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparatorByRName(action->getModel()));
			if (model == modelList.end()) continue;
			action->setModelParam(*model);
		}
	}
}

bool TaskExecuteManager::buildArguments(std::vector<CompositeParamType>& parameterList) {
	Calculator* calculator = new Calculator();
	parameterList.clear(); // R.Hanai
	//引数の組み立て
	for (int idxArg = 0; idxArg<currParam_->getArgList().size(); idxArg++) {
		ArgumentParam* arg = currParam_->getArgList()[idxArg];
		QString valueDesc = arg->getValueDesc();
		//
		if (currParam_->getCommadDefParam() == 0) {
			delete calculator;
			QMessageBox::warning(0, _("Run Task"), _("Target Command does NOT EXIST."));
			return false;
		}
		ArgumentDefParam* argDef = currParam_->getCommadDefParam()->getArgList()[idxArg];
		if (argDef->getType() == "double") {
			bool ret = calculator->calculate(valueDesc, currentTask_);
			if (calculator->getValMode() == VAL_SCALAR) {
				DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), calculator->getResultScalar());
				parameterList.push_back(calculator->getResultScalar());
			} else {
				DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), calculator->getResultVector()[0], calculator->getResultVector()[1], calculator->getResultVector()[2]);
				parameterList.push_back(calculator->getResultVector());
			}

		} else {
			vector<ParameterParam*> paramList = currentTask_->getParameterList();
			vector<ParameterParam*>::iterator targetParam = find_if(paramList.begin(), paramList.end(), ParameterParamComparatorByRName(valueDesc));
			QString strVal;
			if (targetParam != paramList.end()) {
				strVal = QString::fromStdString((*targetParam)->getValues(0));
			} else {
				strVal = valueDesc;
			}
			if (argDef->getType() == "int") {
				parameterList.push_back(strVal.toInt());
			} else {
				parameterList.push_back(strVal.toStdString());
			}
		}
	}
	delete calculator;
	return true;
}

bool TaskExecuteManager::doModelAction() {
	vector<ElementStmActionParam*> actionList = currParam_->getActionList();
	for (int index = 0; index<actionList.size(); index++) {
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

bool TaskExecuteManager::detachAllModelItem() {
	return TaskExecutor::instance()->detachAllModelItem();
}

}
