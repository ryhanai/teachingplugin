#include "DecisionDialog.h"
#include "PythonWrapper.h"
#include "TeachingUtil.h"

#include "TeachingDataHolder.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

DesisionDialog::DesisionDialog(TaskModelParamPtr param, ElementStmParamPtr stmParam, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetTask_ = param;
  this->targetStm_ = stmParam;
  //
  lstModel = UIUtil::makeTableWidget(1, true);
  lstModel->setColumnWidth(0, 200);
  lstModel->setHorizontalHeaderLabels(QStringList() << "ID");
  //
  lstParam = UIUtil::makeTableWidget(6, true);
  lstParam->setColumnWidth(0, 200);
  lstParam->setColumnWidth(1, 125);
  lstParam->setColumnWidth(2, 50);
  lstParam->setColumnWidth(3, 80);
  lstParam->setColumnWidth(4, 80);
  lstParam->setColumnWidth(5, 80);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "ID" << "Type" << "ParamType" << "Model" << "Model Param");
  //
  txtCondition = new QTextEdit;

  QFrame* frmRef = new QFrame;
  frmRef->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
  initSizes.append(650);
  initSizes.append(550);
  splitter->setSizes(initSizes);
  splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
	vector<ParameterParamPtr> paramList = targetTask_->getActiveParameterList();
  vector<ModelParamPtr> modelList = targetTask_->getActiveModelList();
  for (ParameterParamPtr param : paramList) {
    int row = lstParam->rowCount();
    lstParam->insertRow(row);
    UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getRName());
    UIUtil::makeTableItem(lstParam, row, 2, UIUtil::getTypeName(param->getType()));
    UIUtil::makeTableItem(lstParam, row, 3, UIUtil::getParamTypeName(param->getParamType()));

    QString strModel = "";
    QString strModelParam = "";
    if (param->getType() == PARAM_KIND_MODEL) {
      vector<ModelParamPtr>::iterator targetModel = find_if(modelList.begin(), modelList.end(), ModelParamComparator(param->getModelId()));
      if (targetModel != modelList.end()) {
        strModel = (*targetModel)->getRName();
        if((*targetModel)->getModelMaster()) {
          vector<ModelParameterParamPtr> modelParamList = (*targetModel)->getModelMaster()->getActiveModelParamList();
          vector<ModelParameterParamPtr>::iterator targetModelParam = find_if(modelParamList.begin(), modelParamList.end(), ModelMasterParamComparator(param->getModelParamId()));
          if (targetModelParam != modelParamList.end()) {
            strModelParam = (*targetModelParam)->getName();
          }
        }
      }
    }
    UIUtil::makeTableItem(lstParam, row, 4, strModel);
    UIUtil::makeTableItem(lstParam, row, 5, strModelParam);
  }
}

void DesisionDialog::showModelInfo() {
	vector<ModelParamPtr> modelList = targetTask_->getActiveModelList();
	for (ModelParamPtr param : modelList) {
    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItem(lstModel, row, 0, param->getRName());
  }
}

void DesisionDialog::oKClicked() {
  DDEBUG("DesisionDialog::cancelClicked()");

  ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(targetTask_);
  std::stringstream errorMsg;
  //
  QString strCond = txtCondition->toPlainText();
  string strError;
  if (handler->checkSyntax(0, targetTask_, strCond, strError) == false) {
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
////////////////
FlowDesisionDialog::FlowDesisionDialog(FlowParamPtr param, ElementStmParamPtr stmParam, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  targetFlow_ = param;
  this->targetStm_ = stmParam;

  lstParam = UIUtil::makeTableWidget(2, true);
  lstParam->setColumnWidth(0, 100);
  lstParam->setColumnWidth(1, 100);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");

  txtCondition = new QTextEdit;
  //
  QFrame* frmRef = new QFrame;
  frmRef->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QVBoxLayout* refLayout = new QVBoxLayout;
  refLayout->setContentsMargins(0, 0, 0, 0);
  frmRef->setLayout(refLayout);
  refLayout->addWidget(lstParam);
  //
  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(frmRef);
  splitter->addWidget(txtCondition);
  QList<int> initSizes;
  initSizes.append(250);
  initSizes.append(350);
  splitter->setSizes(initSizes);
  splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
  resize(600, 300);
  //
  txtCondition->setText(targetStm_->getCondition());
  showParamInfo();
}

void FlowDesisionDialog::showParamInfo() {
  vector<FlowParameterParamPtr> paramList = targetFlow_->getActiveFlowParamList();
  for (FlowParameterParamPtr param : paramList) {
    int row = lstParam->rowCount();
    lstParam->insertRow(row);
    UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getValue());
  }
}

void FlowDesisionDialog::oKClicked() {
  DDEBUG("FlowDesisionDialog::cancelClicked()");

  ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(targetFlow_);
  std::stringstream errorMsg;
  //
  QString strCond = txtCondition->toPlainText();
  string strError;
  if (handler->checkSyntax(targetFlow_, 0, strCond, strError) == false) {
    QMessageBox::warning(this, _("Decision condition setting"), _("Decision node condition is INVALID."));
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

void FlowDesisionDialog::cancelClicked() {
  DDEBUG("FlowDesisionDialog::cancelClicked()");

  close();
}

}
