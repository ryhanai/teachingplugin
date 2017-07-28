#include "DecisionDialog.h"
#include "PythonWrapper.h"
#include "TeachingUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

DesisionDialog::DesisionDialog(TaskModelParam* param, ElementStmParam* stmParam, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetTask_ = param;
  this->targetStm_ = stmParam;
  //
  lstModel = UIUtil::makeTableWidget(2, true);
  lstModel->setColumnWidth(0, 200);
  lstModel->setColumnWidth(1, 150);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Name" << "ID");
  //
  lstParam = UIUtil::makeTableWidget(2, true);
  lstParam->setColumnWidth(0, 200);
  lstParam->setColumnWidth(1, 150);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "ID");
  //
  txtCondition = new QTextEdit;

  QFrame* frmRef = new QFrame;
  QVBoxLayout* refLayout = new QVBoxLayout;
  refLayout->setContentsMargins(0, 0, 0, 0);
  frmRef->setLayout(refLayout);
  refLayout->addWidget(lstModel);
  refLayout->addWidget(lstParam);
  //
  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(frmRef);
  splitter->addWidget(txtCondition);
  QList<int> initSizes;
  initSizes.append(400);
  initSizes.append(800);
  splitter->setSizes(initSizes);
  //
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
  mainLayout->addWidget(splitter);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Decision condition setting"));
  resize(1200, 450);
  //
  showModelInfo();
  showParamInfo();
  txtCondition->setText(targetStm_->getCondition());
}

void DesisionDialog::showParamInfo() {
  for (int index = 0; index < targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstParam->rowCount();
    lstParam->insertRow(row);
    UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getRName());
  }
}

void DesisionDialog::showModelInfo() {
  for (int index = 0; index < targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItem(lstModel, row, 0, param->getName());
    UIUtil::makeTableItem(lstModel, row, 1, param->getRName());
  }
}

void DesisionDialog::oKClicked() {
  DDEBUG("DesisionDialog::cancelClicked()");

  ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(targetTask_);
  std::stringstream errorMsg;
  //
  QString strCond = txtCondition->toPlainText();
  string strError;
  if (handler->checkSyntax(targetTask_, strCond, strError) == false) {
    QMessageBox::warning(this, _("Decision condition setting"), QString::fromStdString(strError));
    EstimatorFactory::getInstance().deleteArgEstimator(handler);
    return;
  }
  EstimatorFactory::getInstance().deleteArgEstimator(handler);
  //
  targetStm_->setCondition(strCond);
  targetStm_->setUpdate();
  //
  close();
}

void DesisionDialog::cancelClicked() {
  DDEBUG("DesisionDialog::cancelClicked()");

  close();
}

}
