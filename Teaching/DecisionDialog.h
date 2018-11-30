#ifndef TEACHING_DECISION_DIALOG_H_INCLUDED
#define TEACHING_DECISION_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class DesisionDialog : public QDialog {
  Q_OBJECT
public:
  DesisionDialog(TaskModelParamPtr param, ElementStmParamPtr stmParam, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTableWidget* lstModel;
  QTableWidget* lstParam;
  QTextEdit* txtCondition;

	TaskModelParamPtr targetTask_;
	ElementStmParamPtr targetStm_;

  void showModelInfo();
  void showParamInfo();
};

class FlowDesisionDialog : public QDialog {
  Q_OBJECT
public:
  FlowDesisionDialog(FlowParamPtr param, ElementStmParamPtr stmParam, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTableWidget * lstParam;
  QTextEdit* txtCondition;

  FlowParamPtr targetFlow_;
  ElementStmParamPtr targetStm_;

  void showParamInfo();
};
}
#endif
