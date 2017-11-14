#ifndef TEACHING_TEACHING_EVENTHANDLER_H_INCLUDED
#define TEACHING_TEACHING_EVENTHANDLER_H_INCLUDED

#include <cnoid/LazyCaller>
#include <cnoid/ConnectionSet>

#include "TeachingTypes.h"
#include "TeachingDataHolder.h"

#include "TaskInstanceView.h"
#include "FlowView.h"
#include "MetaDataView.h"
#include "StateMachineView.h"
#include "ParameterView.h"

#include "FlowSearchDialog.h"
#include "ModelDialog.h"
#include "ParameterDialog.h"
#include "ModelMasterDialog.h"
#include "ArgumentDialog.h"

#include "TaskExecuteManager.h"

using namespace cnoid;

namespace teaching {

class TeachingEventHandler {
public:
  static TeachingEventHandler* instance();
	~TeachingEventHandler() {};

	//TaskInstanceView
	void tiv_Loaded(TaskInstanceViewImpl* view);
	void tiv_TaskSelectionChanged(int selectedId, QString strTask);
	bool tiv_DeleteTaskClicked();
	void tiv_TaskExportClicked(QString strTask);
	bool tiv_TaskImportClicked();
	void tiv_SearchClicked(QString cond);
	void tiv_RegistTaskClicked(QString strTask);
	void tiv_RegistNewTaskClicked(QString strTask, QString strCond);

	void tiv_InitPosClicked();

	void tiv_SearchTaskInstance(QString cond);

	//FlowView
	void flv_Loaded(FlowViewImpl* view);
	void flv_NewFlowClicked();
	void flv_SearchClicked();
	void flv_SelectionChanged(TaskModelParamPtr target);
	void flv_RegistFlowClicked(QString name, QString comment);
	void flv_DeleteTaskClicked();
	void flv_FlowExportClicked(QString name, QString comment);
	void flv_FlowImportClicked();
	void flv_RunFlowClicked();
	void flv_InitPosClicked();

	//MetaDataView
	void mdv_Loaded(MetaDataViewImpl* view);
	void mdv_ModelClicked();
	int mdv_DropEventImage(QString name, QString fileName);
	int mdv_DropEventFile(QString name, QString fileName);
	void mdv_FileOutputClicked(int id);
	void mdv_ImageOutputClicked(int id);
	void mdv_FileDeleteClicked(int id);
	void mdv_ImageDeleteClicked(int id);
	void mdv_ImageShowClicked(int id);
	FileDataParamPtr mdv_FileShowClicked(int id);
	void mdv_UpdateComment(QString comment);
	void mdv_UpdateFileSeq(int id, int seq);
	void mdv_UpdateImageSeq(int id, int seq);

	//StateMachineView
	void stv_Loaded(StateMachineViewImpl* view);
	void stv_EditClicked(ElementNode* target);
	void stv_SetClicked(ConnectionNode* target, QString value);

	//ParameterView
	void prv_Loaded(ParameterViewType type, ParameterViewImpl* view);
	void prv_SetInputValues();

	//TaskExecutionView
	void tev_setBreak(bool value) { executor_->setBreak(value); }
	void tev_stm_RunClicked(bool isReal, ElementNode* target);
	void tev_stm_StepClicked();
	void tev_stm_ContClicked();
	void tev_RunTaskClicked();
	void tev_AbortClicked();

	//FlowSearchDialog
	void fsd_Loaded(FlowSearchDialog* dialog);
	void fsd_SeachClicked(QString condition);
	bool fsd_DeleteClicked(int targetId);
	void fsd_OKClicked(int targetId);

	//ParameterDialog
	void prd_Loaded(ParameterDialog* dialog);
	void prd_ParamSelectionChanged(int newId, QString name, QString id, int type, QString unit, QString num, int elemType, QString model, int hide);
	void prd_AddParamClicked(QString name, QString id, int type, QString unit, QString num, int elemType, QString model, int hide);
	bool prd_DeleteParamClicked();
	bool prd_OkClicked(QString name, QString id, int type, QString unit, QString num, int elemType, QString model, int hide);

	//ModelDialog
	bool mdd_Loaded(ModelDialog* dialog);
	void mdd_ModelSelectionChanged(int newId, QString name, QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ);
	void mdd_ModelMasterSelectionChanged(int newId);
	void mdd_CurrentBodyItemChanged(BodyItem* bodyItem);
	void mdd_updateKinematicState(bool blockSignals);
	void mdd_ModelPositionChanged(double posX, double posY, double posZ, double rotX, double rotY, double rotZ);
	bool mdd_AddModelClicked();
	bool mdd_DeleteModelClicked();
	void mdd_OkClicked(QString name, QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ);
	void mdd_CancelClicked();

	//ModelMasterDialog
	void mmd_Loaded(ModelMasterDialog* dialog);
	void mmd_ModelSelectionChanged(int newId, QString name, QString fileName);
	void mmd_RefClicked();
	void mmd_AddModelClicked();
	void mmd_DeleteModelClicked(int id);
	bool mmd_OkClicked(QString name, QString fileName, QString errMessage);

	//ArgumentDialog
	void agd_Loaded(ArgumentDialog* dialog);
	void agd_ArgSelectionChanged(int selectedId, QString strDef);
	void agd_ActionSelectionChanged(int selectedId, QString strAct, QString strModel, QString strTarget);
	void agd_AddClicked(QString strAct, QString strModel, QString strTarget);
	void agd_DeleteClicked();
	bool agd_OKClicked(QString strName, QString strAct, QString strModel, QString strTarget, QString strArgDef);
	void agd_CancelClicked();
	void agd_Update(QString strAct, QString strModel, QString strTarget);
	void agd_SetSeq(int selected, int seq);

	//TaskExecuteManager
	inline void setTaskExecutor(TaskExecuteManager* executor) { this->executor_ = executor; }
	bool checkPaused();

private:
	TeachingEventHandler() 
		: flv_(0), flv_CurrentId_(NULL_ID), flv_CurrentFlow_(0), isFlowDeleted_(false),
			tiv_(0), tiv_CurrentTask_(0),
			com_CurrentTask_(0), com_CurrParam_(0),
			mdv_(0), m_FigDialog_(0),
			stv_(0),
			prv_(0),
			//tev_currentTask_(0),
			fsd_(0),
			mdd_(0), mdd_CurrentId_(NULL_ID), mdd_CurrentModel_(0),
			mdd_CurrentMasterId_(NULL_ID), mdd_CurrentModelMaster_(0),
			mdd_updateKinematicStateLater(bind(&TeachingEventHandler::mdd_updateKinematicState, this, true), IDLE_PRIORITY_LOW),
			prd_(0), prd_CurrentParam_(0),
			mmd_(0), mmd_CurrentId_(NULL_ID), mmd_CurrentModel_(0),
			agd_(0), agd_Current_Stm_(0), agd_Current_Arg_(0), agd_Current_Action_(0),
		  executor_(0) {
	};

	bool eventSkip_ = false;

	TaskInstanceViewImpl* tiv_;
	TaskModelParamPtr tiv_CurrentTask_;

	FlowViewImpl* flv_;
	int flv_CurrentId_;
	FlowParamPtr flv_CurrentFlow_;
	bool isFlowDeleted_;

	TaskModelParamPtr com_CurrentTask_;
	ElementStmParamPtr com_CurrParam_;

	MetaDataViewImpl* mdv_;
	FigureDialog* m_FigDialog_;

	StateMachineViewImpl* stv_;

	ParameterViewImpl* prv_;
	ParameterViewImpl* fpv_;

	FlowSearchDialog* fsd_;

	ModelDialog* mdd_;
	int mdd_CurrentId_;
	ModelParamPtr mdd_CurrentModel_;
	int mdd_CurrentMasterId_;
	ModelMasterParamPtr mdd_CurrentModelMaster_;
	ModelParamPtr mdd_selectedModel_;
	BodyItemPtr mdd_BodyItem_;
	Connection mdd_connectionToKinematicStateChanged;
	Connection mdd_currentBodyItemChangeConnection;
	LazyCaller mdd_updateKinematicStateLater;

	ParameterDialog* prd_;
	ParameterParamPtr prd_CurrentParam_;

	ModelMasterDialog* mmd_;
	int mmd_CurrentId_;
	ModelMasterParamPtr mmd_CurrentModel_;

	ArgumentDialog* agd_;
	ElementStmParamPtr agd_Current_Stm_;
	ArgumentParamPtr agd_Current_Arg_;
	ElementStmActionParamPtr agd_Current_Action_;

	TaskExecuteManager* executor_;

	//ParameterDialog
	void prd_UpdateParam(QString name, QString id, int type, QString unit, QString num, int elemType, QString model, int hide);

	void unloadTaskModelItems();
	void updateComViews(TaskModelParamPtr targetTask);


};

}
#endif