#include "ExecEnvDialog.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

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
