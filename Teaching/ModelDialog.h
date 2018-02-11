#ifndef TEACHING_MODEL_DIALOG_H_INCLUDED
#define TEACHING_MODEL_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class MetaDataViewImpl;

class ModelDialog : public QDialog {
  Q_OBJECT
public:
  ModelDialog(QWidget* parent = 0);

	void showModelGrid(const vector<ModelParamPtr>& source);
	void showModelMasterGrid(const vector<ModelMasterParamPtr>& source);

	void updateContents(const ModelParamPtr& source);

private Q_SLOTS:
  void modelSelectionChanged();
	void modelMasterSelectionChanged();
  void addModelClicked();
  void deleteModelClicked();
  void modelPositionChanged();
  void okClicked();
	void cancelClicked();

private:
  QTableWidget* lstModel;
	QTableWidget* lstModelMaster;

	QLineEdit* leMaster;
	QLineEdit* leModelRName;
  QComboBox* cmbType;
  QLineEdit* leX;
  QLineEdit* leY;
  QLineEdit* leZ;
  QLineEdit* leRx;
  QLineEdit* leRy;
  QLineEdit* leRz;

	int currentIndex_;
	int currentMasterIndex_;

  QString getTypeName(int source);
};

}
#endif
