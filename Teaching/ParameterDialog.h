#ifndef TEACHING_PARAMETER_DIALOG_H_INCLUDED
#define TEACHING_PARAMETER_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class ParameterDialog : public QDialog {
  Q_OBJECT
public:
  ParameterDialog(QWidget* parent = 0);

  void showModelInfo(const std::vector<ModelParamPtr>& modelList);
	void showParamInfo(const std::vector<ParameterParamPtr>& paramList);
	void setTaskName(QString taskName);

	void updateContents(const ParameterParamPtr& param);
	void insertParameter(const ParameterParamPtr& param);

private Q_SLOTS:
  void paramSelectionChanged();
  void addParamClicked();
  void deleteParamClicked();
  void oKClicked();
  void rejected();

private:
	QLabel* lblTaskName;

  QTableWidget* lstModel;
  QTableWidget* lstParam;
  //
  QLineEdit* leName;
  QLineEdit* leId;
  QLineEdit* leUnit;
  QLineEdit* leNum;
	QComboBox* cmbHide;

  int currentRowIndex_;
};

}
#endif
