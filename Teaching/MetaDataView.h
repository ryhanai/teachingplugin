#ifndef TEACHING_METADATA_VIEW_H_INCLUDED
#define TEACHING_METADATA_VIEW_H_INCLUDED

#include <cnoid/View>
#include <cnoid/LazyCaller>
#include <QtGui>
#include <string>
#include "TeachingTypes.h"
#include <boost/signal.hpp>

using namespace cnoid;
using namespace std;

namespace teaching {

class ImageView : public QGraphicsView {
public:
  ImageView(QWidget *parent);
  ~ImageView(void);
  void setImg(QImage& img);

private:
  void paintEvent(QPaintEvent* event);
  QImage m_img;
};

class FigureDialog : public QDialog {
public:
  FigureDialog(QImage& source, QWidget* parent = 0);
};

class TextDialog : public QDialog {
public:
  TextDialog(QString& source, QWidget* parent = 0);
};

class MetaDataViewImpl : public QWidget {
  Q_OBJECT
public:
  MetaDataViewImpl(QWidget* parent = 0);
  ~MetaDataViewImpl();
  void setTaskParam(TaskModelParam* param);
  void updateTaskParam();
  void clearTaskParam();

private Q_SLOTS:
  void modelSelectionChanged();
  void refClicked();
  void addModelClicked();
  void deleteModelClicked();
  void modelPositionChanged();
  void fileOutputClicked();
  void fileShowClicked();
  void fileDeleteClicked();
  void imageOutputClicked();
  void imageShowClicked();
  void imageDeleteClicked();
  void processFinished();

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

private:
  QTextEdit* textEdit;

  QTableWidget* lstModel;
  //
  QLineEdit* leTask;
  QLineEdit* leModel;
  QLineEdit* leModelRName;
  QComboBox* cmbType;
  QLineEdit* leFile;
  QPushButton* btnRef;
  QLineEdit* leX;
  QLineEdit* leY;
  QLineEdit* leZ;
  QLineEdit* leRx;
  QLineEdit* leRy;
  QLineEdit* leRz;
  QPushButton* btnAddModel;
  QPushButton* btnDeleteModel;
  //
  QListWidget* lstFileName;
  QPushButton* btnFileOutput;
  QPushButton* btnFileShow;
  QPushButton* btnFileDelete;
  QListWidget* lstImage;
  QPushButton* btnImageOutput;
  QPushButton* btnImageShow;
  QPushButton* btnImageDelete;

  bool isSkip_;
  bool isWidgetSkip_;
  int currentModelIndex_;
  ModelParam* currentModel_;
  ModelParam* selectedModel_;

  QProcess* m_proc_;
  vector<QString> writtenFiles_;

  TaskModelParam* targetTask_;

  BodyItemPtr currentBodyItem_;
  boost::signals::connection connectionToKinematicStateChanged;
  LazyCaller updateKinematicStateLater;

  void showModelGrid(TaskModelParam* source);
  void updateTaskModelInfo();
  void clearModelDetail();
  QString getTypeName(int source);

  void setAllEnable();
  void setAllDisable();
  void setAllClear();

  void onCurrentBodyItemChanged(BodyItem* bodyItem);
  void updateKinematicState(bool blockSignals);
};

class MetaDataView : public cnoid::View {
public:
  MetaDataView();
  ~MetaDataView();
  void setTaskParam(TaskModelParam* param) { this->viewImpl->setTaskParam(param); }
  void updateTaskParam() { this->viewImpl->updateTaskParam(); }
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }

private:
  MetaDataViewImpl* viewImpl;
};

}
#endif
