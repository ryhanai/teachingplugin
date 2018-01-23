#include "ParameterView.h"
#include "ParameterDialog.h"

#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;
using namespace boost;

namespace teaching {

ParameterViewImpl::ParameterViewImpl(QWidget* parent) : QWidget(parent) {
	TeachingEventHandler::instance()->prv_Loaded(this);
}

void ParameterViewImpl::setTaskParam(TaskModelParamPtr param) {
  DDEBUG_V("ParameterViewImpl::setTaskParam() %d", param->getId());

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
  btnEdit->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnEdit->setToolTip(_("Edit Parameters"));

  topLayout->addWidget(btnEdit);
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topFrame);

	vector<ParameterParamPtr> paramList = param->getActiveParameterList();
	vector<ParameterParamPtr>::iterator itParam = paramList.begin();
  while (itParam != paramList.end()) {
    (*itParam)->clearControlList();
    QFrame* eachFrame = new QFrame(this);
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

    int elem_num = (*itParam)->getElemNum();
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
