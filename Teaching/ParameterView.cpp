#include "ParameterView.h"
#include <cnoid/BodyBar>
#include <cnoid/EigenUtil>
#include <boost/bind.hpp>
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "ParameterDialog.h"

#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;
using namespace boost;

namespace teaching {

ModelParameterGroup::ModelParameterGroup(ParameterParamPtr source, ModelParamPtr model, QHBoxLayout* layout, QWidget* parent)
  : targetParam_(source), targetModel_(model),
  currentBodyItem_(0),
  updateKinematicStateLater(bind(&ModelParameterGroup::updateKinematicState, this, true), IDLE_PRIORITY_LOW),
  QWidget(parent) {
  leX_ = new QLineEdit;
  leX_->setText(QString::number(model->getPosX(), 'f', 4));
  layout->addWidget(leX_);
  source->addControl(leX_);
  //
  leY_ = new QLineEdit;
  leY_->setText(QString::number(model->getPosY(), 'f', 4));
  layout->addWidget(leY_);
  source->addControl(leY_);
  //
  leZ_ = new QLineEdit;
  leZ_->setText(QString::number(model->getPosZ(), 'f', 4));
  layout->addWidget(leZ_);
  source->addControl(leZ_);
  //
  leRx_ = new QLineEdit;
  leRx_->setText(QString::number(model->getRotRx(), 'f', 4));
  layout->addWidget(leRx_);
  source->addControl(leRx_);
  //
  leRy_ = new QLineEdit;
  leRy_->setText(QString::number(model->getRotRy(), 'f', 4));
  layout->addWidget(leRy_);
  source->addControl(leRy_);
  //
  leRz_ = new QLineEdit;
  leRz_->setText(QString::number(model->getRotRz(), 'f', 4));
  layout->addWidget(leRz_);
  source->addControl(leRz_);

  if (targetModel_->getModelMaster()->getModelItem() == NULL) {
    return;
  }
  currentBodyItem_ = targetModel_->getModelMaster()->getModelItem().get();
  connectionToKinematicStateChanged = targetModel_->getModelMaster()->getModelItem().get()->sigKinematicStateChanged().connect(updateKinematicStateLater);
}

void ModelParameterGroup::updateKinematicState(bool blockSignals) {
  if (currentBodyItem_) {
    Link* currentLink = currentBodyItem_->body()->rootLink();
    targetModel_->setPosX(currentLink->p()[0]);
    targetModel_->setPosY(currentLink->p()[1]);
    targetModel_->setPosZ(currentLink->p()[2]);

    const Matrix3 R = currentLink->attitude();
    const Vector3 rpy = rpyFromRot(R);
    targetModel_->setRotRx(degree(rpy[0]));
    targetModel_->setRotRy(degree(rpy[1]));
    targetModel_->setRotRz(degree(rpy[2]));

    leX_->setText(QString::number(targetModel_->getPosX(), 'f', 4));
    leY_->setText(QString::number(targetModel_->getPosY(), 'f', 4));
    leZ_->setText(QString::number(targetModel_->getPosZ(), 'f', 4));
    leRx_->setText(QString::number(targetModel_->getRotRx(), 'f', 4));
    leRy_->setText(QString::number(targetModel_->getRotRy(), 'f', 4));
    leRz_->setText(QString::number(targetModel_->getRotRz(), 'f', 4));
  }
}

void ModelParameterGroup::disconnectKinematics() {
  DDEBUG("ModelParameterGroup::disconnectKinematics");
  if (connectionToKinematicStateChanged.connected()) {
    connectionToKinematicStateChanged.disconnect();
  }
}
/////
 ParameterViewImpl::ParameterViewImpl(QWidget* parent)
   : btnEdit(0), canEdit_(true), QWidget(parent) {
	TeachingEventHandler::instance()->prv_Loaded(this);
}
 
 ParameterViewImpl::~ParameterViewImpl() {
   DDEBUG("ParameterViewImpl Destruct");
 }

 void ParameterViewImpl::setTaskParam(TaskModelParamPtr param, bool canEdit) {
  DDEBUG_V("ParameterViewImpl::setTaskParam() %d", param->getId());

	clearView();
  this->canEdit_ = canEdit;
  //
  QFrame* topFrame = new QFrame(this);
  frameList_.push_back(topFrame);
  QHBoxLayout* topLayout = new QHBoxLayout(topFrame);
  topLayout->setContentsMargins(0, 0, 0, 0);
  lblName = new QLabel;
  lblName->setText(param->getName());
  topLayout->addWidget(lblName);
  topLayout->addStretch();

  btnEdit = new QPushButton(_("Edit"));
  btnEdit->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnEdit->setToolTip(_("Edit Parameters"));
  btnEdit->setEnabled(TeachingEventHandler::instance()->canEdit() && this->canEdit_);

  topLayout->addWidget(btnEdit);
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topFrame);

	vector<ParameterParamPtr> paramList = param->getActiveParameterList();
	vector<ParameterParamPtr>::iterator itParam = paramList.begin();
  while (itParam != paramList.end()) {
    (*itParam)->clearControlList();
    QFrame* eachFrame = new QFrame(this);
    if ((*itParam)->getType() == PARAM_KIND_MODEL) eachFrame->setVisible(false);
    eachFrame->setEnabled(canEdit);
    frameList_.push_back(eachFrame);
    QHBoxLayout* eachLayout = new QHBoxLayout;
    eachLayout->setContentsMargins(0, 0, 0, 0);
    eachFrame->setLayout(eachLayout);

    QLabel* lblName = new QLabel((*itParam)->getName());
    eachLayout->addWidget(lblName);
		if ((*itParam)->getHide() != 0) {
			QPalette pal = lblName->palette();
			pal.setColor(QPalette::WindowText, Qt::red);
			lblName->setPalette(pal);
		}

    if ((*itParam)->getType() == PARAM_KIND_MODEL) {
      vector<ModelParamPtr> modelList = param->getActiveModelList();
      for (int index = 0; index < modelList.size(); index++) {
        ModelParamPtr model = modelList[index];
        if (model->getId() == (*itParam)->getModelId()) {
          ModelParameterGroupPtr modelParam = std::make_shared<ModelParameterGroup>(*itParam, model, eachLayout);
          modelList_.push_back(modelParam);
          break;
        }
      }

    } else {
      int elem_num = 1;
      DDEBUG_V("ParamType: %d", (*itParam)->getParamType());
      if ((*itParam)->getParamType() == PARAM_TYPE_FRAME) elem_num = 6;
      for (int index = 0; index < elem_num; index++) {
        QLineEdit* txtEach;
        if (index < (*itParam)->getControlNum()) {
          txtEach = (*itParam)->getControl(index);
        } else {
          txtEach = new QLineEdit;
          (*itParam)->addControl(txtEach);
        }
        txtEach->setText(QString::fromStdString((*itParam)->getValues(index)).trimmed());
        eachLayout->addWidget(txtEach);
        textList_.push_back(txtEach);
      }
    }
    QLabel* lblUnit = new QLabel((*itParam)->getUnit());
    eachLayout->addWidget(lblUnit);
    eachLayout->addStretch();
    mainLayout->addWidget(eachFrame);
    ++itParam;
  }
  mainLayout->addStretch();
  setLayout(mainLayout);
}

void ParameterViewImpl::clearView() {
  DDEBUG("ParameterViewImpl::clearView()");
  if (layout()) {
		layout()->removeWidget(lblName);
		delete lblName;
		layout()->removeWidget(btnEdit);
		delete btnEdit;
    btnEdit = 0;
		//
    vector<QLineEdit*>::iterator itText = textList_.begin();
		while (itText != textList_.end()) {
      layout()->removeWidget(*itText);
      ++itText;
    }
		textList_.clear();
		//
    vector<ModelParameterGroupPtr>::iterator itModel = modelList_.begin();
    while (itModel != modelList_.end()) {
      (*itModel)->disconnectKinematics();
      ++itModel;
    }
    modelList_.clear();
    //
    vector<QFrame*>::iterator itFrame = frameList_.begin();
		while (itFrame != frameList_.end()) {
      layout()->removeWidget(*itFrame);
      delete *itFrame;
      ++itFrame;
    }
		frameList_.clear();
    //
		delete layout();
  }
}

void ParameterViewImpl::clearTaskParam() {
	TeachingEventHandler::instance()->prv_SetInputValues();
  clearView();
}

void ParameterViewImpl::editClicked() {
  DDEBUG("ParameterViewImpl::editClicked()");

  ParameterDialog dialog(this);
  dialog.exec();
}

void ParameterViewImpl::setEditMode(bool canEdit) {
  if (btnEdit) {
    btnEdit->setEnabled(canEdit && this->canEdit_);
  }
}
/////
ParameterView::ParameterView() : viewImpl(0) {
  setName(_("Parameter"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new ParameterViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::LEFT_BOTTOM);
}

ParameterView::~ParameterView() {
};
}
