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
  lstModel = UIUtil::makeTableWidget(2, true);
  lstModel->setColumnWidth(0, 100);
  lstModel->setColumnWidth(1, 100);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Name" << "ID");
  //
  lstParam = UIUtil::makeTableWidget(5, false);
  lstParam->setColumnWidth(0, 100);
  lstParam->setColumnWidth(1, 100);
  lstParam->setColumnWidth(2, 150);
  lstParam->setColumnWidth(3, 100);
  lstParam->setColumnWidth(4, 50);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Model" << "Unit" << "Num");

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
  leModelName = new QLineEdit;
  QLabel* lblUnit = new QLabel(_("Unit:"));
  leUnit = new QLineEdit;
  QLabel* lblNum = new QLabel(_("Num:"));
  leNum = new QLineEdit;
  QLabel* lblElemType = new QLabel(_("Type:"));
  cmbElemType = new QComboBox(this);
  cmbElemType->addItem("double");
  cmbElemType->addItem("int");
  cmbElemType->addItem("string");
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
  paramLayout->addWidget(leModelName, 5, 1, 1, 1);
  paramLayout->addWidget(lblUnit, 6, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leUnit, 6, 1, 1, 1);
  paramLayout->addWidget(lblNum, 7, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leNum, 7, 1, 1, 1);
  paramLayout->addWidget(lblElemType, 8, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(cmbElemType, 8, 1, 1, 1);
	paramLayout->addWidget(lblVisibility, 9, 0, 1, 1, Qt::AlignRight);
	paramLayout->addWidget(cmbHide, 9, 1, 1, 1);
	//
  QFrame* frmMain = new QFrame;
  QHBoxLayout* formLayout = new QHBoxLayout;
  formLayout->setContentsMargins(0, 0, 0, 0);
  frmMain->setLayout(formLayout);
  formLayout->addWidget(lstModel);
  formLayout->addWidget(frmParam);
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
  connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeSelectionChanged(int)));
  connect(btnAddParam, SIGNAL(clicked()), this, SLOT(addParamClicked()));
  connect(btnDeleteParam, SIGNAL(clicked()), this, SLOT(deleteParamClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(rejected()));
	connect(this, SIGNAL(rejected()), this, SLOT(rejected()));

  setWindowTitle(_("Task Parameter"));
  setFixedHeight(sizeHint().height());
  setFixedWidth(800);
  //
  btnOK->setFocus();

	TeachingEventHandler::instance()->prd_Loaded(this);
}

void ParameterDialog::showModelInfo(const vector<ModelParamPtr>& modelList) {
	for (int index = 0; index < modelList.size(); index++) {
		ModelParamPtr param = modelList[index];
		int row = lstModel->rowCount();
		lstModel->insertRow(row);
		UIUtil::makeTableItem(lstModel, row, 0, param->getName());
		UIUtil::makeTableItem(lstModel, row, 1, param->getRName());
	}
}

void ParameterDialog::showParamInfo(const vector<ParameterParamPtr>& paramList) {
	for (int index = 0; index < paramList.size(); index++) {
		ParameterParamPtr param = paramList[index];

		int row = lstParam->rowCount();
		lstParam->insertRow(row);

		UIUtil::makeTableItemWithData(lstParam, row, 0, param->getName(), param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 1, getTypeName(param->getType()), param->getId());
		QString strModel = "";
		QString strUnit = "";
		QString strElemNum = "";
		if (param->getType() == PARAM_KIND_MODEL) {
			strModel = param->getModelName();
			strUnit = param->getUnit();
		} else {
			strElemNum = param->getElemNum();
		}
		UIUtil::makeTableItemWithData(lstParam, row, 2, strModel, param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 3, strUnit, param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 4, strElemNum, param->getId());
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
	  leModelName->setText("");
	  leUnit->setText(param->getUnit());
	  leNum->setText(QString::number(param->getElemNum()));
	  cmbElemType->setCurrentIndex(param->getElemTypeNo());
	} else {
	  leModelName->setText(param->getModelName());
	  leUnit->setText("");
	  leNum->setText("");
	  cmbElemType->setCurrentIndex(0);
	}
}

void ParameterDialog::paramSelectionChanged() {
	DDEBUG("ParameterDialog::paramSelectionChanged()");

	QString strName = leName->text();
	QString strId = leId->text();
	int type = cmbType->currentIndex();
	QString strUnit = leUnit->text();
	QString strNum = leNum->text();
	int elemType = cmbElemType->currentIndex();
	QString strModel = leModelName->text();
	int hide = cmbHide->currentIndex();

	if (currentRowIndex_ != NULL_ID) {
		lstParam->item(currentRowIndex_, 0)->setText(strName);
		lstParam->item(currentRowIndex_, 1)->setText(getTypeName(type));
		if (type == 0) {
			lstParam->item(currentRowIndex_, 2)->setText("");
			lstParam->item(currentRowIndex_, 3)->setText(strUnit);
			lstParam->item(currentRowIndex_, 4)->setText(strNum);
		} else {
			lstParam->item(currentRowIndex_, 2)->setText(strModel);
			lstParam->item(currentRowIndex_, 3)->setText("");
			lstParam->item(currentRowIndex_, 4)->setText("");
		}
	}
	currentRowIndex_ = lstParam->currentRow();
	//
	QTableWidgetItem* item = lstParam->currentItem();
	if (item == 0) return;
	int selected = item->data(Qt::UserRole).toInt();

	TeachingEventHandler::instance()->prd_ParamSelectionChanged(selected, strName, strId, type, strUnit, strNum, elemType, strModel, hide);
}

void ParameterDialog::insertParameter(const ParameterParamPtr& param) {
	DDEBUG("ParameterDialog::insertParameter()");

	int row = lstParam->rowCount();
	lstParam->insertRow(row);
	UIUtil::makeTableItemWithData(lstParam, row, 0, param->getName(), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 1, getTypeName(param->getType()), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 2, QString::fromLatin1(""), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 3, param->getUnit(), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 4, QString::number(param->getElemNum()), param->getId());
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
		cmbElemType->currentIndex(),
		leModelName->text(),
		cmbHide->currentIndex());
}

void ParameterDialog::deleteParamClicked() {
  DDEBUG("ParameterDialog::deleteParamClicked()");

	if(TeachingEventHandler::instance()->prd_DeleteParamClicked()==false) return;

  leName->setText("");
  leId->setText("");
  cmbType->setCurrentIndex(0);
  leModelName->setText("");
  leUnit->setText("");
  leNum->setText("");
	cmbHide->setCurrentIndex(0);
  cmbElemType->setCurrentIndex(0);
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
							cmbElemType->currentIndex(),
							leModelName->text(),
							cmbHide->currentIndex());
	if(ret) close();
}

void ParameterDialog::rejected() {
  DDEBUG("ParameterDialog::rejected()");
  close();
}

void ParameterDialog::typeSelectionChanged(int index) {
	DDEBUG("ParameterDialog::typeSelectionChanged()");

	if (index == 0) {
		leModelName->setEnabled(false);
		leUnit->setEnabled(true);
		leNum->setEnabled(true);
		cmbElemType->setEnabled(true);
	} else {
		leModelName->setEnabled(true);
		leUnit->setEnabled(false);
		leNum->setEnabled(false);
		cmbElemType->setEnabled(false);
	}
}

QString ParameterDialog::getTypeName(int source) {
	QString result = "";

	switch (source) {
	case 0:
		result = "Normal";
		break;
	case 1:
		result = "Model";
		break;
	}
	return result;
}

}
