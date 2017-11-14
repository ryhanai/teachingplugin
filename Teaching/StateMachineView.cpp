#include <cnoid/InfoBar>

#include "StateMachineView.h"
#include "TeachingUtil.h"
#include "TaskExecutor.h"
#include "ControllerManager.h"

#include "TeachingEventHandler.h"

// R.Hanai
#include <cnoid/ItemList>// This should be included afger ACtivityEditorBase.h
#include <cnoid/RootItem>
#include <cnoid/BodyItem>
#include <cnoid/JointPath>
#include <cnoid/ValueTree>
#include <cnoid/SceneView>
// R.Hanai

#include "gettext.h"
#include "LoggerUtil.h"


#ifdef __TASK_PARAM_ADJUSTER__
#include "TaskParamAdjust/TaskParameterAdjuster.h"
#endif

using namespace std;
using namespace cnoid;

namespace teaching {

  StateMachineViewImpl::StateMachineViewImpl(QWidget* parent) : isExec_(false) {
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


#ifdef __TASK_PARAM_ADJUSTER__
    btnTrain = new QPushButton(_("Train"));
    btnTrain->setToolTip(_("Record data for parameter adjustment"));
    btnTrain->setEnabled(false);
#endif


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


#ifdef __TASK_PARAM_ADJUSTER__
    topLayout->addWidget(btnTrain);
#endif


    topLayout->addWidget(btnDelete);
    QFrame* frmTop = new QFrame;
    frmTop->setLayout(topLayout);

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
    LoggerUtil::startLog((LogLevel)SettingManager::getInstance().getLogLevel(), SettingManager::getInstance().getLogDir());
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

#ifdef __TASK_PARAM_ADJUSTER
    connect(btnTrain, SIGNAL(clicked()), this, SLOT(trainClicked()));
#endif

    connect(btnRun, SIGNAL(clicked()), this, SLOT(runClicked()));
    connect(btnStep, SIGNAL(clicked()), this, SLOT(stepClicked()));
    connect(btnCont, SIGNAL(clicked()), this, SLOT(contClicked()));
    connect(btnBP, SIGNAL(clicked()), this, SLOT(bpToggled()));

		TeachingEventHandler::instance()->stv_Loaded(this);
	}

  void StateMachineViewImpl::setButtonEnableMode(bool isEnable) {
    DDEBUG("StateMachineViewImpl::setExecuteMode");
    btnDelete->setEnabled(isEnable);
    btnEdit->setEnabled(isEnable);
    btnRun->setEnabled(isEnable);
    isExec_ = !isEnable;
  }

  void StateMachineViewImpl::setBPStatus(bool isActive, bool isSet) {
    btnBP->setEnabled(isActive);
    btnBP->setChecked(isSet);
  }

  void StateMachineViewImpl::createStateCommands() {
    commandList_ = TaskExecutor::instance()->getCommandDefList();
    vector<CommandDefParam*>::iterator itCmd = commandList_.begin();
    while (itCmd != commandList_.end()) {
      createCommandNodeTarget((*itCmd)->getId(), (*itCmd)->getDispName());
      ++itCmd;
    }
  }

  void StateMachineViewImpl::setTaskParam(TaskModelParamPtr param) {
    DDEBUG("StateMachineViewImpl::setTaskParam()");

    lblTarget->setText(param->getName());
    //
    if (isExec_ == false) {
      btnTrans->setEnabled(true);
      lstItem->setEnabled(true);
      grhStateMachine->setEnabled(true);
      rdTrue->setEnabled(true);
      rdFalse->setEnabled(true);
      btnSet->setEnabled(true);
      btnEdit->setEnabled(true);

#ifdef __TASK_PARAM_ADJUSTER
      btnTrain->setEnabled(true);
#endif

      btnDelete->setEnabled(true);
      btnRun->setEnabled(true);

      btnStep->setEnabled(false);
      btnCont->setEnabled(false);
    }
    //
    grhStateMachine->setTaskParam(param);
		vector<ElementStmParamPtr> stateList = param->getActiveStateList();
		vector<ConnectionStmParamPtr> transList = param->getActiveTransitionList();
		grhStateMachine->createStateMachine(stateList, transList);
  }

  void StateMachineViewImpl::clearTaskParam() {
    lblTarget->setText("");
		grhStateMachine->setTaskParam(0);
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
    QPixmap *pix = new QPixmap(30, 30);
    pix->fill(QColor(212, 206, 199));
    QPainter* painter = new QPainter(pix);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::black, 3.0));
    painter->drawEllipse(5, 5, 20, 20);
    lstItem->setIconSize(QSize(30, 30));
    QListWidgetItem* item = new QListWidgetItem(name, lstItem);
    item->setIcon(QIcon(*pix));
    item->setData(Qt::UserRole, id);
  }

  void StateMachineViewImpl::setClicked() {
    DDEBUG("StateMachineViewImpl::setClicked()");

    ConnectionNode* conn = grhStateMachine->getCurrentConnection();
		QString strVal = QString::fromStdString("false");
		if (rdTrue->isChecked()) {
			strVal = QString::fromStdString("true");
		}
		TeachingEventHandler::instance()->stv_SetClicked(conn, strVal);
  }

  void StateMachineViewImpl::modeChanged() {
    DDEBUG("StateMachineViewImpl::modeChanged()");

    grhStateMachine->setCntMode(btnTrans->isChecked());
  }

  void StateMachineViewImpl::stepClicked() {
		TeachingEventHandler::instance()->tev_stm_StepClicked();
  }

  void StateMachineViewImpl::contClicked() {
		TeachingEventHandler::instance()->tev_stm_ContClicked();
  }

  void StateMachineViewImpl::bpToggled() {
    DDEBUG("StateMachineViewImpl::bpToggled()");

    grhStateMachine->setBreakPoint(btnBP->isChecked());
  }

  void StateMachineViewImpl::setStepStatus(bool isActive) {
    DDEBUG("StateMachineViewImpl::setStepStatus()");

    btnBP->setChecked(false);
    btnStep->setEnabled(isActive);
    btnCont->setEnabled(isActive);
		TeachingEventHandler::instance()->tev_setBreak(isActive);
  }
  void StateMachineViewImpl::deleteClicked() {
    DDEBUG("StateMachineViewImpl::deleteClicked()");

    grhStateMachine->deleteCurrent();
  }

  void StateMachineViewImpl::editClicked() {
		TeachingEventHandler::instance()->stv_EditClicked(grhStateMachine->getCurrentNode());
  }

#ifdef __TASK_PARAM_ADJUSTER__
  BodyItem* findItemByName (const string& name)
  {
    cnoid::ItemList<BodyItem> bodyItems;
    bodyItems.extractChildItems(RootItem::instance());
    for (size_t i = 0; i < bodyItems.size(); i++) {
      BodyItem* item = bodyItems.get(i);
      if (item->name() == name) { return item; }
    }

    return NULL;
  }
  
  void StateMachineViewImpl::trainClicked() {
    TaskParameterAdjuster::instance()->createLog();

    std::vector<std::tuple<double,double,double,double,double,double> > samples;
    //samplePoses(samples);
    samplePosesRandom(samples);

    bool isReal = SettingManager::getInstance().getIsReal();
    TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());

    ElementNode* currentNode = grhStateMachine->getCurrentNode();
    if (!currentNode) { 
      std::cerr << "Please select a command node" << std::endl;
      return; 
    }
		ElementStmParamPtr stm = currentNode->getElemParam();
    if (stm->getType() != ELEMENT_COMMAND) {
      std::cerr << "Selected node is not a command node" << std::endl;
      return;
    }

    stringstream hdrss;
    QString taskName = currentTask_->getName(); //TaskModelParam*
    hdrss << "Task=" << taskName.toStdString();
    TaskParameterAdjuster::instance()->putIndex(hdrss.str());

    int nodeId = stm->getId();
    QString cmdName = stm->getCmdName();
    QString cmdDspName = stm->getCmdDspName();
    hdrss.str("");
    hdrss.clear(stringstream::goodbit);
    hdrss << "Node=" << cmdDspName.toStdString() 
          << "(" << nodeId << "," << cmdName.toStdString() << ")";
    TaskParameterAdjuster::instance()->putIndex(hdrss.str());

    std::vector<ArgumentParamPtr> argList = stm->getArgList();
    hdrss.str("");
    hdrss.clear(stringstream::goodbit);
    for (auto arg: argList) {
      hdrss << arg->getName().toStdString() << "="
            << arg->getValueDesc().toStdString() << ", ";
    }
    TaskParameterAdjuster::instance()->putIndex(hdrss.str());

    BodyPtr robotBody = findItemByName(SettingManager::getInstance().getRobotModelName())->body();
    Link* wristLink = robotBody->link("RARM_JOINT5");

    VectorXd p0(3);
    p0 = wristLink->p();
    Matrix3d Rw = wristLink->attitude();
    Matrix3d R0;
    Matrix3d R;
    VectorXd dp(3);
    VectorXd rpy0(3);
    VectorXd p(3);
    VectorXd rpy(3);
    double duration = 0.5;
    int armID = 1;
    std::vector<CompositeParamType> params;

    for (auto s: samples) {
      params.clear();

      rpy0(0) = std::get<3>(s); // samples are give in degree
      rpy0(1) = std::get<4>(s);
      rpy0(2) = std::get<5>(s);
      rpy0 = rpy0 * PI/180; 
      R0 = rotFromRpy(rpy0);
      R = Rw * R0; 
      rpy = rpyFromRot(R);
      rpy = rpy * 180/PI;
      dp(0) = std::get<0>(s);
      dp(1) = std::get<1>(s);
      dp(2) = std::get<2>(s);
      p = p0 + Rw * dp;

      Quat q(R0);
      q.normalize();
      
      cout << p[0] << "," << p[1] << "," << p[2] << ", "
           << q.w() << "," << q.x() << "," << q.y() << "," << q.z() << endl;

      params.push_back(p);
      params.push_back(rpy);
      params.push_back(duration);
      params.push_back(armID);
      bool cmdRet = TaskExecutor::instance()->executeCommand("moveArm", params, isReal);

      std::string fileName = TaskParameterAdjuster::instance()->getNextFileName();
      stringstream ss;
      ss << fileName << " "
         << dp(0) << " " << dp(1) << " " << dp(2) << " "
         << q.w() << " " << q.x() << " " << q.y() << " " << q.z();
      TaskParameterAdjuster::instance()->putIndex(ss.str());
      
      if (isReal) {
        TaskParameterAdjuster::instance()->saveFrame(fileName);
      } else {
        SceneView* sceneView = SceneView::instance();
        //sceneView->sceneWidget()->saveImage(fileName);
        QImage qimg = sceneView->sceneWidget()->getImage();
        TaskParameterAdjuster::instance()->postProcessAndSaveImage(qimg, fileName);
      }
    }

    TaskParameterAdjuster::instance()->closeLog();
  }

  template<class T>
  void StateMachineViewImpl::samplePoses(std::vector<T>& samples) 
  {
    samples.clear();
    const std::vector<double> dxs{-0.01, -0.005, 0.005, 0.01};
    const std::vector<double> dys{-0.01, -0.005, 0.005, 0.01};    
    const std::vector<double> dzs{-0.01, -0.005, 0.005, 0.01};    
    const std::vector<double> dRs{0};
    const std::vector<double> dPs{0};
    const std::vector<double> dYs{0};
    // const std::vector<double> dRs{-10, -5, 5, 10};
    // const std::vector<double> dPs{-10, -5, 5, 10};
    // const std::vector<double> dYs{-10, -5, 5, 10};
    
    for (auto dx: dxs) {
      for (auto dy: dys) {
        for (auto dz: dzs) {
          for (auto dR: dRs) {
            for (auto dP: dPs) {
              for (auto dY: dYs) {
                samples.push_back(std::forward_as_tuple(dx,dy,dz,dR,dP,dY));
              }
            }
          }
        }
      }
    }
  }

  template<class T>
  void StateMachineViewImpl::samplePosesRandom(std::vector<T>& samples)
  {
    samples.clear();
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_real_distribution<> rand01(-0.01, 0.01);
    const int nSamples = 500;
//const int nDims = std::tuple_size<T>::value;
    for (int i = 0; i < nSamples; i++) {
      samples.push_back(std::forward_as_tuple(rand01(mt),
                                              rand01(mt),
                                              rand01(mt),
                                              0.0,
                                              0.0,
                                              0.0));
    }
  }
#endif

  void StateMachineViewImpl::runClicked() {
    bool isReal = SettingManager::getInstance().getIsReal();
    ElementNode* target = grhStateMachine->getCurrentNode();

		TeachingEventHandler::instance()->tev_stm_RunClicked(isReal, target);
  }
/////
  StateMachineView::StateMachineView() : viewImpl(0) {
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
