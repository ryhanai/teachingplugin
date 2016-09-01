#include "ArgumentDialog.h"
#include "Calculator.h"
#include "DataBaseManager.h"
#include "TeachingUtil.h"

#include "LoggerUtil.h"

namespace teaching {

ArgumentDialog::ArgumentDialog(TaskModelParam* param, ElementStmParam* stmParam, QWidget* parent) 
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
    curArgIdx_(-1), curArgParam_(0), curActionIdx_(-1), curActionParam_(0) {
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
  QFrame* frmRef = new QFrame;
  QVBoxLayout* refLayout = new QVBoxLayout;
  refLayout->setContentsMargins(0, 0, 0, 0);
  frmRef->setLayout(refLayout);
  refLayout->addWidget(lstModel);
  refLayout->addWidget(lstParam);
  //
  lstHandling = UIUtil::makeTableWidget(3, false);
  lstHandling->setColumnWidth(0, 100);
  lstHandling->setColumnWidth(1, 150);
  lstHandling->setColumnWidth(2, 150);
  lstHandling->setHorizontalHeaderLabels(QStringList() << "Action" << "Model" << "Parameter");

  QPushButton* btnUp = new QPushButton(tr("Up"));
  btnUp->setIcon(QIcon(":/Teaching/icons/Up.png"));
  btnUp->setToolTip(tr("Action Up"));

  QPushButton* btnDown = new QPushButton(tr("Down"));
  btnDown->setIcon(QIcon(":/Teaching/icons/Down.png"));
  btnDown->setToolTip(tr("Action Down"));

  QPushButton* btnAdd = new QPushButton(tr("Add"));
  btnAdd->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAdd->setToolTip(tr("Add Action"));

  QPushButton* btnDelete = new QPushButton(tr("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(tr("Delete Action"));

  QFrame* frmParamButtons = new QFrame;
  QHBoxLayout* buttonParamLayout = new QHBoxLayout;
  buttonParamLayout->setContentsMargins(0, 0, 0, 0);
  frmParamButtons->setLayout(buttonParamLayout);
  buttonParamLayout->addWidget(btnUp);
  buttonParamLayout->addWidget(btnDown);
  buttonParamLayout->addStretch();
  buttonParamLayout->addWidget(btnAdd);
  buttonParamLayout->addWidget(btnDelete);

  QLabel* lblAction = new QLabel(tr("Action:"));
  cmbAction = new QComboBox(this);
  cmbAction->addItem("Attach");
  cmbAction->addItem("Detach");
  QLabel* lblModel = new QLabel(tr("Model:"));
  cmbModel = new QComboBox(this);
  for(int index=0; index<targetTask_->getModelList().size(); index++) {
    ModelParam* model = targetTask_->getModelList()[index];
    cmbModel->addItem(model->getRName());
  }

  QLabel* lblTarget = new QLabel(tr("Parameter:"));
  cmbTarget = new QComboBox(this);
  cmbTarget->addItem("");
  for(int index=0; index<targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    cmbTarget->addItem(param->getRName());
  }
  //
  lstArg = UIUtil::makeTableWidget(2, false);
  lstArg->setColumnWidth(0, 100);
  lstArg->setColumnWidth(1, 600);
  lstArg->setHorizontalHeaderLabels(QStringList() << "Name" << "Definition");

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
  initSizes.append(400);
  initSizes.append(800);
  splitter->setSizes(initSizes);
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(tr("OK"));
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);
  //
  QLabel* lblTaskName = new QLabel("Command Name: " + targetStm_->getCmdName());

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(lblTaskName);
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
  connect(this, SIGNAL(rejected()), this, SLOT(okClicked()));

  setWindowTitle(tr("Command"));
  resize(1200, 700);
  //
  showModelInfo();
  showParamInfo();
  showActionInfo();
  showArgInfo();
}

void ArgumentDialog::addClicked() {
  saveCurrentAction();
  //
  ElementStmActionParam* newAction = new ElementStmActionParam(-1, targetStm_->getId(), targetStm_->getActionList().size(), "attach", "", "", true);
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
  if(curActionParam_) {
    curActionParam_->setDelete();
    cmbAction->setCurrentIndex(0);
    cmbModel->setCurrentIndex(0);
    cmbTarget->setCurrentIndex(0);
    curActionParam_ = 0;
    curArgIdx_ = -1;

    int currRow = lstHandling->currentRow();
    lstHandling->removeRow(currRow);
    lstHandling->setFocus();
  }
}

void ArgumentDialog::upClicked() {
  saveCurrentAction();
  //
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if(sourceIdx==0) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx-1);
  lstHandling->setFocus();
}

void ArgumentDialog::downClicked() {
  saveCurrentAction();
  //
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if(lstHandling->rowCount()<=sourceIdx) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx+1);
  lstHandling->setFocus();
}

void ArgumentDialog::actionSelectionChanged() {
  saveCurrentAction();
  //
  QTableWidgetItem* item = lstHandling->currentItem();
  if(item) {
    curActionIdx_ = lstHandling->currentRow();
    curActionParam_ = targetStm_->getActionList()[item->data(Qt::UserRole).toInt()];
    if(curActionParam_->getAction()=="attach") {
      cmbAction->setCurrentIndex(0);
    } else if(curActionParam_->getAction()=="detach") {
      cmbAction->setCurrentIndex(1);
    }
    cmbModel->setCurrentIndex(cmbModel->findText(curActionParam_->getModel()));
    cmbTarget->setCurrentIndex(cmbTarget->findText(curActionParam_->getTarget()));
  }
}

void ArgumentDialog::saveCurrentAction() {
  if(curActionParam_) {
    int selAct = cmbAction->currentIndex();
    QString strAct = "";
    if(selAct==ACTION_ATTACH) {
      strAct = "attach";
    } else if(selAct==ACTION_DETACH) {
      strAct = "detach";
    }
    if( curActionParam_->getAction() != strAct) {
      curActionParam_->setAction(strAct);
    }
    //
    QString strModel = cmbModel->itemText(cmbModel->currentIndex());
    if( curActionParam_->getModel() != strModel) {
      curActionParam_->setModel(strModel);
    }
    //
    QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());
    if( curActionParam_->getTarget() != strTarget) {
      curActionParam_->setTarget(strTarget);
    }
    /////
    lstHandling->item(curActionIdx_, 0)->setText(curActionParam_->getAction());
    lstHandling->item(curActionIdx_, 1)->setText(curActionParam_->getModel());
    lstHandling->item(curActionIdx_, 2)->setText(curActionParam_->getTarget());
  }
}

void ArgumentDialog::argSelectionChanged() {
  saveCurrentArg();
  if(curArgParam_) {
    lstArg->item(curArgIdx_, 1)->setText(curArgParam_->getValueDesc());
  }
  //
  QTableWidgetItem* item = lstArg->currentItem();
  if(item) {
    curArgIdx_ = lstArg->currentRow();
    curArgParam_ = targetStm_->getArgList()[item->data(Qt::UserRole).toInt()];
    txtArgDef->setText(curArgParam_->getValueDesc());
  }
}

void ArgumentDialog::saveCurrentArg() {
  if(curArgParam_) {
    QString strDef = txtArgDef->toPlainText();
    if( curArgParam_->getValueDesc() != strDef) {
      curArgParam_->setValueDesc(strDef);
    }
  }
}

void ArgumentDialog::showActionInfo() {
  for(int index=0; index<targetStm_->getActionList().size(); index++) {
    ElementStmActionParam* param = targetStm_->getActionList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstHandling->rowCount();
    lstHandling->insertRow(row);
    UIUtil::makeTableItemWithData(lstHandling, row, 0, param->getAction(), index);
    UIUtil::makeTableItemWithData(lstHandling, row, 1, param->getModel(), index);
    UIUtil::makeTableItemWithData(lstHandling, row, 2, param->getTarget(), index);
  }
}

void ArgumentDialog::showArgInfo() {
  for(int index=0; index<targetStm_->getArgList().size(); index++) {
    ArgumentParam* param = targetStm_->getArgList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstArg->rowCount();
    lstArg->insertRow(row);
    UIUtil::makeTableItemWithData(lstArg, row, 0, param->getName(), index);
    UIUtil::makeTableItemWithData(lstArg, row, 1, param->getValueDesc(), index);
  }
}

void ArgumentDialog::showParamInfo() {
  for(int index=0; index<targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstParam->rowCount();
    lstParam->insertRow(row);
    UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getRName());
  }
}

void ArgumentDialog::showModelInfo() {
  for(int index=0; index<targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItem(lstModel, row, 0, param->getName());
    UIUtil::makeTableItem(lstModel, row, 1, param->getRName());
  }
}

void ArgumentDialog::oKClicked() {
  saveCurrentAction();
  saveCurrentArg();
  /////
  int seq = 0;
  for(int index=0; index<lstHandling->rowCount(); index++) {
    int logicalIdx = lstHandling->verticalHeader()->logicalIndex(index);
    ElementStmActionParam* param = targetStm_->getActionList()[logicalIdx];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;
    if(param->getModel().length()==0) {
      QMessageBox::warning(this, tr("Argument"), tr("Error : Model Definition."));
      return;
    }
    if(param->getModel()==param->getTarget()) {
      QMessageBox::warning(this, tr("Argument"), tr("Error : Model and Target are SAME."));
      return;
    }
    if(param->getSeq()!=seq) {
      param->setSeq(seq);
    }
    seq++;
  }
  /////
  Calculator* calculator = new Calculator();
  for(int index=0; index<targetStm_->getArgList().size(); index++) {
    ArgumentParam* param = targetStm_->getArgList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    if(0<param->getValueDesc().trimmed().length()) {
      if(calculator->calculate(param->getValueDesc(), targetTask_)==false) {
        DDEBUG_V("%s", param->getValueDesc().toStdString().c_str());
        QMessageBox::warning(this, tr("Argument"), tr("Error : Argument Definition."));
        delete calculator;
        return;
      }
    }
  }
  delete calculator;
  //
  if(DatabaseManager::getInstance().saveStateParameter(targetStm_)==false ) {
    QMessageBox::warning(this, tr("Save Argument Error"), DatabaseManager::getInstance().getErrorStr());
    return;
  }
  targetStm_->clearActionList();
  vector<ElementStmActionParam*> actionList = DatabaseManager::getInstance().getStmActionList(targetStm_->getId());
  for(int index=0; index<actionList.size(); index++) {
    targetStm_->addModelAction(actionList[index]);
  }
  //
  close();
}

}
