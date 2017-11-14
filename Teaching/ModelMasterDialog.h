#ifndef TEACHING_MODELMASTER_DIALOG_H_INCLUDED
#define TEACHING_MODELMASTER_DIALOG_H_INCLUDED

#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class ModelMasterDialog : public QDialog {
  Q_OBJECT
public:
	ModelMasterDialog(QWidget* parent = 0);

	void showGrid(const vector<ModelMasterParamPtr>& masterList);
	void updateContents(QString name, QString fileName);
	void addModel(int id, QString name);

private Q_SLOTS:
  void modelSelectionChanged();
  void refClicked();
  void addModelClicked();
  void deleteModelClicked();
  void okClicked();
	void cancelClicked();

private:
  QTableWidget* lstModel;

  QLineEdit* leModel;
  QLineEdit* leFile;
  QPushButton* btnRef;

	int currentIndex_;
};

}
#endif
