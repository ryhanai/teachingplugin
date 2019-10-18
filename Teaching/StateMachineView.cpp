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

#include "NodeEditor/NodeStyle.hpp"
#include "NodeEditor/ConnectionStyle.hpp"
#include "NodeEditor/FlowViewStyle.hpp"

#include "NodeEditor/FlowScene.hpp"
#include "NodeEditor/models.hpp"

#include "gettext.h"
#include "LoggerUtil.h"


#ifdef __TASK_PARAM_ADJUSTER__
#include "TaskParamAdjust/TaskParameterAdjuster.h"
#endif

namespace teaching {

  StateMachineViewImpl::StateMachineViewImpl(QWidget* parent) : isExec_(false) {
    lblTarget = new QLabel;
    lblTarget->setText("");

    btnDelete = new QPushButton(_("Delete"));
    btnDelete->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    btnDelete->setToolTip(_("Delete selected element"));
    btnDelete->setEnabled(false);

    btnEdit = new QPushButton(_("Edit"));
    btnEdit->setIcon(QIcon(":/Teaching/icons/edit.png"));
    btnEdit->setToolTip(_("Edit target state"));
    btnEdit->setEnabled(false);


#ifdef __TASK_PARAM_ADJUSTER__
    btnTrain = new QPushButton(_("Train"));
    btnTrain->setToolTip(_("Record data for parameter adjustment"));
    btnTrain->setEnabled(false);
#endif


    btnRun = new QPushButton(_("Command"));
    btnRun->setIcon(QIcon(":/Teaching/icons/run_command.png"));
    btnRun->setToolTip(_("Run selected Command"));
    btnRun->setEnabled(false);

    btnBP = new QPushButton(_("B.P."));
    btnBP->setIcon(QIcon(":/Teaching/icons/set_breakpoint.png"));
    btnBP->setToolTip(_("Set/Unset BreakPoint"));
    btnBP->setCheckable(true);
    btnBP->setEnabled(false);

    btnStep = new QPushButton(_("Step"));
    btnStep->setIcon(QIcon(":/Teaching/icons/step.png"));
    btnStep->setToolTip(_("Run by Step"));
    btnStep->setEnabled(false);

    btnCont = new QPushButton(_("Cont."));
    btnCont->setIcon(QIcon(":/Teaching/icons/continue.png"));
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

    lstItem = new ItemList(QString::fromStdString("application/StateMachineItem"));
    lstItem->setStyleSheet("background-color: rgb( 212, 206, 199 );");
    lstItem->setEnabled(false);
    lstItem->createInitialNodeTarget();
    lstItem->createFinalNodeTarget();
    lstItem->createDecisionNodeTarget();
    lstItem->createMergeNodeTarget();
    lstItem->createMoveCNodeTarget();

		setStyle();
		FlowScene* scene = new FlowScene(registerDataModels());
		grhStateMachine = new StateMachineEditor(scene, this);
		grhStateMachine->setAcceptDrops(true);
		grhStateMachine->setEnabled(false);

    setAcceptDrops(true);

    QVBoxLayout* itemLayout = new QVBoxLayout;
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->addWidget(lstItem);
    frmItem_ = new QFrame;
    frmItem_->setLayout(itemLayout);

    QSplitter* splBase = new QSplitter(Qt::Horizontal);
    splBase->addWidget(frmItem_);
    splBase->addWidget(grhStateMachine);
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

  StateMachineViewImpl::~StateMachineViewImpl() {
    DDEBUG("StateMachineViewImpl Destruct");
  }

  void StateMachineViewImpl::setBPStatus(bool isActive, bool isSet) {
    btnBP->setEnabled(isActive);
    btnBP->setChecked(isSet);
  }

  void StateMachineViewImpl::createStateCommands() {
  	DDEBUG("StateMachineViewImpl::createStateCommands");
    commandList_ = TaskExecutor::instance()->getCommandDefList();
    vector<CommandDefParam*>::iterator itCmd = commandList_.begin();
    while (itCmd != commandList_.end()) {
      DDEBUG_V("CmdName:%s, DispName:%s", (*itCmd)->getCmdName().toStdString().c_str(), (*itCmd)->getDispName().toStdString().c_str())
      createCommandNodeTarget((*itCmd)->getCmdName(), (*itCmd)->getDispName());
      ++itCmd;
    }
  }

  void StateMachineViewImpl::setTaskParam(TaskModelParamPtr param) {
    DDEBUG("StateMachineViewImpl::setTaskParam()");

    lblTarget->setText(param->getName());
    //
	  if (isExec_ == false) {
		  lstItem->setEnabled(true);
		  grhStateMachine->setEnabled(true);
//	    btnEdit->setEnabled(true);

#ifdef __TASK_PARAM_ADJUSTER
      btnTrain->setEnabled(true);
#endif

//      btnDelete->setEnabled(true);
//      btnRun->setEnabled(true);

      btnStep->setEnabled(false);
      btnCont->setEnabled(false);
    }
    //
    grhStateMachine->setTargetParam(param);
	  grhStateMachine->createStateMachine(param->getActiveStateList(), param->getActiveTransitionList());
  }

  void StateMachineViewImpl::clearTaskParam() {
    lblTarget->setText("");
		grhStateMachine->setTargetParam(0);
		//
    lstItem->setEnabled(false);
    grhStateMachine->removeAll();
    grhStateMachine->setEnabled(false);
    btnRun->setEnabled(false);
  }

  void StateMachineViewImpl::createCommandNodeTarget(QString cmdName, QString dispName) {
    QPixmap *pix = new QPixmap(30, 30);
    pix->fill(QColor(212, 206, 199));
    QPainter* painter = new QPainter(pix);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::black, 2.0));
    painter->drawEllipse(5, 5, 20, 20);
    lstItem->setIconSize(QSize(30, 30));
    QListWidgetItem* item = new QListWidgetItem(dispName, lstItem);
    item->setIcon(QIcon(*pix));
    item->setData(Qt::UserRole, cmdName);
  }

  void StateMachineViewImpl::stepClicked() {
		TeachingEventHandler::instance()->tev_stm_StepClicked();
  }

  void StateMachineViewImpl::contClicked() {
		TeachingEventHandler::instance()->tev_stm_ContClicked();
  }

  void StateMachineViewImpl::bpToggled() {
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
    grhStateMachine->deleteSelectedNodes();
  }

  void StateMachineViewImpl::editClicked() {
		DDEBUG("StateMachineViewImpl::editClicked()");
		ElementStmParamPtr target = grhStateMachine->getCurrentNode();
		TeachingEventHandler::instance()->stv_EditClicked(target);
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
    TeachingEventHandler::instance()->updateExecState(false);
		TeachingEventHandler::instance()->tev_stm_RunClicked(grhStateMachine->getCurrentNode());
	}

void StateMachineViewImpl::setStyle() {
	FlowViewStyle::setStyle(
			R"(
  {
    "FlowViewStyle": {
      "BackgroundColor": [255, 255, 240],
      "FineGridColor": [245, 245, 230],
      "CoarseGridColor": [235, 235, 220]
    }
  }
  )");

		NodeStyle::setNodeStyle(
			R"(
  {
    "NodeStyle": {
      "NormalBoundaryColor": "darkgray",
      "SelectedBoundaryColor": "deepskyblue",
      "GradientColor0": "mintcream",
      "GradientColor1": "mintcream",
      "GradientColor2": "mintcream",
      "GradientColor3": "mintcream",
      "ShadowColor": [200, 200, 200],
      "FontColor": [10, 10, 10],
      "FontColorFaded": [100, 100, 100],
      "ConnectionPointColor": "white",
      "PenWidth": 2.0,
      "HoveredPenWidth": 2.5,
      "ConnectionPointDiameter": 10.0,
      "Opacity": 1.0
    }
  }
  )");

		ConnectionStyle::setConnectionStyle(
			R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "black",
      "SelectedColor": "gray",
      "SelectedHaloColor": "deepskyblue",
      "HoveredColor": "deepskyblue",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": false
    }
  }
  )");
}

std::shared_ptr<DataModelRegistry> StateMachineViewImpl::registerDataModels() {
	auto ret = std::make_shared<DataModelRegistry>();

	ret->registerModel<TaskDataModel>("Tasks");
	ret->registerModel<ParamDataModel>("Variables");
	ret->registerModel<DecisionDataModel>("Syntaxes");
  ret->registerModel<MergeDataModel>("Syntaxes");
  ret->registerModel<FinalDataModel>("Syntaxes");
	ret->registerModel<InitialDataModel>("Syntaxes");

	return ret;
}

void StateMachineViewImpl::setEditMode(bool canEdit) {
  DDEBUG_V("StateMachineViewImpl::setEditMode : %d", canEdit);

  btnRun->setEnabled(!canEdit);
  btnBP->setEnabled(!canEdit);
  //
  btnDelete->setEnabled(canEdit);
  btnEdit->setEnabled(canEdit);

  if (canEdit) {
    frmItem_->setHidden(false);
  } else {
    frmItem_->setHidden(true);
  }

  grhStateMachine->setEditMode(canEdit);
}

void StateMachineViewImpl::setExecState(bool isActive) {
  btnRun->setEnabled(isActive);
}

void StateMachineViewImpl::updateCommandList() {
	DDEBUG("StateMachineViewImpl::updateCommandList");
  lstItem->clear();
  clearTaskParam();

  lstItem->createInitialNodeTarget();
  lstItem->createFinalNodeTarget();
  lstItem->createDecisionNodeTarget();
  lstItem->createMergeNodeTarget();
  lstItem->createMoveCNodeTarget();
  createStateCommands();
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
  
//////////
ItemList::ItemList(QString elemType, QWidget* parent)
	: elemType_(elemType), QListWidget(parent) {
}

void ItemList::mousePressEvent(QMouseEvent* event) {
	DDEBUG("ItemList::mousePressEvent");
	if (event->button() == Qt::LeftButton) {
		startPos = event->pos();
	}
	QListWidget::mousePressEvent(event);
}

void ItemList::mouseMoveEvent(QMouseEvent* event) {
	DDEBUG("ItemList::mouseMoveEvent");
	if (event->buttons() == Qt::LeftButton) {
		int distance = (event->pos() - startPos).manhattanLength();
		if (QApplication::startDragDistance() <= distance) {
			QModelIndexList indexes = selectionModel()->selection().indexes();
			if (0 < indexes.size()) {
				QModelIndex selected = indexes.at(0);
				if (0 <= selected.row()) {
					QByteArray itemData;
					QMimeData* mimeData = new QMimeData;
					mimeData->setData(elemType_, itemData);
					mimeData->setText(item(selected.row())->text());
					mimeData->setProperty("CommandId", item(selected.row())->data(Qt::UserRole));
					QDrag* drag = new QDrag(this);
					drag->setMimeData(mimeData);
					drag->exec(Qt::CopyAction);
				}
			}
		}
	}
	QListWidget::mouseMoveEvent(event);
}

void ItemList::createInitialNodeTarget() {
	QPixmap *pix = new QPixmap(30, 30);
	pix->fill(QColor(212, 206, 199));
	QPainter* painter = new QPainter(pix);
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
	painter->drawEllipse(5, 5, 20, 20);

	this->setIconSize(QSize(30, 30));
	QListWidgetItem* item = new QListWidgetItem("Start", this);
	item->setIcon(QIcon(*pix));
}

void ItemList::createFinalNodeTarget() {
	QPixmap *pix = new QPixmap(30, 30);
	pix->fill(QColor(212, 206, 199));
	QPainter* painter = new QPainter(pix);
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
  painter->setPen(QPen(Qt::black, 2.0));
	painter->drawEllipse(5, 5, 20, 20);
	painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
	painter->drawEllipse(10, 10, 10, 10);

	this->setIconSize(QSize(30, 30));
	QListWidgetItem* item = new QListWidgetItem("Final", this);
	item->setIcon(QIcon(*pix));
}

void ItemList::createDecisionNodeTarget() {
	QPixmap *pix = new QPixmap(30, 30);
	pix->fill(QColor(212, 206, 199));
	QPainter* painter = new QPainter(pix);
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
	QPolygon polygon;
	polygon << QPoint(15.0, 5.0) << QPoint(0.0, 15.0)
		<< QPoint(15.0, 25.0) << QPoint(30.0, 15.0) << QPoint(15.0, 5.0);
	painter->drawPolygon(polygon);
	this->setIconSize(QSize(30, 30));
	QListWidgetItem* item = new QListWidgetItem("Decision", this);
	item->setIcon(QIcon(*pix));
}

void ItemList::createMergeNodeTarget() {
  QPixmap *pix = new QPixmap(30, 30);
  pix->fill(QColor(212, 206, 199));
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  QPolygon polygon;
  polygon << QPoint(15.0, 5.0) << QPoint(0.0, 15.0)
    << QPoint(15.0, 25.0) << QPoint(30.0, 15.0) << QPoint(15.0, 5.0);
  painter->drawPolygon(polygon);
  this->setIconSize(QSize(30, 30));
  QListWidgetItem* item = new QListWidgetItem("Merge", this);
  item->setIcon(QIcon(*pix));
}

void ItemList::createMoveCNodeTarget() {
	QPixmap *pix = new QPixmap(30, 30);
	pix->fill(QColor(212, 206, 199));
	QPainter* painter = new QPainter(pix);
	painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 2.0));
	painter->drawEllipse(5, 5, 20, 20);

  QPolygon polygon;
  polygon << QPoint(10.0, 8.0) << QPoint(23.0, 15.0)
    << QPoint(10.0, 22.0) << QPoint(10.0, 8.0);
  painter->drawPolygon(polygon);

	this->setIconSize(QSize(30, 30));
	QListWidgetItem* item = new QListWidgetItem("MoveC", this);
	item->setIcon(QIcon(*pix));
}
//////////
ExecEnvDialog::ExecEnvDialog(TaskModelParamPtr param, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetTask_ = param;
  //
  txtEnv = new QTextEdit;

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
  mainLayout->addWidget(txtEnv);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Python Import"));
  resize(400, 100);
  //
  txtEnv->setText(targetTask_->getExecEnv());
}

void ExecEnvDialog::oKClicked() {
  DDEBUG("ExecEnvDialog::cancelClicked()");

  targetTask_->setExecEnv(txtEnv->toPlainText());
  targetTask_->setUpdate();
  //
  close();
}

void ExecEnvDialog::cancelClicked() {
  DDEBUG("ExecEnvDialog::cancelClicked()");

  close();
}

}
