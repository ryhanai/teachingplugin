#include "TeachingTypes.h"
#include "ChoreonoidUtil.h"
#include "TeachingUtil.h"
//
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
void ElementNode::updateSelect(bool isActive) {
  this->isActive_ = isActive;
  if (type_ == ELEMENT_START) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_FINAL) {
    if (isActive) {
      item_->setPen(QPen(Qt::red, 3.0));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
    }

  } else if (type_ == ELEMENT_DECISION) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_FORK) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_COMMAND) {
    if (isBreak_) {
      if (isActive) {
        item_->setPen(QPen(Qt::cyan, 3.0));
      } else {
        item_->setPen(QPen(Qt::green, 3.0));
      }
    } else {
      if (isActive) {
        item_->setPen(QPen(Qt::red, 3.0));
      } else {
        item_->setPen(QPen(Qt::black, 3.0));
      }
    }
  } else if (type_ == ELEMENT_POINT) {
    if (isActive) {
      item_->setPen(QPen(Qt::red, 3.0));
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }
  }
}

void ElementNode::setBreak(bool isBreak) {
  this->isBreak_ = isBreak;
  //
  if (isBreak_) {
    if (isActive_) {
      item_->setPen(QPen(Qt::cyan, 3.0));
    } else {
      item_->setPen(QPen(Qt::green, 3.0));
    }
  } else {
    if (isActive_) {
      item_->setPen(QPen(Qt::red, 3.0));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
    }
  }
}

void ElementNode::updateActive(bool isActive) {
  if (type_ == ELEMENT_START) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_FINAL) {
    if (isActive) {
      item_->setPen(QPen(Qt::blue, 3.0));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
    }

  } else if (type_ == ELEMENT_DECISION) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_FORK) {
    if (isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if (type_ == ELEMENT_COMMAND) {
    if (isActive) {
      if (isBreak_) {
        item_->setPen(QPen(Qt::magenta, 3.0));
      } else {
        item_->setPen(QPen(Qt::blue, 3.0));
      }
    } else {
      if (isBreak_) {
        item_->setPen(QPen(Qt::green, 3.0));
      } else {
        item_->setPen(QPen(Qt::black, 3.0));
      }
    }
  }
}
/////
void ElementStmParam::updateSelect(bool isActive) {
  if (this->realElem_) {
    this->realElem_->updateSelect(isActive);
  }
}

void ElementStmParam::updateActive(bool isActive) {
  if (this->realElem_) {
    this->realElem_->updateActive(isActive);
  }
}

void ElementStmParam::clearActionList() {
  actionList_.clear();
}

vector<ArgumentParamPtr> ElementStmParam::getActiveArgumentList() {
	std::vector<ArgumentParamPtr> result;
	for (int index = 0; index < argList_.size(); index++) {
		ArgumentParamPtr param = argList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

ArgumentParamPtr ElementStmParam::getArgumentById(int id) {
	for (int index = 0; index < argList_.size(); index++) {
		ArgumentParamPtr param = argList_[index];
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
	for (int index = 0; index < actionList_.size(); index++) {
		ElementStmActionParamPtr param = actionList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	sort(result.begin(), result.end(), compareAction);
	return result;
}

ElementStmActionParamPtr ElementStmParam::getStateActionById(int id) {
	for (int index = 0; index < actionList_.size(); index++) {
		ElementStmActionParamPtr param = actionList_[index];
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

ElementStmParam::ElementStmParam(int id, int type, QString cmdName, QString cmdDspName, double posX, double posY, QString condition)
  : id_(id), type_(type), cmdName_(cmdName), cmdDspName_(cmdDspName), posX_(posX), posY_(posY), condition_(condition),
  nextElem_(0), trueElem_(0), falseElem_(0), realElem_(0), commandDef_(0), taskParam_(0), isBreak_(false) {
}

ElementStmParam::ElementStmParam(const ElementStmParamPtr source)
  : id_(source->id_), org_id_(source->org_id_), type_(source->type_),
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
  delete realElem_;
}

void ConnectionStmParam::addChildNode(ElementStmParamPtr target) {
  this->childList_.push_back(target);
}

void ConnectionStmParam::addChildNode(ElementStmParamPtr prev, ElementStmParamPtr target){
  DDEBUG("ConnectionStmParam::addChildNode");
  if (childList_.size() == 0) {
    this->childList_.push_back(target);
  } else {
    vector<ElementStmParamPtr>::iterator iter = find(childList_.begin(), childList_.end(), prev);
    if (iter != childList_.end()) {
      DDEBUG("ConnectionStmParam::addChildNode NOT FOUND");
      childList_.insert(iter + 1, target);
    } else {
      DDEBUG("ConnectionStmParam::addChildNode FOUND");
      childList_.insert(childList_.begin(), target);
    }
  }
}

void ConnectionStmParam::removeChildNode(ElementStmParamPtr target) {
  this->childList_.erase(std::remove(this->childList_.begin(), this->childList_.end(), target), this->childList_.end());
}

ConnectionStmParam::ConnectionStmParam(const ConnectionStmParamPtr source)
  : id_(source->id_),
  sourceId_(source->sourceId_), targetId_(source->targetId_),
  condition_(source->condition_), DatabaseParam(source.get())
{
  for (unsigned int index = 0; index < source->childList_.size(); index++) {
		ElementStmParamPtr param = std::make_shared<ElementStmParam>(source->childList_[index]);
    param->setNewForce();
    this->childList_.push_back(param);
  }
};

ConnectionStmParam::~ConnectionStmParam() {
  childList_.clear();
}
/////
void ParameterParam::setElemTypes(QString value) {
  this->elem_types_ = value;
  int elemTypeNo = getElemTypeNo();
  this->elem_types_ = QString::number(elemTypeNo);
  setUpdate();
}

QString ParameterParam::getElemTypeStr() {
  QString result = "";

  if (elem_types_.length() == 0) {
    result = "double";

  } else if (elem_types_.toLower() == "int" || elem_types_.toLower() == "double" || elem_types_.toLower() == "string") {
    result = elem_types_.toLower();

  } else {
    switch (elem_types_.toInt()) {
      case 0:
      result = "double";
      break;
      case 1:
      result = "int";
      break;
      case 2:
      result = "string";
      break;
    }
  }
  return result;
}

int ParameterParam::getElemTypeNo() {
  int result = 0;

  if (elem_types_.length() == 0) {
    result = 0;

  } else if (elem_types_.toLower() == "double") {
    result = 0;
  } else if (elem_types_.toLower() == "int") {
    result = 1;
  } else if (elem_types_.toLower() == "string") {
    result = 2;

  } else {
    result = elem_types_.toInt();
  }
  return result;
}

std::string ParameterParam::getValues(int index) {
  if (index < 0 || valueList_.size() <= index) return "";
  return valueList_[index].toUtf8().constData();
}

double ParameterParam::getNumValues(int index) {
  if (index < 0 || valueList_.size() <= index) return 0;
  return valueList_[index].toDouble();
}

void ParameterParam::saveValues() {
	DDEBUG("ParameterParam::saveValues");
  for (int index = 0; index < controlList_.size(); index++) {
		QLineEdit* target = controlList_[index];
		if (index < valueList_.size()) {
			QString source = valueList_[index];
			if (source != target->text()) {
				valueList_[index] = target->text();
				setUpdate();
      }

    } else {
			valueList_.push_back(target->text());
			setUpdate();
    }
  }
}

void ParameterParam::setDBValues(QString source) {
  if (source.size() == 0) return;
  QStringList valList = source.split(",");
  valueList_.clear();
  for (int index = 0; index < valList.size(); index++) {
    QString each = valList.at(index);
    valueList_.push_back(each.toUtf8().constData());
  }
}

QString ParameterParam::getDBValues() {
  QString result = QString("");
  if (valueList_.size() == 0) return result;
  //
  for (int index = 0; index < valueList_.size(); index++) {
    if (index != 0) {
      result += QString(", ");
    }
    result += valueList_[index];
  }
  return result;
}

void ParameterParam::setValues(int index, QString source) {
  valueList_[index] = source;
  controlList_[index]->setText(source);
}

void ParameterParam::buildElemTypeList() {
	//DDEBUG("ParameterParam::buildElemTypeList");
	elemTypeList_.clear();
  for (int index = 0; index < elem_num_; index++) {
    elemTypeList_.push_back(0);
  }
  //
  QStringList targetList = elem_types_.split(",");
  for (unsigned int index = 0; index < targetList.size(); index++) {
    QString each = targetList[index].trimmed();
    elemTypeList_[index] = each.toInt();
  }
}

ParameterParam::ParameterParam(ParameterParam* source)
  : id_(source->id_), type_(source->type_),
		elem_num_(source->elem_num_), parent_id_(source->parent_id_),
		model_name_(source->model_name_), name_(source->name_),
		rname_(source->rname_), unit_(source->unit_), 
		elem_types_(source->elem_types_), hide_(source->hide_),
	  DatabaseParam(source) {
	DDEBUG("ParameterParam copy");
	buildElemTypeList();
  for (unsigned int index = 0; index < source->valueList_.size(); index++) {
    this->valueList_.push_back(source->valueList_[index]);
  }
}

ParameterParam::~ParameterParam() {
	controlList_.clear();
}
/////
void ModelMasterParam::deleteModelDetails() {
  for (int index = 0; index < modelDetailList_.size(); index++) {
    modelDetailList_[index]->setDelete();
  }
}

std::vector<ModelParameterParamPtr> ModelMasterParam::getActiveParamList() {
	vector<ModelParameterParamPtr> result;
	for (int index = 0; index < modelParameterList_.size(); index++) {
		ModelParameterParamPtr param = modelParameterList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}
/////
bool ModelParam::isChangedPosition() {
	DDEBUG_V("ModelParam::isChangedPosition x:%f, %f, y:%f, %f, z:%f, %f, Rx:%f, %f, Ry:%f, %f, Rz:%f, %f", posX_, orgPosX_, posY_, orgPosY_, posZ_, orgPosZ_, rotRx_, orgRotRx_, rotRy_, orgRotRy_, rotRz_, orgRotRz_);

	if (dbl_eq(posX_, orgPosX_) && dbl_eq(posY_, orgPosY_) && dbl_eq(posZ_,orgPosZ_)
    && dbl_eq(rotRx_, orgRotRx_) && dbl_eq(rotRy_, orgRotRy_) && dbl_eq(rotRz_, orgRotRz_) ) return false;
  return true;
}

void ModelParam::setInitialPos() {
  if (master_->getModelItem()) {
    ChoreonoidUtil::updateModelItemPosition(master_->getModelItem(), orgPosX_, orgPosY_, orgPosZ_, orgRotRx_, orgRotRy_, orgRotRz_);
  }
  posX_ = orgPosX_;
  posY_ = orgPosY_;
  posZ_ = orgPosZ_;
  rotRx_ = orgRotRx_;
  rotRy_ = orgRotRy_;
  rotRz_ = orgRotRz_;
}

/////
void TaskModelParam::setAllNewData() {
  this->mode_ = DB_MODE_INSERT;
  //
  for (int idxModel = 0; idxModel < modelList_.size(); idxModel++) {
		ModelParamPtr model = modelList_[idxModel];
    model->setNewForce();
    //
  }
  //
  for (int idxState = 0; idxState < stmElemList_.size(); idxState++) {
		ElementStmParamPtr state = stmElemList_[idxState];
    state->setNewForce();
    for (int idxAction = 0; idxAction < state->getActionList().size(); idxAction++) {
			ElementStmActionParamPtr action = state->getActionList()[idxAction];
      action->setNewForce();
    }
    for (int idxArg = 0; idxArg < state->getArgList().size(); idxArg++) {
			ArgumentParamPtr arg = state->getArgList()[idxArg];
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
  isLoaded_(false), isModelLoaded_(false), nextTask_(0), stateParam_(0),
	ActivityParam(id, name, comment, created_date, last_updated_date) {
}

TaskModelParam::TaskModelParam(const TaskModelParam* source)
  :	exec_env_(source->exec_env_), flow_id_(source->flow_id_),
  isLoaded_(source->isLoaded_), isModelLoaded_(source->isModelLoaded_),
  nextTask_(source->nextTask_), stateParam_(source->stateParam_),
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
	for (int index = 0; index < modelList_.size(); index++) {
		ModelParamPtr param = modelList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

vector<FileDataParamPtr> TaskModelParam::getActiveFileList() {
	std::vector<FileDataParamPtr> result;
	for (int index = 0; index < fileList_.size(); index++) {
		FileDataParamPtr param = fileList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

FileDataParamPtr TaskModelParam::getFileById(int id) {
	for (int index = 0; index < fileList_.size(); index++) {
		FileDataParamPtr param = fileList_[index];
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
	for (int index = 0; index < imageList_.size(); index++) {
		ImageDataParamPtr param = imageList_[index];
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
ActivityParam::ActivityParam(const ActivityParam* source)
  : id_(source->id_), name_(source->name_), comment_(source->comment_),
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

bool ActivityParam::checkAndOrderStateMachine() {
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
    if ((*itConnChk)->getMode() == DB_MODE_DELETE || (*itConnChk)->getMode() == DB_MODE_IGNORE) {
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
    if (std::find(decisionNodeIds.begin(), decisionNodeIds.end(), sourceId) != decisionNodeIds.end()) {
      if ((*itConnChk)->getCondition().length() == 0) {
        errContents_ = "Condition is not set for the flow from decisionNode.";
        return true;
      }
    }
    ++itConnChk;
  }
  /////
  //é¿çsèáèòÇÃëgÇ›óßÇƒ
  std::vector<ElementStmParamPtr>::iterator itElem = stmElemList_.begin();
	DDEBUG_V("states:%d, trans:%d", stmElemList_.size(), stmConnectionList_.size());

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
      if ((*itConn)->getMode() == DB_MODE_DELETE || (*itConn)->getMode() == DB_MODE_IGNORE) {
        ++itConn;
        continue;
      }
      DDEBUG_V("id:%d, source:%d, target:%d",(*itConn)->getId(), (*itConn)->getSourceId(), (*itConn)->getTargetId())
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
          if ((*itConn)->getCondition().startsWith(QString("true"))) {
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
  return false;
}

std::vector<ParameterParamPtr> ActivityParam::getActiveParameterList() {
	std::vector<ParameterParamPtr> result;
	for (int index = 0; index < parameterList_.size(); index++) {
		ParameterParamPtr param = parameterList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

ParameterParamPtr ActivityParam::getParameterById(int id) {
	for (int index = 0; index < parameterList_.size(); index++) {
		ParameterParamPtr param = parameterList_[index];
		if (param->getId() == id) {
			return param;
		}
	}
	return 0;
}

std::vector<ElementStmParamPtr> ActivityParam::getActiveStateList() {
	std::vector<ElementStmParamPtr> result;
	for (int index = 0; index < stmElemList_.size(); index++) {
		ElementStmParamPtr param = stmElemList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

vector<ConnectionStmParamPtr> ActivityParam::getActiveTransitionList() {
	std::vector<ConnectionStmParamPtr> result;
	for (int index = 0; index < stmConnectionList_.size(); index++) {
		ConnectionStmParamPtr param = stmConnectionList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}
	return result;
}

}
