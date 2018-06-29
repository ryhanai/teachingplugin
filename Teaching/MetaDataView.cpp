#include "MetaDataView.h"
#include <cnoid/UTF8>
#include <cnoid/ViewManager>
#include "TeachingUtil.h"

#include "TeachingEventHandler.h"
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
MetaDataViewImpl::MetaDataViewImpl(QWidget* parent) : QWidget(parent) {
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

	TeachingEventHandler::instance()->mdv_Loaded(this);
}

void MetaDataViewImpl::modelClicked() {
	DDEBUG("MetaDataViewImpl::modelClicked");
	TeachingEventHandler::instance()->mdv_ModelClicked();
}

MetaDataViewImpl::~MetaDataViewImpl() {
  DDEBUG("MetaDataViewImpl Destruct");
  if (m_proc_->state() == QProcess::Running) {
    m_proc_->kill();
    m_proc_->waitForFinished();
  }
  delete m_proc_;
  DDEBUG("MetaDataViewImpl Destruct End");
}

void MetaDataViewImpl::updateTaskParam() {
  DDEBUG("MetaDataViewImpl::updateTaskParam");

	TeachingEventHandler::instance()->mdv_UpdateComment(textEdit->toPlainText());

	for (int index = 0; index < this->lstFileName->count(); index++) {
		int selected = this->lstFileName->item(index)->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->mdv_UpdateFileSeq(selected, index + 1);
	}
	for (int index = 0; index < this->lstImage->count(); index++) {
		int selected = this->lstImage->item(index)->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->mdv_UpdateImageSeq(selected, index + 1);
	}
}

void MetaDataViewImpl::setTaskParam(TaskModelParamPtr param) {
  DDEBUG("MetaDataViewImpl::setTaskParam");

  setAllClear();
  setAllEnable();
  //
  textEdit->setText(param->getComment());
  setEditMode(TeachingEventHandler::instance()->canEdit());
  //
  for (FileDataParamPtr param : param->getActiveFileList()) {
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    lstFileName->addItem(item);
    item->setData(Qt::UserRole, param->getId());
	}
  //
  for (ImageDataParamPtr param : param->getActiveImageList()) {
    QListWidgetItem* item = new QListWidgetItem(param->getName());
    item->setIcon(QIcon(QPixmap::fromImage(param->getData())));
    lstImage->addItem(item);
    item->setData(Qt::UserRole, param->getId());
	}
}

void MetaDataViewImpl::clearTaskParam() {
  DDEBUG("MetaDataViewImpl::clearTaskParam");

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

  if (TeachingEventHandler::instance()->canEdit()) {
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
          int newId = TeachingEventHandler::instance()->mdv_DropEventImage(strName, fileName);
          item->setData(Qt::UserRole, newId);

        } else {
          QListWidgetItem* item = new QListWidgetItem(strName);
          lstFileName->addItem(item);
          //
          int newId = TeachingEventHandler::instance()->mdv_DropEventFile(strName, fileName);
          item->setData(Qt::UserRole, newId);
        }
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

	TeachingEventHandler::instance()->mdv_FileOutputClicked(selected);
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

		FileDataParamPtr target = TeachingEventHandler::instance()->mdv_FileShowClicked(selected);
    QByteArray data = target->getData();
    QString source = QString::fromLatin1(data);
    QString strFile = target->getName();

    QString strExt = QFileInfo(strFile).suffix();
    QString targetApp = QString::fromStdString(SettingManager::getInstance().getTargetApp(strExt.toUpper().toStdString()));
    DDEBUG_V("targetApp : %s", targetApp.toStdString().c_str());
    if (QFile::exists(targetApp)) {
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
  } catch (...) {
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
		TeachingEventHandler::instance()->mdv_FileDeleteClicked(selected);
  }
}

void MetaDataViewImpl::imageOutputClicked() {
  DDEBUG("MetaDataViewImpl::imageOutputClicked");

  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if (itemList.size() <= 0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();

	TeachingEventHandler::instance()->mdv_ImageOutputClicked(selected);
}

void MetaDataViewImpl::imageShowClicked() {
  DDEBUG("MetaDataViewImpl::imageShowClicked");
  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  if (itemList.size() <= 0) return;
  QListWidgetItem *item = itemList.at(0);
  int selected = item->data(Qt::UserRole).toInt();
	TeachingEventHandler::instance()->mdv_ImageShowClicked(selected);
}

void MetaDataViewImpl::imageDeleteClicked() {
  DDEBUG("MetaDataViewImpl::imageDeleteClicked");

  QList<QListWidgetItem*> itemList = this->lstImage->selectedItems();
  int itemCnt = itemList.size();
  for (int index = 0; index < itemCnt; index++) {
    QListWidgetItem *item = itemList.at(index);
    int selected = item->data(Qt::UserRole).toInt();
    delete item;
		TeachingEventHandler::instance()->mdv_ImageDeleteClicked(selected);
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

void MetaDataViewImpl::setEditMode(bool canEdit) {
  btnModel->setEnabled(canEdit);
  textEdit->setEnabled(canEdit);

  btnFileOutput->setEnabled(canEdit);
  btnFileDelete->setEnabled(canEdit);

  btnImageOutput->setEnabled(canEdit);
  btnImageDelete->setEnabled(canEdit);
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
