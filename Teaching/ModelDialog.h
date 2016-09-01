#ifndef TEACHING_MODEL_DIALOG_H_INCLUDED
#define TEACHING_MODEL_DIALOG_H_INCLUDED

#include <string>
#include <cnoid/LazyCaller>
#include <cnoid/ConnectionSet>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class MetaDataViewImpl;

class ModelDialog : public QDialog {
  Q_OBJECT
public:
  ModelDialog(MetaDataViewImpl* view, QWidget* parent = 0);
  void setTaskModel(TaskModelParam* param);
  void changeTaskModel(TaskModelParam* param);

private Q_SLOTS:
  void modelSelectionChanged();
  void refClicked();
  void addModelClicked();
  void deleteModelClicked();
  void modelPositionChanged();
  void okClicked();

private:
  QTableWidget* lstModel;

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

  TaskModelParam* targetTask_;
  int currentModelIndex_;
  ModelParam* currentModel_;
  ModelParam* selectedModel_;

  bool isWidgetSkip_;
  MetaDataViewImpl* parentView_;

	Connection connectionToKinematicStateChanged;
	Connection currentBodyItemChangeConnection;
	BodyItemPtr currentBodyItem_;
  LazyCaller updateKinematicStateLater;

  void showGrid();
  void updateTaskModelInfo();
  void clearModelDetail();
  QString getTypeName(int source);

  void onCurrentBodyItemChanged(BodyItem* bodyItem);
  void updateKinematicState(bool blockSignals);
};

}
#endif
