#include "ArgumentDialog.h"
#include "TeachingUtil.h"
#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

ArgumentDialog::ArgumentDialog(QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
  curArgIdx_(NULL_ID), curActionIdx_(NULL_ID) {
  //
  lstModel = UIUtil::makeTableWidget(1, true);
  lstModel->setColumnWidth(0, 200);
  lstModel->setHorizontalHeaderLabels(QStringList() << "ID");
	//
	lstModelParam = UIUtil::makeTableWidget(2, true);
	lstModelParam->setColumnWidth(0, 100);
	lstModelParam->setColumnWidth(1, 130);
	lstModelParam->setHorizontalHeaderLabels(QStringList() << "Name" << "Definition");
	//
  lstParam = UIUtil::makeTableWidget(6, true);
  lstParam->setColumnWidth(0, 200);
  lstParam->setColumnWidth(1, 140);
  lstParam->setColumnWidth(2, 50);
  lstParam->setColumnWidth(3, 80);
  lstParam->setColumnWidth(4, 80);
  lstParam->setColumnWidth(5, 80);
  lstParam->setHorizontalHeaderLabels(QStringList() << "Name" << "ID" << "Type" << "ParamType" << "Model" << "Model Param");
  //
  QFrame* frmRef = new QFrame;
  frmRef->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QGridLayout* refLayout = new QGridLayout;
  refLayout->setContentsMargins(0, 0, 0, 0);
  frmRef->setLayout(refLayout);
  refLayout->addWidget(lstModel, 0, 0, 1, 1);
	refLayout->addWidget(lstModelParam, 0, 1, 1, 1);
	refLayout->addWidget(lstParam, 1, 0, 1, 2);
  /////
  lstHandling = UIUtil::makeTableWidget(3, false);
  lstHandling->setColumnWidth(0, 200);
  lstHandling->setColumnWidth(1, 200);
  lstHandling->setColumnWidth(2, 200);
  lstHandling->setHorizontalHeaderLabels(QStringList() << "Action" << "Model" << "Parameter");

  QPushButton* btnUp = new QPushButton(_("Up"));
  btnUp->setIcon(QIcon(":/Teaching/icons/up.png"));
  btnUp->setToolTip(_("Action Up"));

  QPushButton* btnDown = new QPushButton(_("Down"));
  btnDown->setIcon(QIcon(":/Teaching/icons/down.png"));
  btnDown->setToolTip(_("Action Down"));

  QPushButton* btnAdd = new QPushButton(_("Add"));
  btnAdd->setIcon(QIcon(":/Teaching/icons/plus.png"));
  btnAdd->setToolTip(_("Add Action"));

  QPushButton* btnDelete = new QPushButton(_("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/minus.png"));
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

  QLabel* lblTarget = new QLabel(_("Parameter:"));
  cmbTarget = new QComboBox(this);
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
  QLabel* lblName = new QLabel("Name: ");
  txtStateName = new QLineEdit();
  QFrame* frmName = new QFrame;
  QHBoxLayout* nameLayout = new QHBoxLayout(frmName);
  nameLayout->setContentsMargins(2, 2, 2, 2);
  nameLayout->addWidget(lblName);
  nameLayout->addWidget(txtStateName);

  QLabel* lblCmdName = new QLabel("Command Name: ");
  txtCmdName = new QLineEdit();
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
	connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
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
  resize(1250, 700);
  //
	TeachingEventHandler::instance()->agd_Loaded(this);
}

void ArgumentDialog::showModelInfo(const vector<ModelParamPtr>& modelList) {
	for (const auto& param : modelList) {
		int row = lstModel->rowCount();
		lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, param->getRName(), param->getMasterId());
		cmbModel->addItem(param->getRName());
	}
}

void ArgumentDialog::showModelParamInfo(const vector<ModelParameterParamPtr>& paramList) {
	lstModelParam->setRowCount(0);

	for (const auto& param : paramList) {
		int row = lstModelParam->rowCount();
		lstModelParam->insertRow(row);
		UIUtil::makeTableItem(lstModelParam, row, 0, param->getName());
		UIUtil::makeTableItem(lstModelParam, row, 1, param->getValueDesc());
	}
}

void ArgumentDialog::showParamInfo(const vector<ParameterParamPtr>& paramList, const vector<ModelParamPtr>& modelList) {
	cmbTarget->addItem("");

	for (const auto& param : paramList) {
		int row = lstParam->rowCount();
		lstParam->insertRow(row);
		UIUtil::makeTableItem(lstParam, row, 0, param->getName());
    UIUtil::makeTableItem(lstParam, row, 1, param->getRName());
    UIUtil::makeTableItem(lstParam, row, 2, UIUtil::getTypeName(param->getType()));
    UIUtil::makeTableItem(lstParam, row, 3, UIUtil::getParamTypeName(param->getParamType()));

    QString strModel = "";
    QString strModelParam = "";
    if (param->getType() == PARAM_KIND_MODEL) {
      vector<ModelParamPtr>::const_iterator targetModel = find_if(modelList.begin(), modelList.end(), ModelParamComparator(param->getModelId()));
      if (targetModel != modelList.end()) {
        strModel = (*targetModel)->getRName();
        vector<ModelParameterParamPtr> paramList = TeachingEventHandler::instance()->prd_ModelSelectionChanged(param->getModelId());
        for (int index = 0; index < paramList.size(); index++) {
          if (param->getModelParamId() == paramList[index]->getId()) {
            strModelParam = paramList[index]->getName();
            break;
          }
        }
      }
    }
    UIUtil::makeTableItem(lstParam, row, 4, strModel);
    UIUtil::makeTableItem(lstParam, row, 5, strModelParam);

		cmbTarget->addItem(param->getRName());
	}
}

void ArgumentDialog::showArgInfo(const ElementStmParamPtr target, const vector<ArgumentParamPtr>& argList) {
	txtStateName->setText(target->getCmdDspName());
	txtCmdName->setText(target->getCmdName());

	for (int index = 0; index < argList.size(); index++) {
		ArgumentParamPtr param = argList[index];

		int row = lstArg->rowCount();
		lstArg->insertRow(row);
		UIUtil::makeTableItemWithData(lstArg, row, 0, param->getName(), param->getId());
		int dir = target->getCommadDefParam()->getArgList()[index]->getDirection();
		QString strDir = "in";
		if (dir == 1) strDir = "out";
		UIUtil::makeTableItemWithData(lstArg, row, 1, strDir, param->getId());
		UIUtil::makeTableItemWithData(lstArg, row, 2, param->getValueDesc(), param->getId());
		//
		param->setValueDescOrg(param->getValueDesc());
	}
}

void ArgumentDialog::showActionInfo(const vector<ElementStmActionParamPtr>& actionList) {
	for (ElementStmActionParamPtr param : actionList) {
		int row = lstHandling->rowCount();
		lstHandling->insertRow(row);
		UIUtil::makeTableItemWithData(lstHandling, row, 0, param->getAction(), param->getId());
		UIUtil::makeTableItemWithData(lstHandling, row, 1, param->getModel(), param->getId());
		UIUtil::makeTableItemWithData(lstHandling, row, 2, param->getTarget(), param->getId());
	}
}

void ArgumentDialog::argSelectionChanged() {
	DDEBUG("ArgumentDialog::argSelectionChanged");

	if (curArgIdx_ != NULL_ID) {
		lstArg->item(curArgIdx_, 2)->setText(txtArgDef->toPlainText());
	}

	int selected = NULL_ID;
	curArgIdx_ = NULL_ID;
	QTableWidgetItem* item = lstArg->currentItem();
	if (item) {
		curArgIdx_ = lstArg->currentRow();
		selected = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->agd_ArgSelectionChanged(selected, txtArgDef->toPlainText());
}

void ArgumentDialog::updateArgument(QString currText) {
	txtArgDef->setText(currText);
}

void ArgumentDialog::actionSelectionChanged() {
	DDEBUG("ArgumentDialog::actionSelectionChanged");

	QString strAct = cmbAction->currentIndex()== ACTION_ATTACH ? "attach": "detach";
	QString strModel = cmbModel->itemText(cmbModel->currentIndex());
	QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());

	if (curActionIdx_ != NULL_ID) {
		lstHandling->item(curActionIdx_, 0)->setText(strAct);
		lstHandling->item(curActionIdx_, 1)->setText(strModel);
		lstHandling->item(curActionIdx_, 2)->setText(strTarget);
	}
	//
	QTableWidgetItem* item = lstHandling->currentItem();
	int selected = NULL_ID;
	curActionIdx_ = NULL_ID;
	if (item) {
		curActionIdx_ = lstHandling->currentRow();
		selected = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->agd_ActionSelectionChanged(selected, strAct, strModel, strTarget);
}

void ArgumentDialog::updateAction(ElementStmActionParamPtr& target) {
		if (target->getAction() == "attach") {
			cmbAction->setCurrentIndex(0);
		} else if (target->getAction() == "detach") {
			cmbAction->setCurrentIndex(1);
		}
		cmbModel->setCurrentIndex(cmbModel->findText(target->getModel()));
		cmbTarget->setCurrentIndex(cmbTarget->findText(target->getTarget()));
}

void ArgumentDialog::modelSelectionChanged() {
	int selected = NULL_ID;
	QTableWidgetItem* item = lstModel->currentItem();
	if (item) {
		selected = item->data(Qt::UserRole).toInt();
	}
	TeachingEventHandler::instance()->agd_ModelSelectionChanged(selected);
}

void ArgumentDialog::addClicked() {
  DDEBUG("ArgumentDialog::addClicked");

	QString strAct = cmbAction->currentIndex() == ACTION_ATTACH ? "attach" : "detach";
	QString strModel = cmbModel->itemText(cmbModel->currentIndex());
	QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());

	if (curActionIdx_ != NULL_ID) {
		lstHandling->item(curActionIdx_, 0)->setText(strAct);
		lstHandling->item(curActionIdx_, 1)->setText(strModel);
		lstHandling->item(curActionIdx_, 2)->setText(strTarget);
	}
	//
	TeachingEventHandler::instance()->agd_AddClicked(strAct, strModel, strTarget);
}

void ArgumentDialog::updateAddAction(ElementStmActionParamPtr& target) {
	int row = lstHandling->rowCount();
	lstHandling->insertRow(row);
	UIUtil::makeTableItemWithData(lstHandling, row, 0, target->getAction(), target->getId());
	UIUtil::makeTableItemWithData(lstHandling, row, 1, target->getModel(), target->getId());
	UIUtil::makeTableItemWithData(lstHandling, row, 2, target->getTarget(), target->getId());
}

void ArgumentDialog::deleteClicked() {
	TeachingEventHandler::instance()->agd_DeleteClicked();

	if(curActionIdx_ != NULL_ID) {
    cmbAction->setCurrentIndex(0);
    cmbModel->setCurrentIndex(0);
    cmbTarget->setCurrentIndex(0);

    int currRow = lstHandling->currentRow();
    lstHandling->removeRow(currRow);
    lstHandling->setFocus();

		curActionIdx_ = NULL_ID;
	}
}

void ArgumentDialog::upClicked() {
  DDEBUG("ArgumentDialog::upClicked");

	QString strAct = cmbAction->currentIndex() == ACTION_ATTACH ? "attach" : "detach";
	QString strModel = cmbModel->itemText(cmbModel->currentIndex());
	QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());

	if (curActionIdx_ != NULL_ID) {
		lstHandling->item(curActionIdx_, 0)->setText(strAct);
		lstHandling->item(curActionIdx_, 1)->setText(strModel);
		lstHandling->item(curActionIdx_, 2)->setText(strTarget);
	}
	TeachingEventHandler::instance()->agd_Update(strAct, strModel, strTarget);
  //
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if (sourceIdx == 0) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx - 1);
  lstHandling->setFocus();
}

void ArgumentDialog::downClicked() {
  DDEBUG("ArgumentDialog::downClicked");

	QString strAct = cmbAction->currentIndex() == ACTION_ATTACH ? "attach" : "detach";
	QString strModel = cmbModel->itemText(cmbModel->currentIndex());
	QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());

	if (curActionIdx_ != NULL_ID) {
		lstHandling->item(curActionIdx_, 0)->setText(strAct);
		lstHandling->item(curActionIdx_, 1)->setText(strModel);
		lstHandling->item(curActionIdx_, 2)->setText(strTarget);
	}
	TeachingEventHandler::instance()->agd_Update(strAct, strModel, strTarget);
	//
  int sourceIdx = lstHandling->verticalHeader()->visualIndex(lstHandling->currentRow());
  if (lstHandling->rowCount() <= sourceIdx) return;
  lstHandling->verticalHeader()->swapSections(sourceIdx, sourceIdx + 1);
  lstHandling->setFocus();
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

	QString strAct = cmbAction->currentIndex() == ACTION_ATTACH ? "attach" : "detach";
	QString strModel = cmbModel->itemText(cmbModel->currentIndex());
	QString strTarget = cmbTarget->itemText(cmbTarget->currentIndex());
	QString strArgDef = txtArgDef->toPlainText();
  /////
  int seq = 1;
	for (int index = 0; index < lstHandling->rowCount(); index++) {
		int logicalIdx = lstHandling->verticalHeader()->logicalIndex(index);
		int selected = lstHandling->item(logicalIdx, 0)->data(Qt::UserRole).toInt();
		DDEBUG_V("%d %d %d", index, logicalIdx, selected);
		TeachingEventHandler::instance()->agd_SetSeq(selected, seq);
		seq++;
	}

	if(TeachingEventHandler::instance()->agd_OKClicked(strName, strAct, strModel, strTarget, strArgDef)==false) return;
  //
  isOK_ = true;
  close();
}

void ArgumentDialog::cancelClicked() {
	TeachingEventHandler::instance()->agd_CancelClicked();
  isOK_ = false;
  close();
}

void ArgumentDialog::rejected() {
  DDEBUG("ArgumentDialog::rejected");
  close();
}

}
