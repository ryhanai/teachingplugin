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

 ParameterViewImpl::ParameterViewImpl(QWidget* parent)
   : btnEdit(0), canEdit_(true), isFlowView_(false), QWidget(parent) {
	TeachingEventHandler::instance()->prv_Loaded(this);
}
 
 ParameterViewImpl::~ParameterViewImpl() {
   DDEBUG("ParameterViewImpl Destruct");
 }

 void ParameterViewImpl::setTaskParam(TaskModelParamPtr param, bool isFlowView) {
  DDEBUG_V("ParameterViewImpl::setTaskParam() %d, %d", param->getId(), isFlowView);

  this->isFlowView_ = isFlowView;
	clearView();
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
  btnEdit->setIcon(QIcon(":/Teaching/icons/edit.png"));
  btnEdit->setToolTip(_("Edit Parameters"));
  btnEdit->setEnabled(TeachingEventHandler::instance()->canEdit() && this->canEdit_ && !isFlowView);

  topLayout->addWidget(btnEdit);
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topFrame);

  for (ParameterParamPtr targetParam : param->getActiveParameterList()) {
    targetParam->setActive(false);
    if (targetParam->getType() == PARAM_KIND_MODEL) continue;
    //if (isFlowView && targetParam->getHide() == 0) continue;

    targetParam->setActive(true);
    targetParam->clearControlList();
    QFrame* eachFrame = new QFrame(this);
    eachFrame->setProperty("hide", targetParam->getHide());
    if (isFlowView_  && targetParam->getHide() == 0) {
      eachFrame->setEnabled(false);
    } else {
      eachFrame->setEnabled(TeachingEventHandler::instance()->canEdit() && this->canEdit_);
    }
    frameList_.push_back(eachFrame);
    QHBoxLayout* eachLayout = new QHBoxLayout;
    eachLayout->setContentsMargins(0, 0, 0, 0);
    eachFrame->setLayout(eachLayout);

    QLabel* lblName = new QLabel(targetParam->getName());
    eachLayout->addWidget(lblName);
    if (targetParam->getHide() != 0) {
      QPalette pal = lblName->palette();
      pal.setColor(QPalette::WindowText, Qt::red);
      lblName->setPalette(pal);
    }

    int elem_num = 1;
    //DDEBUG_V("ParamType: %d", targetParam->getParamType());
    if (targetParam->getParamType() == PARAM_TYPE_FRAME) elem_num = 6;
    for (int index = 0; index < elem_num; index++) {
      QLineEdit* txtEach;
      if (index < targetParam->getControlNum()) {
        txtEach = targetParam->getControl(index);
      } else {
        txtEach = new QLineEdit;
        targetParam->addControl(txtEach);
      }
      txtEach->setText(QString::fromStdString(targetParam->getValues(index)).trimmed());
      eachLayout->addWidget(txtEach);
      textList_.push_back(txtEach);
    }
    QLabel* lblUnit = new QLabel(targetParam->getUnit());
    eachLayout->addWidget(lblUnit);
    eachLayout->addStretch();
    mainLayout->addWidget(eachFrame);
  }
  mainLayout->addStretch();
  setLayout(mainLayout);
  DDEBUG("ParameterViewImpl::setTaskParam() End");
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
  DDEBUG("ParameterViewImpl::clearView():End");
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
    btnEdit->setEnabled(canEdit && this->canEdit_ && !isFlowView_);
    for(QFrame* frame : frameList_) {
      int hide = frame->property("hide").toInt();
      if (isFlowView_  && hide ==0 ) {
        frame->setEnabled(false);
      } else {
        frame->setEnabled(canEdit && this->canEdit_);
      }
    }
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
