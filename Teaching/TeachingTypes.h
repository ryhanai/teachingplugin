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

class DatabaseParam {
public:
  DatabaseParam() : mode_(DB_MODE_NORMAL) {};
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
    : id_(source->id_), fileName_(source->fileName_), data_(source->data_)
  {  mode_ = DatabaseMode(source->getMode()); };

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
/////
class ModelParam : public DatabaseParam {
public:
  ModelParam(int id, int type, QString name, QString rname, QString fileName, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz, bool isNew)
    : id_(id), type_(type), name_(name), rname_(rname), fileName_(fileName),
      posX_(posX), posY_(posY), posZ_(posZ), rotRx_(rotRx), rotRy_(rotRy), rotRz_(rotRz),
      orgPosX_(posX), orgPosY_(posY), orgPosZ_(posZ), orgRotRx_(rotRx), orgRotRy_(rotRy), orgRotRz_(rotRz),
      item_(0) {
    if( isNew ) setNew();
  };
  ModelParam(const ModelParam* source)
    : id_(source->id_), type_(source->type_), name_(source->name_), rname_(source->rname_), fileName_(source->fileName_),
      data_(source->data_),
      posX_(source->posX_), posY_(source->posY_), posZ_(source->posZ_),
      rotRx_(source->rotRx_), rotRy_(source->rotRy_), rotRz_(source->rotRz_),
      orgPosX_(source->orgPosX_), orgPosY_(source->orgPosY_), orgPosZ_(source->orgPosZ_),
      orgRotRx_(source->orgRotRx_), orgRotRy_(source->orgRotRy_), orgRotRz_(source->orgRotRz_),
      item_(source->item_)
  {  mode_ = DatabaseMode(source->getMode()); };

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; setUpdate(); }
  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) { this->rname_ = value; setUpdate(); }

  inline int getType() const { return this->type_; }
  inline void setType(int value) { this->type_ = value; setUpdate(); }
  inline QString getFileName() const { return this->fileName_; }
  inline void setFileName(QString value) { this->fileName_ = value; setUpdate(); }

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

  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

  inline void setModelItem(cnoid::BodyItemPtr value) { this->item_ = value; }
  inline cnoid::BodyItemPtr getModelItem() const { return this->item_; }

  inline std::vector<ModelDetailParam*> getModelDetailList() const { return this->modeDetailList_; }
  inline void addModelDetail(ModelDetailParam* target){ this->modeDetailList_.push_back(target); }

  void deleteModelDetails();
  void setInitialPos();

private:
  int id_;
  int type_;
  QString name_;
  QString rname_;
  QString fileName_;
  double posX_, posY_, posZ_;
  double rotRx_, rotRy_, rotRz_;
  double orgPosX_, orgPosY_, orgPosZ_;
  double orgRotRx_, orgRotRy_, orgRotRz_;
  QByteArray data_;
  cnoid::BodyItemPtr item_;
  std::vector<ModelDetailParam*> modeDetailList_;
};
/////
class ConnectionNode : public QGraphicsItemGroup {
public:
  explicit ConnectionNode(double sourceX, double sourceY, double targetX, double targetY);

  inline void setSource(ElementNode* elem) { this->source = elem; }
  inline void setTarget(ElementNode* elem) { this->target = elem; }
  inline ElementNode* getSource() { return this->source; }
  inline ElementNode* getTarget() { return this->target; }

  inline void setConnParam(ConnectionStmParam* elem) { this->parentElem_ = elem; }
  inline ConnectionStmParam* getConnParam() { return this->parentElem_; }

  inline QString getCondition() const { return this->condition; }

  void reDrawConnection();
  void setLine(double sourceX, double sourceY, double targetX, double targetY);
  void setPen(QPen target);
  void setText(QString target);

private:
  ConnectionStmParam* parentElem_;
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
  inline ConnectionNode* getCurrentConnection() { return this->lineList_[this->lineList_.size()-1]; }
  inline void removeCurrentConnection() { this->lineList_.pop_back(); }
  inline void removeTargetConnection(ConnectionNode* target) {
    this->lineList_.erase(std::remove(this->lineList_.begin(), this->lineList_.end(), target), this->lineList_.end());
  }
  inline std::vector<ConnectionNode*> getConnectionList() { return this->lineList_; }
  inline ElementType getElementType() { return this->type_; }

  inline void setElemParam(ElementStmParam* elem) { this->parentElem_ = elem; }
  inline ElementStmParam* getElemParam() { return this->parentElem_; }

	void setBreak(bool isBreak);
	inline bool isBreak() { return this->isBreak_; }

	void updatePosition(double x, double y);
  void updateActive(bool isActive);
  void updateSelect(bool isActive);

private:
  ElementType type_;
  std::vector<ConnectionNode*> lineList_;
  ElementStmParam* parentElem_;
  QAbstractGraphicsShapeItem* item_;
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
    name_(source->name_), valueDesc_(source->valueDesc_) {};

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

private:
  int id_;
  int state_id_;
  int seq_;
  QString name_;
  QString valueDesc_;
};

class ElementStmActionParam : public DatabaseParam {
public:
  ElementStmActionParam(int id, int stateId, int seq, QString action, QString model, QString target, bool isNew)
   : id_(id), state_id_(stateId), seq_(seq), action_(action), model_(model), target_(target) {
   if( isNew ) setNew();
  };
  ElementStmActionParam(const ElementStmActionParam* source)
   : id_(source->id_), state_id_(source->state_id_), seq_(source->seq_),
     action_(source->action_), model_(source->model_), target_(source->target_) {
    mode_ = DatabaseMode(source->getMode());
  };

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

  inline ModelParam* getModelParam() const { return this->targetModel_; }
  inline void setModelParam(ModelParam* value) { this->targetModel_ = value; }

private:
  int id_;
  int state_id_;
  int seq_;
  QString action_;
  QString model_;
  QString target_;

  ModelParam* targetModel_;
};

class ElementStmParam : public DatabaseParam {
public:
	ElementStmParam(int id, int type, QString cmdName, QString cmdDspName, double posX, double posY);
  ElementStmParam(const ElementStmParam* source);
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
  inline void setCmdDspName(QString value) { this->cmdDspName_ = value; }

  inline std::vector<ElementStmActionParam*> getActionList() const { return this->actionList_; }
  inline void addModelAction(ElementStmActionParam* target){ this->actionList_.push_back(target); }

  inline std::vector<ArgumentParam*> getArgList() const { return this->argList_; }
  inline void addArgument(ArgumentParam* target){ this->argList_.push_back(target); }

  inline void setRealElem(ElementNode* elem) { this->realElem_ = elem; }
  inline ElementNode* getRealElem() const { return this->realElem_; }

  inline void setNextElem(ElementStmParam* elem) { this->nextElem_ = elem; }
  inline ElementStmParam* getNextElem() const { return this->nextElem_; }
  inline void setTrueElem(ElementStmParam* elem) { this->trueElem_ = elem; }
  inline ElementStmParam* getTrueElem() const { return this->trueElem_; }
  inline void setFalseElem(ElementStmParam* elem) { this->falseElem_ = elem; }
  inline ElementStmParam* getFalseElem() const { return this->falseElem_; }

  inline CommandDefParam* getCommadDefParam() const { return this->commandDef_; }
  inline void setCommadDefParam(CommandDefParam* value) { this->commandDef_ = value; }

	inline TaskModelParam* getTaskParam() const { return this->taskParam_; }
	inline void setTaskParam(TaskModelParam* value) {	this->taskParam_ = value;	}

	inline void setBreak(bool isBreak) { this->isBreak_ = isBreak; }
	inline bool isBreak() { return this->isBreak_; }

	inline ConnectionStmParam* getParentConn() const { return this->parentConn_; }
	inline void setParentConn(ConnectionStmParam* value) { this->parentConn_ = value; }

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
  ElementNode* realElem_;

  std::vector<ElementStmActionParam*> actionList_;
  std::vector<ArgumentParam*> argList_;

  ElementStmParam* nextElem_;
  ElementStmParam* trueElem_;
  ElementStmParam* falseElem_;

	ConnectionStmParam* parentConn_;

  CommandDefParam* commandDef_;
	TaskModelParam* taskParam_;

	bool isBreak_;
};

class ConnectionStmParam : public DatabaseParam {
public:
  ConnectionStmParam(int id, int sourceId, int targetId, QString cond)
   : id_(id), sourceId_(sourceId), targetId_(targetId),condition_(cond) {};
  ConnectionStmParam(const ConnectionStmParam* source)
   : id_(source->id_), sourceId_(source->sourceId_), targetId_(source->targetId_),condition_(source->condition_)
  {  mode_ = DatabaseMode(source->getMode()); };
	virtual ~ConnectionStmParam();

  inline int getId() const { return this->id_; }
	inline void setId(int value) { this->id_ = value; }

	inline int getSourceId() const { return this->sourceId_; }
  inline void setSourceId(int value) { this->sourceId_ = value; }

  inline int getTargetId() const { return this->targetId_; }
  inline void setTargetId(int value) { this->targetId_ = value; }

  inline QString getCondition() const { return this->condition_; }
  inline void setCondition(QString value) { this->condition_ = value; }

	void addChildNode(ElementStmParam* prev, ElementStmParam* target);
	void addChildNode(ElementStmParam* target);
	void removeChildNode(ElementStmParam* target);
	inline std::vector<ElementStmParam*> getChildList() const { return this->childList_; }

private:
  int id_;
  int sourceId_;
  int targetId_;
  QString condition_;
	std::vector<ElementStmParam*> childList_;
};

class ParameterParam : public DatabaseParam {
public:
  ParameterParam(int id, int type, QString model_name, int elem_num, QString elem_types, int task_inst_id, QString name, QString rname, QString unit)
   : id_(id), type_(type), model_name_(model_name), elem_num_(elem_num), elem_types_(elem_types), 
   task_inst_id_(task_inst_id), name_(name), rname_(rname), unit_(unit)
   { buildElemTypeList(); };
  ParameterParam(ParameterParam* source)
   : id_(source->id_), type_(source->type_), model_name_(source->model_name_),
     elem_num_(source->elem_num_), elem_types_(source->elem_types_),
     task_inst_id_(source->task_inst_id_), name_(source->name_), rname_(source->rname_), unit_(source->unit_)
  {  mode_ = DatabaseMode(source->getMode());
     buildElemTypeList();};

  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getType() const { return this->type_; }
  inline void setType(int value) { this->type_ = value; setUpdate(); }

  inline QString getModelName() const { return this->model_name_; }
  inline void setModelName(QString value) { this->model_name_ = value; setUpdate(); }

  inline int getElemNum() const { return this->elem_num_; }
  inline void setElemNum(int value) { this->elem_num_ = value; setUpdate(); }

  inline QString getElemTypes() const { return this->elem_types_; }
  inline void setElemTypes(QString value) { this->elem_types_ = value; setUpdate(); }
  inline int getElemType(int index) const { return this->elemTypeList_[index]; }

  inline int getTaskInstId() const { return this->task_inst_id_; }
  inline void setTaskInstId(int value) { this->task_inst_id_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; setUpdate(); }
  inline QString getRName() const { return this->rname_; }
  inline void setRName(QString value) { this->rname_ = value; setUpdate();}
  inline QString getUnit() const { return this->unit_; }
  inline void setUnit(QString value) { this->unit_ = value; setUpdate(); }

  inline void addControl(QLineEdit* target) { this->controlList_.push_back(target); }

  std::string getValues(int index);
  double getNumValues(int index);
  void saveValues();
  void setDBValues(QString source);
  QString getDBValues();
  void clearControlList() { this->controlList_.clear(); }

private:
  int id_;
  int type_;
  int elem_num_;
  int task_inst_id_;
  QString model_name_;
  QString name_;
  QString rname_;
  QString unit_;
  QString elem_types_;

  std::vector<int> elemTypeList_;
  std::vector<QLineEdit*> controlList_;
  std::vector<QString> valueList_;

  void buildElemTypeList();
};
////
class FileDataParam : public DatabaseParam {
public:
  FileDataParam(int id, QString name) : id_(id), name_(name) {};
  FileDataParam(FileDataParam* source)
    : id_(source->id_), name_(source->name_), data_(source->data_)
  {  mode_ = DatabaseMode(source->getMode()); };

  inline int getId() const { return this->id_; }
  inline QString getName() const { return this->name_; }

  inline void setData(QByteArray value) { this->data_ = value; }
  inline QByteArray getData() const { return this->data_; }

private:
  int id_;
  QString name_;
  QByteArray data_;
};

class ImageDataParam : public DatabaseParam {
public:
  ImageDataParam(int id, QString name) : id_(id), name_(name) {};
  ImageDataParam(ImageDataParam* source) 
    : id_(source->id_), name_(source->name_), data_(source->data_)
  {  mode_ = DatabaseMode(source->getMode()); };

  inline int getId() const { return this->id_; }
  inline QString getName() const { return this->name_; }

  inline void setData(QImage value) { this->data_ = value; }
  inline QImage getData() const { return this->data_; }
  inline void setRawData(QByteArray value) { this->rawData_ = value; }
  inline QByteArray getRawData() const { return this->rawData_; }

private:
  int id_;
  QString name_;
  QImage data_;
  QByteArray rawData_;
};
/////
class ActivityParam : public DatabaseParam {
public:
	inline std::vector<ElementStmParam*> getStmElementList() const { return this->stmElemList_; }
	inline void addStmElement(ElementStmParam* target){ this->stmElemList_.push_back(target); }
	inline std::vector<ConnectionStmParam*> getStmConnectionList() const { return this->stmConnectionList_; }
	inline void addStmConnection(ConnectionStmParam* target){ this->stmConnectionList_.push_back(target); }

	inline QString getErrStr() const { return this->errContents_; }
	inline ElementStmParam* getStartParam() const { return this->startParam_; }


	bool checkAndOrderStateMachine();

protected:
	std::vector<ElementStmParam*> stmElemList_;
	std::vector<ConnectionStmParam*> stmConnectionList_;

	ElementStmParam* startParam_;

	QString errContents_;
};
/////
class TaskModelParam : public ActivityParam {
public:
  TaskModelParam(int id, QString name, QString comment, int flow_id, int seq, QString created_date, QString last_updated_date);
  TaskModelParam(const TaskModelParam* source);
  virtual ~TaskModelParam();
  
  inline int getId() const { return this->id_; }
  inline void setId(int value) { this->id_ = value; }

  inline int getFlowId() const { return this->flow_id_; }

  inline int getSeq() const { return this->seq_; }
  inline void setSeq(int value) { this->seq_ = value; }

  inline QString getName() const { return this->name_; }
  inline void setName(QString value) { this->name_ = value; setUpdate(); }

  inline QString getComment() const { return this->comment_; }
  inline void setComment(QString value) { this->comment_ = value;  setUpdate(); }

  inline QString getCreatedDate() const { return this->created_date_; }
  inline void setCreatedDate(QString value) { this->created_date_ = value; }
  inline QString getLastUpdatedDate() const { return this->last_updated_date_; }
  inline void setLastUpdatedDate(QString value) { this->last_updated_date_ = value; }

  inline std::vector<ModelParam*> getModelList() const { return this->modeList_; }
  inline void addModel(ModelParam* target){ this->modeList_.push_back(target); }

  inline std::vector<ParameterParam*> getParameterList() const { return this->parameterList_; }
  inline void addParameter(ParameterParam* target){ this->parameterList_.push_back(target); }

  inline std::vector<FileDataParam*> getFileList() const { return this->fileList_; }
  inline void addFile(FileDataParam* target){ this->fileList_.push_back(target); }
  inline std::vector<ImageDataParam*> getImageList() const { return this->imageList_; }
  inline void addImage(ImageDataParam* target){ this->imageList_.push_back(target); }

	inline TaskModelParam* getNextTask() const { return this->nextTask_; }
	inline void setNextTask(TaskModelParam* value) { this->nextTask_ = value; }

	inline ElementStmParam* getStateParam() const { return this->stateParam_; }
	inline void setStateParam(ElementStmParam* value) { this->stateParam_ = value; }

	void setAllNewData();
  void deleteModelById(const int id);
  ModelParam* getModelById(const int id);
  void clearParameterList();
  void clearDetailParams();

  inline bool IsLoaded() const { return this->isLoaded_; }
  inline void setLoaded(bool value) { this->isLoaded_ = value; }

	inline bool IsModelLoaded() const { return this->isModelLoaded_; }
	inline void setModelLoaded(bool value) { this->isModelLoaded_ = value; }

private:
  int id_;
  QString name_;
  QString comment_;
  int flow_id_;
  int seq_;
  QString created_date_;
  QString last_updated_date_;

  bool isLoaded_;
	bool isModelLoaded_;

  std::vector<ModelParam*> modeList_;
  std::vector<ParameterParam*> parameterList_;
  std::vector<FileDataParam*> fileList_;
  std::vector<ImageDataParam*> imageList_;

	TaskModelParam* nextTask_;
	ElementStmParam* stateParam_;
};

class FlowParam : public ActivityParam {
public:
  FlowParam(int id, QString name, QString comment, QString created_date, QString last_updated_date, bool isNew)
    : id_(id), name_(name), comment_(comment), created_date_(created_date), last_updated_date_(last_updated_date) {
    if( isNew ) setNew();
  };
  FlowParam(const FlowParam* source) :
    id_(source->id_), name_(source->name_), comment_(source->comment_),
    created_date_(source->created_date_), last_updated_date_(source->last_updated_date_) {};
  virtual ~FlowParam() {};

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

private:
  int id_;
  QString name_;
  QString comment_;
  QString created_date_;
  QString last_updated_date_;
};
/////
struct ModelParamComparator {
  int id_;
  ModelParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ModelParam* elem) const {
    return (elem->getId()==id_ 
			&& (elem->getMode() != DB_MODE_DELETE && elem->getMode() != DB_MODE_IGNORE));
  }
};

struct ModelParamComparatorByRName {
  QString rname_;
  ModelParamComparatorByRName(QString value) {
    rname_ = value;
  }
  bool operator()(const ModelParam* elem) const {
    return elem->getRName()==rname_;
  }
};

struct ElementStmParamComparator {
  int id_;
  ElementStmParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ElementStmParam* elem) const {
    return elem->getOrgId()==id_;
  }
};

}
#endif
