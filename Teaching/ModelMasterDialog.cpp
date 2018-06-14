#include "ModelMasterDialog.h"
#include "TeachingUtil.h"
#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

#include "DataBaseManager.h"

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

  QLabel* lblImage = new QLabel(_("Image File:"));
  leImage = new QLineEdit;

  btnRefImage = new QPushButton();
  btnRefImage->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRefImage->setToolTip(_("Ref..."));
  btnRefImage->setEnabled(false);
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
  taskLayout->addWidget(lblImage, 3, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leImage, 3, 1, 1, 3);
  taskLayout->addWidget(btnRefImage, 3, 4, 1, 1);
  taskLayout->addWidget(btnAddModel, 4, 0, 1, 1);
	taskLayout->addWidget(btnDeleteModel, 4, 4, 1, 1);
  ///////
  imageView = new QGraphicsView();
  scene = new QGraphicsScene();
  imageView->setScene(scene);

  QPushButton* btnDeleteImage = new QPushButton(_("Delete"));
  btnDeleteImage->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteImage->setToolTip(_("Delete Model Image."));

  QFrame* frmImage = new QFrame;
  QGridLayout* imageLayout = new QGridLayout;
  imageLayout->setContentsMargins(2, 0, 2, 0);
  frmImage->setLayout(imageLayout);
  imageLayout->addWidget(imageView, 0, 0, 1, 3);
  imageLayout->addWidget(btnDeleteImage, 1, 1, 1, 1);
  ///////
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
	baseLayout->addWidget(frmTask, 1);
  baseLayout->addWidget(frmImage, 1);
  baseLayout->addWidget(frmParam, 1);

  QFrame* frmBotButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
  QPushButton* btnReNew = new QPushButton(_("ReNew"));
  QPushButton* btnCancel = new QPushButton(_("Cancel"));
	QHBoxLayout* buttonBotLayout = new QHBoxLayout(frmBotButtons);
  buttonBotLayout->setContentsMargins(2, 2, 2, 2);
	buttonBotLayout->addWidget(btnCancel);
  buttonBotLayout->addStretch();
  buttonBotLayout->addWidget(btnReNew);
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
  connect(btnRefImage, SIGNAL(clicked()), this, SLOT(refImageClicked()));
  connect(btnDeleteImage, SIGNAL(clicked()), this, SLOT(deleteImageClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));
  connect(btnReNew, SIGNAL(clicked()), this, SLOT(reNewClicked()));

  setWindowTitle(_("Models"));
  resize(1200, 500);

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

void ModelMasterDialog::updateContents(QString name, QString fileName, QString imageFileName, QImage* targetImage) {
	DDEBUG_V("ModelMasterDialog::updateContents %s, %s",name.toStdString().c_str(), fileName.toStdString().c_str());

	leModel->setText(name);
	leFile->setText(fileName);
	btnRef->setEnabled(true);

  leImage->setText(imageFileName);
  scene->clear();
  if (targetImage) {
    QPixmap pixmap = QPixmap::fromImage(*targetImage);
    if (!pixmap.isNull()) {
      pixmap = pixmap.scaled(imageView->width() - 5, imageView->height() - 5, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
    scene->addPixmap(pixmap);
  }
  btnRefImage->setEnabled(true);
}

void ModelMasterDialog::updateImage(QString fileName, QImage targetImage) {
  leImage->setText(fileName);

  scene->clear();
  QPixmap pixmap = QPixmap::fromImage(targetImage);
  if (!pixmap.isNull()) {
    pixmap = pixmap.scaled(imageView->width()-5, imageView->height()-5, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  scene->addPixmap(pixmap);
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

void ModelMasterDialog::refImageClicked() {
  TeachingEventHandler::instance()->mmd_RefImageClicked();
}

void ModelMasterDialog::deleteImageClicked() {
  TeachingEventHandler::instance()->mmd_DeleteImageClicked();
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
    QMessageBox::warning(this, _("Model Master"), _("The same model has already been registered."));
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

void ModelMasterDialog::reNewClicked() {
  //ハッシュ値の生成
  vector<ModelMasterParamPtr> modelList = DatabaseManager::getInstance().getModelMasterList();
  for (int index = 0; index < modelList.size(); index++) {
    ModelMasterParamPtr target = modelList[index];
    if (0 < target->getHash().length()) continue;
    QString txtData = QString::fromUtf8(target->getData());
    QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
    target->setHash(strHash);
    target->setUpdate();
  }
  DatabaseManager::getInstance().saveModelMasterList(modelList);
  //重複データの削除
  modelList = DatabaseManager::getInstance().getModelMasterList();
  for (int index = 0; index < modelList.size(); index++) {
    ModelMasterParamPtr target = modelList[index];
    DatabaseManager::getInstance().reNewModelMaster(target);
  }
  //T_TASK_MODEL_INSTの整理
  vector<TaskModelParamPtr> taskList = DatabaseManager::getInstance().getAllTask();
  for (int index = 0; index < taskList.size(); index++) {
    TaskModelParamPtr targetTask = taskList[index];
    int flowId = targetTask->getFlowId();
    if (flowId < 0) continue;
    if (DatabaseManager::getInstance().checkFlowState(targetTask->getId())) continue;
    //
    DatabaseManager::getInstance().deleteTaskModel(targetTask->getId());
  }
  QMessageBox::information(this, _("Database"), _("Database updated"));
}

}
