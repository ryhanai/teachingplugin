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
	void showParamGrid(const vector<ModelParameterParamPtr>& paramList);
	void updateContents(QString name, QString fileName, QString imageFileName, QImage* targetImage);
  void updateImage(QString fileName, QImage targetImage);
  void updateParamContents(QString name, QString desc);
	void addModel(int id, QString name);

private Q_SLOTS:
  void modelSelectionChanged();
	void modelParamSelectionChanged();
	void refClicked();
  void refImageClicked();
  void addModelClicked();
	void addModelParamClicked();
	void deleteModelClicked();
	void deleteModelParamClicked();
  void deleteImageClicked();
  void okClicked();
	void cancelClicked();
  void reNewClicked();

private:
  QTableWidget* lstModel;
	QTableWidget* lstParam;

  QLineEdit* leModel;
  QLineEdit* leFile;
  QPushButton* btnRef;
  QLineEdit* leImage;
  QPushButton* btnRefImage;

  QGraphicsView* imageView;
  QGraphicsScene* scene;

	QLineEdit* leParam;
	QTextEdit* txtDef;

	int currentIndex_;
	int currentParamIndex_;
	bool eventCancel_;
};

}
#endif
