#ifndef TEACHING_TEACHING_MASTER_EVENTHANDLER_H_INCLUDED
#define TEACHING_TEACHING_MASTER_EVENTHANDLER_H_INCLUDED

#include "TeachingTypes.h"

#include "ModelMasterDialog.h"

using namespace cnoid;

namespace teaching {

class TeachingMasterEventHandler {
public:
  static TeachingMasterEventHandler* instance();
	~TeachingMasterEventHandler();

	//ModelMasterDialog
	void mmd_Loaded(ModelMasterDialog* dialog);
	void mmd_ModelSelectionChanged(int newId, QString name, QString fileName);
	void mmd_ModelParameterSelectionChanged(int newId, QString name, QString desc);
	void mmd_RefClicked();
  void mmd_RefImageClicked();
  void mmd_DeleteImageClicked();
  void mmd_AddModelClicked();
	bool mmd_DeleteModelClicked();
	void mmd_AddModelParamClicked();
	bool mmd_DeleteModelParamClicked();
	bool mmd_OkClicked(QString name, QString fileName, QString& errMessage);
  bool mmd_Check();
	void mmd_Close();

private:
	TeachingMasterEventHandler() 
		: mmd_(0), mmd_CurrentId_(NULL_ID), mmd_CurrentModel_(0), mmd_CurrentParam_(0) {
  };

	ModelMasterDialog* mmd_;
	int mmd_CurrentId_;
	ModelMasterParamPtr mmd_CurrentModel_;
	ModelParameterParamPtr mmd_CurrentParam_;
};

}
#endif
