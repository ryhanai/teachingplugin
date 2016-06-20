#ifndef TEACHING_PARAMETER_VIEW_H_INCLUDED
#define TEACHING_PARAMETER_VIEW_H_INCLUDED

#include <cnoid/View>
#include <cnoid/MessageView>  /* modified by qtconv.rb 0th rule*/  
#include <cnoid/LazyCaller>
#include <QtGui>
#include <string>
#include "TeachingTypes.h"
#include <boost/signal.hpp>

using namespace cnoid;
using namespace std;

namespace teaching {

class ModelParameterGroup : public QWidget {
  Q_OBJECT
public:
  ModelParameterGroup(ParameterParam* source, ModelParam* model, QHBoxLayout* layout, QWidget* parent = 0);
  void disconnectKinematics();

private Q_SLOTS:
  void modelPositionChanged();

private:
  QLineEdit* leX_;
  QLineEdit* leY_;
  QLineEdit* leZ_;
  QLineEdit* leRx_;
  QLineEdit* leRy_;
  QLineEdit* leRz_;

  ParameterParam* targetParam_;
  ModelParam* targetModel_;

  BodyItemPtr currentBodyItem_;
  boost::signals::connection connectionToKinematicStateChanged;
  LazyCaller updateKinematicStateLater;

  void updateKinematicState(bool blockSignals);

  std::ostream& os_;
};

class ParameterViewImpl : public QWidget {
  Q_OBJECT
public:
  ParameterViewImpl(QWidget* parent = 0);
  void setTaskParam(TaskModelParam* param);
  void clearTaskParam();
  void setInputValues();

private Q_SLOTS:
  void editClicked();

private:
  QLabel* lblName;
  QPushButton* btnEdit;
  vector<QFrame*> frameList_;
  vector<QLineEdit*> textList_;

  vector<ModelParameterGroup*> modelList_;

  TaskModelParam* targetTask_;

  std::ostream& os_;

  void clearView();
};

class ParameterView : public cnoid::View {
public:
  ParameterView();
  ~ParameterView();
  void setTaskParam(TaskModelParam* param) { this->viewImpl->setTaskParam(param); }
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }
  void setInputValues() { this->viewImpl->setInputValues(); }

private:
  ParameterViewImpl* viewImpl;
};

}
#endif
