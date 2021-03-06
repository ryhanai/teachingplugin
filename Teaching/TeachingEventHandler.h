#ifndef TEACHING_TEACHING_EVENTHANDLER_H_INCLUDED
#define TEACHING_TEACHING_EVENTHANDLER_H_INCLUDED

#include <cnoid/LazyCaller>
#include <cnoid/ConnectionSet>
#include <cnoid/SceneView>

#include "TeachingTypes.h"
#include "TeachingDataHolder.h"

#include "TaskInstanceView.h"
#include "FlowView.h"
#include "MetaDataView.h"
#include "StateMachineView.h"
#include "ParameterView.h"

#include "ModelDialog.h"
#include "ParameterDialog.h"
#include "ArgumentDialog.h"

#include "TaskExecuteManager.h"
#include "exportdecl.h"

using namespace cnoid;

#define EVENT_HANDLER( exp ) TeachingEventHandler::instance()->exp

namespace teaching {

class CNOID_EXPORT TeachingEventHandler {
public:
  static TeachingEventHandler* instance();
	~TeachingEventHandler();

  inline bool canEdit() const { return this->canEdit_; }
  inline bool isAllModelDisp() const { return this->allModelDisp_; }
  inline void setComCurrentTask(TaskModelParamPtr param) { this->com_CurrentTask_ = param; }

	//TaskInstanceView
	void tiv_Loaded(TaskInstanceViewImpl* view);
  void tiv_TaskSelectionChanged(int selectedId, QString strTask);
	bool tiv_DeleteTaskClicked(int selectedId);
	void tiv_TaskExportClicked(int selectedId, QString strTask);
	bool tiv_TaskImportClicked();
        bool tiv_TaskImport(QString strFName);
	void tiv_SearchClicked(QString cond);
	bool tiv_RegistTaskClicked(int selectedId, QString strTask);
	bool tiv_RegistNewTaskClicked(int selectedId, QString strTask, QString strCond);

	void tiv_InitPosClicked();

	void tiv_SearchTaskInstance(QString cond);

	//FlowView
	void flv_Loaded(FlowViewImpl* view);
	void flv_NewFlowClicked();
	void flv_SearchClicked(bool canEdit);
	void flv_SelectionChanged(TaskModelParamPtr target);
	bool flv_RegistFlowClicked(QString name, QString comment);
	void flv_DeleteTaskClicked();
	void flv_FlowExportClicked(QString name, QString comment);
	void flv_FlowImportClicked();
	bool flv_RunFlowClicked();
	bool flv_InitPosClicked();
  void flv_EditClicked(ElementStmParamPtr target);
  void flv_ModelParamChanged(int flowModelId, ModelMasterParamPtr masterParam);
  bool flv_Connected(QtNodes::Connection& target);
  void flv_Disconnected(QtNodes::Connection& target);
  void flv_PortDispSetting(bool isActive);
  void flv_AllModelDisp(bool checked);
  void flv_HideAllModels();
  
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
	void stv_EditClicked(ElementStmParamPtr target);

	//ParameterView
	void prv_Loaded(ParameterViewImpl* view);
	void prv_SetInputValues();

	//TaskExecutionView
	void tev_setBreak(bool value) { executor_->setBreak(value); }
	void tev_stm_RunClicked(ElementStmParamPtr target);
	void tev_stm_StepClicked();
	void tev_stm_ContClicked();
	void tev_RunTaskClicked(int selectedId, bool isFlow);
	void tev_AbortClicked();

	//FlowSearchDialog
	void fsd_Loaded(FlowSearchDialog* dialog);
	void fsd_SearchClicked(QString condition);
	bool fsd_DeleteClicked(int targetId);
	void fsd_OKClicked(int targetId);

	//ParameterDialog
	void prd_Loaded(ParameterDialog* dialog);
  void prd_ParamSelectionChanged(int newId, QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide);
  void prd_AddParamClicked(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide);
	bool prd_DeleteParamClicked();
  bool prd_OkClicked(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide);
  void prd_ModelTableSelectionChanged(int selectedId);
  vector<ModelParameterParamPtr> prd_ModelSelectionChanged(int selectedId);
	void prd_AddFPClicked(QString name);

	//ModelDialog
	bool mdd_Loaded(ModelDialog* dialog);
	void mdd_ModelSelectionChanged(int newId, QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ, int hide);
	void mdd_ModelMasterSelectionChanged(int newId);
	void mdd_CurrentBodyItemChanged(BodyItem* bodyItem);
	void mdd_updateKinematicState(bool blockSignals);
	void mdd_ModelPositionChanged(double posX, double posY, double posZ, double rotX, double rotY, double rotZ);
	bool mdd_AddModelClicked();
	bool mdd_DeleteModelClicked();
  bool mdd_CheckModel(QString target);
  void mdd_OkClicked(QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ, int hide);
	void mdd_CancelClicked();

	//ArgumentDialog
	void agd_ModelSelectionChanged(int selectedId);
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

  void updateExecState(bool isActive);

  ////For Test
  inline void setTest(bool value) { this->isTest = value; }
  inline bool checkTest() const { return !this->isTest; }
	//FlowView
  bool flv_ConnectNodes(QString from, QString fromPort, QString to, QString toPort);
  bool tst_DeleteAllFlows();
  bool tst_DeleteAllTasks();
  void tst_ClearFlowScene();
  bool flv_RenameNode(QString currentName, QString newName);
  bool flv_CreateNode(QString modelName, QPoint posView);

private:
	TeachingEventHandler() 
		: canEdit_(false), 
      flv_(0), flv_CurrentId_(NULL_ID), flv_CurrentFlow_(0), isFlowDeleted_(false), isFlowSkip_(false), allModelDisp_(false),
			tiv_(0), tiv_CurrentTask_(0),
			com_CurrentTask_(0), com_CurrParam_(0),
			mdv_(0), m_FigDialog_(0),
			stv_(0),
			prv_(0),
			fsd_(0),
			mdd_(0), mdd_CurrentId_(NULL_ID), mdd_CurrentModel_(0),
			mdd_CurrentMasterId_(NULL_ID), mdd_CurrentModelMaster_(0),
			mdd_updateKinematicStateLater(bind(&TeachingEventHandler::mdd_updateKinematicState, this, true), IDLE_PRIORITY_LOW),
			prd_(0), prd_CurrentParam_(0),
			agd_(0), agd_Current_Stm_(0), agd_Current_Arg_(0), agd_Current_Action_(0),
		  executor_(0),
      isTest(false),
      updateEditStateLater(bind(&TeachingEventHandler::updateEditState, this, true), IDLE_PRIORITY_LOW) {
    connectionToEditStateChanged = SceneView::instance()->sceneWidget()->sigStateChanged().connect(updateEditStateLater);
  };

	bool eventSkip_ = false;

	TaskInstanceViewImpl* tiv_;
	TaskModelParamPtr tiv_CurrentTask_;

	FlowViewImpl* flv_;
	int flv_CurrentId_;
	FlowParamPtr flv_CurrentFlow_;
	bool isFlowDeleted_;
  bool isFlowSkip_;
  bool allModelDisp_;

	TaskModelParamPtr com_CurrentTask_;
	ElementStmParamPtr com_CurrParam_;

	MetaDataViewImpl* mdv_;
	FigureDialog* m_FigDialog_;

	StateMachineViewImpl* stv_;

	ParameterViewImpl* prv_;

	FlowSearchDialog* fsd_;

	ModelDialog* mdd_;
	int mdd_CurrentId_;
	ModelParamPtr mdd_CurrentModel_;
	int mdd_CurrentMasterId_;
	ModelMasterParamPtr mdd_CurrentModelMaster_;
	ModelParamPtr mdd_selectedModel_;
	BodyItemPtr mdd_BodyItem_;
	cnoid::Connection mdd_connectionToKinematicStateChanged;
	cnoid::Connection mdd_currentBodyItemChangeConnection;
	LazyCaller mdd_updateKinematicStateLater;

	ParameterDialog* prd_;
	ParameterParamPtr prd_CurrentParam_;

	ArgumentDialog* agd_;
	ElementStmParamPtr agd_Current_Stm_;
	ArgumentParamPtr agd_Current_Arg_;
	ElementStmActionParamPtr agd_Current_Action_;

	TaskExecuteManager* executor_;
  bool canEdit_;

	//ParameterDialog
  void prd_UpdateParam(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide);

	void unloadTaskModelItems();
	void updateComViews(TaskModelParamPtr targetTask, bool isFlowView=false);

  cnoid::Connection connectionToEditStateChanged;
  cnoid::LazyCaller updateEditStateLater;
  void updateEditState(bool blockSignals);

  //For Test
  bool isTest;
};

}
#endif
