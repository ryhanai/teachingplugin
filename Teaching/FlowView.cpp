#include <cnoid/InfoBar>

#include "FlowView.h"

#include "TeachingEventHandler.h"
#include "DecisionDialog.h"

#include "NodeEditor/NodeStyle.hpp"
#include "NodeEditor/ConnectionStyle.hpp"
#include "NodeEditor/FlowViewStyle.hpp"

#include "NodeEditor/FlowScene.hpp"
#include "NodeEditor/models.hpp"
#include "TeachingUtil.h"

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {
FlowSearchDialog::FlowSearchDialog(bool canEdit, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(_("Condition:"));
  leCond = new QLineEdit;
  QPushButton* btnSearch = new QPushButton(_("Search"));
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Flow"));

  QHBoxLayout* topLayout = new QHBoxLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond);
  topLayout->addWidget(leCond);
  topLayout->addWidget(btnSearch);
  //
  lstFlow = new QTableWidget(0, 4);
  lstFlow->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstFlow->setSelectionMode(QAbstractItemView::SingleSelection);
  lstFlow->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lstFlow->verticalHeader()->setVisible(false);
  lstFlow->setColumnWidth(0, 200);
  lstFlow->setColumnWidth(1, 400);
  lstFlow->setColumnWidth(2, 150);
  lstFlow->setColumnWidth(3, 150);
  lstFlow->setRowCount(0);
  lstFlow->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnSelect = new QPushButton(_("Open"));
  btnSelect->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnSelect->setToolTip(_("Open selected Flow"));

  QPushButton* btnDelete = new QPushButton(_("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(_("Delete selected Flow"));

  QPushButton* btnCancel = new QPushButton(_("Cancel"));
  btnCancel->setIcon(QIcon(":/Teaching/icons/Cancel.png"));
  btnCancel->setToolTip(_("Cancel Flow select"));

  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addWidget(btnSelect);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnCancel);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(condFrame);
  mainLayout->addWidget(lstFlow);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(leCond, SIGNAL(editingFinished()), this, SLOT(searchClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnSelect, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(lstFlow, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(oKClicked()));
  //
  setWindowTitle(_("Flow List"));
  setMinimumHeight(sizeHint().height());
  setMinimumWidth(600);
  //
  btnDelete->setEnabled(canEdit);
  //
	TeachingEventHandler::instance()->fsd_Loaded(this);
}

void FlowSearchDialog::showGrid(const vector<FlowParamPtr>& flowList) {
	lstFlow->clear();
	lstFlow->setRowCount(0);
	lstFlow->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");

	for (FlowParamPtr param : flowList) {
		int row = lstFlow->rowCount();
		lstFlow->insertRow(row);

		QTableWidgetItem* itemName = new QTableWidgetItem;
		lstFlow->setItem(row, 0, itemName);
		itemName->setText(param->getName());
		itemName->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemComment = new QTableWidgetItem;
		lstFlow->setItem(row, 1, itemComment);
		itemComment->setText(param->getComment());
		itemComment->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemCreated = new QTableWidgetItem;
		lstFlow->setItem(row, 2, itemCreated);
		itemCreated->setText(param->getCreatedDate());
		itemCreated->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemLastUpdated = new QTableWidgetItem;
		lstFlow->setItem(row, 3, itemLastUpdated);
		itemLastUpdated->setText(param->getLastUpdatedDate());
		itemLastUpdated->setData(Qt::UserRole, param->getId());
	}
}

void FlowSearchDialog::searchClicked() {
  DDEBUG("FlowSearchDialog::searchClicked()");
	TeachingEventHandler::instance()->fsd_SeachClicked(leCond->text());
}

void FlowSearchDialog::deleteClicked() {
  DDEBUG("FlowSearchDialog::deleteClicked()");

  QTableWidgetItem* item = lstFlow->currentItem();
  if (item) {
    int targetId = item->data(Qt::UserRole).toInt();
    if (TeachingEventHandler::instance()->fsd_DeleteClicked(targetId)) {
      QMessageBox::information(this, _("Database"), _("Database updated"));
    } else {
      QMessageBox::warning(this, _("Database Error"), TeachingDataHolder::instance()->getErrorStr());
    }

  } else {
    QMessageBox::warning(this, _("Flow List"), _("Select Delete Flow"));
    return;
  }
}

void FlowSearchDialog::oKClicked() {
  DDEBUG("FlowSearchDialog::oKClicked()");
  QTableWidgetItem* item = lstFlow->currentItem();
  if (item) {
    int targetId = item->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->fsd_OKClicked(targetId);
  } else {
    QMessageBox::warning(this, _("Flow List"), _("Select Target Flow"));
    return;
  }
	this->setResult(QDialog::Accepted);
}

void FlowSearchDialog::cancelClicked() {
  DDEBUG("FlowSearchDialog::cancelClicked()");
	this->setResult(QDialog::Rejected);
	close();
}
//////////
NodeDispDialog::NodeDispDialog(FlowParamPtr param, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
    this->targetParam_ = param;

    lstNode_ = UIUtil::makeTableWidget(2, false);
    lstNode_->setColumnWidth(0, 25);
    lstNode_->setColumnWidth(1, 200);
    lstNode_->setHorizontalHeaderLabels(QStringList() << "" << "Node");

    lstModel_ = UIUtil::makeTableWidget(2, false);
    lstModel_->setColumnWidth(0, 25);
    lstModel_->setColumnWidth(1, 200);
    lstModel_->setHorizontalHeaderLabels(QStringList() << "" << "Model");

    lstParam_ = UIUtil::makeTableWidget(2, false);
    lstParam_->setColumnWidth(0, 25);
    lstParam_->setColumnWidth(1, 200);
    lstParam_->setHorizontalHeaderLabels(QStringList() << "" << "Param");

    QFrame* frmGrid = new QFrame;
    QHBoxLayout* gridLayout = new QHBoxLayout(frmGrid);
    gridLayout->setContentsMargins(2, 2, 2, 2);
    gridLayout->addWidget(lstNode_);
    gridLayout->addWidget(lstModel_);
    gridLayout->addWidget(lstParam_);
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
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(frmGrid);
    mainLayout->addWidget(frmButtons);
    setLayout(mainLayout);
    //
    connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

    setWindowTitle(_("Disp"));
    resize(800, 300);
    //
    showNodeList();
    showModelList();
    showParamList();
}

void NodeDispDialog::showNodeList() {
  lstNode_->setRowCount(0);

  for (ElementStmParamPtr node : targetParam_->getActiveStateList()) {
    QtNodes::Node* target = node->getRealElem();
    if (target) {
      int row = lstNode_->rowCount();
      lstNode_->insertRow(row);

      QTableWidgetItem* itemDisp = new QTableWidgetItem;
      lstNode_->setItem(row, 0, itemDisp);
      if (target->isHide()) {
        itemDisp->setCheckState(Qt::Unchecked);
      } else {
        itemDisp->setCheckState(Qt::Checked);
      }
      itemDisp->setData(Qt::UserRole, node->getId());
      itemDisp->setTextAlignment(Qt::AlignCenter);

      QTableWidgetItem* itemName = new QTableWidgetItem;
      lstNode_->setItem(row, 1, itemName);
      itemName->setText(target->nodeDataModel()->caption());
      itemName->setData(Qt::UserRole, node->getId());
    }
  }
}

void NodeDispDialog::showModelList() {
  lstModel_->setRowCount(0);
  vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();

  for (FlowModelParamPtr model : targetParam_->getActiveModelList()) {
    QtNodes::Node* target = model->getRealElem();
    if (target) {
      int row = lstModel_->rowCount();
      lstModel_->insertRow(row);

      QTableWidgetItem* itemDisp = new QTableWidgetItem;
      lstModel_->setItem(row, 0, itemDisp);
      if (target->isHide()) {
        itemDisp->setCheckState(Qt::Unchecked);
      } else {
        itemDisp->setCheckState(Qt::Checked);
      }
      itemDisp->setData(Qt::UserRole, model->getId());
      itemDisp->setTextAlignment(Qt::AlignCenter);

      QTableWidgetItem* itemName = new QTableWidgetItem;
      lstModel_->setItem(row, 1, itemName);
      int masterId = model->getMasterId();
      vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
      if (masterParamItr == modelMasterList.end()) return;
      itemName->setText((*masterParamItr)->getName());
      itemName->setData(Qt::UserRole, model->getId());
    }
  }
}

void NodeDispDialog::showParamList() {
  lstParam_->setRowCount(0);
  for (FlowParameterParamPtr param : targetParam_->getActiveFlowParamList()) {
    QtNodes::Node* target = param->getRealElem();
    if (target) {
      int row = lstParam_->rowCount();
      lstParam_->insertRow(row);

      QTableWidgetItem* itemDisp = new QTableWidgetItem;
      lstParam_->setItem(row, 0, itemDisp);
      if (target->isHide()) {
        itemDisp->setCheckState(Qt::Unchecked);
      } else {
        itemDisp->setCheckState(Qt::Checked);
      }
      itemDisp->setData(Qt::UserRole, param->getId());
      itemDisp->setTextAlignment(Qt::AlignCenter);

      QTableWidgetItem* itemName = new QTableWidgetItem;
      lstParam_->setItem(row, 1, itemName);
      itemName->setText(param->getName());
      itemName->setData(Qt::UserRole, param->getId());
    }
  }
}

void NodeDispDialog::oKClicked() {
  DDEBUG("NodeDispDialog::oKClicked()");

  int nodeNum = lstNode_->rowCount();
  vector<ElementStmParamPtr> nodeList = targetParam_->getActiveStateList();

  for (int index = 0; index < nodeNum; index++) {
    QTableWidgetItem* item = lstNode_->item(index, 0);
    if (item) {
      int targetId = item->data(Qt::UserRole).toInt();
      vector<ElementStmParamPtr>::iterator targetElem = find_if(nodeList.begin(), nodeList.end(), ElementStmParamComparator(targetId));
      if (targetElem == nodeList.end()) continue;
      //
      QtNodes::Node* target = (*targetElem)->getRealElem();
      if (target) {
        Qt::CheckState state = item->checkState();
        if (state == Qt::Checked) {
          target->setHide(false);
        } else {
          target->setHide(true);
        }
      }
    }
  }
  //
  int modelNum = lstModel_->rowCount();
  vector<FlowModelParamPtr> modelList = targetParam_->getActiveModelList();

  for (int index = 0; index < modelNum; index++) {
    QTableWidgetItem* item = lstModel_->item(index, 0);
    if (item) {
      int targetId = item->data(Qt::UserRole).toInt();
      vector<FlowModelParamPtr>::iterator targetElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(targetId));
      if (targetElem == modelList.end()) continue;
      //
      QtNodes::Node* target = (*targetElem)->getRealElem();
      if (target) {
        Qt::CheckState state = item->checkState();
        if (state == Qt::Checked) {
          target->setHide(false);
        } else {
          target->setHide(true);
        }
      }
    }
  }
  //
  int paramNum = lstParam_->rowCount();
  vector<FlowParameterParamPtr> paramList = targetParam_->getFlowParamList();

  for (int index = 0; index < paramNum; index++) {
    QTableWidgetItem* item = lstParam_->item(index, 0);
    if (item) {
      int targetId = item->data(Qt::UserRole).toInt();
      vector<FlowParameterParamPtr>::iterator targetElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(targetId));
      if (targetElem == paramList.end()) continue;
      //
      QtNodes::Node* target = (*targetElem)->getRealElem();
      if (target) {
        Qt::CheckState state = item->checkState();
        if (state == Qt::Checked) {
          target->setHide(false);
        } else {
          target->setHide(true);
        }
      }
    }
  }

  close();
}

void NodeDispDialog::cancelClicked() {
  DDEBUG("NodeDispDialog::cancelClicked()");
  close();
}
//////////
PortDispDialog::PortDispDialog(TaskModelParamPtr param, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetParam_ = param;

  lstModel_ = UIUtil::makeTableWidget(2, false);
  lstModel_->setColumnWidth(0, 25);
  lstModel_->setColumnWidth(1, 200);
  lstModel_->setHorizontalHeaderLabels(QStringList() << "" << "Model Port");

  lstParam_ = UIUtil::makeTableWidget(2, false);
  lstParam_->setColumnWidth(0, 25);
  lstParam_->setColumnWidth(1, 200);
  lstParam_->setHorizontalHeaderLabels(QStringList() << "" << "Param Port");

  QFrame* frmGrid = new QFrame;
  QHBoxLayout* gridLayout = new QHBoxLayout(frmGrid);
  gridLayout->setContentsMargins(2, 2, 2, 2);
  gridLayout->addWidget(lstModel_);
  gridLayout->addWidget(lstParam_);
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
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frmGrid);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Disp"));
  resize(600, 400);
  //
  showModelList();
  showParamList();
}

void PortDispDialog::showModelList() {
  lstModel_->setRowCount(0);
  vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();

  for (ModelParamPtr model : targetParam_->getActiveModelList()) {
    int row = lstModel_->rowCount();
    lstModel_->insertRow(row);

    QTableWidgetItem* itemDisp = new QTableWidgetItem;
    lstModel_->setItem(row, 0, itemDisp);
    if (model->getHide()) {
      itemDisp->setCheckState(Qt::Unchecked);
    } else {
      itemDisp->setCheckState(Qt::Checked);
    }
    itemDisp->setData(Qt::UserRole, model->getId());
    itemDisp->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem* itemName = new QTableWidgetItem;
    lstModel_->setItem(row, 1, itemName);
    itemName->setText(model->getRName());
    itemName->setData(Qt::UserRole, model->getId());
  }
}

void PortDispDialog::showParamList() {
  lstParam_->setRowCount(0);
  for (ParameterParamPtr param : targetParam_->getActiveParameterList()) {
    int row = lstParam_->rowCount();
    lstParam_->insertRow(row);

    QTableWidgetItem* itemDisp = new QTableWidgetItem;
    lstParam_->setItem(row, 0, itemDisp);
    if (param->getHide()) {
      itemDisp->setCheckState(Qt::Unchecked);
    } else {
      itemDisp->setCheckState(Qt::Checked);
    }
    itemDisp->setData(Qt::UserRole, param->getId());
    itemDisp->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem* itemName = new QTableWidgetItem;
    lstParam_->setItem(row, 1, itemName);
    itemName->setText(param->getName());
    itemName->setData(Qt::UserRole, param->getId());
  }
}

void PortDispDialog::oKClicked() {
  int paramNum = lstParam_->rowCount();
  vector<ParameterParamPtr> paramList = targetParam_->getActiveParameterList();

  //
  int modelNum = lstModel_->rowCount();
  vector<ModelParamPtr> modelList = targetParam_->getActiveModelList();

  for (int index = 0; index < modelNum; index++) {
    QTableWidgetItem* item = lstModel_->item(index, 0);
    if (item) {
      int targetId = item->data(Qt::UserRole).toInt();
      vector<ModelParamPtr>::iterator targetElem = find_if(modelList.begin(), modelList.end(), ModelParamComparator(targetId));
      if (targetElem == modelList.end()) continue;
      //
      Qt::CheckState state = item->checkState();
      if (state == Qt::Checked) {
        (*targetElem)->setHide(false);
      } else {
        (*targetElem)->setHide(true);
      }
    }
  }
  //
  for (int index = 0; index < paramNum; index++) {
    QTableWidgetItem* item = lstParam_->item(index, 0);
    if (item) {
      int targetId = item->data(Qt::UserRole).toInt();
      vector<ParameterParamPtr>::iterator targetElem = find_if(paramList.begin(), paramList.end(), ParameterParamComparator(targetId));
      if (targetElem == paramList.end()) continue;
      //
      Qt::CheckState state = item->checkState();
      if (state == Qt::Checked) {
        (*targetElem)->setHide(false);
      } else {
        (*targetElem)->setHide(true);
      }
    }
  }

  close();
}

void PortDispDialog::cancelClicked() {
  DDEBUG("PortDispDialog::cancelClicked()");
  close();
}
//////////
TaskInfoDialog::TaskInfoDialog(ElementStmParamPtr param, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  this->targetParam_ = param;
  //
  txtName = new QLineEdit;

  QFrame* frmButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
  QPushButton* btnCancel = new QPushButton(_("Cancel"));
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addWidget(btnCancel);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(txtName);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(cancelClicked()));

  setWindowTitle(_("Task Instance Name"));
  resize(400, 100);
  //
  txtName->setText(targetParam_->getTaskParam()->getName());
  txtName->setSelection(0, targetParam_->getTaskParam()->getName().length());
}

void TaskInfoDialog::oKClicked() {
  DDEBUG("TaskInfoDialog::oKClicked()");

  QString strName = txtName->text();
  if (strName.trimmed().length() == 0) {
    QMessageBox::warning(this, _("TaskInstance"), _("Please input Task Instance Name."));
    txtName->setFocus();
    txtName->setSelection(0, strName.length());
    return;
  }
	targetParam_->getTaskParam()->setName(strName);
	targetParam_->getTaskParam()->setUpdate();
	targetParam_->getRealElem()->nodeDataModel()->setTaskName(strName);
	targetParam_->getRealElem()->nodeGraphicsObject().update();
  //
  close();
}

void TaskInfoDialog::cancelClicked() {
  DDEBUG("TaskInfoDialog::cancelClicked()");

  close();
}
//////////
FlowViewImpl::FlowViewImpl(QWidget* parent) {
  QFrame* flowFrame = new QFrame;
  QLabel* lblName = new QLabel(_("Flow Name:"));
  leName = new QLineEdit;
  leName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Flow"));

  btnNewFlow = new QPushButton();
  btnNewFlow->setIcon(QIcon(":/Teaching/icons/Create.png"));
  btnNewFlow->setToolTip(_("Create New Flow"));

  btnRegistFlow = new QPushButton();
  btnRegistFlow->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnRegistFlow->setToolTip(_("Regist Current Flow"));

  QLabel* lblComment = new QLabel(_("Comment:"));
  leComment = new QLineEdit;

  QGridLayout* flowLayout = new QGridLayout;
  flowFrame->setLayout(flowLayout);
  flowLayout->addWidget(lblName, 0, 0, 1, 1, Qt::AlignRight);
  flowLayout->addWidget(leName, 0, 1, 1, 1);
  flowLayout->addWidget(btnSearch, 0, 2, 1, 1);
  flowLayout->addWidget(btnNewFlow, 0, 3, 1, 1);
  flowLayout->addWidget(btnRegistFlow, 0, 4, 1, 1);
  flowLayout->addWidget(lblComment, 1, 0, 1, 1, Qt::AlignRight);
  flowLayout->addWidget(leComment, 1, 1, 1, 5);
  //
	setStyle();
	FlowScene* scene = new FlowScene(registerDataModels());
	grhStateMachine = new FlowEditor(scene, this);
	grhStateMachine->setAcceptDrops(true);
	grhStateMachine->setEnabled(false);
	
	setAcceptDrops(true);

  //
  QFrame* frmTask = new QFrame;
  btnHide = new QPushButton(_("Hide"));
  btnHide->setToolTip(_("Hide selected Node"));

  btnDisp = new QPushButton(_("Disp"));
  btnDisp->setToolTip(_("Set display state of node."));

  btnParamDisp = new QPushButton(_("Port Disp"));
  btnParamDisp->setToolTip(_("Set port visibility."));

  btnModelDisp = new QPushButton(_("Model Disp"));
  btnModelDisp->setCheckable(true);
  btnModelDisp->setToolTip(_("Show all models used in flow."));

  btnDeleteTask = new QPushButton(_("Delete"));
  btnDeleteTask->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteTask->setToolTip(_("Delete selected Task"));

  btnEdit = new QPushButton(_("Edit"));
  btnEdit->setIcon(QIcon(":/Teaching/icons/Settings.png"));
  btnEdit->setToolTip(_("Edit target task"));

  btnExport = new QPushButton(_("Export"));
  btnExport->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnExport->setToolTip(_("Export Flow"));

  btnImport = new QPushButton(_("Import"));
  btnImport->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnImport->setToolTip(_("Import Flow"));

  btnRunFlow = new QPushButton(_("Flow"));
  btnRunFlow->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunFlow->setToolTip(_("Run Flow"));

  btnRunTask = new QPushButton(_("Task"));
  btnRunTask->setIcon(QIcon(":/Base/icons/play.png"));
  btnRunTask->setToolTip(_("Run selected Task"));

  btnAbort = new QPushButton(_("Abort"));
  //btnAbort->setIcon(QIcon(":/Teaching/icons/Apply.png"));
  btnAbort->setToolTip(_("Abort Operation"));
  btnAbort->setEnabled(false);

  btnInitPos = new QPushButton();
  btnInitPos->setIcon(QIcon(":/Teaching/icons/Refresh.png"));
  btnInitPos->setToolTip(_("Reset models position"));

  QHBoxLayout* taskLayout = new QHBoxLayout;
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(btnHide);
  taskLayout->addWidget(btnDisp);
  taskLayout->addWidget(btnParamDisp);
  taskLayout->addWidget(btnModelDisp);
  taskLayout->addStretch();
  taskLayout->addWidget(btnDeleteTask);
  taskLayout->addStretch();
  taskLayout->addWidget(btnEdit);
  taskLayout->addStretch();
  taskLayout->addWidget(btnImport);
  taskLayout->addWidget(btnExport);
  taskLayout->addStretch();
  taskLayout->addWidget(btnRunFlow);
  taskLayout->addWidget(btnRunTask);
  taskLayout->addWidget(btnAbort);
  taskLayout->addWidget(btnInitPos);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(flowFrame);
  mainLayout->addWidget(grhStateMachine);
  mainLayout->addWidget(frmTask);

  setLayout(mainLayout);
  //
  changeEnables(false);
  //
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(btnNewFlow, SIGNAL(clicked()), this, SLOT(newFlowClicked()));
  connect(btnRegistFlow, SIGNAL(clicked()), this, SLOT(registFlowClicked()));
  connect(btnDeleteTask, SIGNAL(clicked()), this, SLOT(deleteTaskClicked()));
  connect(btnRunFlow, SIGNAL(clicked()), this, SLOT(runFlowClicked()));
  connect(btnRunTask, SIGNAL(clicked()), this, SLOT(runTaskClicked()));
  connect(btnAbort, SIGNAL(clicked()), this, SLOT(abortClicked()));
  connect(btnInitPos, SIGNAL(clicked()), this, SLOT(initPosClicked()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
  connect(btnExport, SIGNAL(clicked()), this, SLOT(exportFlowClicked()));
  connect(btnImport, SIGNAL(clicked()), this, SLOT(importFlowClicked()));
  connect(btnHide, SIGNAL(clicked()), this, SLOT(hideClicked()));
  connect(btnDisp, SIGNAL(clicked()), this, SLOT(dispClicked()));
  connect(btnParamDisp, SIGNAL(clicked()), this, SLOT(paramDispClicked()));
  connect(btnModelDisp, SIGNAL(clicked()), this, SLOT(modelToggled()));

	TeachingEventHandler::instance()->flv_Loaded(this);
}

FlowViewImpl::~FlowViewImpl() {
  DDEBUG("FlowViewImpl Destruct");
}
//
void FlowViewImpl::setButtonEnableMode(bool isEnable) {
  btnSearch->setEnabled(isEnable);
  btnNewFlow->setEnabled(isEnable);
  btnRegistFlow->setEnabled(isEnable);
  btnDeleteTask->setEnabled(isEnable);
  btnRunFlow->setEnabled(isEnable);
  btnRunTask->setEnabled(isEnable);
  btnInitPos->setEnabled(isEnable);
  btnEdit->setEnabled(isEnable);
  btnImport->setEnabled(isEnable);
  btnExport->setEnabled(isEnable);
  btnHide->setEnabled(isEnable);
  btnDisp->setEnabled(isEnable);
  btnParamDisp->setEnabled(isEnable);

  btnAbort->setEnabled(!isEnable);

  setEditMode(TeachingEventHandler::instance()->canEdit());
}

void FlowViewImpl::dispView(FlowParamPtr& target) {
	changeEnables(true);
  setEditMode(TeachingEventHandler::instance()->canEdit());
	leName->setText(target->getName());
	leComment->setText(target->getComment());
	createStateMachine(target);
}

void FlowViewImpl::createStateMachine(FlowParamPtr& target) {
	grhStateMachine->setTargetParam(target);
	grhStateMachine->createStateMachine(target);
}

void FlowViewImpl::clearView() {
	  leName->setText("");
	  leComment->setText("");
	  grhStateMachine->setTargetParam(0);
    grhStateMachine->removeAll();
	  changeEnables(false);
}
/////
void FlowViewImpl::searchClicked() {
	TeachingEventHandler::instance()->flv_SearchClicked(TeachingEventHandler::instance()->canEdit());
  setEditMode(TeachingEventHandler::instance()->canEdit());
}

void FlowViewImpl::newFlowClicked() {
	TeachingEventHandler::instance()->flv_NewFlowClicked();
  leName->setFocus();
}

void FlowViewImpl::registFlowClicked() {
	TeachingEventHandler::instance()->flv_RegistFlowClicked(leName->text(), leComment->text());
}

void FlowViewImpl::deleteTaskClicked() {
  if (TeachingEventHandler::instance()->checkPaused()) return;

  DDEBUG("FlowViewImpl::deleteTaskClicked()");
  grhStateMachine->deleteSelectedNodes();
	TeachingEventHandler::instance()->flv_DeleteTaskClicked();
}

void FlowViewImpl::hideClicked() {
  DDEBUG("FlowViewImpl::hideClicked()");
  grhStateMachine->hideSelectedNodes();
}

void FlowViewImpl::dispClicked() {
  grhStateMachine->dispSetting();
}

void FlowViewImpl::paramDispClicked() {
  grhStateMachine->portDispSetting();
}

void FlowViewImpl::modelToggled() {
	DDEBUG("FlowViewImpl::modelToggled()");
	TeachingEventHandler::instance()->flv_AllModelDisp(btnModelDisp->isChecked());
}

void FlowViewImpl::editClicked() {
	DDEBUG("FlowViewImpl::editClicked()");

	ElementStmParamPtr target = grhStateMachine->getCurrentNode();
  TeachingEventHandler::instance()->flv_EditClicked(target);
}

void FlowViewImpl::changeEnables(bool value) {
  leName->setEnabled(value);
  leComment->setEnabled(value);
  btnRegistFlow->setEnabled(value);
  //
  grhStateMachine->removeAll();
  grhStateMachine->setEnabled(value);
  //
  btnDeleteTask->setEnabled(value);
  btnRunFlow->setEnabled(value);
  btnRunTask->setEnabled(value);
  btnInitPos->setEnabled(value);
  btnEdit->setEnabled(value);
  btnExport->setEnabled(value);
  btnHide->setEnabled(value);
  btnDisp->setEnabled(value);
  btnParamDisp->setEnabled(value);
}

void FlowViewImpl::exportFlowClicked() {
	TeachingEventHandler::instance()->flv_FlowExportClicked(leName->text(), leComment->text());
}

void FlowViewImpl::importFlowClicked() {
	TeachingEventHandler::instance()->flv_FlowImportClicked();
}

void FlowViewImpl::runFlowClicked() {
	TeachingEventHandler::instance()->flv_RunFlowClicked();
}

void FlowViewImpl::runTaskClicked() {
  QString errMessage;
  if (updateTargetFlowParam(errMessage) == false) {
    QMessageBox::warning(this, _("FlowView"), errMessage);
    return;
  }
	TeachingEventHandler::instance()->tev_RunTaskClicked(NULL_ID);
}

void FlowViewImpl::abortClicked() {
	TeachingEventHandler::instance()->tev_AbortClicked();
}

void FlowViewImpl::initPosClicked() {
	TeachingEventHandler::instance()->flv_InitPosClicked();
}

void FlowViewImpl::flowSelectionChanged(TaskModelParamPtr target) {
	TeachingEventHandler::instance()->flv_SelectionChanged(target);
}

void FlowViewImpl::cancelAllModel() {
  btnModelDisp->setChecked(false);
}

void FlowViewImpl::setStyle() {
	FlowViewStyle::setStyle(
		R"(
  {
    "FlowViewStyle": {
      "BackgroundColor": [255, 255, 240],
      "FineGridColor": [245, 245, 230],
      "CoarseGridColor": [235, 235, 220]
    }
  }
  )");

	NodeStyle::setNodeStyle(
		R"(
  {
    "NodeStyle": {
      "NormalBoundaryColor": "darkgray",
      "SelectedBoundaryColor": "deepskyblue",
      "GradientColor0": "mintcream",
      "GradientColor1": "mintcream",
      "GradientColor2": "mintcream",
      "GradientColor3": "mintcream",
      "ShadowColor": [200, 200, 200],
      "FontColor": [10, 10, 10],
      "FontColorFaded": [100, 100, 100],
      "ConnectionPointColor": "white",
      "PenWidth": 2.0,
      "HoveredPenWidth": 2.5,
      "ConnectionPointDiameter": 10.0,
      "Opacity": 1.0
    }
  }
  )");

	ConnectionStyle::setConnectionStyle(
		R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "black",
      "SelectedColor": "gray",
      "SelectedHaloColor": "deepskyblue",
      "HoveredColor": "deepskyblue",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": false
    }
  }
  )");
}

std::shared_ptr<DataModelRegistry> FlowViewImpl::registerDataModels() {
	auto ret = std::make_shared<DataModelRegistry>();


	ret->registerModel<TaskDataModel>("Tasks");
  ret->registerModel<TransformDataModel>("3D Models");
  ret->registerModel<IntParamDataModel>("Variables");
  ret->registerModel<DoubleParamDataModel>("Variables");
  ret->registerModel<StringParamDataModel>("Variables");
  ret->registerModel<FrameParamDataModel>("Variables");
  ret->registerModel<MergeDataModel>("Syntaxes");
  ret->registerModel<DecisionDataModel>("Syntaxes");
	ret->registerModel<FinalDataModel>("Syntaxes");
	ret->registerModel<InitialDataModel>("Syntaxes");

	return ret;
}

void FlowViewImpl::setEditMode(bool canEdit) {
  btnRunFlow->setEnabled(!canEdit);
  btnRunTask->setEnabled(!canEdit);
  btnInitPos->setEnabled(!canEdit);
  //
  leName->setEnabled(canEdit);
  leComment->setEnabled(canEdit);
  btnNewFlow->setEnabled(canEdit);
  btnRegistFlow->setEnabled(canEdit);

  btnDeleteTask->setEnabled(canEdit);
  btnEdit->setEnabled(canEdit);
  btnExport->setEnabled(canEdit);
  btnImport->setEnabled(canEdit);
  btnHide->setEnabled(canEdit);
  btnDisp->setEnabled(canEdit);
  btnParamDisp->setEnabled(canEdit);
  btnModelDisp->setEnabled(canEdit);

  grhStateMachine->setEditMode(canEdit);
}
/////
FlowView::FlowView() : viewImpl(0) {
  setName(_("FlowModel"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new FlowViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::CENTER);
}

FlowView::~FlowView() {
};

}
