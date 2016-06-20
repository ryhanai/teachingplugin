#include "ParameterView.h"
#include <cnoid/BodyBar>
#include <cnoid/EigenUtil>
#include <boost/bind.hpp>
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "DataBaseManager.h"
#include "ParameterDialog.h"

using namespace std;
using namespace cnoid;
using namespace boost;

namespace teaching {

ModelParameterGroup::ModelParameterGroup(ParameterParam* source, ModelParam* model, QHBoxLayout* layout, QWidget* parent)
  : targetParam_(source), targetModel_(model),
     currentBodyItem_(0),
     updateKinematicStateLater(bind(&ModelParameterGroup::updateKinematicState, this, true), IDLE_PRIORITY_LOW),
     QWidget(parent), os_(MessageView::mainInstance()->cout()) {
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
    //
    connect(leX_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
    connect(leY_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
    connect(leZ_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
    connect(leRx_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
    connect(leRy_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
    connect(leRz_, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));

    if (targetModel_->getModelItem() == NULL) {
      return;
    }
    currentBodyItem_ = targetModel_->getModelItem().get();
    connectionToKinematicStateChanged = targetModel_->getModelItem().get()->sigKinematicStateChanged().connect(updateKinematicStateLater);
}

void ModelParameterGroup::modelPositionChanged() {
  if(targetModel_) {
    if( targetModel_->getModelItem() ) {
      double newX = leX_->text().toDouble();
      double newY = leY_->text().toDouble();
      double newZ = leZ_->text().toDouble();
      double newRx = leRx_->text().toDouble();
      double newRy = leRy_->text().toDouble();
      double newRz = leRz_->text().toDouble();
      if(dbl_eq(newX, targetModel_->getPosX())==false
        || dbl_eq(newY, targetModel_->getPosY())==false
        || dbl_eq(newZ, targetModel_->getPosZ())==false
        || dbl_eq(newRx, targetModel_->getRotRx())==false
        || dbl_eq(newRy, targetModel_->getRotRy())==false
        || dbl_eq(newRz, targetModel_->getRotRz())==false ) {
        ChoreonoidUtil::updateModelItemPosition(targetModel_->getModelItem(), newX, newY, newZ, newRx, newRy, newRz);
        targetModel_->setPosX(newX);
        targetModel_->setPosY(newY);
        targetModel_->setPosZ(newZ);
        targetModel_->setRotRx(newRx);
        targetModel_->setRotRy(newRy);
        targetModel_->setRotRz(newRz);
      }
    }
  }
}

void ModelParameterGroup::updateKinematicState(bool blockSignals) {
  if(currentBodyItem_){
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
  if(connectionToKinematicStateChanged.connected()){
    connectionToKinematicStateChanged.disconnect();
  }
}
/////
ParameterViewImpl::ParameterViewImpl(QWidget* parent)
  : QWidget(parent), targetTask_(0), os_(MessageView::mainInstance()->cout()) {
}

void ParameterViewImpl::setTaskParam(TaskModelParam* param) {
  setInputValues();
  clearView();
  this->targetTask_ = param;
  //
  QFrame* topFrame = new QFrame(this);
  frameList_.push_back(topFrame);
  QHBoxLayout* topLayout = new QHBoxLayout(topFrame);
  topLayout->setContentsMargins(0, 0, 0, 0);
  lblName = new QLabel;
  lblName->setText(param->getName());
  topLayout->addWidget(lblName);
  topLayout->addStretch();
  //btnEdit = new QPushButton("Edit");
  btnEdit = new QPushButton();
  btnEdit->setIcon(QIcon(":/Teaching/icons/Options.png"));
  btnEdit->setToolTip(tr("Edit Parameter"));

  topLayout->addWidget(btnEdit);
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topFrame);

  vector<ParameterParam*> paramList = param->getParameterList();
  vector<ParameterParam*>::iterator itParam = paramList.begin();
  while (itParam != paramList.end() ) {
    (*itParam)->clearControlList();
    QFrame* eachFrame = new QFrame(this);
    frameList_.push_back(eachFrame);
    QHBoxLayout* eachLayout = new QHBoxLayout;
    eachLayout->setContentsMargins(0, 0, 0, 0);
    eachFrame->setLayout(eachLayout);
    QLabel* lblName = new QLabel((*itParam)->getName());
    eachLayout->addWidget(lblName);

    if((*itParam)->getType() == PARAM_KIND_MODEL) {
      vector<ModelParam*> modelList = param->getModelList();
      for(int index=0; index<modelList.size();index++) {
        ModelParam* model = modelList[index];
        if(model->getRName() == (*itParam)->getModelName()) {
          ModelParameterGroup* modelParam = new ModelParameterGroup(*itParam, model, eachLayout);
          modelList_.push_back(modelParam);
          break;
        }
      }

    } else {
      int elem_num = (*itParam)->getElemNum();
      for(int index=0; index<elem_num; index++) {
        QLineEdit* txtEach = new QLineEdit;
        txtEach->setText(QString::fromStdString((*itParam)->getValues(index)).trimmed());
        eachLayout->addWidget(txtEach);
        (*itParam)->addControl(txtEach);
        textList_.push_back(txtEach);
      }
    }
    //QLabel* lblUnit = new QLabel((*itParam)->getUnit());
    //eachLayout->addWidget(lblUnit);
    eachLayout->addStretch();
    mainLayout->addWidget(eachFrame);
    ++itParam;
  }
  mainLayout->addStretch();
  setLayout(mainLayout);
}

void ParameterViewImpl::setInputValues() {
  if(targetTask_) {
    vector<ParameterParam*> paramList = targetTask_->getParameterList();
    vector<ParameterParam*>::iterator itParam = paramList.begin();
    while (itParam != paramList.end() ) {
      (*itParam)->saveValues();
      ++itParam;
    }
  }
}

void ParameterViewImpl::clearView() {
  if( layout() ) {
    layout()->removeWidget(lblName);
    delete lblName;
    layout()->removeWidget(btnEdit);
    delete btnEdit;
    //
    vector<QLineEdit*>::iterator itText = textList_.begin();
    while (itText != textList_.end() ) {
      layout()->removeWidget(*itText);
      delete *itText;
      ++itText;
    }
    textList_.clear();
    //
    vector<QFrame*>::iterator itFrame = frameList_.begin();
    while (itFrame != frameList_.end() ) {
      layout()->removeWidget(*itFrame);
      delete *itFrame;
      ++itFrame;
    }
    frameList_.clear();
    //
    vector<ModelParameterGroup*>::iterator itModel = modelList_.begin();
    while (itModel != modelList_.end() ) {
      (*itModel)->disconnectKinematics();
      delete *itModel;
      ++itModel;
    }
    modelList_.clear();
    //
    delete layout();
  }
}

void ParameterViewImpl::clearTaskParam() {
  setInputValues();
  clearView();
  this->targetTask_ = 0;
}

void ParameterViewImpl::editClicked() {
  ParameterDialog dialog(targetTask_, this);
  dialog.exec();
  setTaskParam(targetTask_);
}

/////
ParameterView::ParameterView(): viewImpl(0) {
    setName("Parameter");
    setDefaultLayoutArea(View::BOTTOM);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewImpl = new ParameterViewImpl(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(viewImpl);
    setLayout(vbox);
}

ParameterView::~ParameterView() {
};

}
