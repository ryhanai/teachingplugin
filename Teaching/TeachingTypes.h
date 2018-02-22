#ifndef TEACHING_TEACHING_TYPES_H_INCLUDED
#define TEACHING_TEACHING_TYPES_H_INCLUDED

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "QtUtil.h"
#include <cnoid/BodyItem>
#include "CommandDefTypes.h"
#include "NodeEditor/Node.hpp"

namespace QtNodes {
	class Node;
}

namespace teaching {
 
static const double DBL_DELTA = 0.0000001;

inline bool dbl_eq(double d1, double d2) {
  if (-DBL_DELTA < (d1 - d2) && (d1 - d2) < DBL_DELTA)
    return true;
  else
    return false;
};

class ElementStmParam;
class ConnectionStmParam;
class TaskModelParam;

static const double PI = 3.14159265358979323846;
static const int NULL_ID = -1;
static const int MESSAGE_PERIOD = 3000;

typedef std::shared_ptr<ElementStmParam> ElementStmParamPtr;
typedef std::shared_ptr<ConnectionStmParam> ConnectionStmParamPtr;
typedef std::shared_ptr<TaskModelParam> TaskModelParamPtr;

enum ModelType {
  MODEL_ENV = 0,
  MODEL_EE,
  MODEL_WORK
};

enum ActionType {
  ACTION_ATTACH = 0,
  ACTION_DETACH
};

enum ElementType {
  ELEMENT_START = 1,
  ELEMENT_FINAL,
  ELEMENT_DECISION,
  ELEMENT_FORK,
  ELEMENT_COMMAND,
  ELEMENT_MERGE
};

enum ParameterKind {
  PARAM_KIND_NORMAL = 0,
  PARAM_KIND_MODEL
};

enum DatabaseMode {
  DB_MODE_NORMAL = 1,
  DB_MODE_UPDATE,
  DB_MODE_DELETE,
  DB_MODE_INSERT,
  DB_MODE_IGNORE
};

enum ExecResult {
  EXEC_FINISHED = 0,
  EXEC_ERROR,
  EXEC_BREAK
};

enum LogLevel {
  LOG_NO = 0,
  LOG_ERROR,
  LOG_DEBUG
};

enum FlowCoonectionType {
  TYPE_TRANSITION = 0,
  TYPE_MODEL_PARAM,
  TYPE_FLOW_PARAM
};

class DatabaseParam {
public:
  DatabaseParam(int id) : mode_(DB_MODE_NORMAL), id_(id) {};
  DatabaseParam(const DatabaseParam* source) : mode_(source->mode_), id_(source->id_) {};

  inline int getMode() const { return this->mode_; }

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  void setNew();
  void setNewForce();
  void setUpdate();
  void setDelete();
  void setIgnore();
  void setNormal();

protected:
  int id_;
  DatabaseMode mode_;
};
/////
class ModelDetailParam : public DatabaseParam {
public:
  ModelDetailParam(int id, QString fileName) : fileName_(fileName), DatabaseParam(id){};
  ModelDetailParam(const ModelDetailParam* source)
    : fileName_(source->fileName_), data_(source->data_), DatabaseParam(source)
  {};


  inline QString getFileName() const { return this->fileName_; }
  inline void setFileName(QString value) {
    if (this->fileName_ != value) {
      this->fileName_ = value;
      setUpdate();
    }
  }

  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

private:
  QString fileName_;
  QByteArray data_;
};
typedef std::shared_ptr<ModelDetailParam> ModelDetailParamPtr;

/////
class ModelParameterParam : public DatabaseParam {
public:
	ModelParameterParam(int masterId, int id, QString name, QString valueDesc)
		: master_id_(masterId), name_(name), valueDesc_(valueDesc), DatabaseParam(id) {
	};
	ModelParameterParam(const ModelParameterParam* source)
		: master_id_(source->master_id_), 
		name_(source->name_), valueDesc_(source->valueDesc_),
    DatabaseParam(source) {
	};

	inline int getMasterId() const { return this->master_id_; }

	inline QString getName() const { return this->name_; }
	inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }

	inline QString getValueDesc() const { return this->valueDesc_; }
	inline void setValueDesc(QString value) {
    if (this->valueDesc_ != value) {
      this->valueDesc_ = value;
      setUpdate();
    }
  }

private:
	int master_id_;
	QString name_;
	QString valueDesc_;
};
typedef std::shared_ptr<ModelParameterParam> ModelParameterParamPtr;
/////
class ModelMasterParam : public DatabaseParam {
public:
	ModelMasterParam(int id, QString name, QString fileName)
		: name_(name), fileName_(fileName), item_(0), isLoaded_(false), DatabaseParam(id) {};
	ModelMasterParam(const ModelMasterParam* source)
		: name_(source->name_), fileName_(source->fileName_), data_(source->data_),
		  item_(source->item_),
      imageFileName_(source->imageFileName_), image_(source->image_), isLoaded_(source->isLoaded_), DatabaseParam(source)
	{
		for (unsigned int index = 0; index < source->modelDetailList_.size(); index++) {
			ModelDetailParam* param = new ModelDetailParam(source->modelDetailList_[index].get());
			param->setNewForce();
			ModelDetailParamPtr paramPtr(param);
			this->modelDetailList_.push_back(paramPtr);
		}
		for (unsigned int index = 0; index < source->modelParameterList_.size(); index++) {
			ModelParameterParam* param = new ModelParameterParam(source->modelParameterList_[index].get());
			param->setNewForce();
			ModelParameterParamPtr paramPtr(param);
			this->modelParameterList_.push_back(paramPtr);
		}
	};

	inline int getOrgId() const { return this->orgId_; }
	inline void setOrgId(int value) { this->orgId_ = value; }

	inline QString getName() const { return this->name_; }
	inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }

	inline QString getFileName() const { return this->fileName_; }
	inline void setFileName(QString value) {
    if (this->fileName_ != value) {
      this->fileName_ = value;
      setUpdate();
    }
  }

	inline void setData(QByteArray value) { this->data_ = value; }
	inline QByteArray getData() const { return this->data_; }

  inline QString getHash() const { return this->hash_; }
  inline void setHash(QString value) {
    if (this->hash_ != value) {
      this->hash_ = value;
      setUpdate();
    }
  }

  inline std::vector<ModelDetailParamPtr> getModelDetailList() const { return this->modelDetailList_; }
	inline void addModelDetail(ModelDetailParamPtr target) { this->modelDetailList_.push_back(target); }

	inline std::vector<ModelParameterParamPtr> getModelParameterList() const { return this->modelParameterList_; }
	inline void addModelParameter(ModelParameterParamPtr target) { this->modelParameterList_.push_back(target); }

	inline void setModelItem(cnoid::BodyItemPtr value) { this->item_ = value; }
	inline cnoid::BodyItemPtr getModelItem() const { return this->item_; }

  inline QString getImageFileName() const { return this->imageFileName_; }
  inline void setImageFileName(QString value) {
    if (this->imageFileName_ != value) {
      this->imageFileName_ = value;
      setUpdate();
    }
  }

  inline void setImage(QImage value) { this->image_ = value; this->isLoaded_ = true; }
  inline QImage getImage() const { return this->image_; }
  inline void setRawData(QByteArray value) { this->rawData_ = value; }
  inline QByteArray getRawData() const { return this->rawData_; }
  void loadData();
  void deleteModelDetails();
	std::vector<ModelParameterParamPtr> getActiveParamList();

private:
	int orgId_;
	QString name_;
	QString fileName_;
	QByteArray data_;
  QString hash_;

	std::vector<ModelDetailParamPtr> modelDetailList_;
	std::vector<ModelParameterParamPtr> modelParameterList_;
	cnoid::BodyItemPtr item_;

  QString imageFileName_;
  QImage image_;
  QByteArray rawData_;
  bool isLoaded_;

  QImage db2Image(const QString& name, const QByteArray& source);
};
typedef std::shared_ptr<ModelMasterParam> ModelMasterParamPtr;

/////
class ModelParam : public DatabaseParam {
public:
  ModelParam(int id, int master_id, int type, QString rname, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz, bool isNew)
    : master_id_(master_id), type_(type), rname_(rname),
    posX_(posX), posY_(posY), posZ_(posZ), rotRx_(rotRx), rotRy_(rotRy), rotRz_(rotRz),
    orgPosX_(posX), orgPosY_(posY), orgPosZ_(posZ), orgRotRx_(rotRx), orgRotRy_(rotRy), orgRotRz_(rotRz),
		master_(0), DatabaseParam(id) {
    if (isNew) setNew();
  };
  ModelParam(const ModelParam* source)
    : master_id_(source->master_id_), type_(source->type_), rname_(source->rname_),
    posX_(source->posX_), posY_(source->posY_), posZ_(source->posZ_),
    rotRx_(source->rotRx_), rotRy_(source->rotRy_), rotRz_(source->rotRz_),
    orgPosX_(source->orgPosX_), orgPosY_(source->orgPosY_), orgPosZ_(source->orgPosZ_),
    orgRotRx_(source->orgRotRx_), orgRotRy_(source->orgRotRy_), orgRotRz_(source->orgRotRz_),
		master_(source->master_),	DatabaseParam(source)
  {
  };

	inline int getMasterId() const { return this->master_id_; }
	inline void setMasterId(int value) {
    if (this->master_id_ != value) {
      this->master_id_ = value;
      setUpdate();
    }
  }

  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) {
    if (this->rname_ != value) {
      this->rname_ = value;
      setUpdate();
    }
  }

  inline int getType() const { return this->type_; }
  inline void setType(int value) {
    if (this->type_ != value) {
      this->type_ = value;
      setUpdate();
    }
  }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) {
    if (dbl_eq(this->posX_, value) == false) {
      this->posX_ = value;
      setUpdate();
    }
  }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) {
    if (dbl_eq(this->posY_, value) == false) {
      this->posY_ = value;
      setUpdate();
    }
  }
  inline double getPosZ() const { return this->posZ_; }
  inline void setPosZ(double value) {
    if (dbl_eq(this->posZ_, value) == false) {
      this->posZ_ = value;
      setUpdate();
    }
  }
  inline double getRotRx() const { return this->rotRx_; }
  inline void setRotRx(double value) {
    if (dbl_eq(this->rotRx_, value) == false) {
      this->rotRx_ = value;
      setUpdate();
    }
  }
  inline double getRotRy() const { return this->rotRy_; }
  inline void setRotRy(double value) {
    if (dbl_eq(this->rotRy_, value) == false) {
      this->rotRy_ = value;
      setUpdate();
    }
  }
  inline double getRotRz() const { return this->rotRz_; }
  inline void setRotRz(double value) {
    if (dbl_eq(this->rotRz_, value) == false) {
      this->rotRz_ = value;
      setUpdate();
    }
  }

	inline void setModelMaster(ModelMasterParamPtr value) { this->master_ = value; }
	inline ModelMasterParamPtr getModelMaster() const { return this->master_; }

  void setInitialPos();
  bool isChangedPosition();

private:
	int master_id_;
	int type_;
  QString rname_;
  double posX_, posY_, posZ_;
  double rotRx_, rotRy_, rotRz_;
  double orgPosX_, orgPosY_, orgPosZ_;
  double orgRotRx_, orgRotRy_, orgRotRz_;
	ModelMasterParamPtr master_;
};
typedef std::shared_ptr<ModelParam> ModelParamPtr;
/////
class ArgumentParam : public DatabaseParam {
public:
  ArgumentParam(int id, int state_id, int seq, QString name, QString valueDesc)
    : state_id_(state_id), seq_(seq), name_(name), valueDesc_(valueDesc), DatabaseParam(id) {};
  ArgumentParam(const ArgumentParam* source)
    : state_id_(source->state_id_), seq_(source->seq_),
    name_(source->name_), valueDesc_(source->valueDesc_), DatabaseParam(source) {};

  inline int getStateId() const { return this->state_id_; }
  inline void setStateId(int value) {
    if (this->state_id_ != value) {
      this->state_id_ = value;
      setUpdate();
    }
  }
  inline int getSeq() const { return this->seq_; }
  inline void setSeq(int value) {
    if (this->seq_ != value) {
      this->seq_ = value;
      setUpdate();
    }
  }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }
  inline QString getValueDesc() const { return this->valueDesc_; }
  inline void setValueDesc(QString value) {
    if (this->valueDesc_ != value) {
      this->valueDesc_ = value;
      setUpdate();
    }
  }
  inline QString getValueDescOrg() const { return this->valueDescOrg_; }
  inline void setValueDescOrg(QString value) { this->valueDescOrg_ = value; }

private:
  int state_id_;
  int seq_;
  QString name_;
  QString valueDesc_;
  QString valueDescOrg_;
};
typedef std::shared_ptr<ArgumentParam> ArgumentParamPtr;

class ElementStmActionParam : public DatabaseParam {
public:
  ElementStmActionParam(int id, int stateId, int seq, QString action, QString model, QString target, bool isNew)
    : state_id_(stateId), seq_(seq), action_(action), model_(model), target_(target), DatabaseParam(id) {
    if (isNew) setNew();
  };
  ElementStmActionParam(const ElementStmActionParam* source)
    : state_id_(source->state_id_), seq_(source->seq_),
    action_(source->action_), model_(source->model_), target_(source->target_),
    DatabaseParam(source) {};

  inline int getStateId() const { return this->state_id_; }
  inline void setStateId(int value) {
    if (this->state_id_ != value) {
      this->state_id_ = value;
      setUpdate();
    }
  }
  inline int getSeq() const { return this->seq_; }
  inline void setSeq(int value) {
    if (this->seq_ != value) {
      this->seq_ = value;
      setUpdate();
    }
  }

  inline QString getAction() const { return this->action_; }
  inline void setAction(QString value) {
    if (this->action_ != value) {
      this->action_ = value;
      setUpdate();
    }
  }
  inline QString getModel() const { return this->model_; }
  inline void setModel(QString value) {
    if (this->model_ != value) {
      this->model_ = value;
      setUpdate();
    }
  }
  inline QString getTarget() const { return this->target_; }
  inline void setTarget(QString value) {
    if (this->target_ != value) {
      this->target_ = value;
      setUpdate();
    }
  }

  inline ModelParamPtr getModelParam() const { return this->targetModel_; }
  inline void setModelParam(ModelParamPtr value) { this->targetModel_ = value; }

private:
  int state_id_;
  int seq_;
  QString action_;
  QString model_;
  QString target_;

	ModelParamPtr targetModel_;
};
typedef std::shared_ptr<ElementStmActionParam> ElementStmActionParamPtr;

class ElementStmParam : public DatabaseParam {
public:
  ElementStmParam(int id, int type, QString cmdName, QString cmdDspName, double posX, double posY, QString condition);
  ElementStmParam(const ElementStmParamPtr source);
  virtual ~ElementStmParam();

  inline int getType() const { return this->type_; }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) {
    if (dbl_eq(this->posX_, value) == false) {
      this->posX_ = value;
      setUpdate();
    }
  }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) {
    if (dbl_eq(this->posY_, value) == false) {
      this->posY_ = value;
      setUpdate();
    }
  }

  inline QString getCmdName() const { return this->cmdName_; }
  inline void setCmdName(QString value) { this->cmdName_ = value; }
  inline QString getCmdDspName() const { return this->cmdDspName_; }
  inline void setCmdDspName(QString value) {
    if (this->cmdDspName_ != value) {
      this->cmdDspName_ = value;
      setUpdate();
    }
  }
  inline QString getCondition() const { return this->condition_; }
  inline void setCondition(QString value) {
    if (this->condition_ != value) {
      this->condition_ = value;
      setUpdate();
    }
  }

  inline std::vector<ElementStmActionParamPtr> getActionList() const { return this->actionList_; }
  inline void addModelAction(ElementStmActionParamPtr target){ this->actionList_.push_back(target); }
	std::vector<ElementStmActionParamPtr> getActiveStateActionList();
	ElementStmActionParamPtr getStateActionById(int id);

  inline std::vector<ArgumentParamPtr> getArgList() const { return this->argList_; }
  inline void addArgument(ArgumentParamPtr target){ this->argList_.push_back(target); }
	std::vector<ArgumentParamPtr> getActiveArgumentList();
	ArgumentParamPtr getArgumentById(int id);

  inline void setRealElem(QtNodes::Node* elem) { this->realElem_ = elem; }
  inline QtNodes::Node* getRealElem() const { return this->realElem_; }

  inline void setNextElem(ElementStmParamPtr elem) { this->nextElem_ = elem; }
  inline ElementStmParamPtr getNextElem() const { return this->nextElem_; }
  inline void setTrueElem(ElementStmParamPtr elem) { this->trueElem_ = elem; }
  inline ElementStmParamPtr getTrueElem() const { return this->trueElem_; }
  inline void setFalseElem(ElementStmParamPtr elem) { this->falseElem_ = elem; }
  inline ElementStmParamPtr getFalseElem() const { return this->falseElem_; }

  inline CommandDefParam* getCommadDefParam() const { return this->commandDef_; }
  inline void setCommadDefParam(CommandDefParam* value) { this->commandDef_ = value; }

  inline TaskModelParamPtr getTaskParam() const { return this->taskParam_; }
  inline void setTaskParam(TaskModelParamPtr value) { this->taskParam_ = value; }

  inline void setBreak(bool isBreak) { this->isBreak_ = isBreak; }
  inline bool isBreak() { return this->isBreak_; }

  inline ConnectionStmParamPtr getParentConn() const { return this->parentConn_; }
  inline void setParentConn(ConnectionStmParamPtr value) { this->parentConn_ = value; }

  void clearNextElems() {
    this->nextElem_ = 0;
    this->trueElem_ = 0;
    this->falseElem_ = 0;
  }
  void updateActive(bool isActive);
  void clearActionList();
	void updatePos();

private:
  int type_;
  double posX_;
  double posY_;

  QString cmdName_;
  QString cmdDspName_;
  QString condition_;

	QtNodes::Node* realElem_;

  std::vector<ElementStmActionParamPtr> actionList_;
  std::vector<ArgumentParamPtr> argList_;

	ElementStmParamPtr nextElem_;
	ElementStmParamPtr trueElem_;
	ElementStmParamPtr falseElem_;

	ConnectionStmParamPtr parentConn_;

  CommandDefParam* commandDef_;
	TaskModelParamPtr taskParam_;

  bool isBreak_;
};

class ConnectionStmParam : public DatabaseParam {
public:
	ConnectionStmParam(int id, int type, int sourceId, int sourceIndex, int targetId, int targetIndex)
		: type_(type), sourceId_(sourceId), sourceIndex_(sourceIndex), targetId_(targetId), targetIndex_(targetIndex), DatabaseParam(id) {
	};
	ConnectionStmParam(const ConnectionStmParamPtr source);
	virtual ~ConnectionStmParam();

	inline int getSourceId() const { return this->sourceId_; }
	inline void setSourceId(int value) {
    if (this->sourceId_ != value) {
      this->sourceId_ = value;
      setUpdate();
    }
  }

  inline int getType() const { return this->type_; }
  inline void setType(int value) {
    if (this->type_ != value) {
      this->type_ = value;
      setUpdate();
    }
  }

  inline int getSourceIndex() const { return this->sourceIndex_; }
  inline void setSourceIndex(int value) {
    if (this->sourceIndex_ != value) {
      this->sourceIndex_ = value;
      setUpdate();
    }
  }
 
  inline int getTargetId() const { return this->targetId_; }
	inline void setTargetId(int value) {
    if (this->targetId_ != value) {
      this->targetId_ = value;
      setUpdate();
    }
  }

	inline int getTargetIndex() const { return this->targetIndex_; }
	inline void setTargetIndex(int value) {
    if (this->targetIndex_ != value) {
      this->targetIndex_ = value;
      setUpdate();
    }
  }

private:
  int type_;
  int sourceId_;
  int sourceIndex_;
  int targetId_;
	int targetIndex_;
};

class ParameterParam : public DatabaseParam {
public:
  ParameterParam(int id, int type, int elem_num, int task_inst_id, QString name, QString rname, QString unit, int model_id, int model_param_id, int hide)
    : type_(type), elem_num_(elem_num), parent_id_(task_inst_id), name_(name), rname_(rname), unit_(unit), model_id_(model_id), model_param_id_(model_param_id), hide_(hide), DatabaseParam(id)
  {};
  ParameterParam(ParameterParam* source);
	~ParameterParam();

  inline int getType() const { return this->type_; }
  inline void setType(int value) {
    if (this->type_ != value) {
      this->type_ = value;
      setUpdate();
    }
  }

  inline int getElemNum() const { return this->elem_num_; }
  inline void setElemNum(int value) {
    if (this->elem_num_ != value) {
      this->elem_num_ = value;
      setUpdate();
    }
  }

  inline int getParentId() const { return this->parent_id_; }
  inline void setParentId(int value) { this->parent_id_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }
  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) {
    if (this->rname_ != value) {
      this->rname_ = value;
      setUpdate();
    }
  }
  inline QString getUnit() const { return this->unit_; }
  inline void setUnit(QString value) {
    if (this->unit_ != value) {
      this->unit_ = value;
      setUpdate();
    }
  }

	inline int getHide() const { return this->hide_; }
	inline void setHide(int value) {
    if (this->hide_ != value) {
      this->hide_ = value;
      setUpdate();
    }
  }

  inline int getModelId() const { return this->model_id_; }
  inline void setModelId(int value) {
    if (this->model_id_ != value) {
      this->model_id_ = value;
      setUpdate();
    }
  }
  inline int getModelParamId() const { return this->model_param_id_; }
  inline void setModelParamId(int value) {
    if (this->model_param_id_ != value) {
      this->model_param_id_ = value;
      setUpdate();
    }
  }

  inline int getExecModelId() const { return this->exec_model_id_; }
  inline int getExecModelParamId() const { return this->exec_model_param_id_; }
  void setExecParamId(int modelId, int model_param_id) {
    this->exec_model_id_ = modelId;
    this->exec_model_param_id_ = model_param_id;
  }
  void updateExecParam() {
    this->exec_model_id_ = model_id_;
    this->exec_model_param_id_ = model_param_id_;
  }

  inline void addControl(QLineEdit* target) { this->controlList_.push_back(target); }
	inline int getControlNum() const { return this->controlList_.size(); }
	inline QLineEdit* getControl(int index) { return this->controlList_[index]; }

  std::string getValues(int index);
  void setValues(int index, QString source);
  double getNumValues(int index);
  void saveValues();
  void setDBValues(QString source);
  QString getDBValues();
  void clearControlList() { this->controlList_.clear(); }
  void setFlowValues(QString source);

private:
  int type_;
  int elem_num_;
  int parent_id_;
  int model_id_;
  int model_param_id_;
  QString name_;
  QString rname_;
  QString unit_;
	int hide_;

  int exec_model_id_;
  int exec_model_param_id_;

  std::vector<QLineEdit*> controlList_;
  std::vector<QString> valueList_;
};
typedef std::shared_ptr<ParameterParam> ParameterParamPtr;
////
class FileDataParam : public DatabaseParam {
public:
  FileDataParam(int id, int seq, QString name) : seq_(seq), name_(name), DatabaseParam(id) {};
  FileDataParam(FileDataParam* source)
    : seq_(source->seq_), name_(source->name_), data_(source->data_),
      DatabaseParam(source) {};

	inline QString getName() const { return this->name_; }

	inline void setSeq(int value) { this->seq_ = value; }
	inline int getSeq() const { return this->seq_; }

  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

private:
	int seq_;
	QString name_;
  QByteArray data_;
};
typedef std::shared_ptr<FileDataParam> FileDataParamPtr;

class ImageDataParam : public DatabaseParam {
public:
  ImageDataParam(int id, int seq, QString name)
		: seq_(seq), name_(name), isLoaded_(false), DatabaseParam(id) {};
  ImageDataParam(ImageDataParam* source)
    : seq_(source->seq_), name_(source->name_), data_(source->data_),
			isLoaded_(source->isLoaded_), DatabaseParam(source) {};

  inline QString getName() const { return this->name_; }

	inline void setSeq(int value) { this->seq_ = value; }
	inline int getSeq() const { return this->seq_; }

	inline void setData(QImage value) { this->data_ = value; this->isLoaded_ = true; }
  inline QImage getData() const { return this->data_; }
  inline void setRawData(QByteArray value) { this->rawData_ = value; }
  inline QByteArray getRawData() const { return this->rawData_; }
	void loadData();

private:
	int seq_;
	QString name_;
  QImage data_;
  QByteArray rawData_;
	bool isLoaded_;

	QImage db2Image(const QString& name, const QByteArray& source);
};
typedef std::shared_ptr<ImageDataParam> ImageDataParamPtr;
/////
class FlowModelParam : public DatabaseParam {
public:
  FlowModelParam(int id, int masterId, int masterParamId)
    : masterId_(masterId), masterParamId_(masterParamId), DatabaseParam(id) {};
  FlowModelParam(FlowModelParam* source)
    : masterId_(source->masterId_), masterParamId_(source->masterParamId_),
      posX_(source->posX_), posY_(source->posY_), realElem_(source->realElem_), DatabaseParam(source) {};
  ~FlowModelParam() {};

  inline int getMasterId() const { return this->masterId_; }
  inline void setMasterId(int value) {
    if (this->masterId_ != value) {
      this->masterId_ = value;
      setUpdate();
    }
  }

  inline int getMasterParamId() const { return this->masterParamId_; }
  inline void setMasterParamId(int value) {
    if (this->masterParamId_ != value) {
      this->masterParamId_ = value;
      setUpdate();
    }
  }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) {
    if (dbl_eq(this->posX_, value) == false) {
      this->posX_ = value;
      setUpdate();
    }
  }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) {
    if (dbl_eq(this->posY_, value) == false) {
      this->posY_ = value;
      setUpdate();
    }
  }

  inline void setRealElem(QtNodes::Node* elem) { this->realElem_ = elem; }
  inline QtNodes::Node* getRealElem() const { return this->realElem_; }

  void updatePos();

private:
  int masterId_;
  int masterParamId_;
  double posX_;
  double posY_;

  QtNodes::Node* realElem_;
};
typedef std::shared_ptr<FlowModelParam> FlowModelParamPtr;
/////
class FlowParameterParam : public DatabaseParam {
public:
  FlowParameterParam(int id, QString name, QString value)
    : name_(name), value_(value), DatabaseParam(id) {};
  FlowParameterParam(FlowParameterParam* source)
    : name_(source->name_), value_(source->value_),
    posX_(source->posX_), posY_(source->posY_), realElem_(source->realElem_), DatabaseParam(source) {};
  ~FlowParameterParam() {};

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }

  inline QString getValue() const { return this->value_; }
  inline void setValue(QString value) {
    if (this->value_ != value) {
      this->value_ = value;
      setUpdate();
    }
  }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) {
    if (dbl_eq(this->posX_, value) == false) {
      this->posX_ = value;
      setUpdate();
    }
  }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) {
    if (dbl_eq(this->posY_, value) == false) {
      this->posY_ = value;
      setUpdate();
    }
  }

  inline void setRealElem(QtNodes::Node* elem) { this->realElem_ = elem; }
  inline QtNodes::Node* getRealElem() const { return this->realElem_; }

  void updatePos();

private:
  QString name_;
  QString value_;
  double posX_;
  double posY_;

  QtNodes::Node* realElem_;
};
typedef std::shared_ptr<FlowParameterParam> FlowParameterParamPtr;
/////
class ActivityParam : public DatabaseParam {
public:
  ActivityParam(int id, QString name, QString comment, QString created_date, QString last_updated_date)
		: name_(name), comment_(comment), created_date_(created_date), last_updated_date_(last_updated_date), DatabaseParam(id) {
	};

  ActivityParam(const ActivityParam* source);
  virtual ~ActivityParam(){};

	inline QString getName() const { return this->name_; }
	inline void setName(QString value) {
    if (this->name_ != value) {
      this->name_ = value;
      setUpdate();
    }
  }

	inline QString getComment() const { return this->comment_; }
	inline void setComment(QString value) {
    if (this->comment_ != value) {
      this->comment_ = value;
      setUpdate();
    }
  }

	inline QString getCreatedDate() const { return this->created_date_; }
	inline void setCreatedDate(QString value) { this->created_date_ = value; }
	inline QString getLastUpdatedDate() const { return this->last_updated_date_; }
	inline void setLastUpdatedDate(QString value) { this->last_updated_date_ = value; }

	inline std::vector<ParameterParamPtr> getParameterList() const { return this->parameterList_; }
	inline void addParameter(ParameterParamPtr target) { this->parameterList_.push_back(target); }
	std::vector<ParameterParamPtr> getActiveParameterList();
	ParameterParamPtr getParameterById(int id);

	inline std::vector<ElementStmParamPtr> getStmElementList() const { return this->stmElemList_; }
  inline void addStmElement(ElementStmParamPtr target){ this->stmElemList_.push_back(target); }
	std::vector<ElementStmParamPtr> getActiveStateList();

  inline std::vector<ConnectionStmParamPtr> getStmConnectionList() const { return this->stmConnectionList_; }
  inline void addStmConnection(ConnectionStmParamPtr target){ this->stmConnectionList_.push_back(target); }
	std::vector<ConnectionStmParamPtr> getActiveTransitionList();

  inline QString getErrStr() const { return this->errContents_; }
  inline ElementStmParamPtr getStartParam() const { return this->startParam_; }

	int getMaxStateId();
	bool checkAndOrderStateMachine();
	void clearTransitionList();

protected:
	QString name_;
	QString comment_;
	QString created_date_;
	QString last_updated_date_;

	std::vector<ParameterParamPtr> parameterList_;
	std::vector<ElementStmParamPtr> stmElemList_;
  std::vector<ConnectionStmParamPtr> stmConnectionList_;

	ElementStmParamPtr startParam_;

  QString errContents_;
};
typedef std::shared_ptr<ActivityParam> ActivityParamPtr;
//////////
class TaskModelParam : public ActivityParam {
public:
  TaskModelParam(int id, QString name, QString comment, QString execEnv, int flow_id, QString created_date, QString last_updated_date);
  TaskModelParam(const TaskModelParam* source);
  virtual ~TaskModelParam();

  inline int getFlowId() const { return this->flow_id_; }
  inline void setFlowId(int value) { this->flow_id_ = value; }

  inline QString getExecEnv() const { return this->exec_env_; }
  inline void setExecEnv(QString value) { this->exec_env_ = value; }

  inline std::vector<ModelParamPtr> getModelList() const { return this->modelList_; }
  inline void addModel(ModelParamPtr target){ this->modelList_.push_back(target); }
	std::vector<ModelParamPtr> getActiveModelList();

  inline std::vector<FileDataParamPtr> getFileList() const { return this->fileList_; }
  inline void addFile(FileDataParamPtr target){ this->fileList_.push_back(target); }
	std::vector<FileDataParamPtr> getActiveFileList();
	FileDataParamPtr getFileById(int id);

  inline std::vector<ImageDataParamPtr> getImageList() const { return this->imageList_; }
  inline void addImage(ImageDataParamPtr target){ this->imageList_.push_back(target); }
	std::vector<ImageDataParamPtr> getActiveImageList();
	ImageDataParamPtr getImageById(int id);

  inline TaskModelParamPtr getNextTask() const { return this->nextTask_; }
  inline void setNextTask(TaskModelParamPtr value) { this->nextTask_ = value; }

  inline ElementStmParamPtr getStateParam() const { return this->stateParam_; }
  inline void setStateParam(ElementStmParamPtr value) { this->stateParam_ = value; }

  void setAllNewData();
  void clearParameterList();
  void clearDetailParams();

  inline bool IsLoaded() const { return this->isLoaded_; }
  inline void setLoaded(bool value) { this->isLoaded_ = value; }

  inline bool IsModelLoaded() const { return this->isModelLoaded_; }
  inline void setModelLoaded(bool value) { this->isModelLoaded_ = value; }

  void updateExecParam();

private:
  int flow_id_;
  QString exec_env_;

  bool isLoaded_;
  bool isModelLoaded_;

  std::vector<ModelParamPtr> modelList_;
  std::vector<FileDataParamPtr> fileList_;
  std::vector<ImageDataParamPtr> imageList_;

	TaskModelParamPtr nextTask_;
	ElementStmParamPtr stateParam_;
};

class FlowParam : public ActivityParam {
public:
  FlowParam(int id, QString name, QString comment, QString created_date, QString last_updated_date)
    : ActivityParam(id, name, comment, created_date, last_updated_date) {
  };
  FlowParam(const FlowParam* source);
  virtual ~FlowParam();

  inline std::vector<FlowModelParamPtr> getModelList() const { return this->modelList_; }
  inline void addModel(FlowModelParamPtr target) { this->modelList_.push_back(target); }

  inline std::vector<FlowParameterParamPtr> getFlowParamList() const { return this->paramList_; }
  inline void addFlowParam(FlowParameterParamPtr target) { this->paramList_.push_back(target); }

  int getMaxModelId();
  int getMaxParamId();
  void updateExecParam();

private:
  std::vector<FlowModelParamPtr> modelList_;
  std::vector<FlowParameterParamPtr> paramList_;

};
typedef std::shared_ptr<FlowParam> FlowParamPtr;

/////
struct ModelParamComparator {
  int id_;
  ModelParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ModelParamPtr elem) const {
    return (elem->getId() == id_
      && (elem->getMode() != DB_MODE_DELETE && elem->getMode() != DB_MODE_IGNORE));
  }
};

struct ModelParamComparatorByRName {
  QString rname_;
  ModelParamComparatorByRName(QString value) {
    rname_ = value;
  }
  bool operator()(const ModelParamPtr elem) const {
    return elem->getRName() == rname_;
  }
};

struct ModelMasterComparator {
  int id_;
  ModelMasterComparator(int value) {
    id_ = value;
  }
  bool operator()(const ModelMasterParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct ModelMasterParamComparator {
	int id_;
  ModelMasterParamComparator(int value) {
    id_ = value;
	}
	bool operator()(const ModelParameterParamPtr elem) const {
		return elem->getId() == id_;
	}
};

struct ElementStmParamComparator {
  int id_;
  ElementStmParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ElementStmParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct FlowModelParamComparator {
  int id_;
  FlowModelParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const FlowModelParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct FlowParameterParamComparator {
  int id_;
  FlowParameterParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const FlowParameterParamPtr elem) const {
    return elem->getId() == id_;
  }
};

}
#endif
