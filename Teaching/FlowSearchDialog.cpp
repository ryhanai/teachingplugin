#include "FlowSearchDialog.h"

namespace teaching {

FlowSearchDialog::FlowSearchDialog(QWidget* parent) 
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(tr("Condition:"));
  leCond = new QLineEdit;
  QPushButton* btnSearch = new QPushButton(tr("Search"));
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(tr("Search Flow"));

  QHBoxLayout* topLayout = new QHBoxLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond);
  topLayout->addWidget(leCond);
  topLayout->addWidget(btnSearch);
  //
  lstFlow = new QTableWidget(0,4);
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
  QPushButton* btnSelect = new QPushButton(tr("Open"));
  btnSelect->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnSelect->setToolTip(tr("Open selected Flow"));

  QPushButton* btnDelete = new QPushButton(tr("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(tr("Delete selected Flow"));

  QPushButton* btnCancel = new QPushButton(tr("Cancel"));
  btnCancel->setIcon(QIcon(":/Teaching/icons/Cancel.png"));
  btnCancel->setToolTip(tr("Cancel Flow select"));

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
  vector<string> condList;
  flowList_ = DatabaseManager::getInstance().searchFlowList(condList, false);
  showGrid();
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

  flowList_ = DatabaseManager::getInstance().searchFlowList(condList, isOr);
  showGrid();
}

void FlowSearchDialog::deleteClicked() {
  QTableWidgetItem* item = lstFlow->currentItem();
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
  QTableWidgetItem* item = lstFlow->currentItem();
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

void FlowSearchDialog::showGrid() {
  lstFlow->clear();
  lstFlow->setRowCount(0);
  lstFlow->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");

  for(int index=0; index<flowList_.size(); index++) {
    FlowParam* param = flowList_[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstFlow->rowCount();
    lstFlow->insertRow(row);

    QTableWidgetItem* itemName = new QTableWidgetItem;
    lstFlow->setItem(row, 0, itemName);
    itemName->setData(Qt::UserRole, param->getId());
    itemName->setText(param->getName());

    QTableWidgetItem* itemComment = new QTableWidgetItem;
    lstFlow->setItem(row, 1, itemComment);
    itemComment->setData(Qt::UserRole, 1);
    itemComment->setText(param->getComment());

    QTableWidgetItem* itemCreated = new QTableWidgetItem;
    lstFlow->setItem(row, 2, itemCreated);
    itemCreated->setData(Qt::UserRole, 1);
    itemCreated->setText(param->getCreatedDate());

    QTableWidgetItem* itemLastUpdated = new QTableWidgetItem;
    lstFlow->setItem(row, 3, itemLastUpdated);
    itemLastUpdated->setData(Qt::UserRole, 1);
    itemLastUpdated->setText(param->getLastUpdatedDate());
  }
}

}
