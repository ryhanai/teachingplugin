#ifndef TEACHING_ARGUMENT_DIALOG_H_INCLUDED
#define TEACHING_ARGUMENT_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace std;
using namespace cnoid;

namespace teaching {

class ArgumentDialog : public QDialog {
  Q_OBJECT
public:
  ArgumentDialog(QWidget* parent = 0);
  inline bool isOK() const { return this->isOK_; }

	void showModelInfo(const vector<ModelParamPtr>& modelList);
	void showModelParamInfo(const vector<ModelParameterParamPtr>& paramList);
	void showParamInfo(const vector<ParameterParamPtr>& paramList, const vector<ModelParamPtr>& modelList);
	void showActionInfo(const vector<ElementStmActionParamPtr>& actionList);
	void showArgInfo(const ElementStmParamPtr target, const vector<ArgumentParamPtr>& argList);

	void updateArgument(QString currText);
	void updateAction(ElementStmActionParamPtr& target);
	void updateAddAction(ElementStmActionParamPtr& target);

private Q_SLOTS:
  void parentSelectionChanged(int index);
  void modelSelectionChanged();
  void addClicked();
  void deleteClicked();
  void upClicked();
  void downClicked();
  void actionSelectionChanged();
  void argSelectionChanged();
  void oKClicked();
  void cancelClicked();
  void rejected();

private:
  QTableWidget* lstModel;
	QTableWidget* lstModelParam;
	QTableWidget* lstParam;
  QTableWidget* lstHandling;
  QTableWidget* lstArg;
  //
  QLineEdit* txtStateName;
	QLineEdit* txtCmdName;
  QRadioButton* radAttach;
  QRadioButton* radDetach;
  QComboBox* cmbParent;
  QComboBox* cmbModel;
  QComboBox* cmbTarget;
  QTextEdit* txtArgDef;

  bool isOK_;
  int curArgIdx_;
  int curActionIdx_;

  QString getActionStr();
  const QString parentRobot_;
};

}
#endif
