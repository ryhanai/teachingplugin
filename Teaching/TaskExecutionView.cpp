#include "TaskExecutionView.h"
#include "ChoreonoidUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace cnoid;

namespace teaching {

TaskExecutionView::TaskExecutionView(QWidget* parent)
  : QWidget(parent),
  currParam_(0), currentTask_(0), isSkipCheck_(false),
  statemachineView_(0), parameterView_(0) {
}

void TaskExecutionView::unloadCurrentModel() {
  if (currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }
}

void TaskExecutionView::runFlow(FlowParam* targetFlow) {
  DDEBUG("TaskExecutionView::runFlow");
  executor_->setCurrentTask(currentTask_);
  executor_->runFlow(targetFlow);
  currentTask_ = executor_->getCurrentTask();
}

void TaskExecutionView::runSingleTask() {
  DDEBUG("TaskExecutionView::runSingleTask");
  executor_->setCurrentTask(currentTask_);
  executor_->setCurrentElement(currParam_);
  executor_->runSingleTask();
}

void TaskExecutionView::abortOperation() {
  DDEBUG("TaskExecutionView::abortOperation");
  executor_->abortOperation();
}

bool TaskExecutionView::runSingleCommand(ElementStmParam* targetStm) {
  executor_->setCurrentTask(currentTask_);
  executor_->setCurrentElement(targetStm);
  return executor_->runSingleCommand();
}

ExecResult TaskExecutionView::doOperationStep() {
  return executor_->doTaskOperationStep();
}

ExecResult TaskExecutionView::doOperationCont() {
  ExecResult ret = executor_->doTaskOperationStep();
  if (ret != ExecResult::EXEC_BREAK) return ret;
  ExecResult retFlow = executor_->doFlowOperationCont();
  if (retFlow == ExecResult::EXEC_FINISHED) {
    currentTask_ = executor_->getCurrentTask();
  }
  return retFlow;
}

ExecResult TaskExecutionView::doTaskOperation() {
  executor_->setCurrentTask(currentTask_);
  executor_->setCurrentElement(currParam_);
  return executor_->doTaskOperation();
}

bool TaskExecutionView::checkPaused() {
  if (isSkipCheck_) return true;
  if (executor_->isBreak() == false) return false;
  //
  QMessageBox::StandardButton ret = QMessageBox::question(this, _("Confirm"),
    _("Cancel pausing processing?"),
    QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::No) return true;
  //
  executor_->setBreak(false);
  return false;
}

}
