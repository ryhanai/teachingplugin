#include "SettingDialog.h"
#include "TeachingUtil.h"
#include "LoggerUtil.h"

#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace teaching {

SettingDialog::SettingDialog(QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint), isDBUpdated_(false), currentRowIndex_(-1) {

  QFrame* frmBase = new QFrame;
  QGridLayout* baseLayout = new QGridLayout();
  baseLayout->setContentsMargins(2, 0, 2, 0);
  frmBase->setLayout(baseLayout);

  QLabel* lblDatabase = new QLabel(_("Database:"));
  leDatabase = new QLineEdit;
  QPushButton* btnDatabase = new QPushButton();
  btnDatabase->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnDatabase->setToolTip(_("Ref..."));
  //
  QLabel* lblRobotModel = new QLabel(_("Robot Model Name:"));
  leRobotModel = new QLineEdit;
  //
  QLabel* lblLogLevel = new QLabel(_("Log:"));
  cmbLogLevel = new QComboBox(this);
  cmbLogLevel->addItem("No");
  cmbLogLevel->addItem("Error");
  cmbLogLevel->addItem("Debug");
  leLogDir = new QLineEdit;
  QPushButton* btnLogDir = new QPushButton();
  btnLogDir->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnLogDir->setToolTip(_("Ref..."));
  //
  lstApp = UIUtil::makeTableWidget(2, true);
  lstApp->setColumnWidth(0, 100);
  lstApp->setColumnWidth(1, 400);
  lstApp->setHorizontalHeaderLabels(QStringList() << "Ext" << "Application Path");
  //
  QLabel* lblExt = new QLabel(_("Ext:"));
  leExt = new QLineEdit;
  //
  QLabel* lblApp = new QLabel(_("Application:"));
  leApp = new QLineEdit;
  //QPushButton* btnRef = new QPushButton(tr("Ref..."));
  QPushButton* btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(_("Ref..."));
  chkReal = new QCheckBox(_("Real"));
  chkReal->setChecked(SettingManager::getInstance().getIsReal());

  baseLayout->addWidget(lblDatabase, 0, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leDatabase, 0, 1, 1, 2);
  baseLayout->addWidget(btnDatabase, 0, 3, 1, 1);

  baseLayout->addWidget(lblRobotModel, 1, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leRobotModel, 1, 1, 1, 3);

  baseLayout->addWidget(lblLogLevel, 2, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(cmbLogLevel, 2, 1, 1, 1);
  baseLayout->addWidget(leLogDir, 2, 2, 1, 1);
  baseLayout->addWidget(btnLogDir, 2, 3, 1, 1);

  baseLayout->addWidget(lstApp, 3, 0, 1, 4);

  baseLayout->addWidget(lblExt, 4, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leExt, 4, 1, 1, 3);

  baseLayout->addWidget(lblApp, 5, 0, 1, 1, Qt::AlignRight);
  baseLayout->addWidget(leApp, 5, 1, 1, 2);
  baseLayout->addWidget(btnRef, 5, 3, 1, 1);

  baseLayout->addWidget(chkReal, 6, 1, 1, 1);
  //
  QFrame* frmButtons = new QFrame;
	QPushButton* btnOK = new QPushButton(_("OK"));
  QPushButton* btnCancel = new QPushButton(_("Cancel"));
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
  cmbLogLevel->setCurrentIndex(SettingManager::getInstance().getLogLevel());
  string strLogDir = SettingManager::getInstance().getLogDir();
  leLogDir->setText(QString::fromStdString(strLogDir));
  //
  connect(btnDatabase, SIGNAL(clicked()), this, SLOT(refDBClicked()));
  connect(lstApp, SIGNAL(itemSelectionChanged()), this, SLOT(appSelectionChanged()));
  connect(btnRef, SIGNAL(clicked()), this, SLOT(refAppClicked()));
  connect(btnLogDir, SIGNAL(clicked()), this, SLOT(refLogClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));

  showAppList();

  setWindowTitle(_("Setting"));
  resize(600, 500);
}

void SettingDialog::showAppList() {
  appList_.clear();
  std::vector<std::string> keyList = SettingManager::getInstance().getExtList();
  for (int index = 0; index < keyList.size(); index++) {
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
  DDEBUG("SettingDialog::refDBClicked()");

  QString strFName = QFileDialog::getOpenFileName(
    this, "Database File", ".",
    "sqlite(*.sqlite3);;all(*.*)");
  if (!strFName.isEmpty()) {
    leDatabase->setText(strFName);
  }
}

void SettingDialog::refLogClicked() {
  DDEBUG("SettingDialog::refLogClicked()");

  QString strFName = QFileDialog::getExistingDirectory(this, "Log Output Directory");
  if (!strFName.isEmpty()) {
    leLogDir->setText(strFName);
  }
}

void SettingDialog::appSelectionChanged() {
  DDEBUG("SettingDialog::appSelectionChanged()");

  if (0 <= currentRowIndex_) {
    QString strExt = leExt->text();
    QString strApp = leApp->text();
    appList_[currentRowIndex_].ext_ = strExt;
    appList_[currentRowIndex_].appPath_ = strApp;
    lstApp->item(currentRowIndex_, 0)->setText(strExt);
    lstApp->item(currentRowIndex_, 1)->setText(strApp);
  }
  //
  QTableWidgetItem* item = lstApp->currentItem();
  if (item) {
    currentRowIndex_ = lstApp->currentRow();
    int selected = item->data(Qt::UserRole).toInt();
    leExt->setText(appList_[selected].ext_);
    leApp->setText(appList_[selected].appPath_);
  }
}

void SettingDialog::refAppClicked() {
  DDEBUG("SettingDialog::refAppClicked()");

  QString strFName = QFileDialog::getOpenFileName(
    this, "Target Application", leApp->text(), "all(*)");
  if (!strFName.isEmpty()) {
    leApp->setText(strFName);
  }
}

void SettingDialog::oKClicked() {
  DDEBUG("SettingDialog::oKClicked()");

  if (0 <= currentRowIndex_) {
    QString strExt = leExt->text();
    QString strApp = leApp->text();
    appList_[currentRowIndex_].ext_ = strExt;
    appList_[currentRowIndex_].appPath_ = strApp;
  }

  QString strDatabase = leDatabase->text();
  if (strDatabase.size() == 0) {
    QMessageBox::warning(this, _("Setting"), _("Please input Database File."));
    leDatabase->setFocus();
    return;
  }
  QString strRobotModelName = leRobotModel->text();
  if (strRobotModelName.size() == 0) {
    QMessageBox::warning(this, _("Setting"), _("Please input Robot Model Name."));
    leRobotModel->setFocus();
    return;
  }
  for (AppExtParam param : appList_) {
    if (param.ext_.size() == 0) {
      QMessageBox::warning(this, _("Setting"), _("Please input Target EXT."));
      return;
    }
    if (param.appPath_.size() == 0) {
      QMessageBox::warning(this, _("Setting"), _("Please input APPLICATION."));
      return;
    }
  }
  //
  bool updatedLog = false;
  int orgLogLevel = SettingManager::getInstance().getLogLevel();
  int logLevel = cmbLogLevel->currentIndex();
  if (orgLogLevel != logLevel) {
    SettingManager::getInstance().setLogLevel(logLevel);
    updatedLog = true;
  }
  //
  string orgLogDir = SettingManager::getInstance().getLogDir();
  QString strLogDir = leLogDir->text();
  if (orgLogDir != string(strLogDir.toUtf8().constData())) {
    SettingManager::getInstance().setLogDir(strLogDir.toUtf8().constData());
    updatedLog = true;
  }
  //
  string orgDB = SettingManager::getInstance().getDatabase();
  if (orgDB != string(strDatabase.toUtf8().constData())) {
    SettingManager::getInstance().setDatabase(strDatabase.toUtf8().constData());
    isDBUpdated_ = true;
  }
  //
  SettingManager::getInstance().clearExtList();
  for (AppExtParam param : appList_) {
    SettingManager::getInstance().setTargetApp(param.ext_.toStdString(), param.appPath_.toStdString());
  }
  SettingManager::getInstance().setIsReal(chkReal->isChecked());

  SettingManager::getInstance().saveSetting();
  if (updatedLog) {
    LoggerUtil::startLog((LogLevel)SettingManager::getInstance().getLogLevel(), SettingManager::getInstance().getLogDir());
  }

  close();
}

void SettingDialog::cancelClicked() {
  DDEBUG("SettingDialog::cancelClicked()");

  close();
}
/////
SearchList::SearchList(int rows, int cols, QWidget* parent)
  : QTableWidget(rows, cols, parent) {
}

void SearchList::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    startPos = event->pos();
  }
  QTableWidget::mousePressEvent(event);
}

void SearchList::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::LeftButton) {
    int distance = (event->pos() - startPos).manhattanLength();
    if (QApplication::startDragDistance() <= distance) {
      QModelIndexList indexes = selectionModel()->selection().indexes();
      QModelIndex selected = indexes.at(0);
      if (0 <= selected.row()) {
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
