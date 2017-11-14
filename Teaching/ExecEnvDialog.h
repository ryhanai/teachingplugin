#ifndef TEACHING_EXEC_ENV_DIALOG_H_INCLUDED
#define TEACHING_EXEC_ENV_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;

namespace teaching {

class ExecEnvDialog : public QDialog {
  Q_OBJECT
public:
  ExecEnvDialog(TaskModelParamPtr param, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTextEdit* txtEnv;

	TaskModelParamPtr targetTask_;
};

}
#endif
