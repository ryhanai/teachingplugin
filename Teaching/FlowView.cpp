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

FlowViewImpl::FlowViewImpl(QWidget* parent) 
			: TaskExecutionView(parent), currentFlow_(0) {
  QFrame* flowFrame = new QFrame;
  QLabel* lblName = new QLabel(_("Flow Name:"));
  leName = new QLineEdit;
  leName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Flow"));

  QPushButton* btnNewFlow = new QPushButton();
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

  btnRunFlow = new QPushButton(_("Flow"));
  btnRunFlow->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunFlow->setToolTip(_("Run Flow"));

  btnRunTask = new QPushButton(_("Task"));
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(_("Run selected Task"));

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(_("Reset models position"));

  QHBoxLayout* taskLayout = new QHBoxLayout;
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(btnDeleteTask);
  taskLayout->addStretch();
  taskLayout->addWidget(btnRunFlow);
  taskLayout->addWidget(btnRunTask);
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
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
}
//
void FlowViewImpl::searchClicked() {
	if (checkPaused()) {
		return;
	}
	statemachineView_->setStepStatus(false);
	//
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
		//lstFlow->clear();
		currentTask_ = 0;
    currParam_ = 0;
  }
	currentFlow_ = DatabaseManager::getInstance().getFlowParamById(selected);
  changeEnables(true);
  leName->setText(currentFlow_->getName());
  leComment->setText(currentFlow_->getComment());
	grhStateMachine->setFlowParam(currentFlow_);
	grhStateMachine->createStateMachine(currentFlow_);
}

void FlowViewImpl::newFlowClicked() {
	if (checkPaused()) {
		return;
	}
	statemachineView_->setStepStatus(false);
	//
  if(!currentFlow_) delete currentFlow_;
  currentFlow_ = new FlowParam(-1, "", "", "", "", true);

  leName->setText("");
  leComment->setText("");

	grhStateMachine->setFlowParam(currentFlow_);
	grhStateMachine->createStateMachine(currentFlow_);

	changeEnables(true);
}

void FlowViewImpl::registFlowClicked() {
	if (checkPaused()) {
		return;
	}
	statemachineView_->setStepStatus(false);
	//
	if (currentFlow_) {
    QString strName = leName->text();
    if( currentFlow_->getName() != strName) {
      currentFlow_->setName(strName);
    }
    QString strComment = leComment->text();
    if( currentFlow_->getComment() != strComment) {
      currentFlow_->setComment(strComment);
    }
    
    if(DatabaseManager::getInstance().saveFlowModel(currentFlow_) ) {
			currentFlow_ = DatabaseManager::getInstance().getFlowParamById(currentFlow_->getId());
      QMessageBox::information(this, _("Database"), _("Database updated"));
    } else {
      QMessageBox::warning(this, _("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }
  }
}

void FlowViewImpl::deleteTaskClicked() {
	if (checkPaused()) {
		return;
	}
	statemachineView_->setStepStatus(false);
	grhStateMachine->deleteCurrent();
  currentFlow_->setUpdate();
}

void FlowViewImpl::modeChanged() {
	grhStateMachine->setCntMode(btnTrans->isChecked());
}

void FlowViewImpl::runFlowClicked() {
	runFlow(currentFlow_);
}

void FlowViewImpl::runTaskClicked() {
  runSingleTask();
}

void FlowViewImpl::initPosClicked() {
  if( !currentFlow_ ) return;
	if (checkPaused()) {
		return;
	}
	statemachineView_->setStepStatus(false);
	//
	for (int idxState = 0; idxState<currentFlow_->getStmElementList().size(); idxState++) {
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
}

void FlowViewImpl::flowSelectionChanged(TaskModelParam* target) {
		if (checkPaused()) {
			return;
		}
		statemachineView_->setStepStatus(false);
		//
		this->taskInstView->unloadCurrentModel();
	  if(currentTask_) {
	    ChoreonoidUtil::unLoadTaskModelItem(currentTask_);
	  }
		//
		currentTask_ = target;
		if (currentTask_) {
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
	/////
FlowView::FlowView(): viewImpl(0) {
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
