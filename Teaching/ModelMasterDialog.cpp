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
		currentIndex_(-1) {
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
  QFrame* frmButtons = new QFrame;
	QPushButton* btnAddModel = new QPushButton(_("Add"));
  btnAddModel->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAddModel->setToolTip(_("Add new Model"));

	QPushButton* btnDeleteModel = new QPushButton(_("Delete"));
  btnDeleteModel->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteModel->setToolTip(_("Delete selected Model"));

  QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setContentsMargins(2, 0, 2, 0);
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(lblModel, 1, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModel, 1, 1, 1, 7);
  taskLayout->addWidget(lblFile, 2, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leFile, 2, 2, 1, 5);
  taskLayout->addWidget(btnRef, 2, 7, 1, 1);

  //
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(btnAddModel);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDeleteModel);

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
  mainLayout->addWidget(lstModel);
  mainLayout->addWidget(frmTask);
  mainLayout->addWidget(frmButtons);
  mainLayout->addWidget(frmBotButtons);
  setLayout(mainLayout);
  ////
  connect(btnAddModel, SIGNAL(clicked()), this, SLOT(addModelClicked()));
  connect(btnDeleteModel, SIGNAL(clicked()), this, SLOT(deleteModelClicked()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
  connect(btnRef, SIGNAL(clicked()), this, SLOT(refClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Models"));
  resize(450, 350);

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

void ModelMasterDialog::updateContents(QString name, QString fileName) {
	DDEBUG_V("ModelMasterDialog::updateContents %s, %s",name.toStdString().c_str(), fileName.toStdString().c_str());

	leModel->setText(name);
	leFile->setText(fileName);
	btnRef->setEnabled(true);
}
//////////
void ModelMasterDialog::modelSelectionChanged() {
	DDEBUG("ModelMasterDialog::modelSelectionChanged()");

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

void ModelMasterDialog::okClicked() {
  DDEBUG("ModelDialog::okClicked()");

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
