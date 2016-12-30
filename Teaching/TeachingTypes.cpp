#include "TeachingTypes.h"
#include "ChoreonoidUtil.h"
//
#include "LoggerUtil.h"

namespace teaching {

void DatabaseParam::setNew() {
  if( mode_==DB_MODE_NORMAL) {
    mode_ = DB_MODE_INSERT;
  }
}

void DatabaseParam::setNewForce() {
    mode_ = DB_MODE_INSERT;
}

void DatabaseParam::setUpdate() {
  if( mode_==DB_MODE_NORMAL) {
    mode_ = DB_MODE_UPDATE;
  }
}

void DatabaseParam::setDelete() {
  if( mode_==DB_MODE_NORMAL || mode_==DB_MODE_UPDATE) {
    mode_ = DB_MODE_DELETE;

  } else if( mode_==DB_MODE_INSERT) {
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
  if(type_==ELEMENT_START) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_FINAL) {
    if(isActive) {
      item_->setPen(QPen(Qt::red, 3.0));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
    }

  } else if(type_==ELEMENT_DECISION) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_FORK) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_COMMAND) {
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
		}
		else {
			item_->setPen(QPen(Qt::green, 3.0));
		}
	} else {
		if (isActive_) {
			item_->setPen(QPen(Qt::red, 3.0));
		}
		else {
			item_->setPen(QPen(Qt::black, 3.0));
		}
	}
}

void ElementNode::updateActive(bool isActive) {
  if(type_==ELEMENT_START) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_FINAL) {
    if(isActive) {
      item_->setPen(QPen(Qt::blue, 3.0));
    } else {
      item_->setPen(QPen(Qt::black, 3.0));
    }

  } else if(type_==ELEMENT_DECISION) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_FORK) {
    if(isActive) {
      item_->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    } else {
      item_->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

  } else if(type_==ELEMENT_COMMAND) {
    if(isActive) {
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
  if(this->realElem_) {
    this->realElem_->updateSelect(isActive);
  }
}

void ElementStmParam::updateActive(bool isActive) {
  if(this->realElem_) {
    this->realElem_->updateActive(isActive);
  }
}

void ElementStmParam::clearActionList() {
  std::vector<ElementStmActionParam*>::iterator itAction = actionList_.begin();
  while (itAction != actionList_.end() ) {
    delete *itAction;
    ++itAction;
  }
  actionList_.clear();
}

ElementStmParam::ElementStmParam(int id, int type, QString cmdName, QString cmdDspName, double posX, double posY)
	: id_(id), type_(type), cmdName_(cmdName), cmdDspName_(cmdDspName), posX_(posX), posY_(posY),
		nextElem_(0), trueElem_(0), falseElem_(0), realElem_(0), commandDef_(0), taskParam_(0), isBreak_(false) {
}

ElementStmParam::ElementStmParam(const ElementStmParam* source)
   : id_(source->id_), type_(source->type_),
     cmdName_(source->cmdName_), cmdDspName_(source->cmdDspName_),
     posX_(source->posX_), posY_(source->posY_),
     nextElem_(source->nextElem_), trueElem_(source->trueElem_), falseElem_(source->falseElem_),
		 isBreak_(source->isBreak_) {
    mode_ = DatabaseMode(source->getMode());
    std::copy(source->actionList_.begin(), source->actionList_.end(), back_inserter(actionList_) );
}

ElementStmParam::~ElementStmParam() {
  std::vector<ElementStmActionParam*>::iterator itAction = actionList_.begin();
  while (itAction != actionList_.end() ) {
    delete *itAction;
    ++itAction;
  }
  actionList_.clear();
  //
  std::vector<ArgumentParam*>::iterator itArg = argList_.begin();
  while (itArg != argList_.end() ) {
    delete *itArg;
    ++itArg;
  }
  argList_.clear();
	//
	delete realElem_;
}

void ConnectionStmParam::addChildNode(ElementStmParam* target) {
	this->childList_.push_back(target);
}

void ConnectionStmParam::addChildNode(ElementStmParam* prev, ElementStmParam* target){
	DDEBUG("ConnectionStmParam::addChildNode");
	if (childList_.size() == 0) {
		this->childList_.push_back(target);
	} else {
		vector<ElementStmParam*>::iterator iter = find(childList_.begin(), childList_.end(), prev);
		if (iter != childList_.end()) {
			DDEBUG("ConnectionStmParam::addChildNode NOT FOUND");
			childList_.insert(iter + 1, target);
		} else {
			DDEBUG("ConnectionStmParam::addChildNode FOUND");
			childList_.insert(childList_.begin(), target);
		}
	}
}

void ConnectionStmParam::removeChildNode(ElementStmParam* target) {
	this->childList_.erase(std::remove(this->childList_.begin(), this->childList_.end(), target), this->childList_.end());
}

ConnectionStmParam::~ConnectionStmParam() {
	std::vector<ElementStmParam*>::iterator itChild = childList_.begin();
	while (itChild != childList_.end()) {
		delete *itChild;
		++itChild;
	}
	childList_.clear();
}
/////
std::string ParameterParam::getValues(int index) {
  if(index<0 || valueList_.size()<= index) return "";
  return valueList_[index].toUtf8().constData();
}

double ParameterParam::getNumValues(int index) {
  if(index<0 || valueList_.size()<= index) return 0;
  return valueList_[index].toDouble();
}

void ParameterParam::saveValues() {
  //valueList_.clear();
  for(int index=0; index<controlList_.size(); index++) {
    QLineEdit* target = controlList_[index];
    if(index<valueList_.size()) {
      QString source = valueList_[index];
      if(source != target->text()) {
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
  if(source.size()==0) return;
  QStringList valList = source.split(",");
  for(int index=0; index<valList.size(); index++) {
    QString each = valList.at(index);
    valueList_.push_back(each.toUtf8().constData());
  }
}

QString ParameterParam::getDBValues() {
  QString result = QString("");
  if(valueList_.size()==0) return result;
  //
  for(int index=0; index<valueList_.size(); index++) {
    if(index!=0) {
      result += QString(", ");
    }
    result += valueList_[index];
  }
  return result;
}

void ParameterParam::buildElemTypeList() {
  elemTypeList_.clear();
  for(int index=0; index<elem_num_; index++) {
    elemTypeList_.push_back(0);
  }
  //
  QStringList targetList = elem_types_.split(",");
  for(unsigned int index=0; index<targetList.size(); index++) {
    QString each = targetList[index].trimmed();
    elemTypeList_[index] = each.toInt();
  }
}
/////
void ModelParam::deleteModelDetails() {
  for(int index=0; index<modeDetailList_.size(); index++) {
    modeDetailList_[index]->setDelete();
  }
}

void ModelParam::setInitialPos() {
	if (item_) {
		ChoreonoidUtil::updateModelItemPosition(item_, orgPosX_, orgPosY_, orgPosZ_, orgRotRx_, orgRotRy_, orgRotRz_);
	}
  posX_ = orgPosX_;
  posY_ = orgPosY_;
  posZ_ = orgPosZ_;
  rotRx_ = orgRotRx_;
  rotRy_ = orgRotRy_;
  rotRz_ = orgRotRz_;
}
/////
ModelParam* TaskModelParam::getModelById(const int id) {
  std::vector<ModelParam*>::iterator elem = std::find_if( modeList_.begin(), modeList_.end(), ModelParamComparator(id));
  if(elem== modeList_.end()) return 0;
  return *elem;
}

void TaskModelParam::deleteModelById(const int id) {
  std::vector<ModelParam*>::iterator elem = std::find_if( modeList_.begin(), modeList_.end(), ModelParamComparator(id));
  if(elem== modeList_.end()) return;
  (*elem)->setDelete();
  (*elem)->deleteModelDetails();
}

void TaskModelParam::setAllNewData() {
  this->mode_ = DB_MODE_INSERT;
  //
  for(int idxModel=0; idxModel<modeList_.size(); idxModel++) {
    ModelParam* model = modeList_[idxModel];
    model->setNewForce();
    //
    for(int idxDetail=0; idxDetail<model->getModelDetailList().size(); idxDetail++) {
      ModelDetailParam* detail = model->getModelDetailList()[idxDetail];
      detail->setNewForce();
    }
  }
  //
  for(int idxState=0; idxState<stmElemList_.size(); idxState++) {
    ElementStmParam* state = stmElemList_[idxState];
    state->setNewForce();
    for(int idxAction=0; idxAction<state->getActionList().size(); idxAction++) {
      ElementStmActionParam* action = state->getActionList()[idxAction];
      action->setNewForce();
    }
    for(int idxArg=0; idxArg<state->getArgList().size(); idxArg++) {
      ArgumentParam* arg = state->getArgList()[idxArg];
      arg->setNewForce();
    }
  }
  //
  std::vector<ConnectionStmParam*>::iterator itConn = stmConnectionList_.begin();
  while (itConn != stmConnectionList_.end() ) {
    (*itConn)->setNewForce();
    ++itConn;
  }
  //
  std::vector<ParameterParam*>::iterator itParam = parameterList_.begin();
  while (itParam != parameterList_.end() ) {
    (*itParam)->setNewForce();
    ++itParam;
  }
  //
  std::vector<FileDataParam*>::iterator itFile = fileList_.begin();
  while (itFile != fileList_.end() ) {
    (*itFile)->setNewForce();
    ++itFile;
  }
  //
  std::vector<ImageDataParam*>::iterator itImage = imageList_.begin();
  while (itImage != imageList_.end() ) {
    (*itImage)->setNewForce();
    ++itImage;
  }
}

TaskModelParam::TaskModelParam(int id, QString name, QString comment, int flow_id, int seq, QString created_date, QString last_updated_date)
: id_(id), name_(name), comment_(comment), flow_id_(flow_id), seq_(seq),
  created_date_(created_date), last_updated_date_(last_updated_date),
	isLoaded_(false), isModelLoaded_(false), nextTask_(0), stateParam_(0) {
}

TaskModelParam::TaskModelParam(const TaskModelParam* source)
  : id_(source->id_), name_(source->name_), comment_(source->comment_),
    flow_id_(source->flow_id_), seq_(source->seq_),
    created_date_(source->created_date_), last_updated_date_(source->last_updated_date_),
		isLoaded_(source->isLoaded_), isModelLoaded_(source->isModelLoaded_),
		nextTask_(source->nextTask_), stateParam_(source->stateParam_) {

  mode_ = DatabaseMode(source->getMode());
  std::copy(source->modeList_.begin(), source->modeList_.end(), back_inserter(modeList_) );
  std::copy(source->stmElemList_.begin(), source->stmElemList_.end(), back_inserter(stmElemList_) );
  std::copy(source->stmConnectionList_.begin(), source->stmConnectionList_.end(), back_inserter(stmConnectionList_) );
  std::copy(source->parameterList_.begin(), source->parameterList_.end(), back_inserter(parameterList_) );
  std::copy(source->fileList_.begin(), source->fileList_.end(), back_inserter(fileList_) );
  std::copy(source->imageList_.begin(), source->imageList_.end(), back_inserter(imageList_) );
}

TaskModelParam::~TaskModelParam() {
  clearDetailParams();
}

void TaskModelParam::clearDetailParams() {
  std::vector<ModelParam*>::iterator itModel = modeList_.begin();
  while (itModel != modeList_.end() ) {
    delete *itModel;
    ++itModel;
  }
  modeList_.clear();
  //
  std::vector<ElementStmParam*>::iterator itElem = stmElemList_.begin();
  while (itElem != stmElemList_.end() ) {
    delete *itElem;
    ++itElem;
  }
  stmElemList_.clear();
  //
  std::vector<ConnectionStmParam*>::iterator itConn = stmConnectionList_.begin();
  while (itConn != stmConnectionList_.end() ) {
    delete *itConn;
    ++itConn;
  }
  stmConnectionList_.clear();
  //
  std::vector<ParameterParam*>::iterator itParam = parameterList_.begin();
  while (itParam != parameterList_.end() ) {
    delete *itParam;
    ++itParam;
  }
  parameterList_.clear();
  //
  std::vector<FileDataParam*>::iterator itFile = fileList_.begin();
  while (itFile != fileList_.end() ) {
    delete *itFile;
    ++itFile;
  }
  fileList_.clear();
  //
  std::vector<ImageDataParam*>::iterator itImage = imageList_.begin();
  while (itImage != imageList_.end() ) {
    delete *itImage;
    ++itImage;
  }
  imageList_.clear();
}

void TaskModelParam::clearParameterList() {
  std::vector<ParameterParam*>::iterator itParam = parameterList_.begin();
  while (itParam != parameterList_.end() ) {
    delete *itParam;
    ++itParam;
  }
  parameterList_.clear();
}

bool ActivityParam::checkAndOrderStateMachine() {
  errContents_ = "";
  //
  int startCnt = 0;
  std::vector<int> finalNodeIds;
  std::vector<int> decisionNodeIds;
  std::vector<ElementStmParam*>::iterator itElemChk = stmElemList_.begin();
  while (itElemChk != stmElemList_.end() ) {
    if( (*itElemChk)->getMode()==DB_MODE_DELETE || (*itElemChk)->getMode()==DB_MODE_IGNORE) {
      ++itElemChk;
      continue;
    }
    if( (*itElemChk)->getType()==ELEMENT_FORK) {
      errContents_ = "CANNOT use forkNode in this version.";
      return true;

    } else if( (*itElemChk)->getType()==ELEMENT_START) {
      startCnt++;
      if( 1<startCnt ) {
        errContents_ = "Several startNodes exist.";
        return true;
      }
      startParam_ = *itElemChk;

    } else if( (*itElemChk)->getType()==ELEMENT_FINAL) {
      finalNodeIds.push_back((*itElemChk)->getId());

    } else if( (*itElemChk)->getType()==ELEMENT_DECISION) {
      decisionNodeIds.push_back((*itElemChk)->getId());
    }
    (*itElemChk)->clearNextElems();
    ++itElemChk;
  }
  if(startCnt==0) {
    errContents_ = "StartNode does NOT EXIST.";
    return true;
  }
  //
  int startFlowCnt = 0;
  std::vector<ConnectionStmParam*>::iterator itConnChk = stmConnectionList_.begin();
  while (itConnChk != stmConnectionList_.end() ) {
    if( (*itConnChk)->getMode()==DB_MODE_DELETE || (*itConnChk)->getMode()==DB_MODE_IGNORE) {
      ++itConnChk;
      continue;
    }
    //
    int sourceId = (*itConnChk)->getSourceId();
    if(sourceId==startParam_->getId()) {
      startFlowCnt++;
      if( 1<startFlowCnt ) {
        errContents_ = "Several flows exist from startNodes.";
        return true;
      }

    } else if((*itConnChk)->getTargetId()==startParam_->getId()) {
      errContents_ = "Flow to enter startNode exists.";
      return true;
    }
    if( std::find( finalNodeIds.begin(), finalNodeIds.end(), sourceId) != finalNodeIds.end() ) {
      errContents_ = "Flow from out finalNode exists.";
      return true;
    }
    if( std::find( decisionNodeIds.begin(), decisionNodeIds.end(), sourceId) != decisionNodeIds.end() ) {
      if( (*itConnChk)->getCondition().length()==0) {
        errContents_ = "Condition is not set for the flow from decisionNode.";
        return true;
      }
    }
    ++itConnChk;
  }
  /////
  //é¿çsèáèòÇÃëgÇ›óßÇƒ
  std::vector<ElementStmParam*>::iterator itElem = stmElemList_.begin();
  while (itElem != stmElemList_.end() ) {
    if( (*itElem)->getMode()==DB_MODE_DELETE || (*itElem)->getMode()==DB_MODE_IGNORE) {
      ++itElem;
      continue;
    }
    //
    int sourceId = (*itElem)->getId();
    std::vector<ConnectionStmParam*>::iterator itConn = stmConnectionList_.begin();
    int nextCnt = 0;
    int trueCnt = 0;
    int falseCnt = 0;
    bool isSet = false;
    while (itConn != stmConnectionList_.end() ) {
      if( (*itConn)->getMode()==DB_MODE_DELETE || (*itConn)->getMode()==DB_MODE_IGNORE) {
        ++itConn;
        continue;
      }
      DDEBUG_V("id:%d, source:%d, target:%d",(*itConn)->getId(), (*itConn)->getSourceId(), (*itConn)->getTargetId())
      if( (*itConn)->getSourceId()==sourceId ) {
        int targetId = (*itConn)->getTargetId();
        std::vector<ElementStmParam*>::iterator targetElem = std::find_if( stmElemList_.begin(), stmElemList_.end(), ElementStmParamComparator(targetId));
        if(targetElem== stmElemList_.end()) {
          errContents_ = "target node NOT EXISTS.";
          return true;
        }
        if( (*targetElem)->getMode()==DB_MODE_DELETE || (*targetElem)->getMode()==DB_MODE_IGNORE) {
          errContents_ = "target node is DELETED.";
          return true;
        }
        //
        if( (*itElem)->getType()==ELEMENT_DECISION) {
          if((*itConn)->getCondition().startsWith(QString("true"))) {
            trueCnt++;
            if( 1<trueCnt ) {
              errContents_ = "Several TRUE flows exist from Node.";
              return true;
            }
            (*itElem)->setTrueElem(*targetElem);
          } else {
            falseCnt++;
            if( 1<falseCnt ) {
              errContents_ = "Several FALSE flows exist from Node.";
              return true;
            }
            (*itElem)->setFalseElem(*targetElem);
          }

        } else {
          nextCnt++;
          if( 1<nextCnt ) {
            errContents_ = "Several flows exist from Node. " + (*itElem)->getCmdDspName();
            return true;
          }
          (*itElem)->setNextElem(*targetElem);
        }
        isSet = true;
      }
      ++itConn;
    }
    if(isSet==false) {
      if( (*itElem)->getType()!=ELEMENT_FINAL ) {
        errContents_ = "OUT flow NOT EXIST.";
        return true;
      }
    }
    ++itElem;
  }
  //
  return false;
}

}
