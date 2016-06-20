#include "SettingDialog.h"
#include "TeachingUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

SettingDialog::SettingDialog(QWidget* parent) 
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint), isDBUpdated_(false), currentRowIndex_(-1) {

  QFrame* frmBase = new QFrame;
  QGridLayout* baseLayout = new QGridLayout();
  baseLayout->setContentsMargins(2, 0, 2, 0);
  frmBase->setLayout(baseLayout);

  QLabel* lblDatabase = new QLabel(tr("Database:"));
  leDatabase = new QLineEdit;
  //QPushButton* btnDatabase = new QPushButton(tr("Ref..."));
  QPushButton* btnDatabase = new QPushButton();
  btnDatabase->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnDatabase->setToolTip(tr("Ref..."));
  //
  QLabel* lblRobotModel = new QLabel(tr("Robot Model Name:"));
  leRobotModel = new QLineEdit;
  //
  lstApp = UIUtil::makeTableWidget(2, true);
  lstApp->setColumnWidth(0, 100);
  lstApp->setColumnWidth(1, 400);
  lstApp->setHorizontalHeaderLabels(QStringList() << "Ext" << "Application Path");
  //
  QLabel* lblExt = new QLabel(tr("Ext:"));
  leExt = new QLineEdit;
  //
  QLabel* lblApp = new QLabel(tr("Application:"));
  leApp = new QLineEdit;
  //QPushButton* btnRef = new QPushButton(tr("Ref..."));
  QPushButton* btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(tr("Ref..."));

  baseLayout->addWidget(lblDatabase, 0, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leDatabase, 0, 1, 1, 1);
  baseLayout->addWidget(btnDatabase, 0, 2, 1, 1);
  baseLayout->addWidget(lblRobotModel, 1, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leRobotModel, 1, 1, 1, 2);
  baseLayout->addWidget(lstApp, 2, 0, 1, 3);
  baseLayout->addWidget(lblExt, 3, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leExt, 3, 1, 1, 1);
  baseLayout->addWidget(lblApp, 4, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leApp, 4, 1, 1, 1);
  baseLayout->addWidget(btnRef, 4, 2, 1, 1);
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(tr("OK"));
  QPushButton* btnCancel = new QPushButton(tr("Cancel"));
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addWidget(btnOK);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnCancel);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frmBase);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  string strDB = SettingManager::getInstance().getDatabase();
  leDatabase->setText(QString::fromStdString(strDB));
  string strModelName = SettingManager::getInstance().getRobotModelName();
  leRobotModel->setText(QString::fromStdString(strModelName));
  //
  connect(btnDatabase, SIGNAL(clicked()), this, SLOT(refDBClicked()));
  connect(lstApp, SIGNAL(itemSelectionChanged()), this, SLOT(appSelectionChanged()));
  connect(btnRef, SIGNAL(clicked()), this, SLOT(refAppClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));

  showAppList();

  setWindowTitle(tr("Setting"));
  setFixedHeight(sizeHint().height());
  setFixedWidth(600);
}

void SettingDialog::showAppList() {
  appList_.clear();
  std::vector<std::string> keyList = SettingManager::getInstance().getExtList();
  for(int index=0; index<keyList.size(); index++) {
    string strKey = keyList[index];
    string strApp = SettingManager::getInstance().getTargetApp(strKey);

    int row = lstApp->rowCount();
    lstApp->insertRow(row);
    UIUtil::makeTableItemWithData(lstApp, row, 0, QString::fromStdString(strKey), index);
    UIUtil::makeTableItemWithData(lstApp, row, 1, QString::fromStdString(strApp), index);

    AppExtParam param;
    param.ext_ = QString::fromStdString(strKey);
    param.appPath_ = QString::fromStdString(strApp);
    appList_.push_back(param);
  }
}

void SettingDialog::refDBClicked() {
	QString strFName = QFileDialog::getOpenFileName(
			this, tr( "Database File" ), ".",
			tr( "sqlite(*.sqlite3);;all(*.*)" ));
	if ( !strFName.isEmpty() ){
    leDatabase->setText(strFName);
	}
}

void SettingDialog::appSelectionChanged() {
  if(0<=currentRowIndex_) {
    QString strExt = leExt->text();
    QString strApp = leApp->text();
    appList_[currentRowIndex_].ext_ = strExt;
    appList_[currentRowIndex_].appPath_ = strApp;
    lstApp->item(currentRowIndex_, 0)->setText(strExt);
    lstApp->item(currentRowIndex_, 1)->setText(strApp);
  }
  //
  QTableWidgetItem* item = lstApp->currentItem();
  if(item) {
    currentRowIndex_ = lstApp->currentRow();
    int selected = item->data(Qt::UserRole).toInt();
    leExt->setText(appList_[selected].ext_);
    leApp->setText(appList_[selected].appPath_);
  }
}

void SettingDialog::refAppClicked() {
	QString strFName = QFileDialog::getOpenFileName(
			this, tr( "Target Application" ), leApp->text(),
			tr( "all(*)" ) );
	if ( !strFName.isEmpty() ){
    leApp->setText(strFName);
	}
}

void SettingDialog::oKClicked() {
  if(0<=currentRowIndex_) {
    QString strExt = leExt->text();
    QString strApp = leApp->text();
    appList_[currentRowIndex_].ext_ = strExt;
    appList_[currentRowIndex_].appPath_ = strApp;
  }

  QString strDatabase = leDatabase->text();
  if( strDatabase.size()==0 ) {
    QMessageBox::warning(this, tr("Setting"), tr("Please input Database File."));
    leDatabase->setFocus();
    return;
  }
  QString strRobotModelName = leRobotModel->text();
  if( strRobotModelName.size()==0 ) {
    QMessageBox::warning(this, tr("Setting"), tr("Please input Robot Model Name."));
    leRobotModel->setFocus();
    return;
  }
  for(int index=0; index< appList_.size(); index++) {
    AppExtParam param = appList_[index];
    if( param.ext_.size()==0 ) {
      QMessageBox::warning(this, tr("Setting"), tr("Please input Target EXT."));
      return;
    }
    if( param.appPath_.size()==0 ) {
      QMessageBox::warning(this, tr("Setting"), tr("Please input APPLICATION Path."));
      return;
    }
  }
  //
  string orgDB = SettingManager::getInstance().getDatabase();
  if( orgDB != string(strDatabase.toUtf8().constData()) ) {
    SettingManager::getInstance().setDatabase(strDatabase.toUtf8().constData());
    isDBUpdated_ = true;
  }
  //
  SettingManager::getInstance().clearExtList();
  for(int index=0; index< appList_.size(); index++) {
    AppExtParam param = appList_[index];
    SettingManager::getInstance().setTargetApp(param.ext_.toStdString(), param.appPath_.toStdString());
  }
  SettingManager::getInstance().saveSetting();

  close();
}

void SettingDialog::cancelClicked() {
  close();
}
/////
SearchList::SearchList(int rows, int cols, QWidget* parent)
  : QTableWidget(rows, cols, parent) {
}

void SearchList::mousePressEvent(QMouseEvent* event) {
  if( event->button() == Qt::LeftButton) {
    startPos = event->pos();
  }
  QTableWidget::mousePressEvent(event);
}

void SearchList::mouseMoveEvent(QMouseEvent* event) {
  if( event->buttons() == Qt::LeftButton) {
    int distance = (event->pos() - startPos).manhattanLength();
    if( QApplication::startDragDistance() <= distance ) {
      QModelIndexList indexes = selectionModel()->selection().indexes();
      QModelIndex selected = indexes.at(0);
      if( 0<=selected.row() ) {
        QTableWidgetItem* targetItem = item(selected.row(), 0);
        QByteArray itemData;
        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/TaskInstanceItem", itemData);
        mimeData->setText(item(selected.row(), 0)->text());
        mimeData->setProperty("TaskInstanceItemId", item(selected.row(), 0)->data(Qt::UserRole));
        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
      }
    }
  }
  QTableWidget::mouseMoveEvent(event);
}

}
