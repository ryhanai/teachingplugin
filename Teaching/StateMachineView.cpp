#include <cnoid/InfoBar>

#include "StateMachineView.h"
#include "TeachingUtil.h"
#include "Calculator.h"
#include "TaskExecutor.h"
#include "ArgumentDialog.h"
#include "ControllerManager.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

StateMachineViewImpl::StateMachineViewImpl(QWidget* parent)
	: TaskExecutionView(parent), targetTask_(0) {
  lblTarget = new QLabel;
  lblTarget->setText("");

  btnDelete = new QPushButton(_("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(_("Delete selected element"));
  btnDelete->setEnabled(false);

  btnEdit = new QPushButton(_("Edit"));
  btnEdit->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnEdit->setToolTip(_("Edit target state"));
  btnEdit->setEnabled(false);

  btnRun = new QPushButton(_("Command"));
  btnRun->setIcon(QIcon(":/Base/icons/play.png"));
  btnRun->setToolTip(_("Run selected Command"));
  btnRun->setEnabled(false);

	btnBP = new QPushButton(_("B.P."));
	btnBP->setToolTip(_("Set/Unset BreakPoint"));
	btnBP->setCheckable(true);
	btnBP->setEnabled(false);

	btnStep = new QPushButton(_("Step"));
	btnStep->setToolTip(_("Run by Step"));
	btnStep->setEnabled(false);

	btnCont = new QPushButton(_("Cont."));
	btnCont->setToolTip(_("continue processing"));
	btnCont->setEnabled(false);

	QHBoxLayout* topLayout = new QHBoxLayout;
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->addWidget(lblTarget);
  topLayout->addStretch();
	topLayout->addWidget(btnBP);
	topLayout->addWidget(btnStep);
	topLayout->addWidget(btnCont);
	topLayout->addWidget(btnRun);
  topLayout->addWidget(btnEdit);
  topLayout->addWidget(btnDelete);
  QFrame* frmTop = new QFrame;
  frmTop->setLayout(topLayout);

  btnTrans = new QPushButton();
  btnTrans->setText("Transition");
  btnTrans->setCheckable(true);
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawLine(0, 15, 30, 15);
  painter->drawLine(29, 15, 20, 5);
  painter->drawLine(29, 15, 20, 25);
  btnTrans->setIconSize(QSize(30,30));
  btnTrans->setIcon(QIcon(*pix));
  btnTrans->setStyleSheet("text-align:left;");
  btnTrans->setEnabled(false);

	lstItem = new ItemList(QString::fromStdString("application/StateMachineItem"));
  lstItem->setStyleSheet("background-color: rgb( 212, 206, 199 )};");
  lstItem->setEnabled(false);
	lstItem->createInitialNodeTarget();
	lstItem->createFinalNodeTarget();
	lstItem->createDecisionNodeTarget();
	//createForkNodeTarget();

	grhStateMachine = new ActivityEditor(this, this);
  grhStateMachine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  grhStateMachine->setStyleSheet("background-color: white;");
  grhStateMachine->setAcceptDrops(true);
  grhStateMachine->setEnabled(false);
  setAcceptDrops(true);

  QLabel* lblGuard = new QLabel("Guard:");
  rdTrue = new QRadioButton("True");
  rdFalse = new QRadioButton("False");

  btnSet = new QPushButton(_("Set"));
  btnSet->setIcon(QIcon(":/Teaching/icons/Logout.png"));
  btnSet->setToolTip(_("Set Guard condition"));

  rdTrue->setEnabled(false);
  rdFalse->setEnabled(false);
  btnSet->setEnabled(false);

  QVBoxLayout* itemLayout = new QVBoxLayout;
  itemLayout->setContentsMargins(0, 0, 0, 0);
  itemLayout->addWidget(btnTrans);
  itemLayout->addWidget(lstItem);
  QFrame* frmItem = new QFrame;
  frmItem->setLayout(itemLayout);

  QHBoxLayout* guardLayout = new QHBoxLayout;
  guardLayout->addStretch();
  guardLayout->addWidget(lblGuard);
  guardLayout->addWidget(rdTrue);
  guardLayout->addWidget(rdFalse);
  guardLayout->addWidget(btnSet);
  frmGuard = new QFrame;
  frmGuard->setLayout(guardLayout);

  QVBoxLayout* editorLayout = new QVBoxLayout;
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->addWidget(grhStateMachine);
  editorLayout->addWidget(frmGuard);
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
  SettingManager::getInstance().loadSetting();
  ControllerManager::instance()->setStateMachineView(this);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frmTop);
  mainLayout->addWidget(splBase);
  setLayout(mainLayout);
  //
  connect(btnTrans, SIGNAL(toggled(bool)), this, SLOT(modeChanged()));
  connect(btnSet, SIGNAL(clicked()), this, SLOT(setClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
  connect(btnRun, SIGNAL(clicked()), this, SLOT(runClicked()));
	connect(btnStep, SIGNAL(clicked()), this, SLOT(stepClicked()));
	connect(btnCont, SIGNAL(clicked()), this, SLOT(contClicked()));
	connect(btnBP, SIGNAL(clicked()), this, SLOT(bpToggled()));
}

void StateMachineViewImpl::setBPStatus(bool isActive, bool isSet) {
	btnBP->setEnabled(isActive);
	btnBP->setChecked(isSet);
}

void StateMachineViewImpl::createStateCommands() {
  commandList_ = TaskExecutor::instance()->getCommandDefList();
  vector<CommandDefParam*>::iterator itCmd = commandList_.begin();
  while (itCmd != commandList_.end() ) {
    createCommandNodeTarget((*itCmd)->getId(), (*itCmd)->getDispName());
    ++itCmd;
  }
}

void StateMachineViewImpl::setTaskParam(TaskModelParam* param) {
  this->targetTask_ = param;
  lblTarget->setText(param->getName());
  //
  btnTrans->setEnabled(true);
  lstItem->setEnabled(true);
  grhStateMachine->setEnabled(true);
  rdTrue->setEnabled(true);
  rdFalse->setEnabled(true);
  btnSet->setEnabled(true);
  btnEdit->setEnabled(true);
  btnDelete->setEnabled(true);
  btnRun->setEnabled(true);

	btnStep->setEnabled(false);
	btnCont->setEnabled(false);
	//
  grhStateMachine->setTaskParam(param);
  grhStateMachine->createStateMachine(param);
}

void StateMachineViewImpl::clearTaskParam() {
  this->targetTask_ = 0;
  lblTarget->setText("");
  //
  btnTrans->setEnabled(false);
  lstItem->setEnabled(false);
  grhStateMachine->removeAll();
  grhStateMachine->setEnabled(false);
  rdTrue->setEnabled(false);
  rdFalse->setEnabled(false);
  btnSet->setEnabled(false);
  btnRun->setEnabled(false);
}

void StateMachineViewImpl::createCommandNodeTarget(int id, QString name) {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawEllipse(5, 5, 20, 20);
  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem(name, lstItem);
  item->setIcon(QIcon(*pix));
  item->setData(Qt::UserRole, id);
}

void StateMachineViewImpl::setClicked() {
  ConnectionNode* conn = grhStateMachine->getCurrentConnection();
  if(conn) {
    if( conn->getSource()->getElementType()==ELEMENT_DECISION ) {
      if( rdTrue->isChecked() ) {
        conn->setText("true");
        conn->getConnParam()->setCondition("true");
      } else {
        conn->setText("false");
        conn->getConnParam()->setCondition("false");
      }
      conn->getConnParam()->setUpdate();
    }
  }
}

void StateMachineViewImpl::modeChanged() {
  grhStateMachine->setCntMode(btnTrans->isChecked());
}

void StateMachineViewImpl::stepClicked() {
	if (doOperationStep() == ExecResult::EXEC_BREAK) {
		setStepStatus(true);
	} else {
		setStepStatus(false);
	}
}

void StateMachineViewImpl::contClicked() {
	if (doOperationCont() == ExecResult::EXEC_BREAK) {
		setStepStatus(true);
	} else {
		setStepStatus(false);
	}
}

void StateMachineViewImpl::bpToggled() {
	grhStateMachine->setBreakPoint(btnBP->isChecked());
}

void StateMachineViewImpl::setStepStatus(bool isActive) {
	btnBP->setChecked(false);
	btnStep->setEnabled(isActive);
	btnCont->setEnabled(isActive);
	executor_->setBreak(isActive);
}
void StateMachineViewImpl::deleteClicked() {
  grhStateMachine->deleteCurrent();
}

void StateMachineViewImpl::editClicked() {
  ElementNode* target = grhStateMachine->getCurrentNode();
  if(target) {
    ElementStmParam* targetStm = target->getElemParam();
    if(targetStm->getType()!=ELEMENT_COMMAND) {
      QMessageBox::warning(this, _("Command"), _("Please select Command Element. : ") + QString::number(targetStm->getType()));
      return;
    }
    if( targetStm->getArgList().size()==0 ) {
      DDEBUG("editClicked : No Arg");
      QString strCmd = targetStm->getCmdName();
      for(int index=0; index<commandList_.size(); index++) {
        CommandDefParam* param = commandList_[index];
        if(param->getName()!=strCmd) continue;
        vector<ArgumentDefParam*> argList = param->getArgList();
        for(int idxArg=0; idxArg<argList.size(); idxArg++) {
          ArgumentDefParam* arg = argList[idxArg];
          ArgumentParam* argParam = new ArgumentParam(-1, targetStm->getId(), idxArg+1, QString::fromStdString(arg->getName()), "");
          argParam->setNew();
          targetStm->addArgument(argParam);
        }
      }
    }

    ArgumentDialog dialog(targetTask_, targetStm, this);
    dialog.exec();
  } else {
    QMessageBox::warning(this, _("Command"), _("Please select Target Command."));
    return;
  }
}

void StateMachineViewImpl::runClicked() {
	bool isReal = SettingManager::getInstance().getIsReal();
	ElementNode* target = grhStateMachine->getCurrentNode();
  if(target) {
    //bool isReal = SettingManager::getInstance().getIsReal();
		parameterView_->setInputValues();
		ElementStmParam* targetStm = target->getElemParam();
    if(targetStm->getType()!=ELEMENT_COMMAND) {
      QMessageBox::warning(this, _("Run Command"), _("Please select Command Element."));
      return;
    }
    //
    Calculator* calculator = new Calculator();
    std::vector<CompositeParamType> parameterList;
    //à¯êîÇÃëgÇ›óßÇƒ
    for(int idxArg=0; idxArg<targetStm->getArgList().size();idxArg++) {
      ArgumentParam* arg = targetStm->getArgList()[idxArg];
      QString valueDesc = arg->getValueDesc();
      if(targetStm->getCommadDefParam()==0) {
        delete calculator;
        QMessageBox::warning(this, _("Run Command"), _("Target Command does NOT EXIST."));
        return;
      }
      ArgumentDefParam* argDef = targetStm->getCommadDefParam()->getArgList()[idxArg];
      if(argDef->getType() == "double") {
        bool ret = calculator->calculate(valueDesc, targetTask_);
        if(calculator->getValMode()==VAL_SCALAR) {
          DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), calculator->getResultScalar());
          parameterList.push_back(calculator->getResultScalar());
        } else {
          DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), calculator->getResultVector()[0], calculator->getResultVector()[1], calculator->getResultVector()[2]);
          parameterList.push_back(calculator->getResultVector());
        }

      } else {
        vector<ParameterParam*> paramList = targetTask_->getParameterList();
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
    InfoBar::instance()->showMessage(_("Running Command :") + targetStm->getCmdName());
    TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
    bool cmdRet = TaskExecutor::instance()->executeCommand(targetStm->getCmdName().toStdString(), parameterList, isReal);
    if(cmdRet) {
      //QMessageBox::information(this, tr("Run Command"), tr("Target Command is FINISHED."));
    } else {
      QMessageBox::information(this, _("Run Command"), _("Target Command FAILED."));
    }
    InfoBar::instance()->showMessage(_("Finished Command :") + targetStm->getCmdName());
  }
}
/////
StateMachineView::StateMachineView(): viewImpl(0) {
    setName(_("StateMachine"));
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewImpl = new StateMachineViewImpl(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(viewImpl);
    setLayout(vbox);
		setDefaultLayoutArea(View::RIGHT);
}

StateMachineView::~StateMachineView() {
};

}
