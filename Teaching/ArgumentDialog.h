#ifndef TEACHING_ARGUMENT_DIALOG_H_INCLUDED
#define TEACHING_ARGUMENT_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class ArgumentDialog : public QDialog {
  Q_OBJECT
public:
  ArgumentDialog(TaskModelParam* param, ElementStmParam* stmParam, QWidget* parent = 0);
  inline bool isOK() const { return this->isOK_; }

private Q_SLOTS:
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
  QTableWidget* lstParam;
  QTableWidget* lstHandling;
  QTableWidget* lstArg;
  //
  QLineEdit* txtStateName;
  QComboBox* cmbAction;
  QComboBox* cmbModel;
  QComboBox* cmbTarget;
  QTextEdit* txtArgDef;

  bool isOK_;
  int curArgIdx_;
  ArgumentParam* curArgParam_;
  int curActionIdx_;
  ElementStmActionParam* curActionParam_;

  TaskModelParam* targetTask_;
  ElementStmParam* targetStm_;

  void showModelInfo();
  void showParamInfo();
  void showActionInfo();
  void showArgInfo();

  void saveCurrentArg();
  void saveCurrentAction();
};

}
#endif
