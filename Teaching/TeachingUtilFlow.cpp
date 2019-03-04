#include "TeachingUtil.h"

#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>

#include "DataBaseManager.h"
#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace teaching {

bool TeachingUtil::exportFlow(QString& strFName, FlowParamPtr targetFlow) {
  DDEBUG("TeachingUtil::exportFlow");

  vector<TaskModelParamPtr> taskList;

  QString path = QFileInfo(strFName).absolutePath();
  Listing* archive = new Listing();
  archive->setDoubleFormat("%.9g");
  MappingPtr flowNode = archive->newMapping();
  flowNode->write("flowName", targetFlow->getName().toUtf8(), DOUBLE_QUOTED);
  if (0 < targetFlow->getComment().length()) {
    flowNode->write("comment", targetFlow->getComment().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  }
  //
  vector<ElementStmParamPtr> stateList = targetFlow->getActiveStateList();
  if (0 < stateList.size()) {
    Listing* statesNode = flowNode->createListing("states");
    for (ElementStmParamPtr param : stateList) {
      MappingPtr stateNode = statesNode->newMapping();
      stateNode->write("id", param->getId());
      stateNode->write("type", param->getType());
      if (param->getType() == ELEMENT_COMMAND) {
        stateNode->write("task_id", param->getTaskParam()->getId());
        stateNode->write("task_name", param->getCmdDspName().toUtf8(), DOUBLE_QUOTED);
        taskList.push_back(param->getTaskParam());
      } else if (param->getType() == ELEMENT_DECISION) {
        stateNode->write("condition", param->getCondition().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
      }
      Listing* posList = stateNode->createFlowStyleListing("pos");
      posList->append(param->getPosX());
      posList->append(param->getPosY());
    }
  }
  //
  vector<FlowModelParamPtr> modelList = targetFlow->getActiveModelList();
	vector<int> masterIdList;
  if (0 < modelList.size()) {
    Listing* modelsNode = flowNode->createListing("models");
    for (FlowModelParamPtr param : modelList) {
      MappingPtr modelNode = modelsNode->newMapping();
      modelNode->write("id", param->getId());
      modelNode->write("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      modelNode->write("master_id", param->getMasterId());
      Listing* posList = modelNode->createFlowStyleListing("pos");
      posList->append(param->getPosX());
      posList->append(param->getPosY());
      //
      int masterId = param->getMasterId();
      auto result = std::find(masterIdList.begin(), masterIdList.end(), masterId);
      if (result == masterIdList.end()) {
        masterIdList.push_back(masterId);
      }
    }
  }
  //
  vector<FlowParameterParamPtr> paramList = targetFlow->getActiveFlowParamList();
  if (0 < paramList.size()) {
    Listing* paramsNode = flowNode->createListing("parameters");
    for (FlowParameterParamPtr param : paramList) {
      MappingPtr paramNode = paramsNode->newMapping();
      paramNode->write("id", param->getId());
      paramNode->write("type", param->getType());
      paramNode->write("name", param->getName().toUtf8(), DOUBLE_QUOTED);

      Listing* valueList = paramNode->createFlowStyleListing("value");
      QString strValues = param->getValue().toUtf8();
      QStringList listValue = strValues.split(","); 
      for (int index=0; index<listValue.count(); index++) {
        if(param->getType()==PARAM_TYPE_INTEGER) {
          valueList->append(listValue.at(index).toInt());
        } else {
          valueList->append(listValue.at(index).toDouble());
        }
      }

      Listing* posList = paramNode->createFlowStyleListing("pos");
      posList->append(param->getPosX());
      posList->append(param->getPosY());
    }
  }
  //
  vector<ConnectionStmParamPtr> transList = targetFlow->getActiveTransitionList();
  if (0 < transList.size()) {
    Listing* connsNode = flowNode->createListing("transitions");
    for (ConnectionStmParamPtr param : transList) {
      if (param->getSourceId() == param->getTargetId()) continue;
      MappingPtr connNode = connsNode->newMapping();
      connNode->write("type", param->getType());
      connNode->write("source_id", param->getSourceId());
      connNode->write("target_id", param->getTargetId());
      if (0 < param->getSourceIndex()) {
        connNode->write("source_index", param->getSourceIndex());
      }
      if (0 < param->getTargetIndex()) {
        connNode->write("target_index", param->getTargetIndex());
      }
    }
  }
  //
  if (0 < masterIdList.size()) {
		Listing* mastesrNode = flowNode->createListing("model_master");
		for (int master : masterIdList) {
      ModelMasterParamPtr targetMaster = DatabaseManager::getInstance().getModelMaster(master);
			MappingPtr masterNode = mastesrNode->newMapping();
			masterNode->write("id", targetMaster->getId());
			masterNode->write("name", targetMaster->getName().toUtf8(), DOUBLE_QUOTED);
			masterNode->write("file_name", targetMaster->getFileName().toUtf8(), DOUBLE_QUOTED);
			if (0 < targetMaster->getFileName().length()) {
			  QFile file(path + QString("/") + targetMaster->getFileName());
			  file.open(QIODevice::WriteOnly);
			  QByteArray data = targetMaster->getData();
			  file.write(data);
			  file.close();
			}
			masterNode->write("image_file_name", targetMaster->getImageFileName().toUtf8(), DOUBLE_QUOTED);
			if (0 < targetMaster->getImageFileName().length()) {
			  QImage data = targetMaster->getImage();
        data.save(path + QString("/") + targetMaster->getImageFileName());
			}
			
			for (int idxSub = 0; idxSub < targetMaster->getModelDetailList().size(); idxSub++) {
			  ModelDetailParamPtr detail = targetMaster->getModelDetailList()[idxSub];
			  if (0 < detail->getFileName().length()) {
			    QFile fileSub(path + QString("/") + detail->getFileName());
			    fileSub.open(QIODevice::WriteOnly);
			    QByteArray dataSub = detail->getData();
			    fileSub.write(dataSub);
			    fileSub.close();
			  }
			}

      vector<ModelParameterParamPtr> paramList = targetMaster->getActiveModelParamList();
      if (0 < paramList.size()) {
        Listing* featuresNode = masterNode->createListing("features");
        for (ModelParameterParamPtr feature : paramList) {
          MappingPtr featureNode = featuresNode->newMapping();
          featureNode->write("name", feature->getName().toUtf8(), DOUBLE_QUOTED);
          featureNode->write("value", feature->getValueDesc().toUtf8(), DOUBLE_QUOTED);
        }
      }
		}
	}
  //
  YAMLWriter writer(strFName.toUtf8().constData());
  writer.setKeyOrderPreservationMode(true);
  writer.putNode(archive);
  /////
  if (0 < taskList.size()) {
    QDir baseDir = QFileInfo(strFName).absolutePath();
    for (TaskModelParamPtr targetTask : taskList) {
      int targetId = targetTask->getId();
      QString targetIdStr = QString::number(targetId);
      baseDir.mkdir(targetIdStr);
      QString targetFile = baseDir.absolutePath() + QString("/") + targetIdStr + QString("/") + targetIdStr + ".yaml";
      DDEBUG_V("targetFile %s", targetFile.toStdString().c_str());
      //
      TeachingUtil::loadTaskDetailData(targetTask);
      exportTask(targetFile, targetTask);
    }
  }

  return true;
}

bool TeachingUtil::importFlow(QString& strFName, std::vector<FlowParamPtr>& flowModelList, vector<ModelMasterParamPtr>& modelMasterList, QString& errMessage) {
  DDEBUG_V("TaskDef File : %s", strFName.toStdString().c_str());
  YAMLReader pyaml;
  try {
    if (pyaml.load(strFName.toUtf8().constData()) == false) {
      errMessage = _("Failed to load YAML file : ") + strFName;
      DDEBUG(errMessage.toStdString().c_str());
      return false;
    }

    QString path = QFileInfo(strFName).absolutePath();
    Listing* flowList = pyaml.document()->toListing();

    for (int idxFlow = 0; idxFlow < flowList->size(); idxFlow++) {
      cnoid::ValueNode* eachFlow = flowList->at(idxFlow);
      Mapping* flowMap = eachFlow->toMapping();
      //
      QString flowName = "";
      try {
        flowName = QString::fromStdString(flowMap->get("flowName").toString());
      } catch (...) {
        errMessage = _("Failed to read the flowName of the flow.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (flowName.isEmpty()) {
        errMessage = _("FlowName is EMPTY.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      QString flowComment = "";
      try { flowComment = QString::fromStdString(flowMap->get("comment").toString()).replace("|", "\n"); } catch (...) {}

      FlowParamPtr flowParam = std::make_shared<FlowParam>(NULL_ID, flowName, flowComment, "", "");
      flowParam->setNew();
      flowModelList.push_back(flowParam);
      QString flowNameErr = QString::fromStdString("\n FlowName=[") + flowName + QString::fromStdString("]");

      Listing* stateList = flowMap->findListing("states");
      if (stateList) {
        QStringList existName;
        for (int idxState = 0; idxState < stateList->size(); idxState++) {
          Mapping* stateMap = stateList->at(idxState)->toMapping();
          int id, type, task_id;
          QString taskName = "";
          double posX, posY;
          QString condition = "";

          try {
            id = stateMap->get("id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the id of the state.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            type = stateMap->get("type").toInt();
          } catch (...) {
            errMessage = _("Failed to read the type of the state.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          if (type == ELEMENT_COMMAND) {
            try {
              task_id = stateMap->get("task_id").toInt();
            } catch (...) {
              errMessage = _("Failed to read the task_id of the state.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
            try {
              taskName = QString::fromStdString(stateMap->get("task_name").toString());
            } catch (...) {
              errMessage = _("Failed to read the task_name of the state.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
          } else if (type == ELEMENT_DECISION) {
            try {
              condition = QString::fromStdString(stateMap->get("condition").toString()).replace("|", "\n");
            } catch (...) {
              errMessage = _("Failed to read the condition of the state.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
            if (condition.isEmpty()) {
              errMessage = _("condition of the state is EMPTY.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
          }

          try {
            Listing* pos = stateMap->get("pos").toListing();
            if(pos->size() !=2 ) {
              errMessage = _("Position(pos) of state is invalid.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
            posX = pos->at(0)->toDouble();
            posY = pos->at(1)->toDouble();
          } catch (...) {
            errMessage = _("Failed to read the pos of the state.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          DDEBUG_V("task_name[%s]", taskName.toStdString().c_str());
          ElementStmParamPtr stateParam = std::make_shared<ElementStmParam>(id, type, taskName, taskName, posX, posY, condition);
          stateParam->setNew();
          flowParam->addStmElement(stateParam);
          //
          QString targetIdStr = QString::number(task_id);
          QString targetFile = path + QString("/") + targetIdStr + QString("/") + targetIdStr + ".yaml";
          vector<TaskModelParamPtr> taskInstList;
          if (importTask(targetFile, taskInstList, modelMasterList, errMessage)) {
            if (0 < taskInstList.size()) {
              TaskModelParamPtr task = taskInstList[0];
              task->setName(taskName);
              QString eachName = task->getName();
              if (0 < eachName.length()) {
                if (existName.contains(eachName)) {
                  errMessage = _("Duplicate task names.");
                  DDEBUG(errMessage.toStdString().c_str());
                  return false;
                }
                existName.append(eachName);
              }
              task->setNew();
              stateParam->setTaskParam(task);
            }
          }
        }
      }
      DDEBUG("Load States Finished");
      //
      Listing* modelList = flowMap->findListing("models");
      if (modelList) {
        for (int idxModel = 0; idxModel < modelList->size(); idxModel++) {
          Mapping* modelMap = modelList->at(idxModel)->toMapping();
          int id, master_id;
          double posX, posY;
          QString name;

          try {
            id = modelMap->get("id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the id of the FlowModelParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            master_id = modelMap->get("master_id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the master_id of the FlowModelParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            name = QString::fromStdString(modelMap->get("name").toString());
          } catch (...) {
            errMessage = _("Failed to read the name of the FlowModelParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            Listing* pos = modelMap->get("pos").toListing();
            if(pos->size() !=2 ) {
              errMessage = _("Position(pos) of FlowModelParameter is invalid.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
            posX = pos->at(0)->toDouble();
            posY = pos->at(1)->toDouble();
          } catch (...) {
            errMessage = _("Failed to read the pos of the FlowModelParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          FlowModelParamPtr modelParam = std::make_shared<FlowModelParam>(id, master_id, name);
          modelParam->setPosX(posX);
          modelParam->setPosY(posY);
          modelParam->setNew();
          flowParam->addModel(modelParam);
        }
      }
      DDEBUG("Load Models Finished");
      //
      Listing* paramList = flowMap->findListing("parameters");
      if (paramList) {
        for (int idxParam = 0; idxParam < paramList->size(); idxParam++) {
          Mapping* paramMap = paramList->at(idxParam)->toMapping();
          int id, type;
          QString name, value;
          double posX, posY;

          try {
            id = paramMap->get("id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the id of the FlowParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            type = paramMap->get("type").toInt();
          } catch (...) {
            errMessage = _("Failed to read the type of the FlowParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            name = QString::fromStdString(paramMap->get("name").toString());
          } catch (...) {
            errMessage = _("Failed to read the name of the FlowParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            Listing* values = paramMap->get("value").toListing();
            for(int index=0; index<values->size(); index++) {
              if (0 < index) value.append(",");
              if (type == PARAM_TYPE_INTEGER) {
                value.append(QString::number(values->at(index)->toInt()));
              } else {
                value.append(QString::number(values->at(index)->toDouble(), 'f', 6));
              }
            }
          } catch (...) {
            errMessage = _("Failed to read the value of the FlowParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            Listing* pos = paramMap->get("pos").toListing();
            if(pos->size() !=2 ) {
              errMessage = _("Position(pos) of FlowParameter is invalid.") + flowNameErr;
              DDEBUG(errMessage.toStdString().c_str());
              return false;
            }
            posX = pos->at(0)->toDouble();
            posY = pos->at(1)->toDouble();
          } catch (...) {
            errMessage = _("Failed to read the pos of the FlowParameter.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          FlowParameterParamPtr paramParam = std::make_shared<FlowParameterParam>(id, type, name, value);
          paramParam->setPosX(posX);
          paramParam->setPosY(posY);
          paramParam->setNew();
          flowParam->addFlowParam(paramParam);
        }
      }
      DDEBUG("Load Params Finished");
      //
      Listing* transList;
      transList = flowMap->findListing("transitions");
      if (!transList) {
        transList = flowMap->findListing("transactions");
      }
      if (transList) {
        for (int idxTrans = 0; idxTrans < transList->size(); idxTrans++) {
          Mapping* transMap = transList->at(idxTrans)->toMapping();
          int type, sourceId, targetId;
          int sourceIndex = 0;
          int targetIndex = 0;

          try {
            type = transMap->get("type").toInt();
          } catch (...) {
            errMessage = _("Failed to read the type of the transition.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            sourceId = transMap->get("source_id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the source_id of the transition.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try {
            targetId = transMap->get("target_id").toInt();
          } catch (...) {
            errMessage = _("Failed to read the target_id of the transition.") + flowNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          try { sourceIndex = transMap->get("source_index").toInt(); } catch (...) {}
          try { targetIndex = transMap->get("target_index").toInt(); } catch (...) {}

          ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(NULL_ID, type, sourceId, sourceIndex, targetId, targetIndex);
          connParam->setNew();
          flowParam->addStmConnection(connParam);
        }
        DDEBUG("Load Transactions Finished");
      }
      /////
      if(importMasterModel(flowMap, modelMasterList, path, errMessage)==false) return false;
    }
  } catch (...) {
    DDEBUG("Flow File parse ERROR");
  }
  return true;
}

}
