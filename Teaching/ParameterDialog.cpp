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
  lstModel->setColumnWidth(0, 200);
  lstModel->setColumnWidth(1, 200);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Name" << "ID");
  //
  lstParam = UIUtil::makeTableWidget(4, false);
	lstParam->setColumnWidth(0, 30);
	lstParam->setColumnWidth(1, 250);
  lstParam->setColumnWidth(2, 100);
  lstParam->setColumnWidth(3, 50);
  lstParam->setHorizontalHeaderLabels(QStringList() << "" << "Name" << "Unit" << "Num");

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
  paramLayout->addWidget(lblUnit, 4, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leUnit, 4, 1, 1, 1);
  paramLayout->addWidget(lblNum, 5, 0, 1, 1, Qt::AlignRight);
  paramLayout->addWidget(leNum, 5, 1, 1, 1);
	paramLayout->addWidget(lblVisibility, 6, 0, 1, 1, Qt::AlignRight);
	paramLayout->addWidget(cmbHide, 6, 1, 1, 1);
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
  connect(btnAddParam, SIGNAL(clicked()), this, SLOT(addParamClicked()));
  connect(btnDeleteParam, SIGNAL(clicked()), this, SLOT(deleteParamClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(rejected()));
	connect(this, SIGNAL(rejected()), this, SLOT(rejected()));

  setWindowTitle(_("Task Parameter"));
  setFixedHeight(sizeHint().height());
  setFixedWidth(1000);
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

		QString visibility = "+";
		if (param->getHide() == 1) {
			visibility = "-";
		}
		UIUtil::makeTableItemWithData(lstParam, row, 0, visibility, param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 1, param->getName(), param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 2, param->getUnit(), param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 3, QString::number(param->getElemNum()), param->getId());
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
	leUnit->setText(param->getUnit());
	leNum->setText(QString::number(param->getElemNum()));
}

void ParameterDialog::paramSelectionChanged() {
	DDEBUG("ParameterDialog::paramSelectionChanged()");

	QString strName = leName->text();
	QString strId = leId->text();
	QString strUnit = leUnit->text();
	QString strNum = leNum->text();
	int hide = cmbHide->currentIndex();

	if (currentRowIndex_ != NULL_ID) {
		QString visibility = "+";
		if (hide == 1) {
			visibility = "-";
		}
		lstParam->item(currentRowIndex_, 0)->setText(visibility);
		lstParam->item(currentRowIndex_, 1)->setText(strName);
		lstParam->item(currentRowIndex_, 2)->setText(strUnit);
		lstParam->item(currentRowIndex_, 3)->setText(strNum);
	}
	currentRowIndex_ = lstParam->currentRow();
	//
	QTableWidgetItem* item = lstParam->currentItem();
	if (item == 0) return;
	int selected = item->data(Qt::UserRole).toInt();

	TeachingEventHandler::instance()->prd_ParamSelectionChanged(selected, strName, strId, strUnit, strNum, hide);
}

void ParameterDialog::insertParameter(const ParameterParamPtr& param) {
	DDEBUG("ParameterDialog::insertParameter()");

	int row = lstParam->rowCount();
	lstParam->insertRow(row);
	UIUtil::makeTableItemWithData(lstParam, row, 0, param->getName(), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 1, QString::fromLatin1(""), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 2, param->getUnit(), param->getId());
	UIUtil::makeTableItemWithData(lstParam, row, 3, QString::number(param->getElemNum()), param->getId());
	lstParam->setCurrentCell(row, 0);
}

void ParameterDialog::addParamClicked() {
  DDEBUG("ParameterDialog::addParamClicked()");

	TeachingEventHandler::instance()->prd_AddParamClicked(
		leName->text(),
		leId->text(),
		leUnit->text(),
		leNum->text(),
		cmbHide->currentIndex());
}

void ParameterDialog::deleteParamClicked() {
  DDEBUG("ParameterDialog::deleteParamClicked()");

	if(TeachingEventHandler::instance()->prd_DeleteParamClicked()==false) return;

  leName->setText("");
  leId->setText("");
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
							leUnit->text(),
							leNum->text(),
							cmbHide->currentIndex());
	if(ret) close();
}

void ParameterDialog::rejected() {
  DDEBUG("ParameterDialog::rejected()");
  close();
}

}
