#include "TaskInstanceView.h"

#include "ControllerManager.h"
#include "ModelMasterDialog.h"
#include "DataBaseManager.h"
#include "TeachingEventHandler.h"
#include "TeachingUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

TaskInstanceViewImpl::TaskInstanceViewImpl(QWidget* parent)
  : currentTaskIndex_(-1), isSkip_(false) {

  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(_("Condition:"));
  leCond = new QLineEdit;
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/search.png"));
  btnSearch->setToolTip(_("Search Task"));

	btnModelMaster = new QPushButton();
	btnModelMaster->setIcon(QIcon(":/Teaching/icons/model_master.png"));
	btnModelMaster->setToolTip(_("Model Master"));

	btnSetting = new QPushButton();
  btnSetting->setIcon(QIcon(":/Teaching/icons/preference.png"));
  btnSetting->setToolTip(_("Setting"));

  chkReal = new QCheckBox(_("Real"));
  chkReal->setChecked(SettingManager::getInstance().getIsReal());

  QLabel* lblTaskName = new QLabel(_("Task Name:"));
  leTask = new QLineEdit;
  //
  QGridLayout* topLayout = new QGridLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond, 0, 0, 1, 1, Qt::AlignRight);
  topLayout->addWidget(leCond, 0, 1, 1, 1);
  topLayout->addWidget(btnSearch, 0, 2, 1, 1);
  topLayout->addWidget(btnModelMaster, 0, 3, 1, 1);
	topLayout->addWidget(btnSetting, 0, 4, 1, 1);
	topLayout->addWidget(chkReal, 0, 5, 1, 1);
	topLayout->addWidget(lblTaskName, 1, 0, 1, 1, Qt::AlignRight);
  topLayout->addWidget(leTask, 1, 1, 1, 5);
  //
  lstResult = new SearchList(0, 4);
  lstResult->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstResult->setSelectionMode(QAbstractItemView::SingleSelection);
  lstResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lstResult->verticalHeader()->setVisible(false);
  lstResult->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
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
  btnRunTask->setIcon(QIcon(":/Teaching/icons/run_task.png"));
  btnRunTask->setToolTip(_("Run Task"));

  btnAbort = new QPushButton(_("Abort"));
  btnAbort->setIcon(QIcon(":/Teaching/icons/abort.png"));
  btnAbort->setToolTip(_("Abort Operation"));
  btnAbort->setEnabled(false);

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/reset.png"));
  btnInitPos->setToolTip(_("Reset Models Position"));

  btnLoadTask = new QPushButton();
  btnLoadTask->setIcon(QIcon(":/Teaching/icons/import.png"));
  btnLoadTask->setToolTip(_("Load Task"));

  btnOutputTask = new QPushButton();
  btnOutputTask->setIcon(QIcon(":/Teaching/icons/export.png"));
  btnOutputTask->setToolTip(_("Output Task"));

  btnDeleteTask = new QPushButton();
  btnDeleteTask->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
  btnDeleteTask->setToolTip(_("Delete Task"));

  btnRegistNewTask = new QPushButton();
  btnRegistNewTask->setIcon(QIcon(":/Teaching/icons/duplicate_task.png"));
  btnRegistNewTask->setToolTip(_("Regist as New Task"));

  btnRegistTask = new QPushButton();
  btnRegistTask->setIcon(QIcon(":/Teaching/icons/save.png"));
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
	connect(btnModelMaster, SIGNAL(clicked()), this, SLOT(modelMasterClicked()));
	connect(btnSetting, SIGNAL(clicked()), this, SLOT(settingClicked()));
	connect(chkReal, SIGNAL(clicked()), this, SLOT(realClicked()));
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

	TeachingEventHandler::instance()->tiv_Loaded(this);
}

TaskInstanceViewImpl::~TaskInstanceViewImpl() {
  DDEBUG("TaskInstanceViewImpl Destruct");
  DatabaseManager::getInstance().closeDB();
}

void TaskInstanceViewImpl::loadTaskInfo() {
	DDEBUG("TaskInstanceViewImpl::loadTaskInfo");
	if (DatabaseManager::getInstance().connectDB()) {
		TeachingDataHolder::instance()->loadData();
		vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->getTaskList();
    showGrid(taskList);
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

  setEditMode(TeachingEventHandler::instance()->canEdit());
}

void TaskInstanceViewImpl::taskSelectionChanged() {
	if (isSkip_) return;
	DDEBUG("TaskInstanceViewImpl::taskSelectionChanged()");

  if (TeachingEventHandler::instance()->checkPaused()) {
		isSkip_ = true;
    lstResult->setCurrentCell(currentTaskIndex_, 0);
		isSkip_ = false;
		return;
  }
	int selectedId = getSelectedId();
	TeachingEventHandler::instance()->tiv_TaskSelectionChanged(selectedId, leTask->text());

//	if (isSkip_) return;

  currentTaskIndex_ = lstResult->currentRow();
	TaskModelParamPtr newTask = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
  leTask->setText(newTask->getName());
}

void TaskInstanceViewImpl::searchClicked() {
  if (TeachingEventHandler::instance()->checkPaused()) return;
  DDEBUG("TaskInstanceViewImpl::searchClicked()");

	leTask->setText("");
	currentTaskIndex_ = -1;

	TeachingEventHandler::instance()->tiv_SearchClicked(leCond->text());
}

void TaskInstanceViewImpl::loadTaskClicked() {
	if (TeachingEventHandler::instance()->tiv_TaskImportClicked() == false) return;
	//
	isSkip_ = true;
	lstResult->setCurrentCell(currentTaskIndex_, 0);
	isSkip_ = false;
}

void TaskInstanceViewImpl::registNewTaskClicked() {
  if (TeachingEventHandler::instance()->tiv_RegistNewTaskClicked(getSelectedId(), leTask->text(), leCond->text())) {
    lstResult->setCurrentCell(currentTaskIndex_, 0);
  }
}

void TaskInstanceViewImpl::registTaskClicked() {
  if (TeachingEventHandler::instance()->tiv_RegistTaskClicked(getSelectedId(), leTask->text())) {
    currentTaskIndex_ = lstResult->currentRow();
  }
}

void TaskInstanceViewImpl::deleteTaskClicked() {
  if (TeachingEventHandler::instance()->tiv_DeleteTaskClicked(getSelectedId()) == false) return;

	leTask->setText("");
	currentTaskIndex_ = -1;
}

void TaskInstanceViewImpl::showGrid(const vector<TaskModelParamPtr>& taskList) {
	isSkip_ = true;

  lstResult->clear();
  lstResult->setRowCount(0);
  lstResult->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");

  for (TaskModelParamPtr param : taskList) {
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
		itemComment->setData(Qt::UserRole, param->getId());

    QTableWidgetItem* itemCreated = new QTableWidgetItem;
    lstResult->setItem(row, 2, itemCreated);
    itemCreated->setData(Qt::UserRole, 1);
    itemCreated->setText(param->getCreatedDate());
		itemCreated->setData(Qt::UserRole, param->getId());

    QTableWidgetItem* itemLastUpdated = new QTableWidgetItem;
    lstResult->setItem(row, 3, itemLastUpdated);
    itemLastUpdated->setData(Qt::UserRole, 1);
    itemLastUpdated->setText(param->getLastUpdatedDate());
		itemLastUpdated->setData(Qt::UserRole, param->getId());
	}
	isSkip_ = false;
}

void TaskInstanceViewImpl::updateGrid(TaskModelParamPtr& target) {
	leTask->setText(target->getName());

	lstResult->item(currentTaskIndex_, 0)->setText(target->getName());
	lstResult->item(currentTaskIndex_, 1)->setText(target->getComment());
	lstResult->item(currentTaskIndex_, 2)->setText(target->getCreatedDate());
	lstResult->item(currentTaskIndex_, 3)->setText(target->getLastUpdatedDate());
}

void TaskInstanceViewImpl::setEditMode(bool canEdit) {
  btnRunTask->setEnabled(!canEdit);
  btnInitPos->setEnabled(!canEdit);
  btnAbort->setEnabled(!canEdit);
  //
  btnModelMaster->setEnabled(canEdit);
  btnSetting->setEnabled(canEdit);

  leTask->setEnabled(canEdit);
  btnLoadTask->setEnabled(canEdit);
  btnOutputTask->setEnabled(canEdit);
  btnDeleteTask->setEnabled(canEdit);
  btnRegistNewTask->setEnabled(canEdit);
  btnRegistTask->setEnabled(canEdit);
}

void TaskInstanceViewImpl::setExecState(bool isActive) {
  btnRunTask->setEnabled(isActive);
}

void TaskInstanceViewImpl::clearSelection() {
  isSkip_ = true;
  lstResult->clearSelection();
  isSkip_ = false;
}

int TaskInstanceViewImpl::getSelectedId() {
  int result = NULL_ID;
  QTableWidgetItem* item = lstResult->currentItem();
  if (item) {
    result = item->data(Qt::UserRole).toInt();
  }
  return result;
}

void TaskInstanceViewImpl::modelMasterClicked() {
  ModelMasterDialog dialog(this);
  dialog.exec();
}

void TaskInstanceViewImpl::settingClicked() {
  SettingDialog dialog(this);
  dialog.exec();
  if (dialog.IsDBUpdated()) {
    DatabaseManager::getInstance().reConnectDB();
    searchClicked();
  }
}

void TaskInstanceViewImpl::realClicked() {
  SettingManager::getInstance().setIsReal(chkReal->isChecked());
}

void TaskInstanceViewImpl::runTaskClicked() {
  TeachingEventHandler::instance()->updateExecState(false);
  TeachingEventHandler::instance()->tev_RunTaskClicked(getSelectedId(), false);
}

void TaskInstanceViewImpl::initPosClicked() {
  TeachingEventHandler::instance()->tiv_InitPosClicked();
}

void TaskInstanceViewImpl::abortClicked() {
  TeachingEventHandler::instance()->tev_AbortClicked();
}

void TaskInstanceViewImpl::outputTaskClicked() {
  TeachingEventHandler::instance()->tiv_TaskExportClicked(getSelectedId(), leTask->text());
}

void TaskInstanceViewImpl::widgetClose() {
  DatabaseManager::getInstance().closeDB();
}
///////////////
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
