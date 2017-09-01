#include <cnoid/InfoBar>

#include "FlowView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"
#include "FlowSearchDialog.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

TaskInfoDialog::TaskInfoDialog(TaskModelParam* param, ElementNode* elem, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetTask_ = param;
  this->targetElem_ = elem;
  //
  txtName = new QLineEdit;

  QFrame* frmButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
  QPushButton* btnCancel = new QPushButton(_("Cancel"));
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addWidget(btnCancel);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(txtName);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Task Instance Name"));
  resize(400, 100);
  //
  txtName->setText(targetTask_->getName());
  txtName->setSelection(0, targetTask_->getName().length());
}

void TaskInfoDialog::oKClicked() {
  DDEBUG("TaskInfoDialog::oKClicked()");

  QString strName = txtName->text();
  if (strName.trimmed().length() == 0) {
    QMessageBox::warning(this, _("TaskInstance"), _("Please input Task Instance Name."));
    txtName->setFocus();
    txtName->setSelection(0, strName.length());
    return;
  }
  targetTask_->setName(strName);
  targetTask_->setUpdate();
  targetElem_->setItemText(strName);
  //
  close();
}

void TaskInfoDialog::cancelClicked() {
  DDEBUG("TaskInfoDialog::cancelClicked()");

  close();
}
//////////
FlowViewImpl::FlowViewImpl(QWidget* parent)
  : TaskExecutionView(parent), currentFlow_(0) {
  QFrame* flowFrame = new QFrame;
  QLabel* lblName = new QLabel(_("Flow Name:"));
  leName = new QLineEdit;
  leName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Flow"));

  btnNewFlow = new QPushButton();
  btnNewFlow->setIcon(QIcon(":/Teaching/icons/Create.png"));
  btnNewFlow->setToolTip(_("Create New Flow"));

  btnRegistFlow = new QPushButton();
  btnRegistFlow->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnRegistFlow->setToolTip(_("Regist Current Flow"));

  QLabel* lblComment = new QLabel(_("Comment:"));
  leComment = new QLineEdit;

  QGridLayout* flowLayout = new QGridLayout;
  flowFrame->setLayout(flowLayout);
  flowLayout->addWidget(lblName, 0, 0, 1, 1, Qt::AlignRight);
  flowLayout->addWidget(leName, 0, 1, 1, 1);
  flowLayout->addWidget(btnSearch, 0, 2, 1, 1);
  flowLayout->addWidget(btnNewFlow, 0, 3, 1, 1);
  flowLayout->addWidget(btnRegistFlow, 0, 4, 1, 1);
  flowLayout->addWidget(lblComment, 1, 0, 1, 1, Qt::AlignRight);
  flowLayout->addWidget(leComment, 1, 1, 1, 5);
  //
  btnTrans = new QPushButton();
  btnTrans->setText("Transition");
  btnTrans->setCheckable(true);
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawLine(0, 15, 30, 15);
  painter->drawLine(29, 15, 20, 5);
  painter->drawLine(29, 15, 20, 25);
  btnTrans->setIconSize(QSize(30, 30));
  btnTrans->setIcon(QIcon(*pix));
  btnTrans->setStyleSheet("text-align:left;");
  btnTrans->setEnabled(false);

  lstItem = new ItemList(QString::fromStdString("application/TaskInstanceItem"));
  lstItem->setStyleSheet("background-color: rgb( 212, 206, 199 )};");
  lstItem->setEnabled(false);
  lstItem->createInitialNodeTarget();
  lstItem->createFinalNodeTarget();
  //lstItem->createDecisionNodeTarget();
  //createForkNodeTarget();

  grhStateMachine = new FlowActivityEditor(this, this);
  grhStateMachine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  grhStateMachine->setStyleSheet("background-color: white;");
  grhStateMachine->setAcceptDrops(true);
  grhStateMachine->setEnabled(false);
  setAcceptDrops(true);

  //QLabel* lblGuard = new QLabel("Guard:");
  //rdTrue = new QRadioButton("True");
  //rdFalse = new QRadioButton("False");

  //btnSet = new QPushButton(_("Set"));
  //btnSet->setIcon(QIcon(":/Teaching/icons/Logout.png"));
  //btnSet->setToolTip(_("Set Guard condition"));

  //rdTrue->setEnabled(false);
  //rdFalse->setEnabled(false);
  //btnSet->setEnabled(false);

  QVBoxLayout* itemLayout = new QVBoxLayout;
  itemLayout->setContentsMargins(0, 0, 0, 0);
  itemLayout->addWidget(btnTrans);
  itemLayout->addWidget(lstItem);
  QFrame* frmItem = new QFrame;
  frmItem->setLayout(itemLayout);

  //QHBoxLayout* guardLayout = new QHBoxLayout;
  //guardLayout->addStretch();
  //guardLayout->addWidget(lblGuard);
  //guardLayout->addWidget(rdTrue);
  //guardLayout->addWidget(rdFalse);
  //guardLayout->addWidget(btnSet);
  //frmGuard = new QFrame;
  //frmGuard->setLayout(guardLayout);

  QVBoxLayout* editorLayout = new QVBoxLayout;
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->addWidget(grhStateMachine);
  //	editorLayout->addWidget(frmGuard);
  QFrame* editorFrame = new QFrame;
  editorFrame->setLayout(editorLayout);

  QSplitter* splBase = new QSplitter(Qt::Horizontal);
  splBase->addWidget(frmItem);
  splBase->addWidget(editorFrame);
  splBase->setStretchFactor(0, 0);
  splBase->setStretchFactor(1, 1);
  splBase->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  splBase->show();
  //
  QFrame* frmTask = new QFrame;
  btnDeleteTask = new QPushButton(_("Delete"));
  btnDeleteTask->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteTask->setToolTip(_("Delete selected Task"));

  btnEdit = new QPushButton(_("Edit"));
  btnEdit->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnEdit->setToolTip(_("Edit target task"));

  btnExport = new QPushButton(_("Export"));
  btnExport->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnExport->setToolTip(_("Export Flow"));

  btnImport = new QPushButton(_("Import"));
  btnImport->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnImport->setToolTip(_("Import Flow"));

  btnRunFlow = new QPushButton(_("Flow"));
  btnRunFlow->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunFlow->setToolTip(_("Run Flow"));

  btnRunTask = new QPushButton(_("Task"));
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(_("Run selected Task"));

  btnAbort = new QPushButton(_("Abort"));
  //btnAbort->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnAbort->setToolTip(_("Abort Operation"));
  btnAbort->setEnabled(false);

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(_("Reset models position"));

  QHBoxLayout* taskLayout = new QHBoxLayout;
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(btnDeleteTask);
  taskLayout->addStretch();
  taskLayout->addWidget(btnEdit);
  taskLayout->addStretch();
  taskLayout->addWidget(btnImport);
  taskLayout->addWidget(btnExport);
  taskLayout->addStretch();
  taskLayout->addWidget(btnRunFlow);
  taskLayout->addWidget(btnRunTask);
  taskLayout->addWidget(btnAbort);
  taskLayout->addWidget(btnInitPos);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(flowFrame);
  mainLayout->addWidget(splBase);
  mainLayout->addWidget(frmTask);

  setLayout(mainLayout);
  //
  changeEnables(false);
  //
  connect(btnTrans, SIGNAL(toggled(bool)), this, SLOT(modeChanged()));
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(btnNewFlow, SIGNAL(clicked()), this, SLOT(newFlowClicked()));
  connect(btnRegistFlow, SIGNAL(clicked()), this, SLOT(registFlowClicked()));
  connect(btnDeleteTask, SIGNAL(clicked()), this, SLOT(deleteTaskClicked()));
  connect(btnRunFlow, SIGNAL(clicked()), this, SLOT(runFlowClicked()));
  connect(btnRunTask, SIGNAL(clicked()), this, SLOT(runTaskClicked()));
  connect(btnAbort, SIGNAL(clicked()), this, SLOT(abortClicked()));
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
  connect(btnExport, SIGNAL(clicked()), this, SLOT(exportFlowClicked()));
  connect(btnImport, SIGNAL(clicked()), this, SLOT(importFlowClicked()));
}
//
void FlowViewImpl::setButtonEnableMode(bool isEnable) {
  btnSearch->setEnabled(isEnable);
  btnNewFlow->setEnabled(isEnable);
  btnRegistFlow->setEnabled(isEnable);
  btnDeleteTask->setEnabled(isEnable);
  btnRunFlow->setEnabled(isEnable);
  btnRunTask->setEnabled(isEnable);
  btnInitPos->setEnabled(isEnable);
  btnEdit->setEnabled(isEnable);
  btnImport->setEnabled(isEnable);
  btnExport->setEnabled(isEnable);
  btnAbort->setEnabled(!isEnable);
}

void FlowViewImpl::searchClicked() {
  if (checkPaused()) return;
  DDEBUG("FlowViewImpl::searchClicked()");

  statemachineView_->setStepStatus(false);
  //
  FlowSearchDialog dialog(currentFlow_, this);
  dialog.exec();
  DDEBUG_V("FlowSearchDialog %d, %d", dialog.IsOK(), dialog.IsDeleted());
  if (dialog.IsDeleted()) {
    if (currentFlow_) {
      this->taskInstView->unloadCurrentModel();
      if (currentTask_) {
        ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
      }
      currentTask_ = 0;
      currParam_ = 0;
      currentFlow_ = 0;
      leName->setText("");
      leComment->setText("");
      grhStateMachine->setFlowParam(0);
      grhStateMachine->removeAll();
      changeEnables(false);
      this->metadataView->clearTaskParam();
      this->statemachineView_->clearTaskParam();
      this->parameterView_->clearTaskParam();
    }
    return;
  }
  //
  if (dialog.IsOK() == false) return;
  //
  int selected = dialog.getSelectedIndex();
  DDEBUG_V("selected : %d", selected);
  if (currentFlow_) {
    this->taskInstView->unloadCurrentModel();
    if (currentTask_) {
      ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    }
    currentTask_ = 0;
    currParam_ = 0;
  }
  currentFlow_ = DatabaseManager::getInstance().getFlowParamById(selected);
  if (currentFlow_ == 0) {
    QMessageBox::warning(this, _("Flow"), _("FAILED to Open Flow."));
    return;
  }
  changeEnables(true);
  leName->setText(currentFlow_->getName());
  leComment->setText(currentFlow_->getComment());
  grhStateMachine->setFlowParam(currentFlow_);
  grhStateMachine->createStateMachine(currentFlow_);
}

void FlowViewImpl::newFlowClicked() {
  if (checkPaused()) return;
  DDEBUG("FlowViewImpl::newFlowClicked()");

  statemachineView_->setStepStatus(false);
  //
  if (!currentFlow_) delete currentFlow_;
  currentFlow_ = new FlowParam(NULL_ID, "", "", "", "");
  currentFlow_->setNew();

  leName->setText("");
  leComment->setText("");
  changeEnables(true);

  grhStateMachine->setFlowParam(currentFlow_);
  leName->setFocus();
}

void FlowViewImpl::registFlowClicked() {
  if (checkPaused()) return;
  DDEBUG("FlowViewImpl::registFlowClicked()");

  statemachineView_->setStepStatus(false);
  //
  if (currentFlow_) {
    QString strName = leName->text();
    if (currentFlow_->getName() != strName) {
      currentFlow_->setName(strName);
    }
    QString strComment = leComment->text();
    if (currentFlow_->getComment() != strComment) {
      currentFlow_->setComment(strComment);
    }
    //
    bool isChanged = false;
    for (int idxTask = 0; idxTask < currentFlow_->getStmElementList().size(); idxTask++) {
      ElementStmParam* state = currentFlow_->getStmElementList()[idxTask];
      TaskModelParam* task = state->getTaskParam();
      if (task) {
        for (int index = 0; index < task->getModelList().size(); index++) {
          ModelParam* model = currentTask_->getModelList()[index];
          if (model->isChangedPosition() == false) continue;
          isChanged = true;
          break;
        }
        if (isChanged) break;
      }
    }
    if (isChanged) {
      QMessageBox::StandardButton ret = QMessageBox::question(this, _("Confirm"),
        _("Model Position was changed. Continue?"),
        QMessageBox::Yes | QMessageBox::No);
      if (ret == QMessageBox::No) return;
    }
    //
    if (DatabaseManager::getInstance().saveFlowModel(currentFlow_)) {
      currentFlow_ = DatabaseManager::getInstance().getFlowParamById(currentFlow_->getId());
      QMessageBox::information(this, _("Database"), _("Database updated"));
      grhStateMachine->setFlowParam(currentFlow_);
      grhStateMachine->createStateMachine(currentFlow_);

    } else {
      QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void FlowViewImpl::deleteTaskClicked() {
  if (checkPaused()) return;

  DDEBUG("FlowViewImpl::deleteTaskClicked()");
  statemachineView_->setStepStatus(false);
  grhStateMachine->deleteCurrent();
  currentFlow_->setUpdate();
}

void FlowViewImpl::modeChanged() {
  DDEBUG("FlowViewImpl::modeChanged()");

  grhStateMachine->setCntMode(btnTrans->isChecked());
}

void FlowViewImpl::runFlowClicked() {
  DDEBUG("FlowViewImpl::runFlowClicked()");
  runFlow(currentFlow_);
}

void FlowViewImpl::runTaskClicked() {
  DDEBUG("FlowViewImpl::runTaskClicked()");
  runSingleTask();
}

void FlowViewImpl::abortClicked() {
  DDEBUG("FlowViewImpl::abortClicked");
  abortOperation();
}

void FlowViewImpl::initPosClicked() {
  if (!currentFlow_) return;
  if (checkPaused()) return;

  DDEBUG("FlowViewImpl::initPosClicked()");
  statemachineView_->setStepStatus(false);
  //
  for (int idxState = 0; idxState < currentFlow_->getStmElementList().size(); idxState++) {
    ElementStmParam* targetState = currentFlow_->getStmElementList()[idxState];
    if (targetState->getType() == ELEMENT_COMMAND) {
      TaskModelParam* task = targetState->getTaskParam();
      for (int index = 0; index < task->getModelList().size(); index++) {
        task->getModelList()[index]->setInitialPos();
      }
    }
  }
  executor_->detachAllModelItem();
}

void FlowViewImpl::changeEnables(bool value) {
  leName->setEnabled(value);
  leComment->setEnabled(value);
  btnRegistFlow->setEnabled(value);
  //
  btnTrans->setEnabled(value);
  lstItem->setEnabled(value);
  grhStateMachine->removeAll();
  grhStateMachine->setEnabled(value);
  //rdTrue->setEnabled(false);
  //rdFalse->setEnabled(false);
  //btnSet->setEnabled(false);
  //
  btnDeleteTask->setEnabled(value);
  btnRunFlow->setEnabled(value);
  btnRunTask->setEnabled(value);
  btnInitPos->setEnabled(value);
  btnEdit->setEnabled(value);
  btnExport->setEnabled(value);
}

void FlowViewImpl::flowSelectionChanged(TaskModelParam* target) {
  if (checkPaused()) return;
  DDEBUG("FlowViewImpl::flowSelectionChanged()");
  statemachineView_->setStepStatus(false);
  //
  this->taskInstView->unloadCurrentModel();
  if (currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }
  //
  currentTask_ = target;
  if (currentTask_) {
    if (currentTask_->IsLoaded() == false) {
      TeachingUtil::loadTaskDetailData(currentTask_);
    }
    bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);

    this->metadataView->setTaskParam(currentTask_);
    this->statemachineView_->setTaskParam(currentTask_);
    this->parameterView_->setTaskParam(currentTask_);
    //即更新を行うとエラーになってしまうため
    if (isUpdateTree) {
      ChoreonoidUtil::showAllModelItem();
    }

  } else {
    this->metadataView->clearTaskParam();
    this->statemachineView_->clearTaskParam();
    this->parameterView_->clearTaskParam();
  }
}

void FlowViewImpl::editClicked() {
  DDEBUG("FlowViewImpl::editClicked()");

  ElementNode* target = grhStateMachine->getCurrentNode();
  if (target) {
    ElementStmParam* targetStm = target->getElemParam();
    if (targetStm->getType() != ELEMENT_COMMAND) {
      QMessageBox::warning(this, _("TaskInstance"), _("Please select Task Instance Node. : ") + QString::number(targetStm->getType()));
      return;
    }
    TaskInfoDialog dialog(targetStm->getTaskParam(), target, this);
    dialog.exec();
  }
}

void FlowViewImpl::exportFlowClicked() {
  if (checkPaused()) return;

  DDEBUG("FlowViewImpl::exportFlowClicked()");
  statemachineView_->setStepStatus(false);
  //
  if (!currentFlow_) {
    QMessageBox::warning(this, _("Export Flow"), _("Please select target FLOW"));
    return;
  }
  //
  QFileDialog::Options options;
  QString strSelectedFilter;
  QString strFName = QFileDialog::getSaveFileName(
    this, tr("FlowModel File"), ".",
    tr("YAML(*.yaml);;all(*.*)"),
    &strSelectedFilter, options);
  if (strFName.isEmpty()) return;
  //
  QString strName = leName->text();
  if (currentFlow_->getName() != strName) {
    currentFlow_->setName(strName);
  }
  QString strComment = leComment->text();
  if (currentFlow_->getComment() != strComment) {
    currentFlow_->setComment(strComment);
  }
  if (TeachingUtil::exportFlow(strFName, currentFlow_)) {
    QMessageBox::information(this, _("Export Flow"), _("target FLOW exported"));
  } else {
    QMessageBox::warning(this, _("Export Flow"), _("target FLOW export FAILED"));
  }

}

void FlowViewImpl::importFlowClicked() {
  if (checkPaused()) return;

  DDEBUG("FlowViewImpl::importFlowClicked()");
  statemachineView_->setStepStatus(false);

  QString strFName = QFileDialog::getOpenFileName(
    this, "TaskFlow File", ".", "YAML(*.yaml);;all(*.*)");
  if (strFName.isEmpty()) return;
  //
  vector<FlowParam*> flowModelList;
  if (TeachingUtil::importFlow(strFName, flowModelList) == false) {
    QMessageBox::warning(this, _("Import Flow"), _("FLOW import FAILED"));
    return;
  }
  if (flowModelList.size() == 0) {
    QMessageBox::warning(this, _("Import Flow"), _("FLOW import FAILED"));
    return;
  }
  currentFlow_ = flowModelList[0];
  if (DatabaseManager::getInstance().saveFlowModel(currentFlow_) == false) {
    QMessageBox::warning(this, _("Import Flow"), _("FLOW save FAILED"));
    return;
  }
  currentFlow_ = DatabaseManager::getInstance().getFlowParamById(currentFlow_->getId());

  changeEnables(true);
  leName->setText(currentFlow_->getName());
  leComment->setText(currentFlow_->getComment());
  grhStateMachine->setFlowParam(currentFlow_);
  grhStateMachine->createStateMachine(currentFlow_);

  QMessageBox::information(this, _("Import Flow"), _(" FLOW imported"));
}
/////
FlowView::FlowView() : viewImpl(0) {
  setName(_("FlowModel"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new FlowViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::CENTER);
}

FlowView::~FlowView() {
};

}
