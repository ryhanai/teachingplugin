#ifndef TEACHING_PARAMETER_VIEW_H_INCLUDED
#define TEACHING_PARAMETER_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class ParameterViewImpl : public QWidget {
  Q_OBJECT
public:
  ParameterViewImpl(QWidget* parent = 0);
	void setTaskParam(TaskModelParamPtr param);
	void clearTaskParam();

private Q_SLOTS:
  void editClicked();

private:
  QLabel* lblName;
  QPushButton* btnEdit;
  vector<QFrame*> frameList_;
  vector<QLineEdit*> textList_;

  void clearView();
};

class ParameterView : public cnoid::View {
public:
  ParameterView();
  ~ParameterView();
	void setTaskParam(TaskModelParamPtr param) {
		this->viewImpl->setTaskParam(param);
	}
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }

private:
  ParameterViewImpl* viewImpl;
};
}
#endif
