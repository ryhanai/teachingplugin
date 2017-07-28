#ifndef TEACHING_DECISION_DIALOG_H_INCLUDED
#define TEACHING_DECISION_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class DesisionDialog : public QDialog {
  Q_OBJECT
public:
  DesisionDialog(TaskModelParam* param, ElementStmParam* stmParam, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTableWidget* lstModel;
  QTableWidget* lstParam;
  QTextEdit* txtCondition;

  TaskModelParam* targetTask_;
  ElementStmParam* targetStm_;

  void showModelInfo();
  void showParamInfo();
};

}
#endif
