#ifndef TEACHING_METADATA_VIEW_H_INCLUDED
#define TEACHING_METADATA_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"
#include "ModelDialog.h"

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
  FigureDialog(QWidget* parent = 0);
  void setImage(QImage& source);
private:
  ImageView* m_ImageView_;
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

	void setTaskParam(TaskModelParamPtr param);
  void updateTaskParam();
  void clearTaskParam();
  void setEditMode(bool canEdit);

private Q_SLOTS:
  void modelClicked();
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

  QPushButton* btnModel;
  QListWidget* lstFileName;
  QPushButton* btnFileOutput;
  QPushButton* btnFileShow;
  QPushButton* btnFileDelete;
  QListWidget* lstImage;
  QPushButton* btnImageOutput;
  QPushButton* btnImageShow;
  QPushButton* btnImageDelete;

  QProcess* m_proc_;
  vector<QString> writtenFiles_;

  void setAllEnable();
  void setAllDisable();
  void setAllClear();
};

class MetaDataView : public cnoid::View {
public:
  MetaDataView();
  ~MetaDataView();
  void setTaskParam(TaskModelParamPtr param) {
		this->viewImpl->setTaskParam(param);
	}

private:
  MetaDataViewImpl* viewImpl;
};

}
#endif
