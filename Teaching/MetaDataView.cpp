#include "MetaDataView.h"
#include <cnoid/UTF8>
#include <cnoid/BodyBar>
#include <cnoid/EigenUtil>
#include "DataBaseManager.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include <boost/bind.hpp>
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

ImageView::ImageView(QWidget *parent) : QGraphicsView(parent) {
}

ImageView::~ImageView(void) {
}

void ImageView::paintEvent(QPaintEvent* event ) {
  QPainter widgetpainter( viewport() );

  QImage qimg = m_img.scaled(viewport()->width(), viewport()->height(),
                              Qt::KeepAspectRatio,Qt::FastTransformation);
  widgetpainter.drawImage( 0, 0, qimg );
}

void ImageView::setImg(QImage& img ) {
  m_img = QImage( img );
  viewport()->update();
}
//
FigureDialog::FigureDialog(QImage& source, QWidget* parent) : QDialog(parent) {
  ImageView* imageView = new ImageView(parent);
  imageView->setImg(source);

  QPushButton* btnOK = new QPushButton(tr("OK"));
  QFrame* frmButtons = new QFrame;
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->addWidget(imageView);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);

  connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));

  resize(800, 600);
}
//
TextDialog::TextDialog(QString& source, QWidget* parent) : QDialog(parent) {
  QTextEdit* txtView = new QTextEdit(parent);
  txtView->setText(source);
  txtView->setReadOnly(true);

  QPushButton* btnOK = new QPushButton(tr("OK"));
  QFrame* frmButtons = new QFrame;
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->addWidget(txtView);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);

  connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));

  resize(800, 600);
}
//
MetaDataViewImpl::MetaDataViewImpl(QWidget* parent) : QWidget(parent),
 targetTask_(0), currentModel_(0), currentModelIndex_(-1), selectedModel_(0),
 isSkip_(false), isWidgetSkip_(false),
   currentBodyItem_(0),
   updateKinematicStateLater(bind(&MetaDataViewImpl::updateKinematicState, this, true), IDLE_PRIORITY_LOW) {
  QFrame* taskFrame = new QFrame;
  leTask = new QLineEdit;
  //
  lstModel = UIUtil::makeTableWidget(2, false);
  lstModel->setColumnWidth(0, 50);
  lstModel->setColumnWidth(1, 300);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");

  QFrame* frmTask = new QFrame;
  QLabel* lblTaskName = new QLabel(tr("Task Name:"));
  leTask = new QLineEdit;
  QLabel* lblModel = new QLabel(tr("Model Name:"));
  leModel = new QLineEdit;
  QLabel* lblModelRName = new QLabel(tr("Model ID:"));
  leModelRName = new QLineEdit;
  QLabel* lblFile = new QLabel(tr("Model File:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Env.");
  cmbType->addItem("E.E.");
  cmbType->addItem("Work");
  leFile = new QLineEdit;

  btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(tr("Ref..."));

  btnRef->setEnabled(false);

  QLabel* lblX = new QLabel(tr("Origin X:"));
  leX = new QLineEdit; leX->setAlignment(Qt::AlignRight);
  QLabel* lblY = new QLabel(tr("Y:"));
  leY = new QLineEdit; leY->setAlignment(Qt::AlignRight);
  QLabel* lblZ = new QLabel(tr("Z:"));
  leZ = new QLineEdit; leZ->setAlignment(Qt::AlignRight);
  QLabel* lblRx = new QLabel(tr("Rx:"));
  leRx = new QLineEdit; leRx->setAlignment(Qt::AlignRight);
  QLabel* lblRy = new QLabel(tr("Ry:"));
  leRy = new QLineEdit; leRy->setAlignment(Qt::AlignRight);
  QLabel* lblRz = new QLabel(tr("Rz:"));
  leRz = new QLineEdit; leRz->setAlignment(Qt::AlignRight);
  //
  QFrame* frmButtons = new QFrame;
  //btnAddModel = new QPushButton(tr("Add Model"));
  btnAddModel = new QPushButton();
  btnAddModel->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAddModel->setToolTip(tr("Add Model"));
  //btnDeleteModel = new QPushButton(tr("Delete Model"));
  btnDeleteModel = new QPushButton();
  btnDeleteModel->setIcon(QIcon(":/Teaching/icons/Erase.png"));
  btnDeleteModel->setToolTip(tr("Delete Model"));
  //
  textEdit = new QTextEdit;
  lstFileName = new QListWidget;
  lstFileName->setSelectionMode(QListView::SingleSelection);

  //btnFileOutput = new QPushButton(tr("Output File"));
  btnFileOutput = new QPushButton();
  btnFileOutput->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnFileOutput->setToolTip(tr("Output File"));

  //btnFileShow = new QPushButton(tr("Show File"));
  btnFileShow = new QPushButton();
  btnFileShow->setIcon(QIcon(":/Teaching/icons/View.png"));
  btnFileShow->setToolTip(tr("Show File"));
  //btnFileDelete = new QPushButton(tr("Delete File"));
  btnFileDelete = new QPushButton();
  btnFileDelete->setIcon(QIcon(":/Teaching/icons/Erase.png"));
  btnFileDelete->setToolTip(tr("Delete File"));

  lstImage = new QListWidget;
  lstImage->setSelectionMode(QListView::SingleSelection);
  lstImage->setIconSize(QSize(100,100));
  lstImage->setFlow(QListWidget::LeftToRight);
  lstImage->setWrapping(true);
  //btnImageOutput = new QPushButton(tr("Output Image"));
  btnImageOutput = new QPushButton();
  btnImageOutput->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnImageOutput->setToolTip(tr("Output Image"));
  //btnImageShow = new QPushButton(tr("Show Image"));
  btnImageShow = new QPushButton();
  btnImageShow->setIcon(QIcon(":/Teaching/icons/View.png"));
  btnImageShow->setToolTip(tr("Show Image"));
  //btnImageDelete = new QPushButton(tr("Delete Image"));
  btnImageDelete = new QPushButton();
  btnImageDelete->setIcon(QIcon(":/Teaching/icons/Erase.png"));
  btnImageDelete->setToolTip(tr("Delete Image"));
  //
  QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setContentsMargins(2, 0, 2, 0);
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(lblTaskName, 0, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leTask, 0, 1, 1, 7);
  taskLayout->addWidget(lblModel, 1, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModel, 1, 1, 1, 7);
  taskLayout->addWidget(lblModelRName, 2, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModelRName, 2, 1, 1, 7);
  taskLayout->addWidget(lblFile, 3, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(cmbType, 3, 1, 1, 3);
  taskLayout->addWidget(leFile, 3, 2, 1, 5);
  taskLayout->addWidget(btnRef, 3, 7, 1, 1);

  taskLayout->addWidget(lblX, 4, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leX, 4, 1, 1, 2);
  taskLayout->addWidget(lblY, 4, 3, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leY, 4, 4, 1, 1);
  taskLayout->addWidget(lblZ, 4, 5, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leZ, 4, 6, 1, 2);
  taskLayout->addWidget(lblRx, 5, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRx, 5, 1, 1, 2);
  taskLayout->addWidget(lblRy, 5, 3, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRy, 5, 4, 1, 1);
  taskLayout->addWidget(lblRz, 5, 5, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRz, 5, 6, 1, 2);
  //
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(btnAddModel);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDeleteModel);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(lstModel);
  mainLayout->addWidget(frmTask);
  mainLayout->addWidget(frmButtons);
  mainLayout->addWidget(textEdit);
  mainLayout->addWidget(lstFileName);

  QFrame* frmFile = new QFrame();
  QHBoxLayout* fileLayout = new QHBoxLayout;
  fileLayout->setContentsMargins(0, 0, 0, 0);
  fileLayout->addWidget(btnFileOutput);
  fileLayout->addStretch();
  fileLayout->addWidget(btnFileShow);
  fileLayout->addStretch();
  fileLayout->addWidget(btnFileDelete);
  frmFile->setLayout(fileLayout);
  mainLayout->addWidget(frmFile);

  mainLayout->addWidget(lstImage);
  QFrame* frmImage = new QFrame();
  QHBoxLayout* imageLayout = new QHBoxLayout;
  imageLayout->setContentsMargins(0, 0, 0, 1);
  frmImage->setLayout(imageLayout);
  imageLayout->addWidget(btnImageOutput);
  imageLayout->addStretch();
  imageLayout->addWidget(btnImageShow);
  imageLayout->addStretch();
  imageLayout->addWidget(btnImageDelete);

  mainLayout->addWidget(frmImage);
  setLayout(mainLayout);

  textEdit->setAcceptDrops(false);
  setAcceptDrops(true);

  connect(btnAddModel, SIGNAL(clicked()), this, SLOT(addModelClicked()));
  connect(btnDeleteModel, SIGNAL(clicked()), this, SLOT(deleteModelClicked()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
  connect(btnRef, SIGNAL(clicked()), this, SLOT(refClicked()));

  connect(btnFileDelete, SIGNAL(clicked()), this, SLOT(fileDeleteClicked()));
  connect(btnFileShow, SIGNAL(clicked()), this, SLOT(fileShowClicked()));
  connect(lstFileName, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(fileShowClicked()));
  connect(btnFileOutput, SIGNAL(clicked()), this, SLOT(fileOutputClicked()));

  connect(btnImageDelete, SIGNAL(clicked()), this, SLOT(imageDeleteClicked()));
  connect(btnImageShow, SIGNAL(clicked()), this, SLOT(imageShowClicked()));
  connect(lstImage, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(imageShowClicked()));
  connect(btnImageOutput, SIGNAL(clicked()), this, SLOT(imageOutputClicked()));

  connect(leX, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leY, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leZ, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRx, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRy, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRz, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));

  m_proc_ = new QProcess(this);
  connect(m_proc_, SIGNAL(finished(int)), this, SLOT(processFinished()));

  setAllDisable();

  BodyBar::instance()->sigCurrentBodyItemChanged().connect(
      bind(&MetaDataViewImpl::onCurrentBodyItemChanged, this, _1));
}

MetaDataViewImpl::~MetaDataViewImpl() {
 if( m_proc_->state() == QProcess::Running) {
    m_proc_->kill();
    m_proc_->waitForFinished();
  }
  delete m_proc_;
}

void MetaDataViewImpl::modelPositionChanged() {
  if(isWidgetSkip_) return;
  if(currentModel_) {
    if( currentModel_->getModelItem() ) {
      double newX = leX->text().toDouble();
      double newY = leY->text().toDouble();
      double newZ = leZ->text().toDouble();
      double newRx = leRx->text().toDouble();
      double newRy = leRy->text().toDouble();
      double newRz = leRz->text().toDouble();
      if(dbl_eq(newX, currentModel_->getPosX())==false
        || dbl_eq(newY, currentModel_->getPosY())==false
        || dbl_eq(newZ, currentModel_->getPosZ())==false
        || dbl_eq(newRx, currentModel_->getRotRx())==false
        || dbl_eq(newRy, currentModel_->getRotRy())==false
        || dbl_eq(newRz, currentModel_->getRotRz())==false ) {
        ChoreonoidUtil::updateModelItemPosition(currentModel_->getModelItem(), newX, newY, newZ, newRx, newRy, newRz);
        currentModel_->setPosX(newX);
        currentModel_->setPosY(newY);
        currentModel_->setPosZ(newZ);
        currentModel_->setRotRx(newRx);
        currentModel_->setRotRy(newRy);
        currentModel_->setRotRz(newRz);
      }
    }
  }
}

void MetaDataViewImpl::addModelClicked() {
  if(targetTask_) {

    int newIdx = DatabaseManager::getInstance().getModelMaxIndex();
    ModelParam* param = new ModelParam(newIdx, 1, "New Model", "", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true);
    targetTask_->addModel(param);

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), param->getId());
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), param->getId());
  }
}

void MetaDataViewImpl::deleteModelClicked() {
  QTableWidgetItem* item = lstModel->currentItem();
  if(item) {
    int itemIndex = item->data(Qt::UserRole).toInt();
    targetTask_->deleteModelById(itemIndex);
    if(currentModel_ && currentModel_->getModelItem()) {
      connectionToKinematicStateChanged.disconnect();
      ChoreonoidUtil::unLoadModelItem(currentModel_);
    }

    lstModel->removeRow(lstModel->currentRow());
    currentModel_ = 0;
    currentModelIndex_ = -1;
    lstModel->setFocus();
  }
}

void MetaDataViewImpl::refClicked() {
	QString strFName = QFileDialog::getOpenFileName(
			this, tr( "Model VRML File" ), ".",
			tr( "wrl(*.wrl);;all(*.*)" ) );
	if ( strFName.isEmpty() ) return;
  //
  QString strName = QFileInfo(strFName).fileName();
  QString strPath = QFileInfo(strFName).absolutePath();
  leFile->setText(strName);
  if( !currentModel_ ) return;
  //
  QString currFile = currentModel_->getFileName();
  if(strFName==currFile) return;
  //
  if(currentModel_->getModelItem()) {
    connectionToKinematicStateChanged.disconnect();
    ChoreonoidUtil::unLoadModelItem(currentModel_);
    currentModel_->deleteModelDetails();
  }
  currentModel_->setFileName(strName);
  QFile file(strFName);
  file.open(QIODevice::ReadOnly);
  currentModel_->setData(file.readAll());
  if( ChoreonoidUtil::readModelItem(currentModel_, strFName) ) {
    ChoreonoidUtil::loadModelItem(currentModel_);
    ChoreonoidUtil::showAllModelItem();
  }
  //ŽQÆƒ‚ƒfƒ‹‚Ì“Ç‚Ýž‚Ý
  TeachingUtil::loadModelDetail(strFName, currentModel_);
}

void MetaDataViewImpl::modelSelectionChanged() {
  if(isSkip_) return;
  updateTaskModelInfo();

  if(currentModel_) {
    lstModel->item(currentModelIndex_, 0)->setText(getTypeName(currentModel_->getType()));
    lstModel->item(currentModelIndex_, 1)->setText(currentModel_->getName());
  }

  QTableWidgetItem* item = lstModel->currentItem();
  if(item) {
    currentModelIndex_ = lstModel->currentRow();
    currentModel_ = targetTask_->getModelById(item->data(Qt::UserRole).toInt());
    btnRef->setEnabled(true);

    ChoreonoidUtil::selectTreeItem(currentModel_);

    leModel->setText(currentModel_->getName());
    leModelRName->setText(currentModel_->getRName());
    cmbType->setCurrentIndex(currentModel_->getType());
    leFile->setText(currentModel_->getFileName());
    leX->setText(QString::number(currentModel_->getPosX(), 'f', 6));
    leY->setText(QString::number(currentModel_->getPosY(), 'f', 6));
    leZ->setText(QString::number(currentModel_->getPosZ(), 'f', 6));
    leRx->setText(QString::number(currentModel_->getRotRx(), 'f', 6));
    leRy->setText(QString::number(currentModel_->getRotRy(), 'f', 6));
    leRz->setText(QString::number(currentModel_->getRotRz(), 'f', 6));

  } else {
    clearModelDetail();
    currentModelIndex_ = -1;
    currentModel_ = 0;
  }
}

void MetaDataViewImpl::showModelGrid(TaskModelParam* source) {
  lstModel->clear();
  lstModel->setRowCount(0);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");

  for(int index=0; index<source->getModelList().size(); index++) {
    ModelParam* param = source->getModelList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), param->getId());
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), param->getId());
  }
}

void MetaDataViewImpl::updateTaskModelInfo() {
  if(targetTask_) {
    QString strTask = leTask->text();
    if( targetTask_->getName() != strTask) {
      targetTask_->setName(strTask);
    }
  }
    //
  if(currentModel_) {
    QString strModel = leModel->text();
    if( currentModel_->getName() != strModel) {
      currentModel_->setName(strModel);
    }
    //
    QString strModelRName = leModelRName->text();
    if( currentModel_->getRName() != strModelRName) {
      currentModel_->setRName(strModelRName);
    }
    //
    QString strFile = leFile->text();
    if( currentModel_->getFileName() != strFile) {
      currentModel_->setFileName(strFile);
    }
    //
    int selectedType = cmbType->currentIndex();
    if( currentModel_->getType() != selectedType) {
      currentModel_->setType(selectedType);
    }
    //
    string strPosX = leX->text().toUtf8().constData();
    double posX = std::atof(strPosX.c_str());
    if( currentModel_->getPosX() != posX) {
      currentModel_->setPosX(posX);
    }
    //
    string strPosY = leY->text().toUtf8().constData();
    double posY = std::atof(strPosY.c_str());
    if( currentModel_->getPosY() != posY) {
      currentModel_->setPosY(posY);
    }
    //
    string strPosZ = leZ->text().toUtf8().constData();
    double posZ = std::atof(strPosZ.c_str());
    if( currentModel_->getPosZ() != posZ) {
      currentModel_->setPosZ(posZ);
    }
    //
    string strRotRx = leRx->text().toUtf8().constData();
    double rotRx = std::atof(strRotRx.c_str());
    if( currentModel_->getRotRx() != rotRx) {
      currentModel_->setRotRx(rotRx);
    }
    //
    string strRotRy = leRy->text().toUtf8().constData();
    double rotRy = std::atof(strRotRy.c_str());
    if( currentModel_->getRotRy() != rotRy) {
      currentModel_->setRotRy(rotRy);
    }
    //
    string strRotRz = leRz->text().toUtf8().constData();
    double rotRz = std::atof(strRotRz.c_str());
    if( currentModel_->getRotRz() != rotRz) {
      currentModel_->setRotRz(rotRz);
    }
  }
}

void MetaDataViewImpl::clearModelDetail() {
  leModel->setText("");
  leFile->setText("");
  btnRef->setEnabled(false);
  leX->setText("");
  leY->setText("");
  leZ->setText("");
  leRx->setText("");
  leRy->setText("");
  leRz->setText("");
}

QString MetaDataViewImpl::getTypeName(int source) {
  QString result = "";

  switch(source) {
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

void MetaDataViewImpl::updateTaskParam() {
  if(targetTask_) {
    updateTaskModelInfo();
    targetTask_->setComment(textEdit->toPlainText());
  }
}

void MetaDataViewImpl::setTaskParam(TaskModelParam* param) {
  setAllClear();
  setAllEnable();
  //
  this->targetTask_ = param;
  showModelGrid(targetTask_);
  leTask->setText(targetTask_->getName());
  textEdit->setText(param->getComment());
  //
  vector<FileDataParam*> fileList = param->getFileList();
  for(int index=0; index<fileList.size();index++) {
    FileDataParam* param = fileList[index];
    if(param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    lstFileName->addItem(item);
    item->setData(Qt::UserRole, index);
  }
  //
  vector<ImageDataParam*> imageList = param->getImageList();
  for(int index=0; index<imageList.size();index++) {
    ImageDataParam* param = imageList[index];
    if(param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    item->setIcon(QIcon(QPixmap::fromImage(param->getData())));
    lstImage->addItem(item);
    item->setData(Qt::UserRole, index);
  }
}

void MetaDataViewImpl::clearTaskParam() {
  this->targetTask_ = 0;
  setAllClear();
  setAllDisable();
}

void MetaDataViewImpl::dragEnterEvent(QDragEnterEvent* event) {
  if(event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void MetaDataViewImpl::dropEvent(QDropEvent* event) {
  if(event->mimeData()->hasFormat("text/uri-list")) {
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty()) return;
    //
    for(int index=0; index<urls.length(); index++) {
      QString fileName = urls[index].toLocalFile();
      if(fileName.isEmpty()) continue;
      //
      QString strName = QFileInfo(fileName).fileName();
      if(fileName.endsWith("png") || fileName.endsWith("jpg")) {
        QListWidgetItem* item = new QListWidgetItem(strName);
        item->setIcon(QIcon(fileName));
        lstImage->addItem(item);
        //
        ImageDataParam* param = new ImageDataParam(0, strName);
        param->setNew();
        QImage image(fileName);
        param->setData(image);
        targetTask_->addImage(param);
        item->setData(Qt::UserRole, (unsigned int)(targetTask_->getImageList().size()-1));

      } else {
        QListWidgetItem* item = new QListWidgetItem(strName);
        lstFileName->addItem(item);
        //
        FileDataParam* param = new FileDataParam(0, strName);
        param->setNew();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        param->setData(file.readAll());
        targetTask_->addFile(param);
        item->setData(Qt::UserRole, (unsigned int)(targetTask_->getFileList().size()-1));
      }
    }
  }
  event->acceptProposedAction();
}

void MetaDataViewImpl::fileOutputClicked() {
  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  if(itemList.size()<=0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QByteArray data = targetTask_->getFileList()[selected]->getData();
  //
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if(fileDialog.exec()==false) return;
  QStringList strDirs = fileDialog.selectedFiles();
  //
  QString strDir = strDirs[0];
  strDir += QString("/") + targetTask_->getFileList()[selected]->getName();
  QFile file(strDir);
  file.open(QIODevice::WriteOnly);
  file.write(data);
  file.close();

  QMessageBox::information(this, tr("File Output"), "Target FILE saved");
}

void MetaDataViewImpl::processFinished() {
  for(int index=0; index<writtenFiles_.size(); index++) {
    QFile::remove(writtenFiles_[index]);
    DDEBUG_V("remove : %s", writtenFiles_[index].toStdString().c_str());
  }
  writtenFiles_.clear();
}

void MetaDataViewImpl::fileShowClicked() {
  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  if(itemList.size()<=0) return;
  try {
    QListWidgetItem *item = itemList.at(0);
    int selected = item->data(Qt::UserRole).toInt();
    QByteArray data = targetTask_->getFileList()[selected]->getData();
    QString source = QString::fromAscii(data);
    //
    QString strFile = targetTask_->getFileList()[selected]->getName();

    QString strExt = QFileInfo(strFile).suffix();
    QString targetApp = QString::fromStdString(SettingManager::getInstance().getTargetApp(strExt.toUpper().toStdString()));
    DDEBUG_V("targetApp : %s", targetApp.toStdString().c_str());

    if(0<targetApp.length()) {
      QFile file(strFile);
      file.open(QIODevice::WriteOnly);
      file.write(data);
      file.close();
      //
      if(std::find(writtenFiles_.begin(), writtenFiles_.end(), strFile) == writtenFiles_.end()) {
        writtenFiles_.push_back(strFile);
        DDEBUG_V("%s", strFile.toStdString().c_str());
      }
      QStringList options;
      options << strFile;
      m_proc_->start(targetApp, options);
    } else {
      TextDialog dialog(source, this);
      dialog.exec();
    }
  } catch (...) {
  }
}

void MetaDataViewImpl::fileDeleteClicked() {
  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  int itemCnt = itemList.size();
  for(int index=0; index<itemCnt; index++){
    QListWidgetItem *item = itemList.at( index );
    int selected = item->data(Qt::UserRole).toInt();
    delete item;
    targetTask_->getFileList()[selected]->setDelete();
  }
}

void MetaDataViewImpl::imageOutputClicked() {
  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if(itemList.size()<=0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QImage image = targetTask_->getImageList()[selected]->getData();
  //
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if(fileDialog.exec()==false) return;
  QStringList strDirs = fileDialog.selectedFiles();
  //
  QString strDir = strDirs[0];
  strDir += QString("/") + targetTask_->getImageList()[selected]->getName();
  image.save(strDir);

  QMessageBox::information(this, tr("File Output"), "Target IMAGE saved");
}

void MetaDataViewImpl::imageShowClicked() {
  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if(itemList.size()<=0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QImage image = targetTask_->getImageList()[selected]->getData();

  FigureDialog dialog(image, this);
  dialog.exec();
}

void MetaDataViewImpl::imageDeleteClicked() {
  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  int itemCnt = itemList.size();
  for(int index=0; index<itemCnt; index++){
    QListWidgetItem *item = itemList.at( index );
    int selected = item->data(Qt::UserRole).toInt();
    delete item;
    targetTask_->getImageList()[selected]->setDelete();
  }
}

void MetaDataViewImpl::setAllDisable() {
  lstModel->setEnabled(false);
  leTask->setEnabled(false);
  leModel->setEnabled(false);
  leModelRName->setEnabled(false);
  cmbType->setEnabled(false);
  leFile->setEnabled(false);
  btnRef->setEnabled(false);
  leX->setEnabled(false);
  leY->setEnabled(false);
  leZ->setEnabled(false);
  leRx->setEnabled(false);
  leRy->setEnabled(false);
  leRz->setEnabled(false);

  textEdit->setEnabled(false);
  lstFileName->setEnabled(false);
  btnFileOutput->setEnabled(false);
  btnFileDelete->setEnabled(false);
  lstImage->setEnabled(false);
  btnImageOutput->setEnabled(false);
  btnImageShow->setEnabled(false);
  btnImageDelete->setEnabled(false);
  btnAddModel->setEnabled(false);
  btnDeleteModel->setEnabled(false);
}

void MetaDataViewImpl::setAllEnable() {
  lstModel->setEnabled(true);
  leTask->setEnabled(true);
  leModel->setEnabled(true);
  leModelRName->setEnabled(true);
  cmbType->setEnabled(true);
  leFile->setEnabled(true);
  btnRef->setEnabled(true);
  leX->setEnabled(true);
  leY->setEnabled(true);
  leZ->setEnabled(true);
  leRx->setEnabled(true);
  leRy->setEnabled(true);
  leRz->setEnabled(true);
  btnAddModel->setEnabled(true);
  btnDeleteModel->setEnabled(true);

  textEdit->setEnabled(true);
  lstFileName->setEnabled(true);
  btnFileOutput->setEnabled(true);
  btnFileDelete->setEnabled(true);
  lstImage->setEnabled(true);
  btnImageOutput->setEnabled(true);
  btnImageShow->setEnabled(true);
  btnImageDelete->setEnabled(true);
}

void MetaDataViewImpl::setAllClear() {
  textEdit->setText("");
  lstFileName->clear();
  lstImage->clear();

  if(currentModel_ && currentModel_->getModelItem()) {
    connectionToKinematicStateChanged.disconnect();
    ChoreonoidUtil::unLoadModelItem(currentModel_);
  }
  currentModel_ = 0;
  currentModelIndex_ = -1;

  isSkip_ = true;
  lstModel->clear();
  lstModel->setRowCount(0);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");
  isSkip_ = false;

  leModel->setText("");
  leFile->setText("");
  btnRef->setEnabled(false);
  leX->setText("");
  leY->setText("");
  leZ->setText("");
  leRx->setText("");
  leRy->setText("");
  leRz->setText("");
}

void MetaDataViewImpl::onCurrentBodyItemChanged(BodyItem* bodyItem) {
  if(targetTask_ && bodyItem != currentBodyItem_){
    connectionToKinematicStateChanged.disconnect();
    currentBodyItem_ = bodyItem;
    if(currentBodyItem_) {
      for(int index=0; index<targetTask_->getModelList().size(); index++) {
        ModelParam* model = targetTask_->getModelList()[index];
        if(model->getModelItem().get() == currentBodyItem_) {
          selectedModel_ = model;
          break;
        }
      }
    }
    if(!connectionToKinematicStateChanged.connected() && currentBodyItem_){
      connectionToKinematicStateChanged = currentBodyItem_->sigKinematicStateChanged().connect(
      //    bind(&MetaDataViewImpl::updateKinematicState, this, true));
          updateKinematicStateLater);
    }
  }
}

void MetaDataViewImpl::updateKinematicState(bool blockSignals) {
  if(currentBodyItem_ && selectedModel_){
    Link* currentLink = currentBodyItem_->body()->rootLink();
    selectedModel_->setPosX(currentLink->p()[0]);
    selectedModel_->setPosY(currentLink->p()[1]);
    selectedModel_->setPosZ(currentLink->p()[2]);

    const Matrix3 R = currentLink->attitude();
    const Vector3 rpy = rpyFromRot(R);
    selectedModel_->setRotRx(degree(rpy[0]));
    selectedModel_->setRotRy(degree(rpy[1]));
    selectedModel_->setRotRz(degree(rpy[2]));

    if(selectedModel_==currentModel_) {
      isWidgetSkip_ = true;
      leX->setText(QString::number(currentModel_->getPosX(), 'f', 6));
      leY->setText(QString::number(currentModel_->getPosY(), 'f', 6));
      leZ->setText(QString::number(currentModel_->getPosZ(), 'f', 6));
      leRx->setText(QString::number(currentModel_->getRotRx(), 'f', 6));
      leRy->setText(QString::number(currentModel_->getRotRy(), 'f', 6));
      leRz->setText(QString::number(currentModel_->getRotRz(), 'f', 6));
      isWidgetSkip_ = false;
    }
  }
}
/////
MetaDataView::MetaDataView(): viewImpl(0) {
    setName("MetaData");
    setDefaultLayoutArea(View::BOTTOM);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewImpl = new MetaDataViewImpl(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(viewImpl);
    setLayout(vbox);
}

MetaDataView::~MetaDataView() {
};

}
