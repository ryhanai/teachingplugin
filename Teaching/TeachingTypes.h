#ifndef TEACHING_TEACHING_TYPES_H_INCLUDED
#define TEACHING_TEACHING_TYPES_H_INCLUDED

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "QtUtil.h"
#include <cnoid/BodyItem>
#include "CommandDefTypes.h"

namespace teaching {

class ElementNode;
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

enum TaskParamType {
  TASK_PARAM_NORMAL = 0,
  TASK_PARAM_MODEL
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
  ELEMENT_POINT
};

enum ParameterViewType {
	PARAM_VIEW_TASK = 1,
	PARAM_VIEW_FLOW
};

enum ParameterType {
  PARAM_SCALAR = 1,
  PARAM_VECTOR
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

class DatabaseParam {
public:
  DatabaseParam() : mode_(DB_MODE_NORMAL) {};
  DatabaseParam(const DatabaseParam* source) : mode_(source->mode_) {};
  inline int getMode() const { return this->mode_; }
  void setNew();
  void setNewForce();
  void setUpdate();
  void setDelete();
  void setIgnore();
  void setNormal();

protected:
  DatabaseMode mode_;
};
/////
class ModelDetailParam : public DatabaseParam {
public:
  ModelDetailParam(int id, QString fileName) : id_(id), fileName_(fileName) {};
  ModelDetailParam(const ModelDetailParam* source)
    : id_(source->id_), fileName_(source->fileName_), data_(source->data_), DatabaseParam(source)
  {};

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }
  inline QString getFileName() const { return this->fileName_; }
  inline void setFileName(QString value) { this->fileName_ = value; setUpdate(); }
  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

private:
  int id_;
  QString fileName_;
  QByteArray data_;
};
typedef std::shared_ptr<ModelDetailParam> ModelDetailParamPtr;

/////
class ModelMasterParam : public DatabaseParam {
public:
	ModelMasterParam(int id, QString name, QString fileName)
		: id_(id), name_(name), fileName_(fileName), item_(0) {};
	ModelMasterParam(const ModelMasterParam* source)
		: id_(source->id_), name_(source->name_), fileName_(source->fileName_), data_(source->data_),
		  item_(source->item_), DatabaseParam(source)
	{
		for (unsigned int index = 0; index < source->modeDetailList_.size(); index++) {
			ModelDetailParam* param = new ModelDetailParam(source->modeDetailList_[index].get());
			param->setNewForce();
			ModelDetailParamPtr paramPtr(param);
			this->modeDetailList_.push_back(paramPtr);
		}
	};

	inline int getId() const { return this->id_; }
	inline void setId(int value) { this->id_ = value; }

	inline int getOrgId() const { return this->orgId_; }
	inline void setOrgId(int value) { this->orgId_ = value; }

	inline QString getName() const { return this->name_; }
	inline void setName(QString value) { this->name_ = value; setUpdate(); }

	inline QString getFileName() const { return this->fileName_; }
	inline void setFileName(QString value) { this->fileName_ = value; setUpdate(); }

	inline void setData(QByteArray value) { this->data_ = value; }
	inline QByteArray getData() const { return this->data_; }

	inline std::vector<ModelDetailParamPtr> getModelDetailList() const { return this->modeDetailList_; }
	inline void addModelDetail(ModelDetailParamPtr target) { this->modeDetailList_.push_back(target); }

	inline void setModelItem(cnoid::BodyItemPtr value) { this->item_ = value; }
	inline cnoid::BodyItemPtr getModelItem() const { return this->item_; }

	void deleteModelDetails();

private:
	int id_;
	int orgId_;
	QString name_;
	QString fileName_;
	QByteArray data_;
	std::vector<ModelDetailParamPtr> modeDetailList_;
	cnoid::BodyItemPtr item_;
};
typedef std::shared_ptr<ModelMasterParam> ModelMasterParamPtr;

/////
class ModelParam : public DatabaseParam {
public:
  ModelParam(int id, int master_id, int type, QString name, QString rname, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz, bool isNew)
    : id_(id), master_id_(master_id), type_(type), name_(name), rname_(rname),
    posX_(posX), posY_(posY), posZ_(posZ), rotRx_(rotRx), rotRy_(rotRy), rotRz_(rotRz),
    orgPosX_(posX), orgPosY_(posY), orgPosZ_(posZ), orgRotRx_(rotRx), orgRotRy_(rotRy), orgRotRz_(rotRz),
		master_(0) {
    if (isNew) setNew();
  };
  ModelParam(const ModelParam* source)
    : id_(source->id_), master_id_(source->master_id_), type_(source->type_), name_(source->name_), rname_(source->rname_),
    posX_(source->posX_), posY_(source->posY_), posZ_(source->posZ_),
    rotRx_(source->rotRx_), rotRy_(source->rotRy_), rotRz_(source->rotRz_),
    orgPosX_(source->orgPosX_), orgPosY_(source->orgPosY_), orgPosZ_(source->orgPosZ_),
    orgRotRx_(source->orgRotRx_), orgRotRy_(source->orgRotRy_), orgRotRz_(source->orgRotRz_),
		master_(source->master_),	DatabaseParam(source)
  {
  };

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

	inline int getMasterId() const { return this->master_id_; }
	inline void setMasterId(int value) { this->master_id_ = value; }

	inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; setUpdate(); }
  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) { this->rname_ = value; setUpdate(); }

  inline int getType() const { return this->type_; }
  inline void setType(int value) { this->type_ = value; setUpdate(); }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) { this->posX_ = value; setUpdate(); }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) { this->posY_ = value; setUpdate(); }
  inline double getPosZ() const { return this->posZ_; }
  inline void setPosZ(double value) { this->posZ_ = value; setUpdate(); }
  inline double getRotRx() const { return this->rotRx_; }
  inline void setRotRx(double value) { this->rotRx_ = value; setUpdate(); }
  inline double getRotRy() const { return this->rotRy_; }
  inline void setRotRy(double value) { this->rotRy_ = value; setUpdate(); }
  inline double getRotRz() const { return this->rotRz_; }
  inline void setRotRz(double value) { this->rotRz_ = value; setUpdate(); }

  inline double getOrgPosX() const { return this->orgPosX_; }
  inline double getOrgPosY() const { return this->orgPosY_; }
  inline double getOrgPosZ() const { return this->orgPosZ_; }
  inline double getOrgRotRx() const { return this->orgRotRx_; }
  inline double getOrgRotRy() const { return this->orgRotRy_; }
  inline double getOrgRotRz() const { return this->orgRotRz_; }

	inline void setModelMaster(ModelMasterParamPtr value) { this->master_ = value; }
	inline ModelMasterParamPtr getModelMaster() const { return this->master_; }

  void setInitialPos();
  bool isChangedPosition();

private:
  int id_;
	int master_id_;
	int type_;
  QString name_;
  QString rname_;
  double posX_, posY_, posZ_;
  double rotRx_, rotRy_, rotRz_;
  double orgPosX_, orgPosY_, orgPosZ_;
  double orgRotRx_, orgRotRy_, orgRotRz_;
	ModelMasterParamPtr master_;
};
typedef std::shared_ptr<ModelParam> ModelParamPtr;
/////
class ConnectionNode : public QGraphicsItemGroup {
public:
  explicit ConnectionNode(double sourceX, double sourceY, double targetX, double targetY);

  inline void setSource(ElementNode* elem) { this->source = elem; }
  inline void setTarget(ElementNode* elem) { this->target = elem; }
  inline ElementNode* getSource() { return this->source; }
  inline ElementNode* getTarget() { return this->target; }

  inline void setConnParam(ConnectionStmParamPtr elem) { this->parentElem_ = elem; }
  inline ConnectionStmParamPtr getConnParam() { return this->parentElem_; }

  inline QString getCondition() const { return this->condition; }

  void reDrawConnection();
  void setLine(double sourceX, double sourceY, double targetX, double targetY);
  void setPen(QPen target);
  void setText(QString target);

private:
	ConnectionStmParamPtr parentElem_;
  QGraphicsLineItem* body;
  QGraphicsLineItem* lineRight;
  QGraphicsLineItem* lineLeft;
  QGraphicsSimpleTextItem* text;
  ElementNode* source;
  ElementNode* target;
  QString condition;
};

class ElementNode : public QGraphicsItemGroup {
public:
  explicit ElementNode(QString target);
  explicit ElementNode(int type, QString cmdName);
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  inline void addConnection(ConnectionNode* target) { this->lineList_.push_back(target); }
  inline ConnectionNode* getCurrentConnection() { return this->lineList_[this->lineList_.size() - 1]; }
  inline void removeCurrentConnection() { this->lineList_.pop_back(); }
  inline void removeTargetConnection(ConnectionNode* target) {
    this->lineList_.erase(std::remove(this->lineList_.begin(), this->lineList_.end(), target), this->lineList_.end());
  }
  inline std::vector<ConnectionNode*> getConnectionList() { return this->lineList_; }
  inline ElementType getElementType() { return this->type_; }

  inline void setElemParam(ElementStmParamPtr elem) { this->parentElem_ = elem; }
  inline ElementStmParamPtr getElemParam() { return this->parentElem_; }

  void setBreak(bool isBreak);
  inline bool isBreak() { return this->isBreak_; }

  void setItemText(QString cmdName) {
    itemText_->setText(cmdName);
  }

  void updatePosition(double x, double y);
  void updateActive(bool isActive);
  void updateSelect(bool isActive);

private:
  ElementType type_;
  std::vector<ConnectionNode*> lineList_;
	ElementStmParamPtr parentElem_;
  QAbstractGraphicsShapeItem* item_;
  QGraphicsSimpleTextItem* itemText_;
  bool isBreak_;
  bool isActive_;

  void createStartNode();
  void createFinalNode();
  void createDecisionNode();
  void createForkNode();
  void createPoint();
  void createCommandNode(QString name);
};
/////
class ArgumentParam : public DatabaseParam {
public:
  ArgumentParam(int id, int state_id, int seq, QString name, QString valueDesc)
    : id_(id), state_id_(state_id), seq_(seq), name_(name), valueDesc_(valueDesc) {};
  ArgumentParam(const ArgumentParam* source)
    : id_(source->id_), state_id_(source->state_id_), seq_(source->seq_),
    name_(source->name_), valueDesc_(source->valueDesc_), DatabaseParam(source) {};

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getStateId() const { return this->state_id_; }
  inline void setStateId(int value) { this->state_id_ = value; }
  inline int getSeq() const { return this->seq_; }
  inline void setSeq(int value) { this->seq_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; }
  inline QString getValueDesc() const { return this->valueDesc_; }
  inline void setValueDesc(QString value) { this->valueDesc_ = value; setUpdate(); }
  inline QString getValueDescOrg() const { return this->valueDescOrg_; }
  inline void setValueDescOrg(QString value) { this->valueDescOrg_ = value; }

private:
  int id_;
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
    : id_(id), state_id_(stateId), seq_(seq), action_(action), model_(model), target_(target) {
    if (isNew) setNew();
  };
  ElementStmActionParam(const ElementStmActionParam* source)
    : id_(source->id_), state_id_(source->state_id_), seq_(source->seq_),
    action_(source->action_), model_(source->model_), target_(source->target_),
    DatabaseParam(source) {};

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getStateId() const { return this->state_id_; }
  inline void setStateId(int value) { this->state_id_ = value; }
  inline int getSeq() const { return this->seq_; }
  inline void setSeq(int value) { this->seq_ = value; setUpdate(); }

  inline QString getAction() const { return this->action_; }
  inline void setAction(QString value) { this->action_ = value; setUpdate(); }
  inline QString getModel() const { return this->model_; }
  inline void setModel(QString value) { this->model_ = value; setUpdate(); }
  inline QString getTarget() const { return this->target_; }
  inline void setTarget(QString value) { this->target_ = value; setUpdate(); }

  inline ModelParamPtr getModelParam() const { return this->targetModel_; }
  inline void setModelParam(ModelParamPtr value) { this->targetModel_ = value; }

private:
  int id_;
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

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getOrgId() const { return this->org_id_; }
  inline void setOrgId(int value) { this->org_id_ = value; }

  inline int getType() const { return this->type_; }

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) { this->posX_ = value; }
  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) { this->posY_ = value; }

  inline QString getCmdName() const { return this->cmdName_; }
  inline void setCmdName(QString value) { this->cmdName_ = value; }
  inline QString getCmdDspName() const { return this->cmdDspName_; }
  inline void setCmdDspName(QString value) { this->cmdDspName_ = value; setUpdate(); }
  inline QString getCondition() const { return this->condition_; }
  inline void setCondition(QString value) { this->condition_ = value; }

  inline std::vector<ElementStmActionParamPtr> getActionList() const { return this->actionList_; }
  inline void addModelAction(ElementStmActionParamPtr target){ this->actionList_.push_back(target); }
	std::vector<ElementStmActionParamPtr> getActiveStateActionList();
	ElementStmActionParamPtr getStateActionById(int id);

  inline std::vector<ArgumentParamPtr> getArgList() const { return this->argList_; }
  inline void addArgument(ArgumentParamPtr target){ this->argList_.push_back(target); }
	std::vector<ArgumentParamPtr> getActiveArgumentList();
	ArgumentParamPtr getArgumentById(int id);

  inline void setRealElem(ElementNode* elem) { this->realElem_ = elem; }
  inline ElementNode* getRealElem() const { return this->realElem_; }

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
  void updateId() {
    org_id_ = id_;
  }
  void updateActive(bool isActive);
  void updateSelect(bool isActive);
  void clearActionList();

private:
  int id_;
  int org_id_;
  int type_;
  double posX_;
  double posY_;

  QString cmdName_;
  QString cmdDspName_;
  QString condition_;

  ElementNode* realElem_;

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
  ConnectionStmParam(int id, int sourceId, int targetId, QString cond)
    : id_(id), sourceId_(sourceId), targetId_(targetId), condition_(cond) {};
  ConnectionStmParam(const ConnectionStmParamPtr source);
  virtual ~ConnectionStmParam();

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getSourceId() const { return this->sourceId_; }
  inline void setSourceId(int value) { this->sourceId_ = value; }

  inline int getTargetId() const { return this->targetId_; }
  inline void setTargetId(int value) { this->targetId_ = value; }

  inline QString getCondition() const { return this->condition_; }
  inline void setCondition(QString value) { this->condition_ = value; }

  void addChildNode(ElementStmParamPtr prev, ElementStmParamPtr target);
  void addChildNode(ElementStmParamPtr target);
  void removeChildNode(ElementStmParamPtr target);
  inline std::vector<ElementStmParamPtr> getChildList() const { return this->childList_; }

private:
  int id_;
  int sourceId_;
  int targetId_;
  QString condition_;
  std::vector<ElementStmParamPtr> childList_;
};

class ParameterParam : public DatabaseParam {
public:
  ParameterParam(int id, int type, QString model_name, int elem_num, QString elem_types, int task_inst_id, QString name, QString rname, QString unit, int hide)
    : id_(id), type_(type), model_name_(model_name), elem_num_(elem_num), elem_types_(elem_types),
    parent_id_(task_inst_id), name_(name), rname_(rname), unit_(unit), hide_(hide)
  {
    buildElemTypeList();
  };
  ParameterParam(ParameterParam* source);
	~ParameterParam();

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getType() const { return this->type_; }
  inline void setType(int value) { this->type_ = value; setUpdate(); }

  inline QString getModelName() const { return this->model_name_; }
  inline void setModelName(QString value) { this->model_name_ = value; setUpdate(); }

  inline int getElemNum() const { return this->elem_num_; }
  inline void setElemNum(int value) { this->elem_num_ = value; setUpdate(); }

  inline QString getElemTypes() const { return this->elem_types_; }
  void setElemTypes(QString value);

  inline int getParentId() const { return this->parent_id_; }
  inline void setParentId(int value) { this->parent_id_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; setUpdate(); }
  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) { this->rname_ = value; setUpdate(); }
  inline QString getUnit() const { return this->unit_; }
  inline void setUnit(QString value) { this->unit_ = value; setUpdate(); }

	inline int getHide() const { return this->hide_; }
	inline void setHide(int value) { this->hide_ = value; setUpdate(); }

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
  QString getElemTypeStr();
  int getElemTypeNo();

private:
  int id_;
  int type_;
  int elem_num_;
  int parent_id_;
  QString model_name_;
  QString name_;
  QString rname_;
  QString unit_;
  QString elem_types_;
	int hide_;

  std::vector<int> elemTypeList_;
  std::vector<QLineEdit*> controlList_;
  std::vector<QString> valueList_;

  void buildElemTypeList();
};
typedef std::shared_ptr<ParameterParam> ParameterParamPtr;
////
class FileDataParam : public DatabaseParam {
public:
  FileDataParam(int id, int seq, QString name) : id_(id), seq_(seq), name_(name) {};
  FileDataParam(FileDataParam* source)
    : id_(source->id_), seq_(source->seq_), name_(source->name_), data_(source->data_),
    DatabaseParam(source) {};

  inline int getId() const { return this->id_; }
	inline void setId(int value) { this->id_ = value; }

	inline QString getName() const { return this->name_; }

	inline void setSeq(int value) { this->seq_ = value; }
	inline int getSeq() const { return this->seq_; }

  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

private:
  int id_;
	int seq_;
	QString name_;
  QByteArray data_;
};
typedef std::shared_ptr<FileDataParam> FileDataParamPtr;

class ImageDataParam : public DatabaseParam {
public:
  ImageDataParam(int id, int seq, QString name)
		: id_(id), seq_(seq), name_(name), isLoaded_(false) {};
  ImageDataParam(ImageDataParam* source)
    : id_(source->id_), seq_(source->seq_), name_(source->name_), data_(source->data_),
			isLoaded_(source->isLoaded_), DatabaseParam(source) {};

  inline int getId() const { return this->id_; }
  inline QString getName() const { return this->name_; }

	inline void setSeq(int value) { this->seq_ = value; }
	inline int getSeq() const { return this->seq_; }

	inline void setData(QImage value) { this->data_ = value; this->isLoaded_ = true; }
  inline QImage getData() const { return this->data_; }
  inline void setRawData(QByteArray value) { this->rawData_ = value; }
  inline QByteArray getRawData() const { return this->rawData_; }
	void loadData();

private:
  int id_;
	int seq_;
	QString name_;
  QImage data_;
  QByteArray rawData_;
	bool isLoaded_;

	QImage db2Image(const QString& name, const QByteArray& source);

};
typedef std::shared_ptr<ImageDataParam> ImageDataParamPtr;

/////
class ActivityParam : public DatabaseParam {
public:
  ActivityParam(int id, QString name, QString comment, QString created_date, QString last_updated_date)
		: id_(id), name_(name), comment_(comment), created_date_(created_date), last_updated_date_(last_updated_date) {
	};

  ActivityParam(const ActivityParam* source);

	inline int getId() const { return this->id_; }
	inline void setId(int value) { this->id_ = value; }

	inline QString getName() const { return this->name_; }
	inline void setName(QString value) { this->name_ = value;  setUpdate(); }

	inline QString getComment() const { return this->comment_; }
	inline void setComment(QString value) { this->comment_ = value;  setUpdate(); }

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

  bool checkAndOrderStateMachine();

protected:
	int id_;
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
  FlowParam(const FlowParam* source) :
		ActivityParam(source) {};
  virtual ~FlowParam() {};
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

struct ElementStmParamComparator {
  int id_;
  ElementStmParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ElementStmParamPtr elem) const {
    return elem->getOrgId() == id_;
  }
};

}
#endif
