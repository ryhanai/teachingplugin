#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "DataBaseManager.h"
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>

#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace teaching {

bool TeachingUtil::importTask(QString& strFName, std::vector<TaskModelParamPtr>& taskInstList, vector<ModelMasterParamPtr>& modelMasterList, QString& errMessage) {
  YAMLReader pyaml;
  DDEBUG_V("TaskDef File : %s", strFName.toStdString().c_str());
  try {
    if (pyaml.load(strFName.toUtf8().constData()) == false) {
      errMessage = _("Failed to load YAML file : ") + strFName;
      DDEBUG(errMessage.toStdString().c_str());
      return false;
    }

    QString path = QFileInfo(strFName).absolutePath();
    Listing* taskList = pyaml.document()->toListing();
    for (int idxTask = 0; idxTask < taskList->size(); idxTask++) {
      cnoid::ValueNode* eachTask = taskList->at(idxTask);
      Mapping* taskMap = eachTask->toMapping();
      //
      QString taskName = "";
      QString taskComment = "";
      QString taskExecEnv = "";
      try {
        taskName = QString::fromStdString(taskMap->get("taskName").toString());
      } catch (...) {
        errMessage = _("Failed to read the taskName of the task.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (taskName.isEmpty()) {
        errMessage = _("Task Name is EMPTY.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (TeachingUtil::checkNameStr(taskName) == false) {
        errMessage = _("Characters that can not be used in names are included.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try { taskComment = QString::fromStdString(taskMap->get("comment").toString()).replace("|", "\n"); } catch (...) {}
      //try { taskComment = QString::fromStdString(taskMap->get("comment").toString()); } catch (...) {}
      try { taskExecEnv = QString::fromStdString(taskMap->get("initialize").toString()).replace("|", "\n"); } catch (...) {}

			TaskModelParamPtr taskParam = std::make_shared<TaskModelParam>(NULL_ID, taskName, taskComment, taskExecEnv, -1, "", "");
      taskParam->setNew();
      //taskInstList.push_back(taskParam);
      QString taskNameErr = QString::fromStdString("\n TaskName=[") + taskName + QString::fromStdString("]");
      //
      if (importTaskModel(taskMap, taskParam, taskNameErr, errMessage) == false) return false;
      if (importTaskParameter(taskMap, taskParam, taskNameErr, errMessage) == false) return false;
      if (importTaskState(taskMap, taskParam, taskNameErr, errMessage) == false) return false;
      if (importTaskFile(taskMap, taskParam, path, taskNameErr, errMessage) == false) return false;
      if (importTaskImage(taskMap, taskParam, path, taskNameErr, errMessage) == false) return false;
      if (importMasterModel(taskMap, modelMasterList, path, errMessage) == false) return false;
      //
      taskInstList.push_back(taskParam);
    }
  } catch (...) {
    DDEBUG("TaskInstance File parse ERROR");
  }
  return true;
}

bool TeachingUtil::importTaskModel(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage) {
  Listing* modelList = taskMap->findListing("models");
  if (modelList) {
    for (int idxModel = 0; idxModel < modelList->size(); idxModel++) {
      Mapping* modelMap = modelList->at(idxModel)->toMapping();
      QString modelRName = "";
      int model_id = -1;
      int master_id = -1;
      int hide = 0;
      QString modelType = "";
      double posX = 0.0; double posY = 0.0; double posZ = 0.0;
      double rotX = 0.0; double rotY = 0.0; double rotZ = 0.0;

      try {
        modelRName = QString::fromStdString(modelMap->get("rname").toString());
      } catch (...) {
        errMessage = _("Failed to read the rname of the task model.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (modelRName.isEmpty()) {
        errMessage = _("rname of the task model is EMPTY.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (TeachingUtil::checkNameStr(modelRName) == false) {
        errMessage = _("Characters that can not be used in names are included.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      //
      QString modelNameErr = QString::fromStdString("\n ModelName=[") + modelRName + QString::fromStdString("]");
      try {
        model_id = modelMap->get("model_id").toInt();
      } catch (...) {
        errMessage = _("Failed to read the model_id of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        master_id = modelMap->get("master_id").toInt();
      } catch (...) {
        errMessage = _("Failed to read the master_id of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        modelType = QString::fromStdString(modelMap->get("type").toString());
      } catch (...) {
        errMessage = _("Failed to read the model_type of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        posX = modelMap->get("pos_x").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the pos_x of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        posY = modelMap->get("pos_y").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the pos_y of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        posZ = modelMap->get("pos_z").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the pos_z of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        rotX = modelMap->get("rot_x").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the rot_x of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        rotY = modelMap->get("rot_y").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the rot_y of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        rotZ = modelMap->get("rot_z").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the rot_z of the task model.") + taskNameErr + modelNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try { hide = modelMap->get("hide").toInt(); } catch (...) {}

      if (modelType.length() == 0) modelType = "Env";
      ModelParamPtr modelParam = std::make_shared<ModelParam>(NULL_ID, master_id, getModelType(modelType), modelRName, posX, posY, posZ, rotX, rotY, rotZ, hide, true);
      taskParam->addModel(modelParam);
    }
  }
  DDEBUG("Load Model Finished");
  return true;
}

bool TeachingUtil::importTaskParameter(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage) {
  Listing* paramList = taskMap->findListing("parameters");
  if (paramList) {
    for (int idxParam = 0; idxParam < paramList->size(); idxParam++) {
      Mapping* paramMap = paramList->at(idxParam)->toMapping();
      QString paramName = "";
      QString paramRName = "";
      QString paramUnit = "";
      QString paramValue = "";
      int type, paramType, hide;
      int model_id = NULL_ID;
      int model_param_id = NULL_ID;

      try { paramName = QString::fromStdString(paramMap->get("name").toString()); } catch (...) {}
      QString paramNameErr = QString::fromStdString("\n ParameterName=[") + paramName + QString::fromStdString("]");
      try {
        paramRName = QString::fromStdString(paramMap->get("rname").toString());
      } catch (...) {
        errMessage = _("Failed to read the rname of the task parameter.") + taskNameErr + paramNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (paramRName.isEmpty()) {
        errMessage = _("Rname of the task parameter is EMPTY.") + taskNameErr + paramNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try {
        type = paramMap->get("type").toInt();
      } catch (...) {
        errMessage = _("Failed to read the type of the task parameter.") + taskNameErr + paramNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try {
        paramType = paramMap->get("param_type").toInt();
      } catch (...) {
        errMessage = _("Failed to read the elem_num of the task parameter.") + taskNameErr + paramNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try { paramUnit = QString::fromStdString(paramMap->get("units").toString()); } catch (...) {}
      try { paramValue = QString::fromStdString(paramMap->get("values").toString()); } catch (...) {}
      try { model_id = paramMap->get("model_id").toInt(); } catch (...) {}
      try { model_param_id = paramMap->get("model_param_id").toInt(); } catch (...) {}
      try { hide = paramMap->get("hide").toInt(); } catch (...) {}

      ParameterParamPtr param = std::make_shared<ParameterParam>(NULL_ID, type, paramType, NULL_ID, paramName, paramRName, paramUnit, model_id, model_param_id, hide);
      param->setDBValues(paramValue);
      param->setNewForce();
      taskParam->addParameter(param);
    }
  }
  DDEBUG("Load Parameters Finished");
  return true;
}

bool TeachingUtil::importTaskState(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage) {
  unordered_map<int, int> stateIdMap;
  Listing* stateList = taskMap->findListing("states");
  if (stateList) {
    for (int idxState = 0; idxState < stateList->size(); idxState++) {
      Mapping* stateMap = stateList->at(idxState)->toMapping();
      int id, type;
      QString cmdName = "";
      double posX, posY;
      QString dispName = "";
      QString condition = "";

      try {
        id = stateMap->get("id").toInt();
      } catch (...) {
        errMessage = _("Failed to read the id of the state.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try {
        type = stateMap->get("type").toInt();
      } catch (...) {
        errMessage = _("Failed to read the type of the state.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try {
        posX = stateMap->get("pos_x").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the pos_x of the state.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try {
        posY = stateMap->get("pos_y").toDouble();
      } catch (...) {
        errMessage = _("Failed to read the pos_y of the state.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      try { cmdName = QString::fromStdString(stateMap->get("cmd_name").toString()); } catch (...) {}
      try { condition = QString::fromStdString(stateMap->get("condition").toString()).replace("|", "\n"); } catch (...) {}
      try { dispName = QString::fromStdString(stateMap->get("disp_name").toString()); } catch (...) {}

      int newId = taskParam->getMaxStateId();
      stateIdMap[id] = newId;
      ElementStmParamPtr stateParam = std::make_shared<ElementStmParam>(newId, type, cmdName, dispName, posX, posY, condition);
      stateParam->setNew();
      taskParam->addStmElement(stateParam);
      //
      Listing* stateActionList = stateMap->findListing("model_actions");
      if (stateActionList) {
        for (int idxAction = 0; idxAction < stateActionList->size(); idxAction++) {
          Mapping* actionMap = stateActionList->at(idxAction)->toMapping();
          QString action = "";
          QString model = "";
          QString target = "";

          try {
            action = QString::fromStdString(actionMap->get("action").toString());
          } catch (...) {
            errMessage = _("Failed to read the action of the state action.") + taskNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          if (action.isEmpty()) {
            errMessage = _("Action of the state action is EMPTY.") + taskNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          try {
            model = QString::fromStdString(actionMap->get("model").toString());
          } catch (...) {
            errMessage = _("Failed to read the model of the state action.") + taskNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          if (model.isEmpty()) {
            errMessage = _("Model of the state action is EMPTY.") + taskNameErr;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          try { target = QString::fromStdString(actionMap->get("target").toString()); } catch (...) {}
          DDEBUG_V("action : %s, model : %s, target : %s", action.toStdString().c_str(), model.toStdString().c_str(), target.toStdString().c_str());
          ElementStmActionParamPtr actionParam = std::make_shared<ElementStmActionParam>(NULL_ID, idxAction, action, model, target, true);
          stateParam->addModelAction(actionParam);
        }
      }
      DDEBUG("Load model_actions Finished");
      //
      Listing* argList = stateMap->findListing("arguments");
      for (int idxArg = 0; idxArg < argList->size(); idxArg++) {
        Mapping* argMap = argList->at(idxArg)->toMapping();
        int seq;
        QString name = "";
        QString valueDesc = "";

        try {
          seq = argMap->get("seq").toInt();
        } catch (...) {
          errMessage = _("Failed to read the seq of the argument.") + taskNameErr;
          DDEBUG(errMessage.toStdString().c_str());
          return false;
        }

        try { name = QString::fromStdString(argMap->get("name").toString()); } catch (...) {}
        try { valueDesc = QString::fromStdString(argMap->get("value").toString()).replace("|", "\n"); } catch (...) {}
        DDEBUG_V("seq : %d, name : %s, valueDesc : %s", seq, name.toStdString().c_str(), valueDesc.toStdString().c_str());
        ArgumentParamPtr argParam = std::make_shared<ArgumentParam>(NULL_ID, seq, name, valueDesc);
        argParam->setNew();
        stateParam->addArgument(argParam);
      }
      DDEBUG("Load arguments Finished");
    }
  }
  DDEBUG("Load States Finished");
  //
  Listing* transList;
  transList = taskMap->findListing("transitions");
  if (!transList) {
    transList = taskMap->findListing("transactions");
  }
  if (transList) {
    for (int idxTrans = 0; idxTrans < transList->size(); idxTrans++) {
      Mapping* transMap = transList->at(idxTrans)->toMapping();
      int sourceId, targetId, sourceIndex, targetIndex;

      try {
        sourceId = transMap->get("source_id").toInt();
      } catch (...) {
        errMessage = _("Failed to read the source_id of the transition.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        targetId = transMap->get("target_id").toInt();
      } catch (...) {
        errMessage = _("Failed to read the target_id of the transition.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        sourceIndex = transMap->get("source_index").toInt();
      } catch (...) {
        errMessage = _("Failed to read the source_index of the transition.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        targetIndex = transMap->get("target_index").toInt();
      } catch (...) {
        errMessage = _("Failed to read the target_index of the transition.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      int newSourceId = stateIdMap[sourceId];
      int newTargetId = stateIdMap[targetId];
      ConnectionStmParamPtr connParam = std::make_shared<ConnectionStmParam>(NULL_ID, 0, newSourceId, sourceIndex, newTargetId, targetIndex);
      connParam->setNew();
      taskParam->addStmConnection(connParam);
    }
  }
  DDEBUG("Load Transactions Finished");
  return true;
}

bool TeachingUtil::importTaskFile(Mapping* taskMap, TaskModelParamPtr taskParam, QString& path, QString taskNameErr, QString& errMessage) {
  Listing* filesList = taskMap->findListing("files");
  if (filesList) {
    for (int idxFiles = 0; idxFiles < filesList->size(); idxFiles++) {
      Mapping* fileMap = filesList->at(idxFiles)->toMapping();
      QString fileName = "";

      try {
        fileName = QString::fromStdString(fileMap->get("name").toString());
      } catch (...) {
        errMessage = _("Failed to read the name of the file.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (fileName.isEmpty()) {
        errMessage = _("FILE name is EMPTY.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      FileDataParamPtr fileParam = std::make_shared<FileDataParam>(NULL_ID, NULL_ID, fileName);
      fileParam->setNew();
      QFile file(path + QString("/") + fileName);
      if (file.open(QIODevice::ReadOnly) == false) {
        errMessage = _("Failed to open FILE.") + taskNameErr + " " + fileName;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      fileParam->setData(file.readAll());
      taskParam->addFile(fileParam);
    }
  }
  DDEBUG("Load Files Finished");
  return true;
}

bool TeachingUtil::importTaskImage(Mapping* taskMap, TaskModelParamPtr taskParam, QString& path, QString taskNameErr, QString& errMessage) {
  Listing* imagesList = taskMap->findListing("images");
  if (imagesList) {
    for (int idxImages = 0; idxImages < imagesList->size(); idxImages++) {
      Mapping* imageMap = imagesList->at(idxImages)->toMapping();
      QString fileName = "";

      try {
        fileName = QString::fromStdString(imageMap->get("name").toString());
      } catch (...) {
        errMessage = _("Failed to read the name of the image.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      if (fileName.isEmpty()) {
        errMessage = _("IMAGE name is EMPTY.") + taskNameErr;
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }

      ImageDataParamPtr imageParam = std::make_shared<ImageDataParam>(NULL_ID, NULL_ID, fileName);
      imageParam->setNew();
      QImage image(path + QString("/") + fileName);
      imageParam->setData(image);
      taskParam->addImage(imageParam);
    }
  }
  DDEBUG("Load Images Finished");
  return true;
}

bool TeachingUtil::loadModelDetail(QString& strFName, ModelMasterParamPtr targetModel) {
  DDEBUG_V("TeachingUtil::loadModelDetail:%s", strFName.toStdString().c_str());
  QString strPath = QFileInfo(strFName).absolutePath();
	QFile fileTxt(strFName);
  if (fileTxt.exists() == false) {
    DDEBUG_V("Target file NOT EXIST. %s", strFName.toStdString().c_str());
    return false;
  }
  if (fileTxt.open(QIODevice::ReadOnly) == false) {
    DDEBUG_V("Failed to open file. %s", strFName.toStdString().c_str());
    return false;
  }
	QTextStream in(&fileTxt);
	QString contents = in.readAll();
	QStringList lineList = contents.split("\n");
	QStringList loaded;
	for (int index = 0; index < lineList.length(); index++) {
		QString line = lineList.at(index);
		if (line.contains("url") == false)  continue;
    if (line.contains("wrl") == false) continue;
		QStringList itemList = line.trimmed().split(" ");
		bool isHit = false;
		for (int idxItem = 0; idxItem < itemList.length(); idxItem++) {
			QString item = itemList.at(idxItem);
			if (isHit) {
				QString modelName = item.replace("\"", "");
				if (loaded.contains(modelName)) {
          isHit = false;
          continue;
        }
				loaded.append(modelName);
				ModelDetailParamPtr detail = std::make_shared<ModelDetailParam>(NULL_ID, modelName);
				detail->setNew();
				QString fileName = strPath + QString("/") + modelName;
        DDEBUG_V("TeachingUtil::loadModelDetail Target:%s", fileName.toStdString().c_str());
				QFile fileModel(fileName);
        if (fileModel.exists() == false) {
          DDEBUG_V("Target file NOT EXIST. %s", modelName.toStdString().c_str());
          return false;
        }
        if (fileModel.open(QIODevice::ReadOnly) == false) {
          DDEBUG_V("Failed to open file. %s", modelName.toStdString().c_str());
          return false;
        }
				detail->setData(fileModel.readAll());
				targetModel->addModelDetail(detail);
        if (loadModelDetail(fileName, targetModel) == false) return false;
				isHit = false;
			}
			if (item == QString("url")) {
				isHit = true;
			}
		}
	}
	return true;
}

void TeachingUtil::loadTaskDetailData(TaskModelParamPtr target) {
  if (target->IsLoaded()) return;
  DDEBUG("loadTaskDetailData");

  for (ModelParamPtr model : target->getActiveModelList()) {
    ModelMasterParamPtr master = model->getModelMaster();
    if (master) {
      if (ChoreonoidUtil::makeModelItem(master) == false) {
        master->setModelItem(0);
      }
    }
  }
  for (ImageDataParamPtr param : target->getActiveImageList()) {
    param->loadData();
  }

  target->setLoaded(true);
}

bool TeachingUtil::exportTask(QString& strFName, TaskModelParamPtr targetTask) {
  DDEBUG("TeachingUtil::exportTask");
	vector<ModelMasterParamPtr> masterList;

	QString path = QFileInfo(strFName).absolutePath();
	Listing* archive = new Listing();
  archive->setDoubleFormat("%.9g");
  MappingPtr taskNode = archive->newMapping();
  taskNode->write("taskName", targetTask->getName().toUtf8(), DOUBLE_QUOTED);
  taskNode->write("comment", targetTask->getComment().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  //taskNode->write("comment", targetTask->getComment().toUtf8(), DOUBLE_QUOTED);
  taskNode->write("initialize", targetTask->getExecEnv().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  //
  vector<ModelParamPtr> modelList = targetTask->getActiveModelList();
  if (0 < modelList.size()) {
    Listing* modelsNode = taskNode->createListing("models");
    for (ModelParamPtr param : modelList) {
      MappingPtr modelNode = modelsNode->newMapping();
      modelNode->write("model_id", param->getId());
      modelNode->write("rname", param->getRName().toUtf8(), DOUBLE_QUOTED);
			modelNode->write("master_id", param->getMasterId());
			string strType = "";
      int intType = param->getType();
      if (intType == MODEL_ENV) {
        strType = "Env.";
      } else if (intType == MODEL_EE) {
        strType = "E.E.";
      } else if (intType == MODEL_WORK) {
        strType = "Work";
      }
      modelNode->write("type", strType);
      modelNode->write("pos_x", param->getPosX());
      modelNode->write("pos_y", param->getPosY());
      modelNode->write("pos_z", param->getPosZ());
      modelNode->write("rot_x", param->getRotRx());
      modelNode->write("rot_y", param->getRotRy());
      modelNode->write("rot_z", param->getRotRz());
      modelNode->write("hide", param->getHide());
      //
			bool isExist = false;
			for(int idxMaster = 0; idxMaster < masterList.size(); idxMaster++) {
				ModelMasterParamPtr master = masterList[idxMaster];
				if (master->getId() == param->getMasterId()) {
					isExist = true;
					break;
				}
			}
			if (isExist == false) {
				masterList.push_back(param->getModelMaster());
			}
    }
  }
  //
  std::vector<ElementStmParamPtr> stateList = targetTask->getActiveStateList();
  if (0 < stateList.size()) {
    Listing* statesNode = taskNode->createListing("states");
    for (ElementStmParamPtr param : stateList) {
      MappingPtr stateNode = statesNode->newMapping();
      stateNode->write("id", param->getId());
      stateNode->write("type", param->getType());
      if (param->getType() == ELEMENT_COMMAND) {
        stateNode->write("cmd_name", param->getCmdName().toUtf8(), DOUBLE_QUOTED);
        stateNode->write("disp_name", param->getCmdDspName().toUtf8(), DOUBLE_QUOTED);
      }
      stateNode->write("condition", param->getCondition().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
      stateNode->write("pos_x", param->getPosX());
      stateNode->write("pos_y", param->getPosY());
      //
      vector<ElementStmActionParamPtr> actionList = param->getActiveStateActionList();
      if (0 <actionList.size()) {
        Listing* actionsNode = stateNode->createListing("model_actions");
        for (int idxAction = 0; idxAction < actionList.size(); idxAction++) {
					ElementStmActionParamPtr actParam = actionList[idxAction];
          MappingPtr actionNode = actionsNode->newMapping();
          actionNode->write("action", actParam->getAction().toUtf8(), DOUBLE_QUOTED);
          actionNode->write("model", actParam->getModel().toUtf8(), DOUBLE_QUOTED);
          if (0 < actParam->getTarget().length()) {
            actionNode->write("target", actParam->getTarget().toUtf8(), DOUBLE_QUOTED);
          }
        }
      }
      //
      vector<ArgumentParamPtr> argList = param->getActiveArgumentList();
      if (0 < argList.size()) {
        Listing* argsNode = stateNode->createListing("arguments");
        for (int idxArg = 0; idxArg < argList.size(); idxArg++) {
					ArgumentParamPtr argParam = argList[idxArg];
          MappingPtr argNode = argsNode->newMapping();
          argNode->write("seq", argParam->getSeq());
          argNode->write("name", argParam->getName().toUtf8(), DOUBLE_QUOTED);
          argNode->write("value", argParam->getValueDesc().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
        }
      }
    }
  }
  //
  vector<ConnectionStmParamPtr> transList = targetTask->getActiveTransitionList();
  if (0 < transList.size()) {
    Listing* connsNode = taskNode->createListing("transitions");
    for (ConnectionStmParamPtr param : transList) {
      if (param->getSourceId() == param->getTargetId()) continue;
      MappingPtr connNode = connsNode->newMapping();
      connNode->write("source_id", param->getSourceId());
      connNode->write("target_id", param->getTargetId());
      connNode->write("source_index", param->getSourceIndex());
      connNode->write("target_index", param->getTargetIndex());
    }
  }
  //
  vector<ParameterParamPtr> paramList = targetTask->getActiveParameterList();
  if (0 <paramList.size()) {
    Listing* paramsNode = taskNode->createListing("parameters");
    for (ParameterParamPtr param : paramList) {
      MappingPtr paramNode = paramsNode->newMapping();
      paramNode->write("type", param->getType());
      paramNode->write("param_type", param->getParamType());
      paramNode->write("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      paramNode->write("rname", param->getRName().toUtf8(), DOUBLE_QUOTED);
      paramNode->write("units", param->getUnit().toUtf8(), DOUBLE_QUOTED);
      paramNode->write("values", param->getDBValues().toUtf8(), DOUBLE_QUOTED);
			paramNode->write("model_id", param->getModelId());
      paramNode->write("model_param_id", param->getModelParamId());
      paramNode->write("hide", param->getHide());
    }
  }
  //
  vector<FileDataParamPtr> fileList = targetTask->getActiveFileList();
  if (0 < fileList.size()) {
    Listing* filesNode = taskNode->createListing("files");
    for (FileDataParamPtr param : fileList) {
      MappingPtr fileNode = filesNode->newMapping();
      fileNode->write("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      if (0 < param->getName().length()) {
        QFile file(path + QString("/") + param->getName());
        file.open(QIODevice::WriteOnly);
        QByteArray data = param->getData();
        file.write(data);
        file.close();
      }
    }
  }
  //
  vector<ImageDataParamPtr> imageList = targetTask->getActiveImageList();
  if (0 < imageList.size()) {
    Listing* imagesNode = taskNode->createListing("images");
    for (ImageDataParamPtr param : imageList) {
      MappingPtr imageNode = imagesNode->newMapping();
      imageNode->write("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      if (0 < param->getName().length()) {
        QImage data = param->getData();
        data.save(path + QString("/") + param->getName());
      }
    }
  }
	/////
	if (0 < masterList.size()) {
		Listing* mastesrNode = taskNode->createListing("model_master");
		for (ModelMasterParamPtr master : masterList) {
      ModelMasterParamPtr targetMaster = DatabaseManager::getInstance().getModelMaster(master->getId());
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

  return true;
}

}
