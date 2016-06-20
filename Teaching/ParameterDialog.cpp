#include "ParameterDialog.h"
#include "DataBaseManager.h"
#include "TeachingUtil.h"

namespace teaching {

ParameterDialog::ParameterDialog(TaskModelParam* param, QWidget* parent) 
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint), currentParam_(0), currentRowIndex_(-1) {
  this->targetTask_ = param;
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

  //QPushButton* btnAddParam = new QPushButton(tr("Add"));
  QPushButton* btnAddParam = new QPushButton();
  btnAddParam->setIcon(QIcon(":/Base/icons/projectsave.png"));
  btnAddParam->setToolTip(tr("Add Parameter"));

  //QPushButton* btnDeleteParam = new QPushButton(tr("Delete"));
  QPushButton* btnDeleteParam = new QPushButton();
  btnDeleteParam->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton));
  btnDeleteParam->setToolTip(tr("Delete Parameter"));

  QFrame* frmParamButtons = new QFrame;
  QHBoxLayout* buttonParamLayout = new QHBoxLayout;
  buttonParamLayout->setContentsMargins(0, 0, 0, 0);
  frmParamButtons->setLayout(buttonParamLayout);
  buttonParamLayout->addWidget(btnAddParam);
  buttonParamLayout->addStretch();
  buttonParamLayout->addWidget(btnDeleteParam);

  QLabel* lblName = new QLabel(tr("Name:"));
  leName = new QLineEdit;
  QLabel* lblId = new QLabel(tr("Id:"));
  leId = new QLineEdit;
  QLabel* lblType = new QLabel(tr("Type:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Normal");
  cmbType->addItem("Model");
  QLabel* lblModel = new QLabel(tr("Model Name:"));
  leModelName = new QLineEdit;
  QLabel* lblUnit = new QLabel(tr("Unit:"));
  leUnit = new QLineEdit;
  QLabel* lblNum = new QLabel(tr("Num:"));
  leNum = new QLineEdit;
  QLabel* lblElemType = new QLabel(tr("Elem Type:"));
  leElemType = new QLineEdit;
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
  paramLayout->addWidget(leElemType, 8, 1, 1, 1);
  //
  QFrame* frmMain = new QFrame;
  QHBoxLayout* formLayout = new QHBoxLayout;
  formLayout->setContentsMargins(0, 0, 0, 0);
  frmMain->setLayout(formLayout);
  formLayout->addWidget(lstModel);
  formLayout->addWidget(frmParam);
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(tr("OK"));
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);
  //
  QLabel* lblTaskName = new QLabel("Task Name: " + targetTask_->getName());

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(lblTaskName);
  mainLayout->addWidget(frmMain);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(lstParam, SIGNAL(itemSelectionChanged()), this, SLOT(paramSelectionChanged()));
  connect(cmbType , SIGNAL(currentIndexChanged(int)),this,SLOT(typeSelectionChanged(int)));
  connect(btnAddParam, SIGNAL(clicked()), this, SLOT(addParamClicked()));
  connect(btnDeleteParam, SIGNAL(clicked()), this, SLOT(deleteParamClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));

  setWindowTitle(tr("Task Parameter"));
  setFixedHeight(sizeHint().height());
  setFixedWidth(800);
  //
  showModelInfo();
  showParamInfo();
}

void ParameterDialog::addParamClicked() {
  saveCurrent();
  //
  ParameterParam* newParam = new ParameterParam(-1, 0, "", 1, "", targetTask_->getId(), "New Param", "", "");
  newParam->setNew();
  targetTask_->addParameter(newParam);
  //
  int row = lstParam->rowCount();
  lstParam->insertRow(row);
  int index = targetTask_->getParameterList().size() - 1;
  UIUtil::makeTableItemWithData(lstParam, row, 0, newParam->getName(), index);
  UIUtil::makeTableItemWithData(lstParam, row, 1, getTypeName(newParam->getType()), index);
  UIUtil::makeTableItemWithData(lstParam, row, 2, QString::fromAscii(""), index);
  UIUtil::makeTableItemWithData(lstParam, row, 3, newParam->getUnit(), index);
  UIUtil::makeTableItemWithData(lstParam, row, 4, QString::number(newParam->getElemNum()), index);
}

void ParameterDialog::deleteParamClicked() {
  if(currentParam_) {
    currentParam_->setDelete();
    leName->setText("");
    leId->setText("");
    cmbType->setCurrentIndex(0);
    leModelName->setText("");
    leUnit->setText("");
    leNum->setText("");
    leElemType->setText("");
    currentParam_ = 0;
    currentRowIndex_ = -1;

    int currRow = lstParam->currentRow();
    lstParam->removeRow(currRow);
    lstParam->setFocus();
  }
}

void ParameterDialog::saveCurrent() {
  if(currentParam_) {
    QString strName = leName->text();
    if( currentParam_->getName() != strName) {
      currentParam_->setName(strName);
    }
    //
    QString strId = leId->text();
    if( currentParam_->getRName() != strId) {
      currentParam_->setRName(strId);
    }
    //
    int type = cmbType->currentIndex();
    if( currentParam_->getType() != type) {
      currentParam_->setType(type);
    }
    //
    if(type==0) {
      QString strUnit = leUnit->text();
      if( currentParam_->getUnit() != strUnit) {
        currentParam_->setUnit(strUnit);
      }
      //
      QString strNum = leNum->text();
      if( currentParam_->getElemNum() != strNum.toInt()) {
        currentParam_->setElemNum(strNum.toInt());
      }
      //
      QString strElemTypes = leElemType->text();
      if( currentParam_->getElemTypes() != strElemTypes) {
        currentParam_->setElemTypes(strElemTypes);
      }
      //
      if( currentParam_->getModelName().length() != 0) {
        currentParam_->setModelName("");
      }

    } else {
      QString strModel = leModelName->text();
      if( currentParam_->getModelName() != strModel) {
        currentParam_->setModelName(strModel);
      }
      //
      if( currentParam_->getUnit().length() != 0) {
        currentParam_->setUnit("");
      }
      if( currentParam_->getElemNum() != 6) {
        currentParam_->setElemNum(6);
      }
      if( currentParam_->getElemTypes().length() != 0) {
        currentParam_->setElemTypes("");
      }
    }
  }
}

void ParameterDialog::paramSelectionChanged() {
  saveCurrent();
  if(currentParam_) {
    lstParam->item(currentRowIndex_, 0)->setText(currentParam_->getName());
    lstParam->item(currentRowIndex_, 1)->setText(getTypeName(currentParam_->getType()));
    if(currentParam_->getType()==0) {
      lstParam->item(currentRowIndex_, 2)->setText("");
      lstParam->item(currentRowIndex_, 3)->setText(currentParam_->getUnit());
      lstParam->item(currentRowIndex_, 4)->setText(QString::number(currentParam_->getElemNum()));
    } else {
      lstParam->item(currentRowIndex_, 2)->setText(currentParam_->getModelName());
      lstParam->item(currentRowIndex_, 3)->setText("");
      lstParam->item(currentRowIndex_, 4)->setText("");
    }
  }
  //
  QTableWidgetItem* item = lstParam->currentItem();
  if(item) {
    currentRowIndex_ = lstParam->currentRow();
    int selected = item->data(Qt::UserRole).toInt();
    currentParam_ = targetTask_->getParameterList()[selected];
    leName->setText(currentParam_->getName());
    leId->setText(currentParam_->getRName());
    cmbType->setCurrentIndex(currentParam_->getType());
    typeSelectionChanged(currentParam_->getType());
    if(currentParam_->getType()==0) {
      leModelName->setText("");
      leUnit->setText(currentParam_->getUnit());
      leNum->setText(QString::number(currentParam_->getElemNum()));
      leElemType->setText(currentParam_->getElemTypes());
    } else {
      leModelName->setText(currentParam_->getModelName());
      leUnit->setText("");
      leNum->setText("");
      leElemType->setText("");
    }
  }
}

void ParameterDialog::typeSelectionChanged(int index) {
  if(index==0) {
    leModelName->setEnabled(false);
    leUnit->setEnabled(true);
    leNum->setEnabled(true);
    leElemType->setEnabled(true);
  } else {
    leModelName->setEnabled(true);
    leUnit->setEnabled(false);
    leNum->setEnabled(false);
    leElemType->setEnabled(false);
  }
}

void ParameterDialog::showParamInfo() {
  for(int index=0; index<targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstParam->rowCount();
    lstParam->insertRow(row);

    UIUtil::makeTableItemWithData(lstParam, row, 0, param->getName(), index);
    UIUtil::makeTableItemWithData(lstParam, row, 1, getTypeName(param->getType()), index);

    QTableWidgetItem* itemModel = new QTableWidgetItem;
    lstParam->setItem(row, 2, itemModel);
    itemModel->setData(Qt::UserRole, 1);
    if(param->getType()==TASK_PARAM_MODEL) {
      itemModel->setText(param->getModelName());
    }
    itemModel->setData(Qt::UserRole, index);

    QTableWidgetItem* itemUnit = new QTableWidgetItem;
    lstParam->setItem(row, 3, itemUnit);
    itemUnit->setData(Qt::UserRole, 1);
    if(param->getType()==0) {
      itemUnit->setText(param->getUnit());
    }
    itemUnit->setData(Qt::UserRole, index);

    QTableWidgetItem* itemNum = new QTableWidgetItem;
    lstParam->setItem(row, 4, itemNum);
    itemNum->setData(Qt::UserRole, 1);
    if(param->getType()==0) {
      itemNum->setText(QString::number(param->getElemNum()));
    }
    itemNum->setData(Qt::UserRole, index);
  }
}

QString ParameterDialog::getTypeName(int source) {
  QString result = "";

  switch(source) {
    case 0:
      result = "Normal";
      break;
    case 1:
      result = "Model";
      break;
  }
  return result;
}

void ParameterDialog::showModelInfo() {
  for(int index=0; index<targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItem(lstModel, row, 0, param->getName());
    UIUtil::makeTableItem(lstModel, row, 1, param->getRName());
  }
}

void ParameterDialog::oKClicked() {
  saveCurrent();

  vector<QString> existModels;
  for(int index=0; index<targetTask_->getParameterList().size(); index++) {
    ParameterParam* param = targetTask_->getParameterList()[index];
    if(param->getName().size()==0) {
      QMessageBox::warning(this, tr("Parameter"), tr("Please input Parameter Name."));
      return;
    }
    if(param->getRName().size()==0) {
      QMessageBox::warning(this, tr("Parameter"), tr("Please input Parameter Id."));
      return;
    }
    //
    int type = param->getType();
    if(type==0) {
      if(param->getElemNum()<=0) {
        QMessageBox::warning(this, tr("Parameter"), tr("Please input Element Num."));
        return;
      }

    } else {
      if(param->getModelName().size()==0) {
        QMessageBox::warning(this, tr("Parameter"), tr("Please input Target Model."));
        return;
      }
      bool isExist = false;
      for(int idxModel=0; idxModel<targetTask_->getModelList().size(); idxModel++) {
        ModelParam* model = targetTask_->getModelList()[idxModel];
        if( model->getMode()==DB_MODE_DELETE || model->getMode()==DB_MODE_IGNORE) continue;
        if(model->getRName()==param->getModelName()) {
          isExist = true;
          break;
        }
      }
      if(isExist==false) {
        QMessageBox::warning(this, tr("Parameter"), tr("Target Model is NOT Exist."));
        return;
      }
      //
      if(std::find(existModels.begin(), existModels.end(), param->getModelName()) != existModels.end()) {
        QMessageBox::warning(this, tr("Parameter"), tr("Target Model CANNOT duplicate."));
        return;
      }
      existModels.push_back(param->getModelName());
    }
  }
  //
  if(DatabaseManager::getInstance().saveTaskParameter(targetTask_)==false ) {
    QMessageBox::warning(this, tr("Save Task Parameter Error"), DatabaseManager::getInstance().getErrorStr());
    return;
  }
  targetTask_->clearParameterList();
  vector<ParameterParam*> paramList = DatabaseManager::getInstance().getParameterParams(targetTask_->getTaskId(), targetTask_->getId());
  for(int index=0; index<paramList.size(); index++) {
    ParameterParam* param = paramList[index];
    targetTask_->addParameter(param);
  }
  
  close();
}

}
