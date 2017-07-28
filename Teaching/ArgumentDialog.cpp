#include "ArgumentDialog.h"
#include "PythonWrapper.h"
#include "TeachingUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

ArgumentDialog::ArgumentDialog(TaskModelParam* param, ElementStmParam* stmParam, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
  curArgIdx_(NULL_ID), curArgParam_(0), curActionIdx_(NULL_ID), curActionParam_(0) {
  this->targetTask_ = param;
  this->targetStm_ = stmParam;
  //
  lstModel = UIUtil::makeTableWidget(2, true);
  lstModel->setColumnWidth(0, 200);
  lstModel->setColumnWidth(1, 150);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Name" << "ID");
  //
  lstParam = UIUtil::makeTableWidget(4, true);
  lstParam->setColumnWidth(0, 200);
  lstParam->setColumnWidth(1, 150);
  lstParam->setColumnWidth(2, 50);
  lstParam->setColumnWidth(3, 50);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "ID" << "Type" << "Num");
  //
  QFrame* frmRef = new QFrame;
  QVBoxLayout* refLayout = new QVBoxLayout;
  refLayout->setContentsMargins(0, 0, 0, 0);
  frmRef->setLayout(refLayout);
  refLayout->addWidget(lstModel);
  refLayout->addWidget(lstParam);
  /////

  lstHandling = UIUtil::makeTableWidget(3, false);
  lstHandling->setColumnWidth(0, 100);
  lstHandling->setColumnWidth(1, 150);
  lstHandling->setColumnWidth(2, 150);
  lstHandling->setHorizontalHeaderLabels(QStringList() << "Action" << "Model" << "Parameter");

  QPushButton* btnUp = new QPushButton(_("Up"));
  btnUp->setIcon(QIcon(":/Teaching/icons/Up.png"));
  btnUp->setToolTip(_("Action Up"));

  QPushButton* btnDown = new QPushButton(_("Down"));
  btnDown->setIcon(QIcon(":/Teaching/icons/Down.png"));
  btnDown->setToolTip(_("Action Down"));

  QPushButton* btnAdd = new QPushButton(_("Add"));
  btnAdd->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAdd->setToolTip(_("Add Action"));

  QPushButton* btnDelete = new QPushButton(_("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(_("Delete Action"));

  QFrame* frmParamButtons = new QFrame;
  QHBoxLayout* buttonParamLayout = new QHBoxLayout;
  buttonParamLayout->setContentsMargins(0, 0, 0, 0);
  frmParamButtons->setLayout(buttonParamLayout);
  buttonParamLayout->addWidget(btnUp);
  buttonParamLayout->addWidget(btnDown);
  buttonParamLayout->addStretch();
  buttonParamLayout->addWidget(btnAdd);
  buttonParamLayout->addWidget(btnDelete);

  QLabel* lblAction = new QLabel(_("Action:"));
  cmbAction = new QComboBox(this);
  cmbAction->addItem("Attach");
  cmbAction->addItem("Detach");
  QLabel* lblModel = new QLabel(_("Model:"));
  cmbModel = new QComboBox(this);
  for (int index = 0; index < targetTask_->getModelList().size(); index++) {
    ModelParam* model = targetTask_->getModelList()[index];
    cmbModel->addItem(model->getRName());
  }

  QLabel* lblTarget = new QLabel(_("Parameter:"));
  cmbTarget = new QComboBox(this);
  cmbTarget->addItem("");
  for (int index = 0; index < targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    cmbTarget->addItem(param->getRName());
  }
  //
  lstArg = UIUtil::makeTableWidget(3, false);
  lstArg->setColumnWidth(0, 100);
  lstArg->setColumnWidth(1, 50);
  lstArg->setColumnWidth(2, 550);
  lstArg->setHorizontalHeaderLabels(QStringList() << "Name" << "" << "Definition");

  txtArgDef = new QTextEdit;
  txtArgDef->setMaximumHeight(80);
  //
  QFrame* frmParam = new QFrame;
  QGridLayout* paramLayout = new QGridLayout;
  paramLayout->setContentsMargins(0, 0, 0, 0);
  frmParam->setLayout(paramLayout);
  paramLayout->addWidget(lstHandling, 0, 0, 1, 2);
  paramLayout->addWidget(frmParamButtons, 1, 0, 1, 2);
  paramLayout->addWidget(lblAction, 2, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbAction, 2, 1, 1, 1);
  paramLayout->addWidget(lblModel, 3, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbModel, 3, 1, 1, 1);
  paramLayout->addWidget(lblTarget, 4, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbTarget, 4, 1, 1, 1);
  paramLayout->addWidget(lstArg, 5, 0, 1, 2);
  paramLayout->addWidget(txtArgDef, 6, 0, 1, 2);
  //
  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(frmRef);
  QSplitter* hsplitter = new QSplitter(Qt::Horizontal);
  hsplitter->addWidget(frmParam);
  splitter->addWidget(hsplitter);
  QList<int> initSizes;
  initSizes.append(500);
  initSizes.append(700);
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
  QLabel* lblName = new QLabel("Name: ");
  txtStateName = new QLineEdit();
  txtStateName->setText(targetStm_->getCmdDspName());
  QFrame* frmName = new QFrame;
  QHBoxLayout* nameLayout = new QHBoxLayout(frmName);
  nameLayout->setContentsMargins(2, 2, 2, 2);
  nameLayout->addWidget(lblName);
  nameLayout->addWidget(txtStateName);

  QLabel* lblCmdName = new QLabel("Command Name: ");
  QLineEdit* txtCmdName = new QLineEdit();
  txtCmdName->setText(targetStm_->getCmdName());
  txtCmdName->setReadOnly(true);
  QFrame* frmCmdName = new QFrame;
  QHBoxLayout* cmdLayout = new QHBoxLayout(frmCmdName);
  cmdLayout->setContentsMargins(2, 2, 2, 2);
  cmdLayout->addWidget(lblCmdName);
  cmdLayout->addWidget(txtCmdName);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frmName);
  mainLayout->addWidget(frmCmdName);
  mainLayout->addWidget(splitter);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(lstHandling, SIGNAL(itemSelectionChanged()), this, SLOT(actionSelectionChanged()));
  connect(lstArg, SIGNAL(itemSelectionChanged()), this, SLOT(argSelectionChanged()));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(upClicked()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(downClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(rejected()));

  setWindowTitle(_("Command"));
  resize(1200, 700);
  //
  showModelInfo();
  showParamInfo();
  showActionInfo();
  showArgInfo();
}

void ArgumentDialog::addClicked() {
  DDEBUG("ArgumentDialog::addClicked");

  saveCurrentAction();
  //
  ElementStmActionParam* newAction = new ElementStmActionParam(NULL_ID, targetStm_->getId(), targetStm_->getActionList().size(), "attach", "", "", true);
  targetStm_->addModelAction(newAction);
  //
  int row = lstHandling->rowCount();
  lstHandling->insertRow(row);
  int index = targetStm_->getActionList().size() - 1;
  UIUtil::makeTableItemWithData(lstHandling, row, 0, newAction->getAction(), index);
  UIUtil::makeTableItemWithData(lstHandling, row, 1, newAction->getModel(), index);
  UIUtil::makeTableItemWithData(lstHandling, row, 2, newAction->getTarget(), index);
}

void ArgumentDialog::deleteClicked() {
  DDEBUG("ArgumentDialog::deleteClicked");

  if (curActionParam_) {
    curActionParam_->setDelete();
    cmbAction->setCurrentIndex(0);
    cmbModel->setCurrentIndex(0);
    cmbTarget->setCurrentIndex(0);
    curActionParam_ = 0;
    curArgIdx_ = NULL_ID;

    int currRow = lstHandling->currentRow();
    lstHandling->removeRow(currRow);
    lstHandling->setFocus();
  }
}

void ArgumentDialog::upClicked() {
  DDEBUG("ArgumentDialog::upClicked");

  saveCurrentAction();
  //
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if (sourceIdx == 0) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx - 1);
  lstHandling->setFocus();
}

void ArgumentDialog::downClicked() {
  DDEBUG("ArgumentDialog::downClicked");

  saveCurrentAction();
  //
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if (lstHandling->rowCount() <= sourceIdx) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx + 1);
  lstHandling->setFocus();
}

void ArgumentDialog::actionSelectionChanged() {
  DDEBUG("ArgumentDialog::actionSelectionChanged");

  saveCurrentAction();
  //
  QTableWidgetItem* item = lstHandling->currentItem();
  if (item) {
    curActionIdx_ = lstHandling->currentRow();
    curActionParam_ = targetStm_->getActionList()[item->data(Qt::UserRole).toInt()];
    if (curActionParam_->getAction() == "attach") {
      cmbAction->setCurrentIndex(0);
    } else if (curActionParam_->getAction() == "detach") {
      cmbAction->setCurrentIndex(1);
    }
    cmbModel->setCurrentIndex(cmbModel->findText(curActionParam_->getModel()));
    cmbTarget->setCurrentIndex(cmbTarget->findText(curActionParam_->getTarget()));
  }
}

void ArgumentDialog::saveCurrentAction() {
  if (curActionParam_) {
    int selAct = cmbAction->currentIndex();
    QString strAct = "";
    if (selAct == ACTION_ATTACH) {
      strAct = "attach";
    } else if (selAct == ACTION_DETACH) {
      strAct = "detach";
    }
    if (curActionParam_->getAction() != strAct) {
      curActionParam_->setAction(strAct);
    }
    //
    QString strModel = cmbModel->itemText(cmbModel->currentIndex());
    if (curActionParam_->getModel() != strModel) {
      curActionParam_->setModel(strModel);
    }
    //
    QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());
    if (curActionParam_->getTarget() != strTarget) {
      curActionParam_->setTarget(strTarget);
    }
    /////
    lstHandling->item(curActionIdx_, 0)->setText(curActionParam_->getAction());
    lstHandling->item(curActionIdx_, 1)->setText(curActionParam_->getModel());
    lstHandling->item(curActionIdx_, 2)->setText(curActionParam_->getTarget());
  }
}

void ArgumentDialog::argSelectionChanged() {
  DDEBUG("ArgumentDialog::argSelectionChanged");

  saveCurrentArg();
  if (curArgParam_) {
    lstArg->item(curArgIdx_, 2)->setText(curArgParam_->getValueDesc());
  }
  //
  QTableWidgetItem* item = lstArg->currentItem();
  if (item) {
    curArgIdx_ = lstArg->currentRow();
    curArgParam_ = targetStm_->getArgList()[item->data(Qt::UserRole).toInt()];
    txtArgDef->setText(curArgParam_->getValueDesc());
  }
}

void ArgumentDialog::saveCurrentArg() {
  if (curArgParam_) {
    QString strDef = txtArgDef->toPlainText();
    if (curArgParam_->getValueDesc() != strDef) {
      curArgParam_->setValueDesc(strDef);
    }
  }
}

void ArgumentDialog::showActionInfo() {
  for (int index = 0; index < targetStm_->getActionList().size(); index++) {
    ElementStmActionParam* param = targetStm_->getActionList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstHandling->rowCount();
    lstHandling->insertRow(row);
    UIUtil::makeTableItemWithData(lstHandling, row, 0, param->getAction(), index);
    UIUtil::makeTableItemWithData(lstHandling, row, 1, param->getModel(), index);
    UIUtil::makeTableItemWithData(lstHandling, row, 2, param->getTarget(), index);
  }
}

void ArgumentDialog::showArgInfo() {
  for (int index = 0; index < targetStm_->getArgList().size(); index++) {
    ArgumentParam* param = targetStm_->getArgList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstArg->rowCount();
    lstArg->insertRow(row);
    UIUtil::makeTableItemWithData(lstArg, row, 0, param->getName(), index);
    int dir = targetStm_->getCommadDefParam()->getArgList()[index]->getDirection();
    QString strDir = "in";
    if (dir == 1) strDir = "out";
    UIUtil::makeTableItemWithData(lstArg, row, 1, strDir, index);
    UIUtil::makeTableItemWithData(lstArg, row, 2, param->getValueDesc(), index);
    //
    param->setValueDescOrg(param->getValueDesc());
  }
}

void ArgumentDialog::showParamInfo() {
  for (int index = 0; index < targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstParam->rowCount();
    lstParam->insertRow(row);
    UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getRName());
    if (param->getType() == 0) {
      UIUtil::makeTableItem(lstParam, row, 2, param->getElemTypeStr());
      UIUtil::makeTableItem(lstParam, row, 3, QString::number(param->getElemNum()));
    }
  }
}

void ArgumentDialog::showModelInfo() {
  for (int index = 0; index < targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItem(lstModel, row, 0, param->getName());
    UIUtil::makeTableItem(lstModel, row, 1, param->getRName());
  }
}

void ArgumentDialog::oKClicked() {
  DDEBUG("ArgumentDialog::oKClicked");

  QString strName = txtStateName->text();
  if (strName.trimmed().length() == 0) {
    QMessageBox::warning(this, _("State"), _("Please input State Name."));
    txtStateName->setFocus();
    txtStateName->setSelection(0, strName.length());
    return;
  }
  targetStm_->setCmdDspName(strName);

  saveCurrentAction();
  saveCurrentArg();
  /////
  int seq = 0;
  for (int index = 0; index < lstHandling->rowCount(); index++) {
    int logicalIdx = lstHandling->verticalHeader()->logicalIndex(index);
    ElementStmActionParam* param = targetStm_->getActionList()[logicalIdx];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    if (param->getModel().length() == 0) {
      QMessageBox::warning(this, _("Argument"), _("Error : Model Definition."));
      return;
    }
    //if(param->getModel()==param->getTarget()) {
    //  QMessageBox::warning(this, _("Argument"), _("Error : Model and Target are SAME."));
    //  return;
    //}
    if (param->getSeq() != seq) {
      param->setSeq(seq);
    }
    seq++;
  }
  /////
  ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(targetTask_);
  std::stringstream errorMsg;
  bool existError = false;
  for (int index = 0; index < targetStm_->getArgList().size(); index++) {
    ArgumentParam* param = targetStm_->getArgList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    ArgumentDefParam* argDef = targetStm_->getCommadDefParam()->getArgList()[index];
    if (argDef->getDirection() == 1) {
      QString targetStr = targetStm_->getArgList()[index]->getValueDesc();
      ParameterParam* targetParam = NULL;
      for (int idxParam = 0; idxParam < targetTask_->getParameterList().size(); idxParam++) {
        ParameterParam* parmParm = targetTask_->getParameterList()[idxParam];
        if (parmParm->getRName() == targetStr) {
          targetParam = parmParm;
          break;
        }
      }
      if (targetParam == NULL) {
        errorMsg << "[" << param->getName().toStdString() << "] " << "target parameter [" << targetStr.toStdString() << "] NOT Exists." << std::endl;
        existError = true;
      } else {
        //if (targetParam->getElemTypes().toStdString() != argDef->getType()) {
        //	DDEBUG_V("%s, %s", targetParam->getElemTypes().toStdString().c_str(), argDef->getType().c_str());
        //	errorMsg << "[" << param->getName().toStdString() << "] " << "and target parameter [" << targetStr.toStdString() << "] TYPE Error." << std::endl;
        //	existError = true;
        //}
        if (targetParam->getElemNum() < argDef->getLength()) {
          DDEBUG_V("%d, %d", targetParam->getElemNum(), argDef->getLength());
          errorMsg << "[" << param->getName().toStdString() << "] " << "target parameter [" << targetStr.toStdString() << "] NUM Error." << std::endl;
          existError = true;
        }
      }

    } else {
      if (0 < param->getValueDesc().trimmed().length()) {
        string strError;
        if (handler->checkSyntax(targetTask_, param->getValueDesc(), strError) == false) {
          DDEBUG_V("%s", param->getValueDesc().toStdString().c_str());
          errorMsg << "[" << param->getName().toStdString() << "]" << strError << std::endl;
          existError = true;
        }
      }
      if (existError == false && targetStm_->getMode() == DB_MODE_INSERT) {
        param->setNew();
      }
    }
  }
  EstimatorFactory::getInstance().deleteArgEstimator(handler);
  if (existError) {
    QMessageBox::warning(this, _("Argument"), QString::fromStdString(errorMsg.str()));
    return;
  }
  //
  isOK_ = true;
  close();
}

void ArgumentDialog::cancelClicked() {
  DDEBUG("ArgumentDialog::cancelClicked()");

  for (int index = 0; index < targetStm_->getArgList().size(); index++) {
    ArgumentParam* param = targetStm_->getArgList()[index];
    param->setValueDesc(param->getValueDescOrg());
  }
  isOK_ = false;
  close();
}

void ArgumentDialog::rejected() {
  DDEBUG("ArgumentDialog::rejected");

  close();
}

}
