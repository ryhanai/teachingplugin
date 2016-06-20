#include <cnoid/UTF8>
#include "TaskInstanceView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include <cnoid/InverseKinematics>
#include <cnoid/EigenUtil>
#include <cnoid/ItemTreeView>
#include "TaskExecutor.h"
#include "Calculator.h"

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
  QLabel* lblCond = new QLabel(tr("Condition:"));
  leCond = new QLineEdit;
  //QPushButton* btnSearch = new QPushButton(tr("Search"));
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(tr("Search Task"));

  //QPushButton* btnSetting = new QPushButton(tr("Setting"));
  QPushButton* btnSetting = new QPushButton();
  btnSetting->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnSetting->setToolTip(tr("Setting"));
  //
  QHBoxLayout* topLayout = new QHBoxLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond);
  topLayout->addWidget(leCond);
  topLayout->addWidget(btnSearch);
  topLayout->addWidget(btnSetting);
  //
  lstResult = new SearchList(0,2);
  lstResult->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstResult->setSelectionMode(QAbstractItemView::SingleSelection);
  lstResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lstResult->verticalHeader()->setVisible(false);
  lstResult->setColumnWidth(0, 200);
  lstResult->setColumnWidth(1, 400);
  lstResult->setRowCount(0);
  lstResult->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment");
  //
  //
  QFrame* frmButtons = new QFrame;
  chkReal = new QCheckBox(tr("Real"));
  //QPushButton* btnRunTask = new QPushButton(tr("Run Task"));
  QPushButton* btnRunTask = new QPushButton();
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(tr("Run Task"));

  //QPushButton* btnInitPos = new QPushButton(tr("Init. Pos"));
  QPushButton* btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(tr("Reset Models Position"));

  //QPushButton* btnLoadTask = new QPushButton(tr("Load Task"));
  QPushButton* btnLoadTask = new QPushButton();
  btnLoadTask->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnLoadTask->setToolTip(tr("Load Task"));

  //QPushButton* btnOutputTask = new QPushButton(tr("Output Task"));
  QPushButton* btnOutputTask = new QPushButton();
  btnOutputTask->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnOutputTask->setToolTip(tr("Output Task"));

  //QPushButton* btnDeleteTask = new QPushButton(tr("Delete Task"));
  QPushButton* btnDeleteTask = new QPushButton();
  btnDeleteTask->setIcon(QIcon(":/Teaching/icons/Erase.png"));
  btnDeleteTask->setToolTip(tr("Delete Task"));

  //QPushButton* btnRegistNewTask = new QPushButton(tr("Regist New"));
  QPushButton* btnRegistNewTask = new QPushButton();
  btnRegistNewTask->setIcon(QIcon(":/Teaching/icons/Create.png"));
  btnRegistNewTask->setToolTip(tr("Regist as New Task"));

  //QPushButton* btnRegistTask = new QPushButton(tr("Update"));
  QPushButton* btnRegistTask = new QPushButton();
  btnRegistTask->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnRegistTask->setToolTip(tr("Update Current Task"));
  //
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(chkReal);
  buttonLayout->addWidget(btnRunTask);
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
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
  connect(btnLoadTask, SIGNAL(clicked()), this, SLOT(loadTaskClicked()));
  connect(btnOutputTask, SIGNAL(clicked()), this, SLOT(outputTaskClicked()));
  connect(btnDeleteTask, SIGNAL(clicked()), this, SLOT(deleteTaskClicked()));
  connect(btnRegistNewTask, SIGNAL(clicked()), this, SLOT(registNewTaskClicked()));
  connect(btnRegistTask, SIGNAL(clicked()), this, SLOT(registTaskClicked()));
  //
  if(DatabaseManager::getInstance().connectDB()) {
    taskList_ = DatabaseManager::getInstance().getTaskModels();
    showGrid();
  }
}

void TaskInstanceViewImpl::taskSelectionChanged() {
  LoggerUtil::writeLog("taskSelectionChanged");

  this->metadataView_->updateTaskParam();
  this->flowView_->unloadCurrentModel();
  if(currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    lstResult->item(currentTaskIndex_, 0)->setText(currentTask_->getName());
    lstResult->item(currentTaskIndex_, 1)->setText(currentTask_->getComment());
  }
  if(isSkip_) return;

  currentTaskIndex_ = lstResult->currentRow();
  currentTask_ = DatabaseManager::getInstance().getTaskModel(currentTaskIndex_);
  bool isUpdateTree = false;
  if(currentTask_) {
    if(currentTask_->IsLoaded()==false) {
      TeachingUtil::loadTaskDetailData(currentTask_);
    }
    isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);
  }

  this->metadataView_->setTaskParam(currentTask_);
  this->statemachineView_->setTaskParam(currentTask_);
  this->parameterView_->setTaskParam(currentTask_);
  //即更新を行うとエラーになってしまうため
  if(currentTask_) {
    if( isUpdateTree ) {
      ChoreonoidUtil::showAllModelItem();
    }
  }
}

void TaskInstanceViewImpl::searchClicked() {
  this->metadataView_->updateTaskParam();
  if(currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }

  this->metadataView_->clearTaskParam();
  this->statemachineView_->clearTaskParam();
  this->parameterView_->clearTaskParam();

  isSkip_ = true;
  taskList_.clear();
  currentTaskIndex_ = -1;
  currentTask_ = 0;

  vector<string> condList;
  QStringList targetList;
  bool isOr = false;
  QString strTarget = leCond->text();
  if( strTarget.contains("||") ) {
    isOr = true;
    targetList = strTarget.split("||");
  } else {
    targetList = strTarget.split(" ");
  }
  for(unsigned int index=0; index<targetList.size(); index++) {
    QString each = targetList[index].trimmed();
    condList.push_back(each.toStdString());
  }
  //
  taskList_ = DatabaseManager::getInstance().searchTaskModels(condList, isOr);
  showGrid();
  isSkip_ = false;
}

void TaskInstanceViewImpl::settingClicked() {
  SettingDialog dialog(this);
  dialog.exec();
  if(dialog.IsDBUpdated()) {
    DatabaseManager::getInstance().reConnectDB();
    searchClicked();
  }
}

void TaskInstanceViewImpl::runTaskClicked() {
  runSingleTask(chkReal->isChecked());
}

void TaskInstanceViewImpl::initPosClicked() {
  DDEBUG("initPosClicked");
  if(!currentTask_) return;
  //
  for(int index=0; index<currentTask_->getModelList().size(); index++) {
    ModelParam* model = currentTask_->getModelList()[index];
    model->setInitialPos();
  }
}

void TaskInstanceViewImpl::loadTaskClicked() {
	QString strFName = QFileDialog::getOpenFileName(
			this, tr( "TaskModel File" ), ".",
			tr( "YAML(*.yaml);;all(*.*)" ) );
	if ( strFName.isEmpty() ) return;
  //
  DDEBUG_V("loadTaskClicked : %s", strFName.toStdString().c_str());
  //タスク定義ファイルの読み込み
  vector<TaskModelParam*> taskInstList;
  if(TeachingUtil::loadTaskDef(strFName, taskInstList)==false) {
    QMessageBox::warning(this, tr("Task Load Error"), "Load Error (Task Def)");
    return;
  }
  vector<CommandDefParam*> commandList;
  SettingManager::getInstance().loadSetting();
  commandList = TaskExecutor::instance()->getCommandDefList();
  //
  //タスクの保存
  if(DatabaseManager::getInstance().saveTaskModelsForLoad(taskInstList) ) {
    for(int index=0; index<taskInstList.size(); index++) {
      TaskModelParam* task = taskInstList[index];
      TaskModelParam* newTask = DatabaseManager::getInstance().getTaskModelById(task->getId());
      newTask->setLoaded(true);
      if(newTask) {
        taskList_.push_back(newTask);
      }
    }
    isSkip_ = true;
    showGrid();
    lstResult->setCurrentCell(currentTaskIndex_, 0);
    isSkip_ = false;
    QMessageBox::information(this, tr("Database"), "Database updated");
  } else {
    QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
  }
}

void TaskInstanceViewImpl::outputTaskClicked() {
  if( !currentTask_ ) {
    QMessageBox::warning(this, tr("Output Task"), "Please select target TASK");
    return;
  }
  //
  QFileDialog::Options options;
	QString strSelectedFilter;
	QString strFName = QFileDialog::getSaveFileName(
			this, tr( "TaskModel File" ), ".",
			tr( "YAML(*.yaml);;all(*.*)" ),
			&strSelectedFilter, options );
	if ( strFName.isEmpty() ) return;
  //
  DDEBUG_V("saveTaskClicked : %s", strFName.toStdString().c_str());
  updateCurrentInfo();
  if( TeachingUtil::outputTaskDef(strFName, currentTask_) ) {
    QMessageBox::information(this, tr("Output Task"), "target TASK exported");
  } else {
    QMessageBox::warning(this, tr("Output Task"), "target TASK export FAILED");
  }
}

void TaskInstanceViewImpl::registNewTaskClicked() {
  if(currentTask_) {
    updateCurrentInfo();
    //
    currentTask_->setAllNewData();
    //
    DDEBUG_V("St : %d", currentTask_->getModelList()[0]->getModelDetailList()[0]->getMode());
    if(DatabaseManager::getInstance().saveTaskModelsForNewRegist(currentTask_) ) {
      TaskModelParam* newTask = DatabaseManager::getInstance().getTaskModelById(currentTask_->getId());
      if(newTask) {
        taskList_.push_back(newTask);
      }
      isSkip_ = true;
      showGrid();
      lstResult->setCurrentCell(currentTaskIndex_, 0);
      isSkip_ = false;
      QMessageBox::information(this, tr("Database"), "Database updated");
    } else {
      QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void TaskInstanceViewImpl::registTaskClicked() {
  if(currentTask_) {
    updateCurrentInfo();
    //
    vector<TaskModelParam*> targetTasks;
    targetTasks.push_back(currentTask_);

    if(DatabaseManager::getInstance().saveTaskModels(targetTasks) ) {
      currentTask_->setNormal();
      QMessageBox::information(this, tr("Database"), "Database updated");
    } else {
      QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void TaskInstanceViewImpl::deleteTaskClicked() {
  DDEBUG("deleteTaskClicked");
  if( currentTask_ ) {
    vector<QString> flowList;
    if(DatabaseManager::getInstance().checkFlowItem(currentTask_->getId(), flowList) ) {
      QString errStr = "Target Task is used. [";
      for(int index=0; index<flowList.size(); index++) {
        if(index!=0) errStr += ", " ;
        errStr += flowList[index];
      }
      errStr += "]";
      QMessageBox::warning(this, tr("Task DELETE"), errStr);
      return;
    }
    //
    FlowParam* flow = flowView_->getCurrentFlow();
    if(flow) {
      for(int index=0; index<flow->getTaskList().size(); index++) {
        TaskModelParam* task = flow->getTaskList()[index];
        if(task->getMode()==DB_MODE_DELETE || task->getMode()==DB_MODE_IGNORE) continue;
        if(currentTask_->getId()==task->getId()) {
          QMessageBox::warning(this, tr("Task DELETE"), "Target Task is used in CURRENT Flow.");
          return;
        }
      }
    }
    //
    if(DatabaseManager::getInstance().deleteTaskModel(currentTask_)==false ) {
      QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
      return;
    }
    //
    if(currentTask_) {
      ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    }
    QMessageBox::information(this, tr("Database"), "Database updated");

    currentTaskIndex_ = -1;
    currentTask_ = 0;
    //
    this->metadataView_->clearTaskParam();
    this->statemachineView_->clearTaskParam();
    this->parameterView_->clearTaskParam();
    leCond->setText("");
    searchClicked();
  }
}

void TaskInstanceViewImpl::showGrid() {
  lstResult->clear();
  lstResult->setRowCount(0);
  lstResult->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment");

  for(int index=0; index<taskList_.size(); index++) {
    TaskModelParam* param = taskList_[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstResult->rowCount();
    lstResult->insertRow(row);

    QTableWidgetItem* itemName = new QTableWidgetItem;
    lstResult->setItem(row, 0, itemName);
    itemName->setData(Qt::UserRole, 1);
    itemName->setText(param->getName());
    itemName->setData(Qt::UserRole, param->getId());

    QTableWidgetItem* itemComment = new QTableWidgetItem;
    lstResult->setItem(row, 1, itemComment);
    itemComment->setData(Qt::UserRole, 1);
    itemComment->setText(param->getComment());
  }
}

void TaskInstanceViewImpl::updateCurrentInfo() {
  this->metadataView_->updateTaskParam();
  lstResult->item(currentTaskIndex_, 0)->setText(currentTask_->getName());
  lstResult->item(currentTaskIndex_, 1)->setText(currentTask_->getComment());
  parameterView_->setInputValues();
}

void TaskInstanceViewImpl::widgetClose() {
  DatabaseManager::getInstance().closeDB();
}
/////
TaskInstanceView::TaskInstanceView(): viewImpl(0) {
    setName("Task Model");
    setDefaultLayoutArea(View::BOTTOM);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewImpl = new TaskInstanceViewImpl(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(viewImpl);
    setLayout(vbox);
}

TaskInstanceView::~TaskInstanceView() {
};

}
