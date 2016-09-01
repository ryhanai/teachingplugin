#include <cnoid/InfoBar>

#include "FlowView.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"
#include "FlowSearchDialog.h"

#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

FlowViewImpl::FlowViewImpl(QWidget* parent) 
			: TaskExecutionView(parent), currentFlow_(0) {
  QFrame* flowFrame = new QFrame;
  QLabel* lblName = new QLabel(tr("Flow Name:"));
  leName = new QLineEdit;
  leName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(tr("Search Flow"));

  QPushButton* btnNewFlow = new QPushButton();
  btnNewFlow->setIcon(QIcon(":/Teaching/icons/Create.png"));
  btnNewFlow->setToolTip(tr("Create New Flow"));

  btnRegistFlow = new QPushButton();
  btnRegistFlow->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnRegistFlow->setToolTip(tr("Regist Current Flow"));

  QLabel* lblComment = new QLabel(tr("Comment:"));
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
  lstFlow = new QListWidget;
  lstFlow->setAcceptDrops(false);
  setAcceptDrops(true);
  //
  QFrame* frmTask = new QFrame;
  btnDeleteTask = new QPushButton();
  btnDeleteTask->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteTask->setToolTip(tr("Delete selected Task"));

  btnUpTask = new QPushButton();
  btnUpTask->setIcon(QIcon(":/Teaching/icons/Up.png"));
  btnUpTask->setToolTip(tr("Move Up selected Task"));

  btnDownTask = new QPushButton();
  btnDownTask->setIcon(QIcon(":/Teaching/icons/Down.png"));
  btnDownTask->setToolTip(tr("Move Down selected Task"));

  btnRunFlow = new QPushButton(tr("Flow"));
  btnRunFlow->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunFlow->setToolTip(tr("Run Flow"));

  btnRunTask = new QPushButton(tr("Task"));
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(tr("Run selected Task"));

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(tr("Reset models position"));

  QHBoxLayout* taskLayout = new QHBoxLayout;
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(btnDeleteTask);
  taskLayout->addStretch();
  taskLayout->addWidget(btnUpTask);
  taskLayout->addWidget(btnDownTask);
  taskLayout->addStretch();
  taskLayout->addWidget(btnRunFlow);
  taskLayout->addWidget(btnRunTask);
  taskLayout->addWidget(btnInitPos);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(flowFrame);
  mainLayout->addWidget(lstFlow);
  mainLayout->addWidget(frmTask);

  setLayout(mainLayout);
  //
  changeEnables(false);
  //
  connect(lstFlow, SIGNAL(currentRowChanged(int)), this, SLOT(flowSelectionChanged()));
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(btnNewFlow, SIGNAL(clicked()), this, SLOT(newFlowClicked()));
  connect(btnRegistFlow, SIGNAL(clicked()), this, SLOT(registFlowClicked()));
  connect(btnDeleteTask, SIGNAL(clicked()), this, SLOT(deleteTaskClicked()));
  connect(btnUpTask, SIGNAL(clicked()), this, SLOT(upTaskClicked()));
  connect(btnDownTask, SIGNAL(clicked()), this, SLOT(downTaskClicked()));
  connect(btnRunFlow, SIGNAL(clicked()), this, SLOT(runFlowClicked()));
  connect(btnRunTask, SIGNAL(clicked()), this, SLOT(runTaskClicked()));
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
}

void FlowViewImpl::dragEnterEvent(QDragEnterEvent* event) {
    if(event->mimeData()->hasFormat("application/TaskInstanceItem")){
      event->acceptProposedAction();
    }
}

void FlowViewImpl::dropEvent(QDropEvent* event) {
  if(event->mimeData()->hasFormat("application/TaskInstanceItem")){
    if(currentFlow_) {
      const QMimeData* mimeData = event->mimeData();
      QVariant varData = event->mimeData()->property("TaskInstanceItemId");
      int id = varData.toInt();
      //
      TaskModelParam* param = DatabaseManager::getInstance().getTaskParamById(id);
      TaskModelParam* newParam = new TaskModelParam(param);
      currentFlow_->addTask(newParam);
      //
      QListWidgetItem* item = new QListWidgetItem(lstFlow);
      item->setData(Qt::UserRole, (unsigned int)(currentFlow_->getTaskList().size()-1));
      item->setText(event->mimeData()->text());
      //
      currentFlow_->setUpdate();
    } else {
      QMessageBox::warning(this, tr("Flow"), "Please select target Flow.");
    }
  }
}

void FlowViewImpl::flowSelectionChanged() {
  this->taskInstView->unloadCurrentModel();
  if(currentTask_) {
    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
  }

  QListWidgetItem* item = lstFlow->currentItem();
  if( item ) {
    int itemIndex = item->data(Qt::UserRole).toInt();
    currentTask_ = currentFlow_->getTaskList()[itemIndex];
    if(currentTask_->IsLoaded()==false) {
      TeachingUtil::loadTaskDetailData(currentTask_);
    }
    bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(currentTask_);

    this->metadataView->setTaskParam(currentTask_);
    this->statemachineView_->setTaskParam(currentTask_);
    this->parameterView_->setTaskParam(currentTask_);
    //即更新を行うとエラーになってしまうため
    if( isUpdateTree ) {
      ChoreonoidUtil::showAllModelItem();
    }

  } else {
    this->metadataView->clearTaskParam();
    this->statemachineView_->clearTaskParam();
    this->parameterView_->clearTaskParam();
  }
}

void FlowViewImpl::searchClicked() {
  FlowSearchDialog dialog(this);
  dialog.exec();
  if(dialog.IsOK()==false) return;
  //
  int selected = dialog.getSelectedIndex();
  if(currentFlow_) {
    this->taskInstView->unloadCurrentModel();
    if(currentTask_) {
      ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
    }
		lstFlow->clear();
		currentTask_ = 0;
    currParam_ = 0;
  }
	currentFlow_ = DatabaseManager::getInstance().getFlowParamById(selected);
  changeEnables(true);
  leName->setText(currentFlow_->getName());
  leComment->setText(currentFlow_->getComment());
  //
  vector<TaskModelParam*> taskList = currentFlow_->getTaskList();
  for(int index=0; index<taskList.size(); index++) {
    TaskModelParam* target = taskList[index];
    QListWidgetItem* item = new QListWidgetItem(lstFlow);
    item->setData(Qt::UserRole, index);
    item->setText(target->getName());
  }
}

void FlowViewImpl::newFlowClicked() {
  if(!currentFlow_) delete currentFlow_;
  currentFlow_ = new FlowParam(-1, "", "", "", "", true);

  leName->setText("");
  leComment->setText("");
  changeEnables(true);
  lstFlow->clear();
}

void FlowViewImpl::registFlowClicked() {
  if(currentFlow_) {
    int idxSeq = 1;
    for(int index=0; index<lstFlow->count(); index++) {
      QListWidgetItem *item = lstFlow->item(index);
      int idxTask = item->data(Qt::UserRole).toInt();
			TaskModelParam* targetTask = currentFlow_->getTaskList()[idxTask];
      if(targetTask->getMode()!=DB_MODE_DELETE && targetTask->getMode()!=DB_MODE_IGNORE) {
        targetTask->setSeq(idxSeq);
        idxSeq++;
      }
    }
    
    QString strName = leName->text();
    if( currentFlow_->getName() != strName) {
      currentFlow_->setName(strName);
    }
    QString strComment = leComment->text();
    if( currentFlow_->getComment() != strComment) {
      currentFlow_->setComment(strComment);
    }
    
    if(DatabaseManager::getInstance().saveFlowModel(currentFlow_) ) {
      QMessageBox::information(this, tr("Database"), "Database updated");
    } else {
      QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void FlowViewImpl::deleteTaskClicked() {
  QListWidgetItem* item = lstFlow->currentItem();
  if(item) {
    int itemIndex = item->data(Qt::UserRole).toInt();
    currentFlow_->getTaskList()[itemIndex]->setDelete();
    delete item;
    currentFlow_->setUpdate();
  }
}

void FlowViewImpl::upTaskClicked() {
  int selectedIndex = lstFlow->currentRow();
  if(selectedIndex==0) return;
  QListWidgetItem* item = this->lstFlow->takeItem(selectedIndex);
  lstFlow->insertItem(selectedIndex-1, item);
  lstFlow->setCurrentRow(selectedIndex-1);
  currentFlow_->setUpdate();
}

void FlowViewImpl::downTaskClicked() {
  int selectedIndex = lstFlow->currentRow();
  int count = 0;
  for(int index=0; index<lstFlow->model()->rowCount(); index++) {
    QListWidgetItem* item = lstFlow->item(index);
    if(item->isHidden()==false) count++;
  }
  if(count-1<=selectedIndex) return;

  QListWidgetItem* item = this->lstFlow->takeItem(selectedIndex);
  lstFlow->insertItem(selectedIndex+1, item);
  lstFlow->setCurrentRow(selectedIndex+1);
  currentFlow_->setUpdate();
}

void FlowViewImpl::runFlowClicked() {
  if( !currentFlow_ ) {
    QMessageBox::warning(this, tr("Run Flow"), "Please select target FLOW");
    return;
  }
  //
  parameterView_->setInputValues();
  vector<TaskModelParam*> taskList = currentFlow_->getTaskList();
  for(int index=0; index<taskList.size(); index++) {
    TaskModelParam* targetTask = taskList[index];
    if( targetTask->getMode()==DB_MODE_DELETE || targetTask->getMode()==DB_MODE_IGNORE) {
      continue;
    }
    if( targetTask->IsLoaded()==false ) {
      TeachingUtil::loadTaskDetailData(targetTask);
    }
    if( targetTask->checkAndOrderStateMachine()) {
      QMessageBox::warning(this, tr("Run Flow"), currentTask_->getErrStr());
      return;
    }
  }
  //
  InfoBar::instance()->showMessage(tr("Running Flow :") + currentFlow_->getName());
  TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
  for(int index=0; index<taskList.size(); index++) {
    lstFlow->setCurrentRow(index);
    TaskModelParam* targetTask = taskList[index];
    currParam_ = targetTask->getStartParam();
    currParam_->updateActive(true);
    statemachineView_->repaint();
    ChoreonoidUtil::updateScene();
    if( doTaskOperation(targetTask)==false ) break;
  }
  InfoBar::instance()->showMessage(tr("Finished Flow :") + currentFlow_->getName());
}

void FlowViewImpl::runTaskClicked() {
  runSingleTask();
}

void FlowViewImpl::initPosClicked() {
  if( !currentFlow_ ) return;
  if( currentFlow_->getTaskList().size()==0 ) return;

  for(int idxTask=0; idxTask<currentFlow_->getTaskList().size(); idxTask++) {
    TaskModelParam* targetTask = currentFlow_->getTaskList()[idxTask];
    for(int index=0; index<targetTask->getModelList().size(); index++) {
      targetTask->getModelList()[index]->setInitialPos();
    }
  }
}

void FlowViewImpl::changeEnables(bool value) {
  leName->setEnabled(value);
  leComment->setEnabled(value);
  btnRegistFlow->setEnabled(value);
  lstFlow->setEnabled(value);
  btnDeleteTask->setEnabled(value);
  btnUpTask->setEnabled(value);
  btnDownTask->setEnabled(value);
  btnRunFlow->setEnabled(value);
  btnRunTask->setEnabled(value);
  btnInitPos->setEnabled(value);
}
/////
FlowView::FlowView(): viewImpl(0) {
    setName("Flow");
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
