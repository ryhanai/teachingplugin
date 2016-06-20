#include "FlowSearchDialog.h"
#include <QtGui>

namespace teaching {

FlowSearchDialog::FlowSearchDialog(QWidget* parent) 
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(tr("Condition:"));
  leCond = new QLineEdit;
  //QPushButton* btnSearch = new QPushButton(tr("Search"));
  QPushButton* btnSearch = new QPushButton();
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(tr("Search Flow"));

  QHBoxLayout* topLayout = new QHBoxLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond);
  topLayout->addWidget(leCond);
  topLayout->addWidget(btnSearch);
  //
  lstFlow = new QListWidget;
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnSelect = new QPushButton(tr("Select"));
  QPushButton* btnDelete = new QPushButton(tr("Delete Flow"));

  QPushButton* btnCancel = new QPushButton(tr("Cancel"));
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
  connect(lstFlow, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(oKClicked()));
  //
  setWindowTitle(tr("Flow List"));
  setFixedHeight(sizeHint().height());
  setFixedWidth(600);
  //
  vector<FlowParam*> flowList = DatabaseManager::getInstance().getFlowList();
  std::vector<FlowParam*>::iterator itFlow = flowList.begin();
  while (itFlow != flowList.end() ) {
    QListWidgetItem* item = new QListWidgetItem(lstFlow);
    item->setData(Qt::UserRole, (*itFlow)->getId());
    item->setText((*itFlow)->getName());
    ++itFlow;
  }
}

void FlowSearchDialog::searchClicked() {
  lstFlow->clear();

  vector<string> condList;
  QStringList targetList;
  bool isOr = false;
  QString strTarget = leCond->text();
  if( strTarget.contains("||") ) {
    isOr = true;
    targetList = strTarget.split("||");
  } else {
    targetList = strTarget.split(" ");
  }
  for(unsigned int index=0; index<targetList.size(); index++) {
    condList.push_back(targetList[index].trimmed().toStdString());
  }

  vector<FlowParam*> flowList = DatabaseManager::getInstance().searchFlowList(condList, isOr);
  std::vector<FlowParam*>::iterator itFlow = flowList.begin();
  while (itFlow != flowList.end() ) {
    QListWidgetItem* item = new QListWidgetItem(lstFlow);
    item->setData(Qt::UserRole, (*itFlow)->getId());
    item->setText((*itFlow)->getName());
    ++itFlow;
  }
}

void FlowSearchDialog::deleteClicked() {
  QListWidgetItem* item = lstFlow->currentItem();
  if(item) {
    int targetId = item->data(Qt::UserRole).toInt();
    if(DatabaseManager::getInstance().deleteFlowModel(targetId) ) {
      QMessageBox::information(this, tr("Database"), "Database updated");
    } else {
      QMessageBox::warning(this, tr("Database Error"), DatabaseManager::getInstance().getErrorStr());
    }

  } else {
    QMessageBox::warning(this, tr("Flow List"), "Select Target Flow");
    return;
  }
  //
  searchClicked();
}

void FlowSearchDialog::oKClicked() {
  QListWidgetItem* item = lstFlow->currentItem();
  if(item) {
    selected_ = item->data(Qt::UserRole).toInt();
  } else {
    QMessageBox::warning(this, tr("Flow List"), "Select Target Flow");
    return;
  }
  isOk_ = true;
  close();
}

void FlowSearchDialog::cancelClicked() {
  isOk_ = false;
  close();
}

}
