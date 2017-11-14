#ifndef TEACHING_PARAMETER_VIEW_H_INCLUDED
#define TEACHING_PARAMETER_VIEW_H_INCLUDED

#include <string>
#include <cnoid/View>
#include <cnoid/LazyCaller>
#include <cnoid/ConnectionSet>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class ModelParameterGroup : public QWidget {
  Q_OBJECT
public:
  ModelParameterGroup(ParameterParamPtr source, ModelParamPtr model, QHBoxLayout* layout, QWidget* parent = 0);
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

	ParameterParamPtr targetParam_;
	ModelParamPtr targetModel_;

  BodyItemPtr currentBodyItem_;
  Connection connectionToKinematicStateChanged;
  LazyCaller updateKinematicStateLater;

  void updateKinematicState(bool blockSignals);
};
typedef std::shared_ptr<ModelParameterGroup> ModelParameterGroupPtr;

class ParameterViewImpl : public QWidget {
  Q_OBJECT
public:
  ParameterViewImpl(ParameterViewType type, QWidget* parent = 0);
  void setTaskParam(TaskModelParamPtr param, vector<ParameterParamPtr>& paramList);
  void clearTaskParam();

private Q_SLOTS:
  void editClicked();

private:
	ParameterViewType type_;
  QLabel* lblName;
  QPushButton* btnEdit;
  vector<QFrame*> frameList_;
  vector<QLineEdit*> textList_;

  vector<ModelParameterGroupPtr> modelList_;

  void clearView();
};

class ParameterView : public cnoid::View {
public:
  ParameterView();
  ~ParameterView();
  void setTaskParam(TaskModelParamPtr param, vector<ParameterParamPtr>& paramList) {
		this->viewImpl->setTaskParam(param, paramList);
	}
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }

private:
  ParameterViewImpl* viewImpl;
};

class FlowParameterView : public cnoid::View {
public:
	FlowParameterView();
	~FlowParameterView();

private:
	ParameterViewImpl* viewImpl;
};
}
#endif
