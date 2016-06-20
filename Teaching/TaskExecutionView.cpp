#include "TaskExecutionView.h"
#include "ChoreonoidUtil.h"
#include "TeachingUtil.h"
#include "Calculator.h"
#include "TaskExecutor.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

TaskExecutionView::TaskExecutionView(QWidget* parent)
   : QWidget(parent), currParam_(0), currentTask_(0),
     statemachineView_(0), parameterView_(0) {
}

void TaskExecutionView::unloadCurrentModel() {
  if(currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }
}

void TaskExecutionView::runSingleTask(bool isReal) {
  parameterView_->setInputValues();
  if( currParam_ ) {
    currParam_->updateActive(false);
    statemachineView_->repaint();
  }
  if( !currentTask_ ) {
    QMessageBox::warning(this, tr("Run Task"), "Please select target TASK");
    return;
  }
  //
  if( currentTask_->checkAndOrderStateMachine()) {
    QMessageBox::warning(this, tr("Run Task"), currentTask_->getErrStr());
    return;
  }
  //
  currParam_ = currentTask_->getStartParam();
  currParam_->updateActive(true);
  statemachineView_->repaint();

  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  doTaskOperation(currentTask_, isReal);
  QMessageBox::information(this, tr("Run Task"), tr("Target Task is FINISHED."));
}

bool TaskExecutionView::doTaskOperation(TaskModelParam* targetTask, bool isReal) {
  bool result = false;
  //ƒ‚ƒfƒ‹î•ñ‚Ìİ’è
  vector<ModelParam*> modelList = currentTask_->getModelList();
  vector<ElementStmParam*> stateList = currentTask_->getStmElementList();
  for(int index=0; index<stateList.size(); index++) {
    ElementStmParam* state = stateList[index];
    vector<ElementStmActionParam*> actionList = state->getActionList();
    for(int idxAction=0; idxAction<actionList.size(); idxAction++) {
      ElementStmActionParam* action = actionList[idxAction];
      std::vector<ModelParam*>::iterator model = std::find_if( modelList.begin(), modelList.end(), ModelParamComparatorByRName(action->getModel()));
      if(model== modelList.end()) continue;
      action->setModelParam(*model);
    }
  }
  Calculator* calculator = new Calculator();
  ElementStmParam* nextParam;
  bool cmdRet = false;
  std::vector<CompositeParamType> parameterList;
  while(true) {
    if(currParam_->getType()==ELEMENT_COMMAND) {
      parameterList.clear(); // R.Hanai
      //ˆø”‚Ì‘g‚İ—§‚Ä
      for(int idxArg=0; idxArg<currParam_->getArgList().size();idxArg++) {
        ArgumentParam* arg = currParam_->getArgList()[idxArg];
        QString valueDesc = arg->getValueDesc();
        //
        if(currParam_->getCommadDefParam()==0) {
          delete calculator;
          QMessageBox::warning(this, tr("Run Task"), tr("Target Comannd is NOT EXIST."));
          return false;
        }
        ArgumentDefParam* argDef = currParam_->getCommadDefParam()->getArgList()[idxArg];
        if(argDef->getType() == "double") {
          bool ret = calculator->calculate(valueDesc, currentTask_);
          if(calculator->getValMode()==VAL_SCALAR) {
            DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), calculator->getResultScalar());
            parameterList.push_back(calculator->getResultScalar());
          } else {
            DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), calculator->getResultVector()[0], calculator->getResultVector()[1], calculator->getResultVector()[2]);
            parameterList.push_back(calculator->getResultVector());
          }

        } else {
          vector<ParameterParam*> paramList = currentTask_->getParameterList();
          vector<ParameterParam*>::iterator targetParam = find_if( paramList.begin(), paramList.end(), ParameterParamComparatorByRName(valueDesc));
          QString strVal;
          if(targetParam != paramList.end()) {
            strVal = QString::fromStdString( (*targetParam)->getValues(0) );
          } else {
            strVal = valueDesc;
          }
          if(argDef->getType() == "int") {
            parameterList.push_back(strVal.toInt());
          } else {
            parameterList.push_back(strVal.toStdString());
          }
        }
      }
      cmdRet = TaskExecutor::instance()->executeCommand(currParam_->getCmdName().toStdString(), parameterList, isReal);
      //
      vector<ElementStmActionParam*> actionList = currParam_->getActionList();
      for(int index=0; index<actionList.size(); index++) {
        ElementStmActionParam* action = actionList[index];
        DDEBUG_V("Action : %s = %s, %s, %s", currParam_->getCmdName().toStdString().c_str(), action->getAction().toStdString().c_str(), action->getModel().toStdString().c_str(), action->getTarget().toStdString().c_str());
        //
        if(action->getAction() == "attach" || action->getAction() == "detach") {
          vector<ParameterParam*> paramList = currentTask_->getParameterList();
          vector<ParameterParam*>::iterator targetParam = find_if( paramList.begin(), paramList.end(), ParameterParamComparatorByRName(action->getTarget()));
          QString strVal;
          int intTarget = -1;
          if(targetParam != paramList.end()) {
            strVal = QString::fromStdString( (*targetParam)->getValues(0) );
            intTarget = strVal.toInt();
          }
          //
          if(action->getAction() == "attach") {
            bool ret = TaskExecutor::instance()->attachModelItem(action->getModelParam()->getModelItem(), intTarget);
            if(ret==false) {
              delete calculator;
              QMessageBox::warning(this, tr("Run Task"), tr("Model Attach Failed."));
              return false;
            }

          } else if(action->getAction() == "detach") {
            bool ret = TaskExecutor::instance()->detachModelItem(action->getModelParam()->getModelItem(), intTarget);
            if(ret==false) {
              delete calculator;
              QMessageBox::warning(this, tr("Run Task"), tr("Model Detach Failed."));
              return false;
            }
          }
        }
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
      DDEBUG_V("currParam : %d, nextParam:NOT EXIST.", currParam_->getType());
      return false;
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
      break;
    }
  }
  delete calculator;
  return true;
}

}
