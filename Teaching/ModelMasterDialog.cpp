#include "ModelMasterDialog.h"
#include "TeachingUtil.h"
#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

	ModelMasterDialog::ModelMasterDialog(QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
		currentIndex_(NULL_ID), currentParamIndex_(NULL_ID), eventCancel_(false){
  lstModel = UIUtil::makeTableWidget(1, false);
  lstModel->setColumnWidth(0, 400);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Name");

  QFrame* frmTask = new QFrame;
  QLabel* lblModel = new QLabel(_("Model Name:"));
  leModel = new QLineEdit;
  QLabel* lblFile = new QLabel(_("Model File:"));
  leFile = new QLineEdit;

	btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(_("Ref..."));
	btnRef->setEnabled(false);
  //
	QPushButton* btnAddModel = new QPushButton(_("Add"));
	btnAddModel->setIcon(QIcon(":/Teaching/icons/Plus.png"));
	btnAddModel->setToolTip(_("Add new Model"));

	QPushButton* btnDeleteModel = new QPushButton(_("Delete"));
	btnDeleteModel->setIcon(QIcon(":/Teaching/icons/Delete.png"));
	btnDeleteModel->setToolTip(_("Delete selected Model"));

	QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setContentsMargins(2, 0, 2, 0);
  frmTask->setLayout(taskLayout);
	taskLayout->addWidget(lstModel, 0, 0, 1, 5);
	taskLayout->addWidget(lblModel, 1, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModel, 1, 1, 1, 4);
  taskLayout->addWidget(lblFile, 2, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leFile, 2, 1, 1, 3);
  taskLayout->addWidget(btnRef, 2, 4, 1, 1);
	taskLayout->addWidget(btnAddModel, 3, 0, 1, 1);
	taskLayout->addWidget(btnDeleteModel, 3, 4, 1, 1);
	//
	lstParam = UIUtil::makeTableWidget(2, false);
	lstParam->setColumnWidth(0, 100);
	lstParam->setColumnWidth(1, 300);
	lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "Definition");

	QLabel* lblParam = new QLabel(_("Parameter:"));
	leParam = new QLineEdit;
	txtDef = new QTextEdit;
	txtDef->setMaximumHeight(50);

	QPushButton* btnAddParam = new QPushButton(_("Add"));
	btnAddParam->setIcon(QIcon(":/Teaching/icons/Plus.png"));
	btnAddParam->setToolTip(_("Add new Model Parameter"));

	QPushButton* btnDeleteParam = new QPushButton(_("Delete"));
	btnDeleteParam->setIcon(QIcon(":/Teaching/icons/Delete.png"));
	btnDeleteParam->setToolTip(_("Delete selected Model Parameter"));

	QFrame* frmParam = new QFrame;
	QGridLayout* paramLayout = new QGridLayout;
	paramLayout->setContentsMargins(2, 0, 2, 0);
	frmParam->setLayout(paramLayout);
	paramLayout->addWidget(lstParam, 0, 0, 1, 5);
	paramLayout->addWidget(lblParam, 1, 0, 1, 1, Qt::AlignRight);
	paramLayout->addWidget(leParam, 1, 1, 1, 4);
	paramLayout->addWidget(txtDef, 2, 0, 1, 5);
	paramLayout->addWidget(btnAddParam, 3, 0, 1, 1);
	paramLayout->addWidget(btnDeleteParam, 3, 4, 1, 1);

	QFrame* frmBase = new QFrame;
	QHBoxLayout* baseLayout = new QHBoxLayout(frmBase);
	baseLayout->addWidget(frmTask);
	baseLayout->addWidget(frmParam);

  QFrame* frmBotButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
	QPushButton* btnCancel = new QPushButton(_("Cancel"));
	QHBoxLayout* buttonBotLayout = new QHBoxLayout(frmBotButtons);
  buttonBotLayout->setContentsMargins(2, 2, 2, 2);
	buttonBotLayout->addWidget(btnCancel);
	buttonBotLayout->addStretch();
  buttonBotLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(frmBase);
  mainLayout->addWidget(frmBotButtons);
  setLayout(mainLayout);
  ////
  connect(btnAddModel, SIGNAL(clicked()), this, SLOT(addModelClicked()));
  connect(btnDeleteModel, SIGNAL(clicked()), this, SLOT(deleteModelClicked()));
	connect(btnAddParam, SIGNAL(clicked()), this, SLOT(addModelParamClicked()));
	connect(btnDeleteParam, SIGNAL(clicked()), this, SLOT(deleteModelParamClicked()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
	connect(lstParam, SIGNAL(itemSelectionChanged()), this, SLOT(modelParamSelectionChanged()));
	connect(btnRef, SIGNAL(clicked()), this, SLOT(refClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Models"));
  resize(900, 350);

	TeachingEventHandler::instance()->mmd_Loaded(this);
}

void ModelMasterDialog::showGrid(const vector<ModelMasterParamPtr>& masterList) {
	DDEBUG("ModelMasterDialog::showGrid");

	lstModel->setRowCount(0);

  for (int index = 0; index < masterList.size(); index++) {
		ModelMasterParamPtr param = masterList[index];

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, param->getName(), param->getId());
  }
}

void ModelMasterDialog::showParamGrid(const vector<ModelParameterParamPtr>& paramList) {
	DDEBUG("ModelMasterDialog::showParamGrid");
	eventCancel_ = true;

	lstParam->setRowCount(0);

	for (int index = 0; index < paramList.size(); index++) {
		ModelParameterParamPtr param = paramList[index];

		int row = lstParam->rowCount();
		lstParam->insertRow(row);
		UIUtil::makeTableItemWithData(lstParam, row, 0, param->getName(), param->getId());
		UIUtil::makeTableItemWithData(lstParam, row, 1, param->getValueDesc(), param->getId());
	}
	eventCancel_ = false;
}

void ModelMasterDialog::updateContents(QString name, QString fileName) {
	DDEBUG_V("ModelMasterDialog::updateContents %s, %s",name.toStdString().c_str(), fileName.toStdString().c_str());

	leModel->setText(name);
	leFile->setText(fileName);
	btnRef->setEnabled(true);
}

void ModelMasterDialog::updateParamContents(QString name, QString desc) {
	DDEBUG_V("ModelMasterDialog::updateParamContents %s, %s", name.toStdString().c_str(), desc.toStdString().c_str());
	leParam->setText(name);
	txtDef->setText(desc);
}

//////////
void ModelMasterDialog::modelSelectionChanged() {
	DDEBUG("ModelMasterDialog::modelSelectionChanged()");

	if (currentParamIndex_ != NULL_ID) {
		modelParamSelectionChanged();
		leParam->clear();
		txtDef->clear();
		currentParamIndex_ = NULL_ID;
	}

	if (currentIndex_ != NULL_ID) {
		lstModel->item(currentIndex_, 0)->setText(leModel->text());
	}
	currentIndex_ = lstModel->currentRow();

	int newId = NULL_ID;
	QString strName = "";
	QString strFile = "";
	QTableWidgetItem* item = lstModel->currentItem();
	if (item) {
		strName = leModel->text();
		strFile = leFile->text();
		newId = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->mmd_ModelSelectionChanged(newId, strName, strFile);
}

void ModelMasterDialog::modelParamSelectionChanged() {
	if (eventCancel_) return;
	DDEBUG("ModelMasterDialog::modelParamSelectionChanged()");

	if (currentParamIndex_ != NULL_ID) {
		lstParam->item(currentParamIndex_, 0)->setText(leParam->text());
		lstParam->item(currentParamIndex_, 1)->setText(txtDef->toPlainText());
	}
	currentParamIndex_ = lstParam->currentRow();

	int newId = NULL_ID;
	QString strName = "";
	QString strDef = "";
	QTableWidgetItem* item = lstParam->currentItem();
	if (item) {
		strName = leParam->text();
		strDef = txtDef->toPlainText();
		newId = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->mmd_ModelParameterSelectionChanged(newId, strName, strDef);
}

void ModelMasterDialog::addModel(int id, QString name) {
	DDEBUG("ModelMasterDialog::addModel()");

	int row = lstModel->rowCount();
	lstModel->insertRow(row);
	UIUtil::makeTableItemWithData(lstModel, row, 0, name, id);
	lstModel->selectRow(row);
}

void ModelMasterDialog::refClicked() {
	DDEBUG("ModelMasterDialog::refClicked()");
	TeachingEventHandler::instance()->mmd_RefClicked();
}

void ModelMasterDialog::addModelClicked() {
	DDEBUG("ModelMasterDialog::addModelClicked()");
	TeachingEventHandler::instance()->mmd_AddModelClicked();
}

void ModelMasterDialog::deleteModelClicked() {
	DDEBUG("ModelMasterDialog::deleteModelClicked()");

	QTableWidgetItem* item = lstModel->currentItem();
	if (item) {
		int modelId = item->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->mmd_DeleteModelClicked(modelId);

		lstModel->removeRow(lstModel->currentRow());
		lstModel->setFocus();
		currentIndex_ = lstModel->currentRow();
	}
}

void ModelMasterDialog::addModelParamClicked() {
	TeachingEventHandler::instance()->mmd_AddModelParamClicked();
}

void ModelMasterDialog::deleteModelParamClicked() {
	QTableWidgetItem* item = lstParam->currentItem();
	if (item) {
		int id = item->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->mmd_DeleteModelParamClicked();

		lstParam->removeRow(lstParam->currentRow());
		lstParam->setFocus();
		currentParamIndex_ = lstParam->currentRow();
	}
}

void ModelMasterDialog::okClicked() {
  DDEBUG("ModelDialog::okClicked()");

	if (currentParamIndex_ != NULL_ID) {
		modelParamSelectionChanged();
	}

  if (TeachingEventHandler::instance()->mmd_Check()) {
    QMessageBox::warning(this, _("Model Master"), "The same model has already been registered.");
    return;
  }

	QString strName = leModel->text();
	QString strFile = leFile->text();
	QString errMessage;
	if (TeachingEventHandler::instance()->mmd_OkClicked(strName, strFile, errMessage)) {
		QMessageBox::information(this, _("Database"), _("Database updated"));
		close();
	}	else {
		QMessageBox::warning(this, _("Database Error"), errMessage);
	}
}

void ModelMasterDialog::cancelClicked() {
	close();
}

}
