#include "TeachingTypes.h"
#include "ChoreonoidUtil.h"
#include "TeachingUtil.h"
#include <boost/bind.hpp>
//
#include "NodeEditor/models.hpp"

#include "LoggerUtil.h"

namespace teaching {

void DatabaseParam::setNew() {
  if (mode_ == DB_MODE_NORMAL) {
    mode_ = DB_MODE_INSERT;
  }
}

void DatabaseParam::setNewForce() {
  mode_ = DB_MODE_INSERT;
}

void DatabaseParam::setUpdate() {
  if (mode_ == DB_MODE_NORMAL) {
    mode_ = DB_MODE_UPDATE;
  }
}

void DatabaseParam::setDelete() {
  if (mode_ == DB_MODE_NORMAL || mode_ == DB_MODE_UPDATE) {
    mode_ = DB_MODE_DELETE;

  } else if (mode_ == DB_MODE_INSERT) {
    mode_ = DB_MODE_IGNORE;
  }
}

void DatabaseParam::setIgnore() {
  mode_ = DB_MODE_IGNORE;
}

void DatabaseParam::setNormal() {
  mode_ = DB_MODE_NORMAL;
}
/////
void ElementStmParam::updateActive(bool isActive) {
  if (this->realElem_) {
    this->realElem_->updateActive(isActive);
  }
}

void ElementStmParam::clearActionList() {
  actionList_.clear();
}

void ElementStmParam::updatePos() {
	DDEBUG("ElementStmParam::updatePos");
	if (mode_ == DB_MODE_DELETE || mode_ == DB_MODE_IGNORE) return;

	posX_ = realElem_->nodeGraphicsObject().pos().x();
	posY_ = realElem_->nodeGraphicsObject().pos().y();

	setUpdate();
}

vector<ArgumentParamPtr> ElementStmParam::getActiveArgumentList() {
	std::vector<ArgumentParamPtr> result;
	for (ArgumentParamPtr param : argList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

ArgumentParamPtr ElementStmParam::getArgumentById(int id) {
	for (ArgumentParamPtr param : argList_) {
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

bool compareAction(const ElementStmActionParamPtr& left, const ElementStmActionParamPtr& right) {
	return left->getSeq() < right->getSeq();
}

vector<ElementStmActionParamPtr> ElementStmParam::getActiveStateActionList() {
	std::vector<ElementStmActionParamPtr> result;
	for (ElementStmActionParamPtr param : actionList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	sort(result.begin(), result.end(), compareAction);
	return result;
}

ElementStmActionParamPtr ElementStmParam::getStateActionById(int id) {
	for (ElementStmActionParamPtr param : actionList_) {
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

ElementStmParam::ElementStmParam(int id, int type, QString cmdName, QString cmdDspName, double posX, double posY, QString condition)
  : type_(type), cmdName_(cmdName), cmdDspName_(cmdDspName), posX_(posX), posY_(posY), condition_(condition),
  nextElem_(0), trueElem_(0), falseElem_(0), realElem_(0), commandDef_(0), taskParam_(0), isBreak_(false), DatabaseParam(id) {
}

ElementStmParam::ElementStmParam(const ElementStmParamPtr source)
  : type_(source->type_),
  cmdName_(source->cmdName_), cmdDspName_(source->cmdDspName_),
  posX_(source->posX_), posY_(source->posY_), condition_(source->condition_),
  nextElem_(source->nextElem_), trueElem_(source->trueElem_), falseElem_(source->falseElem_),
  isBreak_(source->isBreak_),
  realElem_(source->realElem_), parentConn_(source->parentConn_), commandDef_(source->commandDef_),
  DatabaseParam(source.get()) {
  for (unsigned int index = 0; index < source->actionList_.size(); index++) {
		ElementStmActionParamPtr param = std::make_shared<ElementStmActionParam>(source->actionList_[index].get());
    param->setNewForce();
    this->actionList_.push_back(param);
  }

  for (unsigned int index = 0; index < source->argList_.size(); index++) {
		ArgumentParamPtr param = std::make_shared<ArgumentParam>(source->argList_[index].get());
    param->setNewForce();
    this->argList_.push_back(param);
  }
}

ElementStmParam::~ElementStmParam() {
  actionList_.clear();
  argList_.clear();
}

ConnectionStmParam::ConnectionStmParam(const ConnectionStmParamPtr source)
  : type_(source->type_),
    sourceId_(source->sourceId_), sourceIndex_(source->sourceIndex_),
    targetId_(source->targetId_), targetIndex_(source->targetIndex_),
	  DatabaseParam(source.get())
{
};

ConnectionStmParam::~ConnectionStmParam() {
}
/////
ParameterValueParam::ParameterValueParam(ParameterValueParam* source) {
  for (unsigned int index = 0; index < source->valueList_.size(); index++) {
    this->valueList_.push_back(source->valueList_[index]);
  }
}

QString ParameterValueParam::getValues(int index) {
  if (index < 0 || valueList_.size() <= index) return "";
  return valueList_[index];
}

double ParameterValueParam::getNumValues(int index) {
  if (index < 0 || valueList_.size() <= index) return 0;
  return valueList_[index].toDouble();
}

void ParameterValueParam::setValue(int index, QString value) {
  if (index < valueList_.size()) {
    QString source = valueList_[index];
    if (source != value) {
      valueList_[index] = value;
    }

  } else {
    valueList_.push_back(value);
  }
}

void ParameterValueParam::clear() {
  valueList_.clear();

}

void ParameterValueParam::setValuesByString(QString source) {
  if (source.size() == 0) return;
  valueList_.clear();
  QStringList valList = source.split(",");
  for (int index = 0; index < valList.size(); index++) {
    valueList_.push_back(valList.at(index));
  }
}

QString ParameterValueParam::getValuesString() {
  QString result = QString("");
  if (valueList_.size() == 0) return result;
  //
  for (int index = 0; index < valueList_.size(); index++) {
    if (index != 0) {
      result += QString(", ");
    }
    result += valueList_[index].trimmed();
  }
  return result;
}

/////
void ParameterParam::saveValues() {
	DDEBUG("ParameterParam::saveValues");
  for (int index = 0; index < controlList_.size(); index++) {
    QLineEdit* target = controlList_[index];
    valueParam_->setValue(index, target->text());
    setUpdate();
  }
}

void ParameterParam::setFlowValues(QString source) {
  DDEBUG_V("ParameterParam::setFlowValues: %s,%d", source.toStdString().c_str(), controlList_.size());
  if (source.size() == 0) return;
  QStringList valList = source.split(",");
  for (int index = 0; index < valList.size(); index++) {
    QString each = valList.at(index);
    valueParam_->setValue(index, each);
    //valueList_[index] = each.toUtf8().constData();
    if (index < controlList_.size()) {
      controlList_[index] = new QLineEdit();
      controlList_[index]->setText(each);
    }
  }
  DDEBUG("ParameterParam::setFlowValues End");
}

void ParameterParam::setOutValues(int index, QString source) {
  valueParam_->setValue(index, source);
  controlList_[index]->setText(source);
}

void ParameterParam::updateOutValues() {
  if (flowParam_) {
    QString strValues = getDBValues();
    flowParam_->setValue(strValues);
    QtNodes::Node* node = flowParam_->getRealElem();
    if (node) {
      QWidget* widget = node->nodeDataModel()->embeddedWidget();
      if (widget) {
        ParamWidget* target = (ParamWidget*)widget;
        target->setValue(strValues);
      }
    }
  }
}

ParameterParam::ParameterParam(ParameterParam* source)
  : type_(source->type_), param_type_(source->param_type_), parent_id_(source->parent_id_),
		name_(source->name_),	rname_(source->rname_), unit_(source->unit_),
    model_id_(source->model_id_), model_param_id_(source->model_param_id_),
    hide_(source->hide_), valueParam_(source->valueParam_),
    flowParam_(source->flowParam_), flowParamParam_(source->flowParamParam_),
	  DatabaseParam(source) {
	DDEBUG("ParameterParam copy");
}

ParameterParam::~ParameterParam() {
	controlList_.clear();
}

void ParameterParam::setFlowParam(FlowParameterParamPtr value) {
  DDEBUG("ParameterParam::setFlowParam");
  this->flowParam_ = value;
  this->flowParamParam_ = value->getParameter();
}

void ParameterParam::restoreParameter() {
  DDEBUG("ParameterParam::restoreParameter");
  this->flowParam_ = 0;
  this->flowParamParam_ = 0;
}
/////
void ModelMasterParam::deleteModelDetails() {
  for (ModelDetailParamPtr model : modelDetailList_) {
    model->setDelete();
  }
}

std::vector<ModelParameterParamPtr> ModelMasterParam::getActiveModelParamList() {
	vector<ModelParameterParamPtr> result;
	for (ModelParameterParamPtr param : modelParameterList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

void ModelMasterParam::loadData() {
  if (this->isLoaded_) return;

  this->image_ = db2Image(name_, rawData_);
  this->isLoaded_ = true;
}

QImage ModelMasterParam::db2Image(const QString& name, const QByteArray& source) {
  string strType = "";
  if (name.toUpper().endsWith("PNG")) {
    strType = "PNG";
  } else if (name.toUpper().endsWith("JPG")) {
    strType = "JPG";
  }
  QImage result = QImage::fromData(source, strType.c_str());

  return result;
}
/////
ModelParam::ModelParam(int id, int master_id, int type, QString rname, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz, bool hide, bool isNew)
    : master_id_(master_id), type_(type), rname_(rname),
	  	hide_(hide), master_(0), isLoaded_(false),
      master_id_org_(master_id), master_org_(0),
      updateKinematicStateLater(bind(&ModelParam::updateKinematicState, this, true), IDLE_PRIORITY_LOW),
      currentBodyItem_(0), DatabaseParam(id) {
    posture = std::make_shared<PostureParam>(posX, posY, posZ, rotRx, rotRy, rotRz);
    postureOrg = std::make_shared<PostureParam>(posX, posY, posZ, rotRx, rotRy, rotRz);
    if (isNew) setNew();
}

ModelParam::ModelParam(const ModelParam* source)
    : master_id_(source->master_id_), type_(source->type_), rname_(source->rname_),
  		master_(source->master_), hide_(source->hide_), isLoaded_(false),
      master_id_org_(source->master_id_org_), master_org_(source->master_org_),
      updateKinematicStateLater(bind(&ModelParam::updateKinematicState, this, true), IDLE_PRIORITY_LOW),
      currentBodyItem_(source->currentBodyItem_),
      DatabaseParam(source)
  {
    posture = std::make_shared<PostureParam>(source->posture.get());
    postureOrg = std::make_shared<PostureParam>(source->postureOrg.get());
  };

bool ModelParam::isChangedPosition() {
	DDEBUG_V("ModelParam::isChangedPosition x:%f, %f, y:%f, %f, z:%f, %f, Rx:%f, %f, Ry:%f, %f, Rz:%f, %f",
    posture->getPosX(), postureOrg->getPosX(),
    posture->getPosY(), postureOrg->getPosY(),
    posture->getPosZ(), postureOrg->getPosZ(),
    posture->getRotRx(), postureOrg->getRotRx(),
    posture->getRotRy(), postureOrg->getRotRy(),
    posture->getRotRz(), postureOrg->getRotRz());

	if (dbl_eq(posture->getPosX(), postureOrg->getPosX())
      && dbl_eq(posture->getPosY(), postureOrg->getPosY())
      && dbl_eq(posture->getPosZ(), postureOrg->getPosZ())
      && dbl_eq(posture->getRotRx(), postureOrg->getRotRx())
      && dbl_eq(posture->getRotRy(), postureOrg->getRotRy())
      && dbl_eq(posture->getRotRz(), postureOrg->getRotRz()) ) return false;
  return true;
}

void ModelParam::setInitialPos() {
  DDEBUG("ModelParam::setInitialPos");
  if (master_ && master_->getModelItem()) {
    ChoreonoidUtil::updateModelItemPosition(master_->getModelItem(),
      postureOrg->getPosX(), postureOrg->getPosY(), postureOrg->getPosZ(),
      postureOrg->getRotRx(), postureOrg->getRotRy(), postureOrg->getRotRz());
  }
  posture->setPosX(postureOrg->getPosX());
  posture->setPosY(postureOrg->getPosY());
  posture->setPosZ(postureOrg->getPosZ());
  posture->setRotRx(postureOrg->getRotRx());
  posture->setRotRy(postureOrg->getRotRy());
  posture->setRotRz(postureOrg->getRotRz());
}

void ModelParam::updateModelMaster(ModelMasterParamPtr value) {
  this->master_org_ = this->master_;
  this->master_id_org_ = this->master_id_;
  //
  this->master_ = value;
  this->master_id_ = value->getId();
  if (!value->getModelItem()) {
    ChoreonoidUtil::makeModelItem(value);
  }
  currentBodyItem_ = value->getModelItem();

  DDEBUG_V("ModelParam::updateModelMaster org : %d, new : %d", master_org_->getId(), master_->getId());
}

void ModelParam::restoreModelMaster() {
  DDEBUG("ModelParam::restoreModelMaster");
  if (!this->master_org_) return;

  this->master_ = this->master_org_;
  this->master_id_ = this->master_id_org_;
  //
  this->master_org_ = 0;
  this->master_id_org_ = NULL_ID;

  DDEBUG_V("ModelParam::restoreModelMaster new : %d", master_->getId());
}

void ModelParam::initializeItem() {
  DDEBUG("ModelParam::initializeItem");
  if(this->master_ && this->master_->getModelItem()) {
    currentBodyItem_ = this->master_->getModelItem().get();
    connectionToKinematicStateChanged = this->master_->getModelItem().get()->sigKinematicStateChanged().connect(updateKinematicStateLater);
  }
}

 void ModelParam::updateKinematicState(bool blockSignals) {
  DDEBUG("ModelParam::updateKinematicState");
  if (currentBodyItem_) {
    Link* currentLink = currentBodyItem_->body()->rootLink();
    posture->setPosX(currentLink->p()[0]);
    posture->setPosY(currentLink->p()[1]);
    posture->setPosZ(currentLink->p()[2]);

    const Matrix3 R = currentLink->attitude();
    const Vector3 rpy = rpyFromRot(R);
    posture->setRotRx(degree(rpy[0]));
    posture->setRotRy(degree(rpy[1]));
    posture->setRotRz(degree(rpy[2]));
  }
 }

 void ModelParam::finalizeItem() {
  DDEBUG("ModelParam::finalizeItem");
	if (connectionToKinematicStateChanged.connected()) {
		connectionToKinematicStateChanged.disconnect();
	}
  currentBodyItem_ = 0;
 }
 /////
void TaskModelParam::setAllNewData() {
  this->mode_ = DB_MODE_INSERT;
  //
  for (ModelParamPtr model : modelList_) {
    model->setNewForce();
    //
  }
  //
  for (ElementStmParamPtr state : stmElemList_) {
    state->setNewForce();
    for (ElementStmActionParamPtr action : state->getActionList()) {
      action->setNewForce();
    }
    for (ArgumentParamPtr arg : state->getArgList()) {
      arg->setNewForce();
    }
  }
  //
  std::vector<ConnectionStmParamPtr>::iterator itConn = stmConnectionList_.begin();
  while (itConn != stmConnectionList_.end()) {
    (*itConn)->setNewForce();
    ++itConn;
  }
  //
  std::vector<ParameterParamPtr>::iterator itParam = parameterList_.begin();
  while (itParam != parameterList_.end()) {
    (*itParam)->setNewForce();
    ++itParam;
  }
  //
  std::vector<FileDataParamPtr>::iterator itFile = fileList_.begin();
  while (itFile != fileList_.end()) {
    (*itFile)->setNewForce();
    ++itFile;
  }
  //
  std::vector<ImageDataParamPtr>::iterator itImage = imageList_.begin();
  while (itImage != imageList_.end()) {
    (*itImage)->setNewForce();
    ++itImage;
  }
}

TaskModelParam::TaskModelParam(int id, QString name, QString comment, QString execEnv, int flow_id, QString created_date, QString last_updated_date)
  : exec_env_(execEnv), flow_id_(flow_id),
  isLoaded_(false), isModelLoaded_(false), nextTask_(0), trueTask_(0), falseTask_(0), stateParam_(0),
	ActivityParam(id, name, comment, created_date, last_updated_date) {
}

TaskModelParam::TaskModelParam(const TaskModelParam* source)
  :	exec_env_(source->exec_env_), flow_id_(source->flow_id_),
  isLoaded_(source->isLoaded_), isModelLoaded_(source->isModelLoaded_),
  nextTask_(source->nextTask_), stateParam_(source->stateParam_),
  trueTask_(source->trueTask_), falseTask_(source->falseTask_), flowCondition_(source->flowCondition_),
	ActivityParam(source) {
	DDEBUG("TaskModelParam::CopyConstructor");

  for (unsigned int index = 0; index < source->modelList_.size(); index++) {
		ModelParamPtr param = std::make_shared<ModelParam>(source->modelList_[index].get());
    param->setNewForce();
    this->modelList_.push_back(param);
  }

  for (unsigned int index = 0; index < source->fileList_.size(); index++) {
		FileDataParamPtr param = std::make_shared<FileDataParam>(source->fileList_[index].get());
    param->setNewForce();
    this->fileList_.push_back(param);
  }

  for (unsigned int index = 0; index < source->imageList_.size(); index++) {
		ImageDataParamPtr param = std::make_shared<ImageDataParam>(source->imageList_[index].get());
    param->setNewForce();
    this->imageList_.push_back(param);
  }
}

TaskModelParam::~TaskModelParam() {
  clearDetailParams();
}

void TaskModelParam::clearDetailParams() {
	//DDEBUG("TaskModelParam::clearDetailParams");
	modelList_.clear();
	stmElemList_.clear();
	stmConnectionList_.clear();
	parameterList_.clear();
	fileList_.clear();
	imageList_.clear();
}

void TaskModelParam::clearParameterList() {
  parameterList_.clear();
}

vector<ModelParamPtr> TaskModelParam::getActiveModelList() {
	vector<ModelParamPtr> result;
	for (ModelParamPtr param : modelList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

vector<ModelParamPtr> TaskModelParam::getVisibleModelList() {
  vector<ModelParamPtr> result;
  for (ModelParamPtr param : modelList_) {
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    if (param->getHide()) continue;
    result.push_back(param);
  }
  return result;
}

ModelParamPtr TaskModelParam::getModelParamById(int id) {
	DDEBUG_V("TaskModelParam::getModelParamById : %d", id);
  for (ModelParamPtr param : modelList_) {
	DDEBUG_V("Param Id : %d", param->getId());
    if (param->getId() == id) {
      return param;
    }
  }
  return 0;
}

vector<FileDataParamPtr> TaskModelParam::getActiveFileList() {
	std::vector<FileDataParamPtr> result;
	for (FileDataParamPtr param : fileList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

FileDataParamPtr TaskModelParam::getFileById(int id) {
	for (FileDataParamPtr param : fileList_) {
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

vector<ImageDataParamPtr> TaskModelParam::getActiveImageList() {
	std::vector<ImageDataParamPtr> result;
	for (int index = 0; index < imageList_.size(); index++) {
		ImageDataParamPtr param = imageList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

ImageDataParamPtr TaskModelParam::getImageById(int id) {
	for (ImageDataParamPtr param : imageList_) {
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}
//////////
void ImageDataParam::loadData() {
	if (this->isLoaded_) return;

	this->data_ = db2Image(name_, rawData_);
	this->isLoaded_ = true;
}

QImage ImageDataParam::db2Image(const QString& name, const QByteArray& source) {
	string strType = "";
	if (name.toUpper().endsWith("PNG")) {
		strType = "PNG";
	} else if (name.toUpper().endsWith("JPG")) {
		strType = "JPG";
	}
	QImage result = QImage::fromData(source, strType.c_str());

	return result;
}
//////////
FlowParam::FlowParam(const FlowParam* source) : ActivityParam(source) {
  for (unsigned int index = 0; index < source->modelList_.size(); index++) {
    FlowModelParamPtr param = std::make_shared<FlowModelParam>(source->modelList_[index].get());
    param->setNewForce();
    this->modelList_.push_back(param);
  }
  for (unsigned int index = 0; index < source->paramList_.size(); index++) {
    FlowParameterParamPtr param = std::make_shared<FlowParameterParam>(source->paramList_[index].get());
    param->setNewForce();
    this->paramList_.push_back(param);
  }
}

FlowParam::~FlowParam() {
  modelList_.clear();
  paramList_.clear();
}

std::vector<FlowModelParamPtr> FlowParam::getActiveModelList() {
  std::vector<FlowModelParamPtr> result;
  for (FlowModelParamPtr param : modelList_) {
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    result.push_back(param);
  }
  return result;

}

std::vector<FlowParameterParamPtr> FlowParam::getActiveFlowParamList() {
  std::vector<FlowParameterParamPtr> result;
  for (FlowParameterParamPtr param : paramList_) {
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    result.push_back(param);
  }
  return result;

}

void FlowModelParam::updatePos() {
  DDEBUG_V("FlowModelParam::updatePos:%d=%d", id_, mode_);
  if (mode_ == DB_MODE_DELETE || mode_ == DB_MODE_IGNORE) return;

  posX_ = realElem_->nodeGraphicsObject().pos().x();
  posY_ = realElem_->nodeGraphicsObject().pos().y();

  masterId_ = ((TransformDataModel*)realElem_->nodeDataModel())->getMasterId();
  name_  = ((TransformDataModel*)realElem_->nodeDataModel())->getName();

  setUpdate();
}

void FlowParameterParam::updatePos() {
  DDEBUG_V("FlowParameterParam::updatePos:%d=%d", id_, mode_);
  if (mode_ == DB_MODE_DELETE || mode_ == DB_MODE_IGNORE) return;

  posX_ = realElem_->nodeGraphicsObject().pos().x();
  posY_ = realElem_->nodeGraphicsObject().pos().y();

  if (type_ == PARAM_TYPE_FRAME) {
    DDEBUG("PARAM_TYPE_FRAME");
    name_ = ((FrameParamDataModel*)realElem_->nodeDataModel())->getName();
    valueParam_->setValuesByString(((FrameParamDataModel*)realElem_->nodeDataModel())->getValue());

  } else {
    DDEBUG("PARAM_TYPE_ETC");
    name_ = ((ParamDataModel*)realElem_->nodeDataModel())->getName();
    valueParam_->setValuesByString(((ParamDataModel*)realElem_->nodeDataModel())->getValue());
  }

  setUpdate();
}

void FlowParameterParam::setInitialValue() {
  DDEBUG_V("FlowParameterParam::setInitialValue=%s", valueParam_org_->getValuesString().toStdString().c_str());
  if (realElem_) {
    QWidget* widget = realElem_->nodeDataModel()->embeddedWidget();
    if (widget) {
      NodeDataModel* model = realElem_->nodeDataModel();
      DDEBUG_V("%s", model->name().toStdString().c_str());
      if(model->name()=="Flow Param (Frame)") {
        FrameParamWidget* target = (FrameParamWidget*)widget;
        target->setValue(valueParam_->getValuesString());
      } else {
        ParamWidget* target = (ParamWidget*)widget;
        target->setValue(valueParam_->getValuesString());
      }
    }
  }
}
//////////
ActivityParam::ActivityParam(const ActivityParam* source)
  : name_(source->name_), comment_(source->comment_),
		created_date_(source->created_date_), last_updated_date_(source->last_updated_date_),
		startParam_(source->startParam_), errContents_(source->errContents_), DatabaseParam(source)	{
	for (unsigned int index = 0; index < source->parameterList_.size(); index++) {
		ParameterParamPtr param = std::make_shared<ParameterParam>(source->parameterList_[index].get());
		param->setNewForce();
		this->parameterList_.push_back(param);
	}

	for (unsigned int index = 0; index < source->stmElemList_.size(); index++) {
		ElementStmParamPtr param = std::make_shared<ElementStmParam>(source->stmElemList_[index]);
    param->setNewForce();
    this->stmElemList_.push_back(param);
  }

  for (unsigned int index = 0; index < source->stmConnectionList_.size(); index++) {
		ConnectionStmParamPtr param = std::make_shared<ConnectionStmParam>(source->stmConnectionList_[index]);
    param->setNewForce();
    this->stmConnectionList_.push_back(param);
  }
}

int ActivityParam::getMaxStateId() {
	int result = 0;
	for (ElementStmParamPtr target : stmElemList_) {
		if (result < target->getId()) {
			result = target->getId();
		}
	}
	result++;
	return result;
}

void ActivityParam::clearTransitionList() {
	stmConnectionList_.clear();
}

bool ActivityParam::checkAndOrderStateMachine() {
  DDEBUG("ActivityParam::checkAndOrderStateMachine");
  errContents_ = "";
  //
  int startCnt = 0;
  std::vector<int> finalNodeIds;
  std::vector<int> decisionNodeIds;
  std::vector<ElementStmParamPtr>::iterator itElemChk = stmElemList_.begin();
  while (itElemChk != stmElemList_.end()) {
    if ((*itElemChk)->getMode() == DB_MODE_DELETE || (*itElemChk)->getMode() == DB_MODE_IGNORE) {
      ++itElemChk;
      continue;
    }
    if ((*itElemChk)->getType() == ELEMENT_FORK) {
      errContents_ = "CANNOT use forkNode in this version.";
      return true;

    } else if ((*itElemChk)->getType() == ELEMENT_START) {
      startCnt++;
      if (1 < startCnt) {
        errContents_ = "Several startNodes exist.";
        return true;
      }
      startParam_ = *itElemChk;

    } else if ((*itElemChk)->getType() == ELEMENT_FINAL) {
      finalNodeIds.push_back((*itElemChk)->getId());

    } else if ((*itElemChk)->getType() == ELEMENT_DECISION) {
      decisionNodeIds.push_back((*itElemChk)->getId());

    } else if ((*itElemChk)->getType() == ELEMENT_COMMAND) {
      (*itElemChk)->getCmdName();



    }
    (*itElemChk)->clearNextElems();
    ++itElemChk;
  }
  if (startCnt == 0) {
    errContents_ = "StartNode does NOT EXIST.";
    return true;
  }
  //
  int startFlowCnt = 0;
  std::vector<ConnectionStmParamPtr>::iterator itConnChk = stmConnectionList_.begin();
  while (itConnChk != stmConnectionList_.end()) {
    if ((*itConnChk)->getMode() == DB_MODE_DELETE || (*itConnChk)->getMode() == DB_MODE_IGNORE || (*itConnChk)->getType() != TYPE_TRANSITION) {
      ++itConnChk;
      continue;
    }
    //
    int sourceId = (*itConnChk)->getSourceId();
    if (sourceId == startParam_->getId()) {
      startFlowCnt++;
      if (1 < startFlowCnt) {
        errContents_ = "Several flows exist from startNodes.";
        return true;
      }

    } else if ((*itConnChk)->getTargetId() == startParam_->getId()) {
      errContents_ = "Flow to enter startNode exists.";
      return true;
    }
    if (std::find(finalNodeIds.begin(), finalNodeIds.end(), sourceId) != finalNodeIds.end()) {
      errContents_ = "Flow from out finalNode exists.";
      return true;
    }
    ++itConnChk;
  }
  /////
  //実行順序の組み立て
  std::vector<ElementStmParamPtr>::iterator itElem = stmElemList_.begin();
	DDEBUG_V("Build Order states:%d, trans:%d", stmElemList_.size(), stmConnectionList_.size());

  while (itElem != stmElemList_.end()) {
    if ((*itElem)->getMode() == DB_MODE_DELETE || (*itElem)->getMode() == DB_MODE_IGNORE) {
      ++itElem;
      continue;
    }
    //
    int sourceId = (*itElem)->getId();
    std::vector<ConnectionStmParamPtr>::iterator itConn = stmConnectionList_.begin();
    int nextCnt = 0;
    int trueCnt = 0;
    int falseCnt = 0;
    bool isSet = false;
    while (itConn != stmConnectionList_.end()) {
      if ((*itConn)->getMode() == DB_MODE_DELETE || (*itConn)->getMode() == DB_MODE_IGNORE || (*itConn)->getType() != TYPE_TRANSITION) {
        ++itConn;
        continue;
      }
      //DDEBUG_V("id:%d, source:%d, target:%d, type:%d",(*itConn)->getId(), (*itConn)->getSourceId(), (*itConn)->getTargetId(), (*itConn)->getType())
      if ((*itConn)->getSourceId() == sourceId) {
        int targetId = (*itConn)->getTargetId();
        std::vector<ElementStmParamPtr>::iterator targetElem = std::find_if(stmElemList_.begin(), stmElemList_.end(), ElementStmParamComparator(targetId));
        if (targetElem == stmElemList_.end()) {
          errContents_ = "target node NOT EXISTS.";
          return true;
        }
        if ((*targetElem)->getMode() == DB_MODE_DELETE || (*targetElem)->getMode() == DB_MODE_IGNORE) {
          errContents_ = "target node is DELETED.";
          return true;
        }
        //
        if ((*itElem)->getType() == ELEMENT_DECISION) {
          if ((*itConn)->getSourceIndex()==0) {
            trueCnt++;
            if (1 < trueCnt) {
              errContents_ = "Several TRUE flows exist from Node.";
              return true;
            }
            (*itElem)->setTrueElem(*targetElem);
          } else {
            falseCnt++;
						if (1 < falseCnt) {
							errContents_ = "Several FALSE flows exist from Node.";
							return true;
						}
						(*itElem)->setFalseElem(*targetElem);
					}

        } else {
          nextCnt++;
          if (1 < nextCnt) {
            errContents_ = "Several flows exist from Node. " + (*itElem)->getCmdDspName();
            return true;
          }
          (*itElem)->setNextElem(*targetElem);
        }
        isSet = true;
      }
      ++itConn;
    }
    if (isSet == false) {
      if ((*itElem)->getType() != ELEMENT_FINAL) {
        errContents_ = "OUT flow NOT EXIST.";
        return true;
      }
    }
    ++itElem;
  }
  //
  DDEBUG("ActivityParam::checkAndOrderStateMachine End");
  return false;
}

std::vector<ParameterParamPtr> ActivityParam::getActiveParameterList() {
	std::vector<ParameterParamPtr> result;
	for (ParameterParamPtr param : parameterList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

std::vector<ParameterParamPtr> ActivityParam::getVisibleParameterList() {
  std::vector<ParameterParamPtr> result;
  for (ParameterParamPtr param : parameterList_) {
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
    if (param->getHide()) continue;
    result.push_back(param);
  }
  return result;
}

ParameterParamPtr ActivityParam::getParameterById(int id) {
	for (ParameterParamPtr param : parameterList_) {
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

std::vector<ElementStmParamPtr> ActivityParam::getActiveStateList() {
	std::vector<ElementStmParamPtr> result;
	for (ElementStmParamPtr param : stmElemList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

vector<ConnectionStmParamPtr> ActivityParam::getActiveTransitionList() {
	std::vector<ConnectionStmParamPtr> result;
	for (ConnectionStmParamPtr param : stmConnectionList_) {
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

int FlowParam::getMaxModelId() {
  int result = 0;
  for (FlowModelParamPtr target : modelList_) {
    if (result < target->getId()) {
      result = target->getId();
    }
  }
  result++;
  return result;
}

int FlowParam::getMaxParamId() {
  int result = 0;
  for (FlowParameterParamPtr target : paramList_) {
    if (result < target->getId()) {
      result = target->getId();
    }
  }
  result++;
  return result;
}

}
