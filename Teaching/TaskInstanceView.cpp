#include <cnoid/UTF8>
#include "TaskInstanceView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include <cnoid/InverseKinematics>
#include <cnoid/EigenUtil>
#include <cnoid/ItemTreeView>
#include "TaskExecutor.h"
#include "ControllerManager.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

TaskInstanceViewImpl::TaskInstanceViewImpl(QWidget* parent)
  : TaskExecutionView(parent),
  currentTaskIndex_(-1), m_item_(0),
  flowView_(0), metadataView_(0),
  isSkip_(false) {

  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(_("Condition:"));
  leCond = new QLineEdit;
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Task"));

  QPushButton* btnSetting = new QPushButton();
  btnSetting->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnSetting->setToolTip(_("Setting"));

  QLabel* lblTaskName = new QLabel(_("Task Name:"));
  leTask = new QLineEdit;
  //
  QGridLayout* topLayout = new QGridLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond, 0, 0, 1, 1, Qt::AlignRight);
  topLayout->addWidget(leCond, 0, 1, 1, 1);
  topLayout->addWidget(btnSearch, 0, 2, 1, 1);
  topLayout->addWidget(btnSetting, 0, 3, 1, 1);
  topLayout->addWidget(lblTaskName, 1, 0, 1, 1, Qt::AlignRight);
  topLayout->addWidget(leTask, 1, 1, 1, 3);
  //
  lstResult = new SearchList(0, 4);
  lstResult->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstResult->setSelectionMode(QAbstractItemView::SingleSelection);
  lstResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lstResult->verticalHeader()->setVisible(false);
  lstResult->setColumnWidth(0, 200);
  lstResult->setColumnWidth(1, 400);
  lstResult->setColumnWidth(2, 150);
  lstResult->setColumnWidth(3, 150);
  lstResult->setRowCount(0);
  lstResult->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");
  //
  //
  QFrame* frmButtons = new QFrame;
  btnRunTask = new QPushButton(_("Task"));
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(_("Run Task"));

  btnAbort = new QPushButton(_("Abort"));
  //btnAbort->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnAbort->setToolTip(_("Abort Operation"));
  btnAbort->setEnabled(false);

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(_("Reset Models Position"));

  btnLoadTask = new QPushButton();
  btnLoadTask->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnLoadTask->setToolTip(_("Load Task"));

  btnOutputTask = new QPushButton();
  btnOutputTask->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnOutputTask->setToolTip(_("Output Task"));

  btnDeleteTask = new QPushButton();
  btnDeleteTask->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteTask->setToolTip(_("Delete Task"));

  btnRegistNewTask = new QPushButton();
  btnRegistNewTask->setIcon(QIcon(":/Teaching/icons/Create.png"));
  btnRegistNewTask->setToolTip(_("Regist as New Task"));

  btnRegistTask = new QPushButton();
  btnRegistTask->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnRegistTask->setToolTip(_("Update Current Task"));
  //
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(btnRunTask);
  buttonLayout->addWidget(btnAbort);
  buttonLayout->addWidget(btnInitPos);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnLoadTask);
  buttonLayout->addWidget(btnOutputTask);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDeleteTask);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnRegistNewTask);
  buttonLayout->addWidget(btnRegistTask);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(condFrame);
  mainLayout->addWidget(lstResult);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(lstResult, SIGNAL(itemSelectionChanged()), this, SLOT(taskSelectionChanged()));
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(leCond, SIGNAL(editingFinished()), this, SLOT(searchClicked()));
  connect(btnSetting, SIGNAL(clicked()), this, SLOT(settingClicked()));
  //
  connect(btnRunTask, SIGNAL(clicked()), this, SLOT(runTaskClicked()));
  connect(btnAbort, SIGNAL(clicked()), this, SLOT(abortClicked()));
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
  connect(btnLoadTask, SIGNAL(clicked()), this, SLOT(loadTaskClicked()));
  connect(btnOutputTask, SIGNAL(clicked()), this, SLOT(outputTaskClicked()));
  connect(btnDeleteTask, SIGNAL(clicked()), this, SLOT(deleteTaskClicked()));
  connect(btnRegistNewTask, SIGNAL(clicked()), this, SLOT(registNewTaskClicked()));
  connect(btnRegistTask, SIGNAL(clicked()), this, SLOT(registTaskClicked()));
  //
  ControllerManager::instance()->setTaskInstanceView(this);
}

void TaskInstanceViewImpl::loadTaskInfo() {
  if (DatabaseManager::getInstance().connectDB()) {
    vector<string> condList;
    taskList_ = DatabaseManager::getInstance().searchTaskModels(condList, false);
    showGrid();
  }
}

void TaskInstanceViewImpl::setButtonEnableMode(bool isEnable) {
  btnRunTask->setEnabled(isEnable);
  btnInitPos->setEnabled(isEnable);
  btnLoadTask->setEnabled(isEnable);
  btnOutputTask->setEnabled(isEnable);
  btnDeleteTask->setEnabled(isEnable);
  btnRegistNewTask->setEnabled(isEnable);
  btnRegistTask->setEnabled(isEnable);
  btnAbort->setEnabled(!isEnable);
}

void TaskInstanceViewImpl::taskSelectionChanged() {
  DDEBUG("TaskInstanceViewImpl::taskSelectionChanged()");

  if (checkPaused()) {
    isSkipCheck_ = true;
    lstResult->setCurrentCell(currentTaskIndex_, 0);
    isSkipCheck_ = false;
    return;
  }
  statemachineView_->setStepStatus(false);
  //
  this->metadataView_->updateTaskParam();
  this->flowView_->unloadCurrentModel();
  if (currentTask_) {
    QString strTask = leTask->text();
    if (currentTask_->getName() != strTask) {
      currentTask_->setName(strTask);
    }
    ChoreonoidUtil::deselectTreeItem();
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    lstResult->item(currentTaskIndex_, 0)->setText(currentTask_->getName());
    lstResult->item(currentTaskIndex_, 1)->setText(currentTask_->getComment());
    lstResult->item(currentTaskIndex_, 2)->setText(currentTask_->getCreatedDate());
    lstResult->item(currentTaskIndex_, 3)->setText(currentTask_->getLastUpdatedDate());
  }
  if (isSkip_) return;

  currentTaskIndex_ = lstResult->currentRow();
  currentTask_ = DatabaseManager::getInstance().getTaskModel(currentTaskIndex_);
  bool isUpdateTree = false;
  if (currentTask_) {
    if (currentTask_->IsLoaded() == false) {
      TeachingUtil::loadTaskDetailData(currentTask_);
    }
    isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);
  }

  leTask->setText(currentTask_->getName());
  this->metadataView_->setTaskParam(currentTask_);
  this->statemachineView_->setTaskParam(currentTask_);
  this->parameterView_->setTaskParam(currentTask_);
  //即更新を行うとエラーになってしまうため
  if (currentTask_) {
    if (isUpdateTree) {
      ChoreonoidUtil::showAllModelItem();
    }
  }
}

void TaskInstanceViewImpl::searchClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::searchClicked()");
  statemachineView_->setStepStatus(false);
  //
  if (ControllerManager::instance()->isExistController() == false) {
    QMessageBox::warning(this, _("Task Search Error"), _("Controller does NOT EXIST."));
    return;
  }
  //
  this->metadataView_->updateTaskParam();
  if (currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }

  leTask->setText("");
  this->metadataView_->clearTaskParam();
  this->statemachineView_->clearTaskParam();
  this->parameterView_->clearTaskParam();

  taskList_.clear();
  currentTaskIndex_ = -1;
  currentTask_ = 0;

  searchTaskInstance();
  isSkip_ = true;
  showGrid();
  isSkip_ = false;
}

void TaskInstanceViewImpl::settingClicked() {
  DDEBUG("TaskInstanceViewImpl::settingClicked()");

  SettingDialog dialog(this);
  dialog.exec();
  if (dialog.IsDBUpdated()) {
    DatabaseManager::getInstance().reConnectDB();
    searchClicked();
  }
}

void TaskInstanceViewImpl::runTaskClicked() {
  DDEBUG("TaskInstanceViewImpl::runTaskClicked()");

  runSingleTask();
}

void TaskInstanceViewImpl::initPosClicked() {
  DDEBUG("TaskInstanceViewImpl::initPosClicked()");

  if (!currentTask_) return;
  if (checkPaused()) {
    return;
  }
  statemachineView_->setStepStatus(false);
  //
  for (int index = 0; index < currentTask_->getModelList().size(); index++) {
    ModelParam* model = currentTask_->getModelList()[index];
    model->setInitialPos();
  }
  executor_->detachAllModelItem();
}

void TaskInstanceViewImpl::abortClicked() {
  DDEBUG("TaskInstanceViewImpl::abortClicked");
  abortOperation();
}

void TaskInstanceViewImpl::loadTaskClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::loadTaskClicked()");
  statemachineView_->setStepStatus(false);
  //
  QString strFName = QFileDialog::getOpenFileName(
    this, "TaskModel File", ".", "YAML(*.yaml);;all(*.*)");
  if (strFName.isEmpty()) return;
  //
  DDEBUG_V("loadTaskClicked : %s", strFName.toStdString().c_str());
  //タスク定義ファイルの読み込み
  vector<TaskModelParam*> taskInstList;
  if (TeachingUtil::importTask(strFName, taskInstList) == false) {
    QMessageBox::warning(this, _("Task Load Error"), "Load Error (Task Def)");
    return;
  }
  vector<CommandDefParam*> commandList;
  SettingManager::getInstance().loadSetting();
  commandList = TaskExecutor::instance()->getCommandDefList();
  //
  //タスクの保存
  if (DatabaseManager::getInstance().saveTaskModelsForLoad(taskInstList)) {
    for (int index = 0; index < taskInstList.size(); index++) {
      TaskModelParam* task = taskInstList[index];
      TaskModelParam* newTask = DatabaseManager::getInstance().getTaskModelById(task->getId());
      TeachingUtil::loadTaskDetailData(newTask);
      newTask->setLoaded(true);
      if (newTask) {
        taskList_.push_back(newTask);
      }
    }
    isSkip_ = true;
    showGrid();
    lstResult->setCurrentCell(currentTaskIndex_, 0);
    isSkip_ = false;
    QMessageBox::information(this, _("Database"), _("Database updated"));
  } else {
    QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
  }
}

void TaskInstanceViewImpl::outputTaskClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::outputTaskClicked()");
  statemachineView_->setStepStatus(false);
  //
  if (!currentTask_) {
    QMessageBox::warning(this, _("Output Task"), _("Please select target TASK"));
    return;
  }
  //
  QFileDialog::Options options;
  QString strSelectedFilter;
  QString strFName = QFileDialog::getSaveFileName(
    this, tr("TaskModel File"), ".",
    tr("YAML(*.yaml);;all(*.*)"),
    &strSelectedFilter, options);
  if (strFName.isEmpty()) return;
  //
  DDEBUG_V("saveTaskClicked : %s", strFName.toStdString().c_str());
  updateCurrentInfo();
  if (TeachingUtil::exportTask(strFName, currentTask_)) {
    QMessageBox::information(this, _("Output Task"), _("target TASK exported"));
  } else {
    QMessageBox::warning(this, _("Output Task"), _("target TASK export FAILED"));
  }
}

void TaskInstanceViewImpl::registNewTaskClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::registNewTaskClicked()");
  statemachineView_->setStepStatus(false);
  //
  if (currentTask_) {
    updateCurrentInfo();
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    //
    currentTask_->setAllNewData();
    //
    vector<TaskModelParam*> taskList;
    taskList.push_back(currentTask_);
    if (DatabaseManager::getInstance().saveTaskModelsForLoad(taskList)) {
      searchTaskInstance();
      isSkip_ = true;
      showGrid();
      lstResult->setCurrentCell(currentTaskIndex_, 0);
      isSkip_ = false;
      //
      currentTaskIndex_ = lstResult->currentRow();
      currentTask_ = DatabaseManager::getInstance().getTaskModel(currentTaskIndex_);
      bool isUpdateTree = false;
      if (currentTask_) {
        if (currentTask_->IsLoaded() == false) {
          TeachingUtil::loadTaskDetailData(currentTask_);
        }
        isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);
      }

      leTask->setText(currentTask_->getName());
      this->metadataView_->setTaskParam(currentTask_);
      this->statemachineView_->setTaskParam(currentTask_);
      this->parameterView_->setTaskParam(currentTask_);
      //即更新を行うとエラーになってしまうため
      if (currentTask_) {
        if (isUpdateTree) {
          ChoreonoidUtil::showAllModelItem();
        }
      }
      lstResult->item(currentTaskIndex_, 3)->setText(currentTask_->getLastUpdatedDate());
      QMessageBox::information(this, _("Database"), _("Database updated"));
    } else {
      QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void TaskInstanceViewImpl::registTaskClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::registTaskClicked()");
  statemachineView_->setStepStatus(false);
  //
  for (int index = 0; index < currentTask_->getModelList().size(); index++) {
    ModelParam* model = currentTask_->getModelList()[index];
    if (model->isChangedPosition() == false) continue;
    //
    QMessageBox::StandardButton ret = QMessageBox::question(this, _("Confirm"),
      _("Model Position was changed. Continue?"),
      QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No) return;
    break;
  }
  //
  if (currentTask_) {
    updateCurrentInfo();
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    //
    if (DatabaseManager::getInstance().saveTaskModel(currentTask_)) {
      currentTask_->clearDetailParams();
      DatabaseManager::getInstance().getDetailParams(currentTask_);
      currentTask_->setNormal();
      currentTask_->setLoaded(false);
      //
      currentTaskIndex_ = lstResult->currentRow();
      currentTask_ = DatabaseManager::getInstance().getTaskModel(currentTaskIndex_);
      bool isUpdateTree = false;
      if (currentTask_) {
        if (currentTask_->IsLoaded() == false) {
          TeachingUtil::loadTaskDetailData(currentTask_);
        }
        isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);
      }

      leTask->setText(currentTask_->getName());
      this->metadataView_->setTaskParam(currentTask_);
      this->statemachineView_->setTaskParam(currentTask_);
      this->parameterView_->setTaskParam(currentTask_);
      //即更新を行うとエラーになってしまうため
      if (currentTask_) {
        if (isUpdateTree) {
          ChoreonoidUtil::showAllModelItem();
        }
      }
      lstResult->item(currentTaskIndex_, 3)->setText(currentTask_->getLastUpdatedDate());
      QMessageBox::information(this, _("Database"), _("Database updated"));
    } else {
      QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void TaskInstanceViewImpl::deleteTaskClicked() {
  if (checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::deleteTaskClicked()");
  statemachineView_->setStepStatus(false);
  //
  if (currentTask_) {
    leTask->setText("");
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    //
    if (DatabaseManager::getInstance().deleteTaskModel(currentTask_) == false) {
      QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
      return;
    }
    //
    QMessageBox::information(this, _("Database"), _("Database updated"));

    currentTaskIndex_ = -1;
    currentTask_ = 0;
    //
    this->metadataView_->clearTaskParam();
    this->statemachineView_->clearTaskParam();
    this->parameterView_->clearTaskParam();
    searchTaskInstance();
    isSkip_ = true;
    showGrid();
    isSkip_ = false;
  }
}

void TaskInstanceViewImpl::searchTaskInstance() {
  vector<string> condList;
  QStringList targetList;
  bool isOr = false;
  QString strTarget = leCond->text();
  if (strTarget.contains("||")) {
    isOr = true;
    targetList = strTarget.split("||");
  } else {
    targetList = strTarget.split(" ");
  }
  for (unsigned int index = 0; index < targetList.size(); index++) {
    QString each = targetList[index].trimmed();
    condList.push_back(each.toStdString());
  }
  taskList_ = DatabaseManager::getInstance().searchTaskModels(condList, isOr);
}

void TaskInstanceViewImpl::showGrid() {
  lstResult->clear();
  lstResult->setRowCount(0);
  lstResult->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");

  for (int index = 0; index < taskList_.size(); index++) {
    TaskModelParam* param = taskList_[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstResult->rowCount();
    lstResult->insertRow(row);

    QTableWidgetItem* itemName = new QTableWidgetItem;
    lstResult->setItem(row, 0, itemName);
    itemName->setText(param->getName());
    itemName->setData(Qt::UserRole, param->getId());

    QTableWidgetItem* itemComment = new QTableWidgetItem;
    lstResult->setItem(row, 1, itemComment);
    itemComment->setData(Qt::UserRole, 1);
    itemComment->setText(param->getComment());

    QTableWidgetItem* itemCreated = new QTableWidgetItem;
    lstResult->setItem(row, 2, itemCreated);
    itemCreated->setData(Qt::UserRole, 1);
    itemCreated->setText(param->getCreatedDate());

    QTableWidgetItem* itemLastUpdated = new QTableWidgetItem;
    lstResult->setItem(row, 3, itemLastUpdated);
    itemLastUpdated->setData(Qt::UserRole, 1);
    itemLastUpdated->setText(param->getLastUpdatedDate());
  }
}

void TaskInstanceViewImpl::updateCurrentInfo() {
  QString strTask = leTask->text();
  if (currentTask_->getName() != strTask) {
    currentTask_->setName(strTask);
  }
  this->metadataView_->updateTaskParam();
  lstResult->item(currentTaskIndex_, 0)->setText(currentTask_->getName());
  lstResult->item(currentTaskIndex_, 1)->setText(currentTask_->getComment());
  lstResult->item(currentTaskIndex_, 2)->setText(currentTask_->getCreatedDate());
  lstResult->item(currentTaskIndex_, 3)->setText(currentTask_->getLastUpdatedDate());
  parameterView_->setInputValues();
}

void TaskInstanceViewImpl::widgetClose() {
  DatabaseManager::getInstance().closeDB();
}
/////
TaskInstanceView::TaskInstanceView() : viewImpl(0) {
  setName(_("TaskInstance"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new TaskInstanceViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::BOTTOM);
}

TaskInstanceView::~TaskInstanceView() {
};

}
