#include "MetaDataView.h"
#include <cnoid/UTF8>
#include <cnoid/ViewManager>
#include "TeachingUtil.h"
//
#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

ImageView::ImageView(QWidget *parent) : QGraphicsView(parent) {
}

ImageView::~ImageView(void) {
}

void ImageView::paintEvent(QPaintEvent* event) {
  QPainter widgetpainter(viewport());

  QImage qimg = m_img.scaled(viewport()->width(), viewport()->height(),
    Qt::KeepAspectRatio, Qt::FastTransformation);
  widgetpainter.drawImage(0, 0, qimg);
}

void ImageView::setImg(QImage& img) {
  m_img = QImage(img);
  viewport()->update();
}
//
FigureDialog::FigureDialog(QWidget* parent) : QDialog(parent) {
  m_ImageView_ = new ImageView(parent);

  this->setWindowTitle("Image");

  QPushButton* btnOK = new QPushButton(_("OK"));
  QFrame* frmButtons = new QFrame;
  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->addWidget(m_ImageView_);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);

  connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));

  resize(800, 600);
}
void FigureDialog::setImage(QImage& source) {
  m_ImageView_->setImg(source);
}
//
TextDialog::TextDialog(QString& source, QWidget* parent) : QDialog(parent) {
  QTextEdit* txtView = new QTextEdit(parent);
  txtView->setText(source);
  txtView->setReadOnly(true);

  QPushButton* btnOK = new QPushButton(_("OK"));
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
////
MetaDataViewImpl::MetaDataViewImpl(QWidget* parent) : QWidget(parent),
  targetTask_(0), m_FigDialog_(0), m_ModelDialog_(0) {
  //
  btnModel = new QPushButton(_("Model Inforamtion"));
  btnModel->setIcon(QIcon(":/Teaching/icons/About.png"));
  btnModel->setToolTip(_("Edit Model information"));

  textEdit = new QTextEdit;

  lstFileName = new QListWidget;
  lstFileName->setSelectionMode(QListView::SingleSelection);
  lstFileName->setAcceptDrops(true);
  lstFileName->setDragEnabled(true);
  lstFileName->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);

  btnFileOutput = new QPushButton(_("Output"));
  btnFileOutput->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnFileOutput->setToolTip(_("Output selected File"));

  btnFileShow = new QPushButton(_("Show"));
  btnFileShow->setIcon(QIcon(":/Teaching/icons/View.png"));
  btnFileShow->setToolTip(_("Show selected File"));

  btnFileDelete = new QPushButton(_("Delete"));
  btnFileDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnFileDelete->setToolTip(_("Delete selected File"));

  lstImage = new QListWidget;
  lstImage->setSelectionMode(QListView::SingleSelection);
  lstImage->setIconSize(QSize(100, 100));
  lstImage->setFlow(QListWidget::LeftToRight);
  lstImage->setWrapping(true);
  lstImage->setAcceptDrops(true);
  lstImage->setDragEnabled(true);
  lstImage->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);

  btnImageOutput = new QPushButton(_("Output"));
  btnImageOutput->setIcon(QIcon(":/Teaching/icons/Save.png"));
  btnImageOutput->setToolTip(_("Output selected Image"));

  btnImageShow = new QPushButton(_("Show"));
  btnImageShow->setIcon(QIcon(":/Teaching/icons/View.png"));
  btnImageShow->setToolTip(_("Show selected Image"));

  btnImageDelete = new QPushButton(_("Delete"));
  btnImageDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnImageDelete->setToolTip(_("Delete selected Image"));
  //
  QFrame* frmButtons = new QFrame;
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(2, 5, 2, 0);
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(btnModel);
  buttonLayout->addStretch();

  QFrame* frmFile = new QFrame();
  QHBoxLayout* fileLayout = new QHBoxLayout;
  fileLayout->setContentsMargins(0, 0, 0, 0);
  fileLayout->addWidget(btnFileOutput);
  fileLayout->addStretch();
  fileLayout->addWidget(btnFileShow);
  fileLayout->addStretch();
  fileLayout->addWidget(btnFileDelete);
  frmFile->setLayout(fileLayout);

  QFrame* frmImage = new QFrame();
  QHBoxLayout* imageLayout = new QHBoxLayout;
  imageLayout->setContentsMargins(0, 0, 0, 1);
  frmImage->setLayout(imageLayout);
  imageLayout->addWidget(btnImageOutput);
  imageLayout->addStretch();
  imageLayout->addWidget(btnImageShow);
  imageLayout->addStretch();
  imageLayout->addWidget(btnImageDelete);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(frmButtons);
  mainLayout->addWidget(textEdit);
  mainLayout->addWidget(lstFileName);
  mainLayout->addWidget(frmFile);
  mainLayout->addWidget(lstImage);
  mainLayout->addWidget(frmImage);
  setLayout(mainLayout);

  textEdit->setAcceptDrops(false);
  setAcceptDrops(true);

  connect(btnModel, SIGNAL(clicked()), this, SLOT(modelClicked()));

  connect(btnFileDelete, SIGNAL(clicked()), this, SLOT(fileDeleteClicked()));
  connect(btnFileShow, SIGNAL(clicked()), this, SLOT(fileShowClicked()));
  connect(lstFileName, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(fileShowClicked()));
  connect(btnFileOutput, SIGNAL(clicked()), this, SLOT(fileOutputClicked()));

  connect(btnImageDelete, SIGNAL(clicked()), this, SLOT(imageDeleteClicked()));
  connect(btnImageShow, SIGNAL(clicked()), this, SLOT(imageShowClicked()));
  connect(lstImage, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(imageShowClicked()));
  connect(btnImageOutput, SIGNAL(clicked()), this, SLOT(imageOutputClicked()));

  m_proc_ = new QProcess(this);
  connect(m_proc_, SIGNAL(finished(int)), this, SLOT(processFinished()));

  setAllDisable();
}

MetaDataViewImpl::~MetaDataViewImpl() {
  if (m_proc_->state() == QProcess::Running) {
    m_proc_->kill();
    m_proc_->waitForFinished();
  }
  delete m_proc_;
}

void MetaDataViewImpl::modelClicked() {
  DDEBUG("MetaDataViewImpl::modelClicked");

  if (targetTask_) {
    if (m_ModelDialog_) {
      m_ModelDialog_->close();
      delete m_ModelDialog_;
      m_ModelDialog_ = 0;
    }
    m_ModelDialog_ = new ModelDialog(this);
    m_ModelDialog_->setAttribute(Qt::WA_DeleteOnClose);
    m_ModelDialog_->setTaskModel(targetTask_);
    m_ModelDialog_->show();
  }
}

void MetaDataViewImpl::closeModelDialog() {
  m_ModelDialog_->close();
  delete m_ModelDialog_;
  m_ModelDialog_ = 0;
}

void MetaDataViewImpl::updateTaskParam() {
  DDEBUG("MetaDataViewImpl::updateTaskParam");
  if (targetTask_) {
    targetTask_->setComment(textEdit->toPlainText());
  }
}

void MetaDataViewImpl::setTaskParam(TaskModelParam* param) {
  DDEBUG("MetaDataViewImpl::setTaskParam");

  setAllClear();
  setAllEnable();
  //
  this->targetTask_ = param;
  textEdit->setText(param->getComment());
  if (m_ModelDialog_) {
    m_ModelDialog_->changeTaskModel(targetTask_);
  }
  //
  vector<FileDataParam*> fileList = param->getFileList();
  for (int index = 0; index < fileList.size(); index++) {
    FileDataParam* param = fileList[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    lstFileName->addItem(item);
    item->setData(Qt::UserRole, index);
  }
  //
  vector<ImageDataParam*> imageList = param->getImageList();
  for (int index = 0; index < imageList.size(); index++) {
    ImageDataParam* param = imageList[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    item->setIcon(QIcon(QPixmap::fromImage(param->getData())));
    lstImage->addItem(item);
    item->setData(Qt::UserRole, index);
  }
}

void MetaDataViewImpl::clearTaskParam() {
  DDEBUG("MetaDataViewImpl::clearTaskParam");

  this->targetTask_ = 0;
  setAllClear();
  setAllDisable();
}

void MetaDataViewImpl::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void MetaDataViewImpl::dropEvent(QDropEvent* event) {
  DDEBUG("MetaDataViewImpl::dropEvent");

  if (event->mimeData()->hasFormat("text/uri-list")) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;
    //
    for (int index = 0; index < urls.length(); index++) {
      QString fileName = urls[index].toLocalFile();
      if (fileName.isEmpty()) continue;
      //
      QString strName = QFileInfo(fileName).fileName();
      if (fileName.endsWith("png") || fileName.endsWith("jpg") || fileName.endsWith("jpeg")) {
        QListWidgetItem* item = new QListWidgetItem(strName);
        item->setIcon(QIcon(fileName));
        lstImage->addItem(item);
        //
        ImageDataParam* param = new ImageDataParam(0, strName);
        param->setNew();
        QImage image(fileName);
        param->setData(image);
        targetTask_->addImage(param);
        item->setData(Qt::UserRole, (unsigned int)(targetTask_->getImageList().size() - 1));

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
        item->setData(Qt::UserRole, (unsigned int)(targetTask_->getFileList().size() - 1));
      }
    }
  }
  event->acceptProposedAction();
}

void MetaDataViewImpl::fileOutputClicked() {
  DDEBUG("MetaDataViewImpl::fileOutputClicked");

  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  if (itemList.size() <= 0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QByteArray data = targetTask_->getFileList()[selected]->getData();
  //
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if (fileDialog.exec() == false) return;
  QStringList strDirs = fileDialog.selectedFiles();
  //
  QString strDir = strDirs[0];
  strDir += QString("/") + targetTask_->getFileList()[selected]->getName();
  QFile file(strDir);
  file.open(QIODevice::WriteOnly);
  file.write(data);
  file.close();

  QMessageBox::information(this, _("File Output"), _("Target FILE saved"));
}

void MetaDataViewImpl::processFinished() {
  for (int index = 0; index < writtenFiles_.size(); index++) {
    QFile::remove(writtenFiles_[index]);
    DDEBUG_V("remove : %s", writtenFiles_[index].toStdString().c_str());
  }
  writtenFiles_.clear();
}

void MetaDataViewImpl::fileShowClicked() {
  DDEBUG("MetaDataViewImpl::fileShowClicked");

  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  if (itemList.size() <= 0) return;
  try {
    QListWidgetItem *item = itemList.at(0);
    int selected = item->data(Qt::UserRole).toInt();
    QByteArray data = targetTask_->getFileList()[selected]->getData();
    QString source = QString::fromLatin1(data);
    //
    QString strFile = targetTask_->getFileList()[selected]->getName();

    QString strExt = QFileInfo(strFile).suffix();
    QString targetApp = QString::fromStdString(SettingManager::getInstance().getTargetApp(strExt.toUpper().toStdString()));
    DDEBUG_V("targetApp : %s", targetApp.toStdString().c_str());
    if (QFile::exists(targetApp) == false) {
      QMessageBox::warning(this, _("File Show"), _("APP does NOT EXIST."));
      return;
    }

    if (0 < targetApp.length()) {
      QFile file(strFile);
      file.open(QIODevice::WriteOnly);
      file.write(data);
      file.close();
      //
      if (std::find(writtenFiles_.begin(), writtenFiles_.end(), strFile) == writtenFiles_.end()) {
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
  }
  catch (...) {
  }
}

void MetaDataViewImpl::fileDeleteClicked() {
  DDEBUG("MetaDataViewImpl::fileDeleteClicked");

  QList<QListWidgetItem*> itemList = this->lstFileName->selectedItems();
  int itemCnt = itemList.size();
  for (int index = 0; index < itemCnt; index++) {
    QListWidgetItem *item = itemList.at(index);
    int selected = item->data(Qt::UserRole).toInt();
    delete item;
    targetTask_->getFileList()[selected]->setDelete();
  }
}

void MetaDataViewImpl::imageOutputClicked() {
  DDEBUG("MetaDataViewImpl::imageOutputClicked");

  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if (itemList.size() <= 0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QImage image = targetTask_->getImageList()[selected]->getData();
  //
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if (fileDialog.exec() == false) return;
  QStringList strDirs = fileDialog.selectedFiles();
  //
  QString strDir = strDirs[0];
  strDir += QString("/") + targetTask_->getImageList()[selected]->getName();
  image.save(strDir);

  QMessageBox::information(this, _("File Output"), _("Target IMAGE saved"));
}

void MetaDataViewImpl::imageShowClicked() {
  DDEBUG("MetaDataViewImpl::imageShowClicked");

  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if (itemList.size() <= 0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
  QImage image = targetTask_->getImageList()[selected]->getData();

  if (!m_FigDialog_) {
    m_FigDialog_ = new FigureDialog(this);
  }
  m_FigDialog_->setImage(image);
  m_FigDialog_->show();
}

void MetaDataViewImpl::imageDeleteClicked() {
  DDEBUG("MetaDataViewImpl::imageDeleteClicked");

  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  int itemCnt = itemList.size();
  for (int index = 0; index < itemCnt; index++) {
    QListWidgetItem *item = itemList.at(index);
    int selected = item->data(Qt::UserRole).toInt();
    delete item;
    targetTask_->getImageList()[selected]->setDelete();
  }
}

void MetaDataViewImpl::setAllDisable() {
  btnModel->setEnabled(false);
  textEdit->setEnabled(false);

  lstFileName->setEnabled(false);
  btnFileOutput->setEnabled(false);
  btnFileShow->setEnabled(false);
  btnFileDelete->setEnabled(false);

  lstImage->setEnabled(false);
  btnImageOutput->setEnabled(false);
  btnImageShow->setEnabled(false);
  btnImageDelete->setEnabled(false);
}

void MetaDataViewImpl::setAllEnable() {
  btnModel->setEnabled(true);
  textEdit->setEnabled(true);

  lstFileName->setEnabled(true);
  btnFileOutput->setEnabled(true);
  btnFileShow->setEnabled(true);
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
}
/////
MetaDataView::MetaDataView() : viewImpl(0) {
  setName(_("MetaData"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new MetaDataViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::LEFT_TOP);
}

MetaDataView::~MetaDataView() {
};

}
