#ifndef TEACHING_PARAMETER_DIALOG_H_INCLUDED
#define TEACHING_PARAMETER_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class ParameterDialog : public QDialog {
  Q_OBJECT
public:
  ParameterDialog(TaskModelParam* param, QWidget* parent = 0);

private Q_SLOTS:
  void paramSelectionChanged();
  void typeSelectionChanged(int index);
  void addParamClicked();
  void deleteParamClicked();
  void oKClicked();

private:
  QTableWidget* lstModel;
  QTableWidget* lstParam;
  //
  QLineEdit* leName;
  QLineEdit* leId;
  QComboBox* cmbType;
  QLineEdit* leModelName;
  QLineEdit* leUnit;
  QLineEdit* leNum;
  QLineEdit* leElemType;

  int currentRowIndex_;
  TaskModelParam* targetTask_;
  ParameterParam* currentParam_;

  void showModelInfo();
  void showParamInfo();
  void saveCurrent();
  QString getTypeName(int source);
};

}
#endif
