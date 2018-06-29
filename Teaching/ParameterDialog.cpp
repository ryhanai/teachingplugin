#include "ParameterDialog.h"
#include "TeachingUtil.h"

#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

ParameterDialog::ParameterDialog(QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
    currentRowIndex_(NULL_ID){
  //
  lstModel = UIUtil::makeTableWidget(1, true);
  lstModel->setColumnWidth(0, 200);
  lstModel->setHorizontalHeaderLabels(QStringList() << "ID");
  //
  lstModelParam = UIUtil::makeTableWidget(2, true);
  lstModelParam->setColumnWidth(0, 70);
  lstModelParam->setColumnWidth(1, 200);
  lstModelParam->setHorizontalHeaderLabels(QStringList() << "Name" << "Definition");
  //
  lstParam = UIUtil::makeTableWidget(7, false);
  lstParam->setColumnWidth(0, 30);
  lstParam->setColumnWidth(1, 200);
  lstParam->setColumnWidth(2, 50);
  lstParam->setColumnWidth(3, 100);
  lstParam->setColumnWidth(4, 100);
  lstParam->setColumnWidth(5, 80);
  lstParam->setColumnWidth(6, 50);
  lstParam->setHorizontalHeaderLabels(QStringList() << "" << "Name" << "Type" << "Model" << "Model Param" << "Unit" << "Num");

  QPushButton* btnAddParam = new QPushButton(_("Add"));
  btnAddParam->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAddParam->setToolTip(_("Add New Parameter"));
  //QShortcut *keyAdd = new QShortcut(QKeySequence("+"), this);
  //connect(keyAdd, SIGNAL(activated()), this, SLOT(addParamClicked()));

  QPushButton* btnDeleteParam = new QPushButton(_("Delete"));
  btnDeleteParam->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteParam->setToolTip(_("Delete selected Parameter"));

  QFrame* frmParamButtons = new QFrame;
  QHBoxLayout* buttonParamLayout = new QHBoxLayout;
  buttonParamLayout->setContentsMargins(0, 0, 0, 0);
  frmParamButtons->setLayout(buttonParamLayout);
  buttonParamLayout->addWidget(btnAddParam);
  buttonParamLayout->addStretch();
  buttonParamLayout->addWidget(btnDeleteParam);

  QLabel* lblName = new QLabel(_("Name:"));
  leName = new QLineEdit;
  QLabel* lblId = new QLabel(_("Id:"));
  leId = new QLineEdit;
  QLabel* lblType = new QLabel(_("Kind:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Normal");
  cmbType->addItem("Model");

  QLabel* lblModel = new QLabel(_("Model Name:"));
  cmbModelName = new QComboBox;
  cmbModelName->addItem("");

  QLabel* lblModelParam = new QLabel(_("Model Param Name:"));
  cmbModelParamName = new QComboBox;
  cmbModelParamName->addItem("", NULL_ID);

  QLabel* lblUnit = new QLabel(_("Unit:"));
  leUnit = new QLineEdit;
  QLabel* lblNum = new QLabel(_("Num:"));
  leNum = new QLineEdit;
	QLabel* lblVisibility = new QLabel(_("Visibility:"));
	cmbHide = new QComboBox(this);
	cmbHide->addItem("public");
	cmbHide->addItem("private");
	//
  QFrame* frmParam = new QFrame;
  QGridLayout* paramLayout = new QGridLayout;
  paramLayout->setContentsMargins(0, 0, 0, 0);
  frmParam->setLayout(paramLayout);
  paramLayout->addWidget(lstParam, 0, 0, 1, 2);
  paramLayout->addWidget(frmParamButtons, 1, 0, 1, 2);
  paramLayout->addWidget(lblName, 2, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leName, 2, 1, 1, 1);
  paramLayout->addWidget(lblId, 3, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leId, 3, 1, 1, 1);
  paramLayout->addWidget(lblType, 4, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbType, 4, 1, 1, 1);
  paramLayout->addWidget(lblModel, 5, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbModelName, 5, 1, 1, 1);
  paramLayout->addWidget(lblModelParam, 6, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbModelParamName, 6, 1, 1, 1);
  paramLayout->addWidget(lblUnit, 7, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leUnit, 7, 1, 1, 1);
  paramLayout->addWidget(lblNum, 8, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leNum, 8, 1, 1, 1);
  paramLayout->addWidget(lblVisibility, 9, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbHide, 9, 1, 1, 1);
  //
  QFrame* frmModel = new QFrame;
  QVBoxLayout* modelLayout = new QVBoxLayout;
  modelLayout->setContentsMargins(0, 0, 0, 0);
  frmModel->setLayout(modelLayout);
  modelLayout->addWidget(lstModel);
  modelLayout->addWidget(lstModelParam);
  //
  QFrame* frmMain = new QFrame;
  QHBoxLayout* formLayout = new QHBoxLayout;
  formLayout->setContentsMargins(0, 0, 0, 0);
  frmMain->setLayout(formLayout);
  formLayout->addWidget(frmModel, 1);
  formLayout->addWidget(frmParam, 2);
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
  lblTaskName = new QLabel();

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(lblTaskName);
  mainLayout->addWidget(frmMain);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(lstParam, SIGNAL(itemSelectionChanged()), this, SLOT(paramSelectionChanged()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelTableSelectionChanged()));
  connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeSelectionChanged(int)));
  connect(cmbModelName, SIGNAL(currentIndexChanged(int)), this, SLOT(modelSelectionChanged(int)));
  connect(btnAddParam, SIGNAL(clicked()), this, SLOT(addParamClicked()));
  connect(btnDeleteParam, SIGNAL(clicked()), this, SLOT(deleteParamClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(rejected()));
	connect(this, SIGNAL(rejected()), this, SLOT(rejected()));

  setWindowTitle(_("Task Parameter"));
  resize(1000, 650);
  //
  btnOK->setFocus();

	TeachingEventHandler::instance()->prd_Loaded(this);
}

void ParameterDialog::showModelInfo(const vector<ModelParamPtr>& modelList) {
	for (ModelParamPtr param : modelList) {
		int row = lstModel->rowCount();
		lstModel->insertRow(row);
		UIUtil::makeTableItemWithData(lstModel, row, 0, param->getRName(), param->getId());
    //
    cmbModelName->addItem(param->getRName(), param->getId());
	}
}

void ParameterDialog::showModelParamInfo(const vector<ModelParameterParamPtr>& paramList) {
  DDEBUG("ParameterDialog::showModelParamInfo");
  lstModelParam->setRowCount(0);

  for (ModelParameterParamPtr param : paramList) {
    int row = lstModelParam->rowCount();
    lstModelParam->insertRow(row);
    UIUtil::makeTableItem(lstModelParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstModelParam, row, 1, param->getValueDesc());
  }
}

void ParameterDialog::showParamInfo(const vector<ParameterParamPtr>& paramList) {
  for (ParameterParamPtr param : paramList) {
    int row = lstParam->rowCount();
    lstParam->insertRow(row);

    QString visibility = "+";
    if (param->getHide() == 1) {
      visibility = "-";
    }
    UIUtil::makeTableItemWithData(lstParam, row, 0, visibility, param->getId());
    UIUtil::makeTableItemWithData(lstParam, row, 1, param->getName(), param->getId());
    UIUtil::makeTableItemWithData(lstParam, row, 2, UIUtil::getTypeName(param->getType()), param->getId());
    QString strModel = "";
    QString strModelParam = "";
    QString strUnit = "";
    QString strElemNum = "";
    if (param->getType() == PARAM_KIND_MODEL) {
      for (int index = 0; index < cmbModelName->count(); index++) {
        if (param->getModelId() == cmbModelName->itemData(index).toInt()) {
          strModel = cmbModelName->itemText(index);
          break;
        }
      }
      vector<ModelParameterParamPtr> paramList = TeachingEventHandler::instance()->prd_ModelSelectionChanged(param->getModelId());
      for (int index = 0; index < paramList.size(); index++) {
        if (param->getModelParamId() == paramList[index]->getId()) {
          strModelParam = paramList[index]->getName();
          break;
        }
      }
    } else {
      strUnit = param->getUnit();
      strElemNum = QString::number(param->getElemNum());
    }
    UIUtil::makeTableItemWithData(lstParam, row, 3, strModel, param->getId());
    UIUtil::makeTableItemWithData(lstParam, row, 4, strModelParam, param->getId());
    UIUtil::makeTableItemWithData(lstParam, row, 5, strUnit, param->getId());
    UIUtil::makeTableItemWithData(lstParam, row, 6, strElemNum, param->getId());
  }
}

void ParameterDialog::setTaskName(QString taskName) {
	lblTaskName->setText("Task Name: " + taskName);
}

void ParameterDialog::updateContents(const ParameterParamPtr& param) {
	DDEBUG("ParameterDialog::updateContents()");

  leName->setText(param->getName());
  leId->setText(param->getRName());
  cmbHide->setCurrentIndex(param->getHide());
  cmbType->setCurrentIndex(param->getType());
  typeSelectionChanged(param->getType());
  if (param->getType() == 0) {
    cmbModelName->setCurrentIndex(0);
    leUnit->setText(param->getUnit());
    leNum->setText(QString::number(param->getElemNum()));
  } else {
    for (int index = 0; index < cmbModelName->count(); index++) {
      if (param->getModelId() == cmbModelName->itemData(index).toInt()) {
        cmbModelName->setCurrentIndex(index);
        break;
      }
    }
    for (int index = 0; index < cmbModelParamName->count(); index++) {
      if (param->getModelParamId() == cmbModelParamName->itemData(index).toInt()) {
        cmbModelParamName->setCurrentIndex(index);
        break;
      }
    }
    leUnit->setText("");
    leNum->setText("");
  }
}

void ParameterDialog::paramSelectionChanged() {
	DDEBUG("ParameterDialog::paramSelectionChanged()");

  QString strName = leName->text();
  QString strId = leId->text();
  int type = cmbType->currentIndex();
  QString strUnit = leUnit->text();
  QString strNum = leNum->text();
  QString strModel = cmbModelName->currentText();
  int modelId = NULL_ID;
  if (0 < cmbModelName->count()) {
    modelId = cmbModelName->currentData().toInt();
  }
  QString strModelParam = cmbModelParamName->currentText();
  int modelParamId = NULL_ID;
  if (0 < cmbModelParamName->count()) {
    modelParamId = cmbModelParamName->currentData().toInt();
  }
  int hide = cmbHide->currentIndex();

  if (currentRowIndex_ != NULL_ID) {
    QString visibility = "+";
    if (hide == 1) {
      visibility = "-";
    }
    lstParam->item(currentRowIndex_, 0)->setText(visibility);
    lstParam->item(currentRowIndex_, 1)->setText(strName);
    lstParam->item(currentRowIndex_, 2)->setText(UIUtil::getTypeName(type));
    if (type == 0) {
      lstParam->item(currentRowIndex_, 3)->setText("");
      lstParam->item(currentRowIndex_, 4)->setText("");
      lstParam->item(currentRowIndex_, 5)->setText(strUnit);
      lstParam->item(currentRowIndex_, 6)->setText(strNum);
    } else {
      lstParam->item(currentRowIndex_, 3)->setText(strModel);
      lstParam->item(currentRowIndex_, 4)->setText(strModelParam);
      lstParam->item(currentRowIndex_, 5)->setText("");
      lstParam->item(currentRowIndex_, 6)->setText("");
    }
  }
  currentRowIndex_ = lstParam->currentRow();
  //
  QTableWidgetItem* item = lstParam->currentItem();
  if (item == 0) return;
  int selected = item->data(Qt::UserRole).toInt();

  TeachingEventHandler::instance()->prd_ParamSelectionChanged(selected, strName, strId, type, strUnit, strNum, modelId, modelParamId, hide);
}

void ParameterDialog::insertParameter(const ParameterParamPtr& param) {
	DDEBUG("ParameterDialog::insertParameter()");

	int row = lstParam->rowCount();
	lstParam->insertRow(row);

  QString visibility = "+";
  if (param->getHide() == 1) {
    visibility = "-";
  }

  UIUtil::makeTableItemWithData(lstParam, row, 0, visibility, param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 1, param->getName(), param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 2, UIUtil::getTypeName(param->getType()), param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 3, QString::fromLatin1(""), param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 4, QString::fromLatin1(""), param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 5, param->getUnit(), param->getId());
  UIUtil::makeTableItemWithData(lstParam, row, 6, QString::number(param->getElemNum()), param->getId());
  lstParam->setCurrentCell(row, 0);
}

void ParameterDialog::addParamClicked() {
  DDEBUG("ParameterDialog::addParamClicked()");

  TeachingEventHandler::instance()->prd_AddParamClicked(
    leName->text(),
    leId->text(),
    cmbType->currentIndex(),
    leUnit->text(),
    leNum->text(),
    cmbModelName->currentData().toInt(),
    cmbModelParamName->currentData().toInt(),
    cmbHide->currentIndex());
}

void ParameterDialog::deleteParamClicked() {
  DDEBUG("ParameterDialog::deleteParamClicked()");

	if(TeachingEventHandler::instance()->prd_DeleteParamClicked()==false) return;

  leName->setText("");
  leId->setText("");
  cmbType->setCurrentIndex(0);
  cmbModelName->setCurrentIndex(0);
  leUnit->setText("");
  leNum->setText("");
  cmbHide->setCurrentIndex(0);
  currentRowIndex_ = -1;

  int currRow = lstParam->currentRow();
  lstParam->removeRow(currRow);
  lstParam->setFocus();
}

void ParameterDialog::oKClicked() {
  DDEBUG("ParameterDialog::oKClicked()");
  bool ret = TeachingEventHandler::instance()->prd_OkClicked(
    leName->text(),
    leId->text(),
    cmbType->currentIndex(),
    leUnit->text(),
    leNum->text(),
    cmbModelName->currentData().toInt(),
    cmbModelParamName->currentData().toInt(),
    cmbHide->currentIndex());
  if(ret) close();
}

void ParameterDialog::rejected() {
  DDEBUG("ParameterDialog::rejected()");
  close();
}

void ParameterDialog::modelTableSelectionChanged() {
  int selected = NULL_ID;
  QTableWidgetItem* item = lstModel->currentItem();
  if (item) {
    selected = item->data(Qt::UserRole).toInt();
  }
  TeachingEventHandler::instance()->prd_ModelTableSelectionChanged(selected);
}

void ParameterDialog::modelSelectionChanged(int index) {
  int modelId = cmbModelName->itemData(index).toInt();
  vector<ModelParameterParamPtr> paramList = TeachingEventHandler::instance()->prd_ModelSelectionChanged(modelId);
  //
  cmbModelParamName->clear();
  cmbModelParamName->addItem("", NULL_ID);
  for (ModelParameterParamPtr param : paramList) {
    cmbModelParamName->addItem(param->getName(), param->getId());
  }
}

void ParameterDialog::typeSelectionChanged(int index) {
  DDEBUG("ParameterDialog::typeSelectionChanged()");

  if (index == 0) {
    cmbModelName->setEnabled(false);
    leUnit->setEnabled(true);
    leNum->setEnabled(true);
  } else {
    cmbModelName->setEnabled(true);
    leUnit->setEnabled(false);
    leNum->setEnabled(false);
  }
}

}
