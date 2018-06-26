#include "ModelDialog.h"

#include "TeachingEventHandler.h"
#include "TeachingUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

ModelDialog::ModelDialog(QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
		currentIndex_(-1), currentMasterIndex_(-1) {

  lstModel = UIUtil::makeTableWidget(2, true);
  lstModel->setColumnWidth(0, 50);
  lstModel->setColumnWidth(1, 200);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");

	lstModelMaster = UIUtil::makeTableWidget(2, true);
	lstModelMaster->setColumnWidth(0, 50);
  lstModelMaster->setColumnWidth(1, 200);
  lstModelMaster->setHorizontalHeaderLabels(QStringList() << "" << "Master Name");
  lstModelMaster->setIconSize(QSize(48, 48));

	QPushButton* btnAddModel = new QPushButton("<<");
	btnAddModel->setToolTip(_("Add new Model"));
	btnAddModel->setFixedWidth(30);
	btnAddModel->setAutoDefault(false);

	QPushButton* btnDeleteModel = new QPushButton(">>");
	btnDeleteModel->setToolTip(_("Delete selected Model"));
	btnDeleteModel->setFixedWidth(30);
	btnDeleteModel->setAutoDefault(false);

	QFrame* frmModelList= new QFrame;
  frmModelList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QGridLayout* modelLayout = new QGridLayout(frmModelList);
	//modelLayout->setContentsMargins(2, 2, 2, 2);
	modelLayout->addWidget(lstModel, 0, 0, 4, 1);
	modelLayout->addWidget(btnAddModel, 1, 1, 1, 1);
	modelLayout->addWidget(btnDeleteModel, 2, 1, 1, 1);
	modelLayout->addWidget(lstModelMaster, 0, 2, 4, 1);
	//
  QFrame* frmTask = new QFrame;
	QLabel* lblMaster = new QLabel(_("Master Name:"));
	leMaster = new QLineEdit;
	leMaster->setEnabled(false);
	QLabel* lblModelRName = new QLabel(_("Model ID:"));
  leModelRName = new QLineEdit;
  QLabel* lblFile = new QLabel(_("Model Type:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Env.");
  cmbType->addItem("E.E.");
  cmbType->addItem("Work");

  QLabel* lblX = new QLabel(_("Origin X:"));
  leX = new QLineEdit; leX->setAlignment(Qt::AlignRight);
  QLabel* lblY = new QLabel(_("Y:"));
  leY = new QLineEdit; leY->setAlignment(Qt::AlignRight);
  QLabel* lblZ = new QLabel(_("Z:"));
  leZ = new QLineEdit; leZ->setAlignment(Qt::AlignRight);
  QLabel* lblRx = new QLabel(_("Rx:"));
  leRx = new QLineEdit; leRx->setAlignment(Qt::AlignRight);
  QLabel* lblRy = new QLabel(_("Ry:"));
  leRy = new QLineEdit; leRy->setAlignment(Qt::AlignRight);
  QLabel* lblRz = new QLabel(_("Rz:"));
  leRz = new QLineEdit; leRz->setAlignment(Qt::AlignRight);

  QLabel* lblVisibility = new QLabel(_("Visibility:"));
  cmbHide = new QComboBox(this);
  cmbHide->addItem("public");
  cmbHide->addItem("private");
  //
  QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setContentsMargins(2, 0, 2, 0);
  frmTask->setLayout(taskLayout);
	taskLayout->addWidget(lblMaster, 0, 0, 1, 1, Qt::AlignRight);
	taskLayout->addWidget(leMaster, 0, 1, 1, 5);
	taskLayout->addWidget(lblModelRName, 1, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModelRName, 1, 1, 1, 5);
  taskLayout->addWidget(lblFile, 2, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(cmbType, 2, 1, 1, 5);

  taskLayout->addWidget(lblX, 3, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leX, 3, 1, 1, 1);
  taskLayout->addWidget(lblY, 3, 2, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leY, 3, 3, 1, 1);
  taskLayout->addWidget(lblZ, 3, 4, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leZ, 3, 5, 1, 1);
  taskLayout->addWidget(lblRx, 4, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRx, 4, 1, 1, 1);
  taskLayout->addWidget(lblRy, 4, 2, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRy, 4, 3, 1, 1);
  taskLayout->addWidget(lblRz, 4, 4, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRz, 4, 5, 1, 1);

  taskLayout->addWidget(lblVisibility, 5, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(cmbHide, 5, 1, 1, 1);
  //
  QFrame* frmBotButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
	btnOK->setAutoDefault(false);
	QPushButton* btnCancel = new QPushButton(_("Cancel"));
	btnCancel->setAutoDefault(false);
	QHBoxLayout* buttonBotLayout = new QHBoxLayout(frmBotButtons);
  buttonBotLayout->setContentsMargins(2, 2, 2, 2);
	buttonBotLayout->addWidget(btnCancel);
	buttonBotLayout->addStretch();
  buttonBotLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(frmModelList);
  mainLayout->addWidget(frmTask);
  mainLayout->addWidget(frmBotButtons);
  setLayout(mainLayout);
  ////
  connect(btnAddModel, SIGNAL(clicked()), this, SLOT(addModelClicked()));
  connect(btnDeleteModel, SIGNAL(clicked()), this, SLOT(deleteModelClicked()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
	connect(lstModelMaster, SIGNAL(itemSelectionChanged()), this, SLOT(modelMasterSelectionChanged()));
	connect(btnOK, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  connect(leX, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leY, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leZ, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRx, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRy, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRz, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));

  setWindowTitle(_("Models"));
  resize(650, 400);

	if (TeachingEventHandler::instance()->mdd_Loaded(this) == false) {
		QMessageBox::warning(this, _("Model"), _("Please select target TASK"));
	}
}

void ModelDialog::showModelGrid(const vector<ModelParamPtr>& source) {
	DDEBUG("ModelDialog::showModelGrid()");

	lstModel->setRowCount(0);

	for (int index = 0; index < source.size(); index++) {
		ModelParamPtr param = source[index];
		int row = lstModel->rowCount();
		lstModel->insertRow(row);
		UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), param->getId());
		UIUtil::makeTableItemWithData(lstModel, row, 1, param->getRName(), param->getId());
	}
}

void ModelDialog::showModelMasterGrid(const vector<ModelMasterParamPtr>& source) {
	lstModelMaster->setRowCount(0);

	for (int index = 0; index < source.size(); index++) {
		ModelMasterParamPtr param = source[index];
		int row = lstModelMaster->rowCount();
		lstModelMaster->insertRow(row);
    lstModelMaster->setRowHeight(row, 50);

    QTableWidgetItem* imageItem = new QTableWidgetItem;
    lstModelMaster->setItem(row, 0, imageItem);
    imageItem->setData(Qt::UserRole, param->getId());
    imageItem->setIcon(QIcon(QPixmap::fromImage(param->getImage())));

		UIUtil::makeTableItemWithData(lstModelMaster, row, 1, param->getName(), param->getId());
	}
}

void ModelDialog::updateContents(const ModelParamPtr& source) {
	if (source) {
		leMaster->setText(source->getModelMaster()->getName());
		leModelRName->setText(source->getRName());
		cmbType->setCurrentIndex(source->getType());
		leX->setText(QString::number(source->getPosX(), 'f', 6));
		leY->setText(QString::number(source->getPosY(), 'f', 6));
		leZ->setText(QString::number(source->getPosZ(), 'f', 6));
		leRx->setText(QString::number(source->getRotRx(), 'f', 6));
		leRy->setText(QString::number(source->getRotRy(), 'f', 6));
		leRz->setText(QString::number(source->getRotRz(), 'f', 6));
    cmbHide->setCurrentIndex(source->getHide());

  }	else {
		leMaster->setText("");
		leModelRName->setText("");
		leX->setText("");
		leY->setText("");
		leZ->setText("");
		leRx->setText("");
		leRy->setText("");
		leRz->setText("");
    cmbHide->setCurrentIndex(0);
  }
}
//////////
void ModelDialog::modelSelectionChanged() {
	DDEBUG("ModelDialog::modelSelectionChanged()");

	QString strModelRName = leModelRName->text();
	int selectedType = cmbType->currentIndex();

	int newId = NULL_ID;
	string strPosX = leX->text().toUtf8().constData();
	double posX = std::atof(strPosX.c_str());
	string strPosY = leY->text().toUtf8().constData();
	double posY = std::atof(strPosY.c_str());
	string strPosZ = leZ->text().toUtf8().constData();
	double posZ = std::atof(strPosZ.c_str());
	string strRotRx = leRx->text().toUtf8().constData();
	double rotRx = std::atof(strRotRx.c_str());
	string strRotRy = leRy->text().toUtf8().constData();
	double rotRy = std::atof(strRotRy.c_str());
	string strRotRz = leRz->text().toUtf8().constData();
	double rotRz = std::atof(strRotRz.c_str());

  int hide = cmbHide->currentIndex();

	if (currentIndex_ != NULL_ID) {
		lstModel->item(currentIndex_, 0)->setText(getTypeName(selectedType));
		lstModel->item(currentIndex_, 1)->setText(strModelRName);
	}
	currentIndex_ = lstModel->currentRow();

	QTableWidgetItem* item = lstModel->currentItem();
	if (item) {
		newId = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->mdd_ModelSelectionChanged(newId, strModelRName, selectedType, posX, posY, posZ, rotRx, rotRy, rotRz, hide);
}

void ModelDialog::modelMasterSelectionChanged() {
	currentMasterIndex_ = lstModelMaster->currentRow();

	int newId = NULL_ID;
	QTableWidgetItem* item = lstModelMaster->currentItem();
	if (item) {
		newId = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->mdd_ModelMasterSelectionChanged(newId);
}

void ModelDialog::modelPositionChanged() {
	double posX = leX->text().toDouble();
	double posY = leY->text().toDouble();
	double posZ = leZ->text().toDouble();
	double rotX = leRx->text().toDouble();
	double rotY = leRy->text().toDouble();
	double rotZ = leRz->text().toDouble();
	TeachingEventHandler::instance()->mdd_ModelPositionChanged(posX, posY, posZ, rotX, rotY, rotZ);
}

void ModelDialog::addModelClicked() {
	DDEBUG("ModelDialog::addModelClicked()");

	if (TeachingEventHandler::instance()->mdd_AddModelClicked() == false) {
		QMessageBox::warning(this, _("Model"), _("Please select target MASTER"));
		return;
	}
}

void ModelDialog::deleteModelClicked() {
	DDEBUG("ModelDialog::deleteModelClicked()");

	if (TeachingEventHandler::instance()->mdd_DeleteModelClicked() == false) {
		QMessageBox::warning(this, _("Model"), _("Please select target Model"));
		return;
	}
}

void ModelDialog::okClicked() {
	DDEBUG("ModelDialog::okClicked()");

  QString strModelRName = leModelRName->text();
  if (0 < strModelRName.trimmed().length()) {
    if (TeachingEventHandler::instance()->mdd_CheckModel(strModelRName) == false) {
      QMessageBox::warning(this, _("Model"), _("Duplicate specified model ID."));
      return;
    }
  }

	int selectedType = cmbType->currentIndex();

	string strPosX = leX->text().toUtf8().constData();
	double posX = std::atof(strPosX.c_str());
	string strPosY = leY->text().toUtf8().constData();
	double posY = std::atof(strPosY.c_str());
	string strPosZ = leZ->text().toUtf8().constData();
	double posZ = std::atof(strPosZ.c_str());
	string strRotRx = leRx->text().toUtf8().constData();
	double rotRx = std::atof(strRotRx.c_str());
	string strRotRy = leRy->text().toUtf8().constData();
	double rotRy = std::atof(strRotRy.c_str());
	string strRotRz = leRz->text().toUtf8().constData();
	double rotRz = std::atof(strRotRz.c_str());

  int hide = cmbHide->currentIndex();

	TeachingEventHandler::instance()->mdd_OkClicked(strModelRName, selectedType, posX, posY, posZ, rotRx, rotRy, rotRz, hide);
}

void ModelDialog::cancelClicked() {
	TeachingEventHandler::instance()->mdd_CancelClicked();
}
/////
QString ModelDialog::getTypeName(int source) {
  QString result = "";

  switch (source) {
    case 0:
    result = "Env.";
    break;
    case 1:
    result = "E.E.";
    break;
    case 2:
    result = "Work";
    break;
  }

  return result;
}

}
