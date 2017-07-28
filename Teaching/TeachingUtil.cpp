#include <fstream>
#include <iostream>
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <cnoid/UTF8>
//#include "TaskExecutor.h"

#include "PythonWrapper.h"
#include "Calculator.h"

#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

bool TeachingUtil::loadModelDetail(QString& strFName, ModelParam* targetModel) {
  QString strPath = QFileInfo(strFName).absolutePath();
  QFile fileTxt(strFName);
  fileTxt.open(QIODevice::ReadOnly);
  QTextStream in(&fileTxt);
  QString contents = in.readAll();
  QStringList lineList = contents.split("\n");
  QStringList loaded;
  for (int index = 0; index < lineList.length(); index++) {
    QString line = lineList.at(index);
    if (line.contains("url") == false)  continue;
    QStringList itemList = line.trimmed().split(" ");
    bool isHit = false;
    for (int idxItem = 0; idxItem < itemList.length(); idxItem++) {
      QString item = itemList.at(idxItem);
      if (isHit) {
        QString modelName = item.replace("\"", "");
        if (loaded.contains(modelName)) continue;
        loaded.append(modelName);
        ModelDetailParam* detail = new ModelDetailParam(NULL_ID, modelName);
        detail->setNew();
        QString fileName = strPath + QString("/") + modelName;
        QFile fileModel(fileName);
        fileModel.open(QIODevice::ReadOnly);
        detail->setData(fileModel.readAll());
        targetModel->addModelDetail(detail);
        loadModelDetail(fileName, targetModel);
        isHit = false;
      }
      if (item == QString("url")) {
        isHit = true;
      }
    }
  }
  return true;
}

bool TeachingUtil::loadTaskDef(QString& strFName, std::vector<TaskModelParam*>& taskInstList) {
  YAMLReader pyaml;
  try {
    DDEBUG_V("TaskDef File : %s", strFName.toStdString().c_str());
    if (pyaml.load(strFName.toUtf8().constData())) {
      QString path = QFileInfo(strFName).absolutePath();
      Listing* taskList = pyaml.document()->toListing();
      for (int idxTask = 0; idxTask < taskList->size(); idxTask++) {
        cnoid::ValueNode* eachTask = taskList->at(idxTask);
        Mapping* taskMap = eachTask->toMapping();
        //
        QString taskName = "";
        QString taskComment = "";
        QString taskExecEnv = "";
        try { taskName = QString::fromStdString(taskMap->get("taskName").toUTF8String()); }
        catch (...) { continue; }
        try { taskComment = QString::fromStdString(taskMap->get("comment").toUTF8String()).replace("|", "\n"); }
        catch (...) {}
        try { taskExecEnv = QString::fromStdString(taskMap->get("initialize").toUTF8String()).replace("|", "\n"); }
        catch (...) {}

        TaskModelParam* taskParam = new TaskModelParam(NULL_ID, taskName, taskComment, taskExecEnv, -1, "", "");
        taskParam->setNew();
        taskInstList.push_back(taskParam);
        //
        try {
          Listing* modelList = taskMap->get("models").toListing();
          for (int idxModel = 0; idxModel < modelList->size(); idxModel++) {
            Mapping* modelMap = modelList->at(idxModel)->toMapping();
            QString modelName = "";
            QString modelRName = "";
            QString modelType = "";
            QString modelFileName = "";
            double posX = 0.0; double posY = 0.0; double posZ = 0.0;
            double rotX = 0.0; double rotY = 0.0; double rotZ = 0.0;

            try { modelName = QString::fromStdString(modelMap->get("name").toUTF8String()); }
            catch (...) {}
            try {
              modelRName = QString::fromStdString(modelMap->get("rname").toUTF8String());
            }
            catch (...) {
              DDEBUG_V("Model RName NOT EXIST : %s", modelName.toStdString().c_str());
              return false;
            }
            try { modelType = QString::fromStdString(modelMap->get("type").toUTF8String()); }
            catch (...) {}
            try { modelFileName = QString::fromStdString(modelMap->get("file_name").toUTF8String()); }
            catch (...) {}
            try { posX = modelMap->get("pos_x").toDouble(); }
            catch (...) {}
            try { posY = modelMap->get("pos_y").toDouble(); }
            catch (...) {}
            try { posZ = modelMap->get("pos_z").toDouble(); }
            catch (...) {}
            try { rotX = modelMap->get("rot_x").toDouble(); }
            catch (...) {}
            try { rotY = modelMap->get("rot_y").toDouble(); }
            catch (...) {}
            try { rotZ = modelMap->get("rot_z").toDouble(); }
            catch (...) {}

            if (modelType.length() == 0) modelType = "Env";
            ModelParam* modelParam = new ModelParam(NULL_ID, getModelType(modelType), modelName, modelRName, modelFileName, posX, posY, posZ, rotX, rotY, rotZ, true);
            DDEBUG_V("Model Name : %s, %s", modelFileName.toStdString().c_str(), modelRName.toStdString().c_str());
            if (0 < modelFileName.length()) {
              QString strFullModelFile = path + QString("/") + modelFileName;
              QFile file(strFullModelFile);
              file.open(QIODevice::ReadOnly);
              modelParam->setData(file.readAll());
              //
              //ŽQÆƒ‚ƒfƒ‹‚Ì“Ç‚Ýž‚Ý
              TeachingUtil::loadModelDetail(strFullModelFile, modelParam);
            }
            taskParam->addModel(modelParam);
          }
          DDEBUG("Load Model Finished");
        }
        catch (...) {
          DDEBUG("Load Model Failed");
        }
        //
        try {
          Listing* paramList = taskMap->get("parameters").toListing();
          for (int idxParam = 0; idxParam < paramList->size(); idxParam++) {
            Mapping* paramMap = paramList->at(idxParam)->toMapping();
            int paramType;
            QString paramModelName = "";
            QString paramName = "";
            QString paramRName = "";
            QString paramUnit = "";
            QString paramValue = "";
            QString paramElemTypes = "";
            int paramNum;

            try { paramType = paramMap->get("type").toInt(); }
            catch (...) {}
            try { paramName = QString::fromStdString(paramMap->get("name").toUTF8String()); }
            catch (...) {}
            try {
              paramRName = QString::fromStdString(paramMap->get("rname").toUTF8String());
            }
            catch (...) {
              DDEBUG_V("Parameter RName NOT EXIST : %s", paramName.toStdString().c_str());
              return false;
            }
            try { paramModelName = QString::fromStdString(paramMap->get("model_name").toUTF8String()); }
            catch (...) {}
            try { paramUnit = QString::fromStdString(paramMap->get("units").toUTF8String()); }
            catch (...) {}
            try { paramNum = paramMap->get("elem_num").toInt(); }
            catch (...) {}
            try { paramElemTypes = QString::fromStdString(paramMap->get("elem_type").toUTF8String()); }
            catch (...) {}
            try { paramValue = QString::fromStdString(paramMap->get("values").toUTF8String()); }
            catch (...) {}
            if (paramType == PARAM_KIND_MODEL) {
              paramNum = 6;
              if (paramModelName.length() == 0) {
                DDEBUG("model_name is REQUREIED");
                return false;
              }
              bool isExist = false;
              for (int index = 0; index < taskParam->getModelList().size(); index++) {
                ModelParam* model = taskParam->getModelList()[index];
                if (paramModelName == model->getRName()) {
                  isExist = true;
                  break;
                }
              }
              if (isExist == false) {
                DDEBUG_V("Target Model[%s] NOT EXIST", paramModelName.toStdString().c_str());
                return false;
              }
            }
            DDEBUG_V("paramName : %s, elem_type : %s", paramName.toStdString().c_str(), paramElemTypes.toStdString().c_str());
            ParameterParam* param = new ParameterParam(NULL_ID, paramType, paramModelName, paramNum, paramElemTypes, -1, paramName, paramRName, paramUnit);
            param->setElemTypes(paramElemTypes);
            param->setDBValues(paramValue);
            param->setNewForce();
            taskParam->addParameter(param);
          }
          DDEBUG("Load Parameters Finished");
        }
        catch (...) {
          DDEBUG("Load Parameters Failed");
        }
        //
        try {
          Listing* stateList = taskMap->get("states").toListing();
          for (int idxState = 0; idxState < stateList->size(); idxState++) {
            Mapping* stateMap = stateList->at(idxState)->toMapping();
            int id, type;
            QString cmdName = "";
            double posX, posY;
            QString dispName = "";
            QString condition = "";

            try { id = stateMap->get("id").toInt(); }
            catch (...) { continue; }
            try { type = stateMap->get("type").toInt(); }
            catch (...) { continue; }
            try { cmdName = QString::fromStdString(stateMap->get("cmd_name").toUTF8String()); }
            catch (...) {}
            try { posX = stateMap->get("pos_x").toDouble(); }
            catch (...) {}
            try { posY = stateMap->get("pos_y").toDouble(); }
            catch (...) {}
            try { condition = QString::fromStdString(stateMap->get("condition").toUTF8String()).replace("|", "\n"); }
            catch (...) {}
            try { dispName = QString::fromStdString(stateMap->get("disp_name").toUTF8String()); }
            catch (...) {}
            DDEBUG_V("cmd_name[%s]", cmdName.toStdString().c_str());
            ElementStmParam* stateParam = new ElementStmParam(NULL_ID, type, cmdName, dispName, posX, posY, condition);
            stateParam->setOrgId(id);
            stateParam->setNew();
            taskParam->addStmElement(stateParam);
            //
            try {
              Listing* stateActionList = stateMap->get("model_actions").toListing();
              for (int idxAction = 0; idxAction < stateActionList->size(); idxAction++) {
                Mapping* actionMap = stateActionList->at(idxAction)->toMapping();
                QString action = "";
                QString model = "";
                QString target = "";

                try { action = QString::fromStdString(actionMap->get("action").toUTF8String()); }
                catch (...) {}
                try { model = QString::fromStdString(actionMap->get("model").toUTF8String()); }
                catch (...) {}
                try { target = QString::fromStdString(actionMap->get("target").toUTF8String()); }
                catch (...) {}
                DDEBUG_V("action : %s, model : %s, target : %s", action.toStdString().c_str(), model.toStdString().c_str(), target.toStdString().c_str());
                ElementStmActionParam* actionParam = new ElementStmActionParam(NULL_ID, NULL_ID, idxAction, action, model, target, true);
                stateParam->addModelAction(actionParam);
              }
              DDEBUG("Load model_actions Finished");
            }
            catch (...) {
            }
            //
            try {
              Listing* argList = stateMap->get("arguments").toListing();
              for (int idxArg = 0; idxArg < argList->size(); idxArg++) {
                Mapping* argMap = argList->at(idxArg)->toMapping();
                int seq;
                QString name = "";
                QString valueDesc = "";

                try { seq = argMap->get("seq").toInt(); }
                catch (...) { continue; }
                try { name = QString::fromStdString(argMap->get("name").toUTF8String()); }
                catch (...) {}
                try { valueDesc = QString::fromStdString(argMap->get("value").toUTF8String()).replace("|", "\n"); }
                catch (...) {}
                DDEBUG_V("seq : %d, name : %s, valueDesc : %s", seq, name.toStdString().c_str(), valueDesc.toStdString().c_str());
                ArgumentParam* argParam = new ArgumentParam(NULL_ID, NULL_ID, seq, name, valueDesc);
                argParam->setNew();
                stateParam->addArgument(argParam);
              }
              DDEBUG("Load arguments Finished");
            }
            catch (...) {
            }
          }
          DDEBUG("Load States Finished");
        }
        catch (...) {
          DDEBUG("Load States Failed");
        }
        //
        try {
          Listing* transList;
          try {
            transList = taskMap->get("transactions").toListing();
          }
          catch (...) {
            transList = taskMap->get("transitions").toListing();
          }
          for (int idxTrans = 0; idxTrans < transList->size(); idxTrans++) {
            Mapping* transMap = transList->at(idxTrans)->toMapping();
            int sourceId, targetId;
            QString condition = "";

            try { sourceId = transMap->get("source_id").toInt(); }
            catch (...) { continue; }
            try { targetId = transMap->get("target_id").toInt(); }
            catch (...) { continue; }
            try { condition = QString::fromStdString(transMap->get("guard").toUTF8String()); }
            catch (...) {}
            ConnectionStmParam* connParam = new ConnectionStmParam(NULL_ID, sourceId, targetId, condition);
            connParam->setNew();
            taskParam->addStmConnection(connParam);
          }
          DDEBUG("Load Transactions Finished");
        }
        catch (...) {
          DDEBUG("Load Transactions Failed");
        }
        //
        try {
          Listing* filesList = taskMap->get("files").toListing();
          for (int idxFiles = 0; idxFiles < filesList->size(); idxFiles++) {
            Mapping* fileMap = filesList->at(idxFiles)->toMapping();
            QString fileName = "";

            try { fileName = QString::fromStdString(fileMap->get("name").toUTF8String()); }
            catch (...) {}
            FileDataParam* fileParam = new FileDataParam(NULL_ID, fileName);
            fileParam->setNew();
            if (0 < fileName.length()) {
              QFile file(path + QString("/") + fileName);
              file.open(QIODevice::ReadOnly);
              fileParam->setData(file.readAll());
            }
            taskParam->addFile(fileParam);
          }
          DDEBUG("Load Files Finished");
        }
        catch (...) {
          DDEBUG("Load Files Failed");
        }
        //
        try {
          Listing* imagesList = taskMap->get("images").toListing();
          for (int idxImages = 0; idxImages < imagesList->size(); idxImages++) {
            Mapping* imageMap = imagesList->at(idxImages)->toMapping();
            QString fileName = "";

            try { fileName = QString::fromStdString(imageMap->get("name").toUTF8String()); }
            catch (...) {}
            ImageDataParam* imageParam = new ImageDataParam(NULL_ID, fileName);
            imageParam->setNew();
            if (0 < fileName.length()) {
              QImage image(path + QString("/") + fileName);
              imageParam->setData(image);
            }
            taskParam->addImage(imageParam);
          }
          DDEBUG("Load Images Finished");
        }
        catch (...) {
          DDEBUG("Load Images Failed");
        }
      }
    }

  }
  catch (...) {
    DDEBUG("TaskInstance File parse ERROR");
  }
  return true;
}

void TeachingUtil::loadTaskDetailData(TaskModelParam* target) {
  if (target->IsLoaded()) return;
  DDEBUG("loadTaskDetailData");

  for (int idxModel = 0; idxModel < target->getModelList().size(); idxModel++) {
    ModelParam* model = target->getModelList()[idxModel];
    if (ChoreonoidUtil::makeModelItem(model) == false) {
      model->setModelItem(0);
    }
  }
  for (int idxFig = 0; idxFig < target->getImageList().size(); idxFig++) {
    ImageDataParam* param = target->getImageList()[idxFig];
    param->setData(db2Image(param->getName(), param->getRawData()));
  }

  target->setLoaded(true);
}

QImage TeachingUtil::db2Image(const QString& name, const QByteArray& source) {
  string strType = "";
  if (name.toUpper().endsWith("PNG")) {
    strType = "PNG";
  } else if (name.toUpper().endsWith("JPG")) {
    strType = "JPG";
  }
  QImage result = QImage::fromData(source, strType.c_str());

  return result;
}

bool TeachingUtil::outputTaskDef(QString& strFName, TaskModelParam* targetTask) {
  QString path = QFileInfo(strFName).absolutePath();
  Listing* archive = new Listing();
  archive->setDoubleFormat("%.9g");
  MappingPtr taskNode = archive->newMapping();
  taskNode->writeUTF8("taskName", targetTask->getName().toUtf8(), DOUBLE_QUOTED);
  taskNode->writeUTF8("comment", targetTask->getComment().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  taskNode->writeUTF8("initialize", targetTask->getExecEnv().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  //
  if (0 < targetTask->getModelList().size()) {
    Listing* modelsNode = taskNode->createListing("models");
    for (int index = 0; index < targetTask->getModelList().size(); index++) {
      ModelParam* param = targetTask->getModelList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr modelNode = modelsNode->newMapping();
      modelNode->writeUTF8("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      modelNode->writeUTF8("rname", param->getRName().toUtf8(), DOUBLE_QUOTED);
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
      modelNode->writeUTF8("file_name", param->getFileName().toUtf8(), DOUBLE_QUOTED);
      modelNode->write("pos_x", param->getPosX());
      modelNode->write("pos_y", param->getPosY());
      modelNode->write("pos_z", param->getPosZ());
      modelNode->write("rot_x", param->getRotRx());
      modelNode->write("rot_y", param->getRotRy());
      modelNode->write("rot_z", param->getRotRz());
      //
      if (0 < param->getFileName().length()) {
        QFile file(path + QString("/") + param->getFileName());
        file.open(QIODevice::WriteOnly);
        QByteArray data = param->getData();
        file.write(data);
        file.close();
      }
      //
      for (int idxSub = 0; idxSub < param->getModelDetailList().size(); idxSub++) {
        ModelDetailParam* detail = param->getModelDetailList()[idxSub];
        if (0 < detail->getFileName().length()) {
          QFile fileSub(path + QString("/") + detail->getFileName());
          fileSub.open(QIODevice::WriteOnly);
          QByteArray dataSub = detail->getData();
          fileSub.write(dataSub);
          fileSub.close();
        }
      }
    }
  }
  //
  if (0 < targetTask->getStmElementList().size()) {
    Listing* statesNode = taskNode->createListing("states");
    for (int index = 0; index < targetTask->getStmElementList().size(); index++) {
      ElementStmParam* param = targetTask->getStmElementList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr stateNode = statesNode->newMapping();
      stateNode->write("id", param->getId());
      stateNode->write("type", param->getType());
      if (param->getType() == ELEMENT_COMMAND) {
        stateNode->writeUTF8("cmd_name", param->getCmdName().toUtf8(), DOUBLE_QUOTED);
        stateNode->writeUTF8("disp_name", param->getCmdDspName().toUtf8(), DOUBLE_QUOTED);
      }
      stateNode->writeUTF8("condition", param->getCondition().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
      stateNode->write("pos_x", param->getPosX());
      stateNode->write("pos_y", param->getPosY());
      //
      if (0 < param->getActionList().size()) {
        Listing* actionsNode = stateNode->createListing("model_actions");
        for (int idxAction = 0; idxAction < param->getActionList().size(); idxAction++) {
          ElementStmActionParam* actParam = param->getActionList()[idxAction];
          MappingPtr actionNode = actionsNode->newMapping();
          actionNode->writeUTF8("action", actParam->getAction().toUtf8(), DOUBLE_QUOTED);
          actionNode->writeUTF8("model", actParam->getModel().toUtf8(), DOUBLE_QUOTED);
          if (0 < actParam->getTarget().length()) {
            actionNode->writeUTF8("target", actParam->getTarget().toUtf8(), DOUBLE_QUOTED);
          }
        }
      }
      //
      if (0 < param->getArgList().size()) {
        Listing* argsNode = stateNode->createListing("arguments");
        for (int idxArg = 0; idxArg < param->getArgList().size(); idxArg++) {
          ArgumentParam* argParam = param->getArgList()[idxArg];
          MappingPtr argNode = argsNode->newMapping();
          argNode->write("seq", argParam->getSeq());
          argNode->writeUTF8("name", argParam->getName().toUtf8(), DOUBLE_QUOTED);
          argNode->writeUTF8("value", argParam->getValueDesc().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
        }
      }
    }
  }
  //
  if (0 < targetTask->getStmConnectionList().size()) {
    Listing* connsNode = taskNode->createListing("transitions");
    for (int index = 0; index < targetTask->getStmConnectionList().size(); index++) {
      ConnectionStmParam* param = targetTask->getStmConnectionList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      if (param->getSourceId() == param->getTargetId()) continue;
      MappingPtr connNode = connsNode->newMapping();
      connNode->write("source_id", param->getSourceId());
      connNode->write("target_id", param->getTargetId());
      if (0 < param->getCondition().size()) {
        connNode->writeUTF8("guard", param->getCondition().toUtf8(), DOUBLE_QUOTED);
      }
    }
  }
  //
  if (0 < targetTask->getParameterList().size()) {
    Listing* paramsNode = taskNode->createListing("parameters");
    for (int index = 0; index < targetTask->getParameterList().size(); index++) {
      ParameterParam* param = targetTask->getParameterList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr paramNode = paramsNode->newMapping();
      paramNode->writeUTF8("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      paramNode->writeUTF8("rname", param->getRName().toUtf8(), DOUBLE_QUOTED);
      paramNode->write("type", param->getType());
      paramNode->writeUTF8("model_name", param->getModelName().toUtf8(), DOUBLE_QUOTED);
      paramNode->writeUTF8("units", param->getUnit().toUtf8(), DOUBLE_QUOTED);
      paramNode->write("elem_num", param->getElemNum());
      paramNode->write("elem_type", param->getElemTypeNo());
      paramNode->writeUTF8("values", param->getDBValues().toUtf8(), DOUBLE_QUOTED);
    }
  }
  //
  if (0 < targetTask->getFileList().size()) {
    Listing* filesNode = taskNode->createListing("files");
    for (int index = 0; index < targetTask->getFileList().size(); index++) {
      FileDataParam* param = targetTask->getFileList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr fileNode = filesNode->newMapping();
      fileNode->writeUTF8("name", param->getName().toUtf8(), DOUBLE_QUOTED);
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
  if (0 < targetTask->getImageList().size()) {
    Listing* imagesNode = taskNode->createListing("images");
    for (int index = 0; index < targetTask->getImageList().size(); index++) {
      ImageDataParam* param = targetTask->getImageList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr imageNode = imagesNode->newMapping();
      imageNode->writeUTF8("name", param->getName().toUtf8(), DOUBLE_QUOTED);
      if (0 < param->getName().length()) {
        QImage data = param->getData();
        data.save(path + QString("/") + param->getName());
      }
    }
  }
  //
  YAMLWriter writer(strFName.toUtf8().constData());
  writer.setKeyOrderPreservationMode(true);
  writer.putNode(archive);

  return true;
}

bool TeachingUtil::outputFlow(QString& strFName, FlowParam* targetFlow) {
  QString path = QFileInfo(strFName).absolutePath();
  Listing* archive = new Listing();
  archive->setDoubleFormat("%.9g");
  MappingPtr flowNode = archive->newMapping();
  flowNode->writeUTF8("flowName", targetFlow->getName().toUtf8(), DOUBLE_QUOTED);
  flowNode->writeUTF8("comment", targetFlow->getComment().replace("\n", "|").toUtf8(), DOUBLE_QUOTED);
  //
  if (0 < targetFlow->getStmElementList().size()) {
    Listing* statesNode = flowNode->createListing("states");
    for (int index = 0; index < targetFlow->getStmElementList().size(); index++) {
      ElementStmParam* param = targetFlow->getStmElementList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      MappingPtr stateNode = statesNode->newMapping();
      stateNode->write("id", param->getId());
      stateNode->write("type", param->getType());
      if (param->getType() == ELEMENT_COMMAND) {
        stateNode->write("task_id", param->getTaskParam()->getId());
        stateNode->writeUTF8("task_name", param->getCmdDspName().toUtf8(), DOUBLE_QUOTED);
      }
      stateNode->write("pos_x", param->getPosX());
      stateNode->write("pos_y", param->getPosY());
    }
  }
  //
  //
  if (0 < targetFlow->getStmConnectionList().size()) {
    Listing* connsNode = flowNode->createListing("transitions");
    for (int index = 0; index < targetFlow->getStmConnectionList().size(); index++) {
      ConnectionStmParam* param = targetFlow->getStmConnectionList()[index];
      if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
      if (param->getSourceId() == param->getTargetId()) continue;
      MappingPtr connNode = connsNode->newMapping();
      connNode->write("source_id", param->getSourceId());
      connNode->write("target_id", param->getTargetId());
      if (0 < param->getCondition().size()) {
        connNode->writeUTF8("guard", param->getCondition().toUtf8(), DOUBLE_QUOTED);
      }
    }
  }
  //
  YAMLWriter writer(strFName.toUtf8().constData());
  writer.setKeyOrderPreservationMode(true);
  writer.putNode(archive);

  return true;
}

int TeachingUtil::getModelType(QString& source) {
  int result = 0;
  if (source == "Env") {
    result = 0;
  } else if (source == "E.E.") {
    result = 1;
  } else if (source == "Work") {
    result = 2;
  }
  return result;
}
/////
QTableWidget* UIUtil::makeTableWidget(int colNo, bool isExpanding) {
  QTableWidget* target = new QTableWidget(0, colNo);
  target->setSelectionBehavior(QAbstractItemView::SelectRows);
  target->setSelectionMode(QAbstractItemView::SingleSelection);
  target->setEditTriggers(QAbstractItemView::NoEditTriggers);
  target->verticalHeader()->setVisible(false);
  target->setRowCount(0);
  if (isExpanding) {
    target->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }
  return target;
}

QTableWidgetItem* UIUtil::makeTableItemWithData(QTableWidget* table, int rowNo, int colNo, const QString& text, int data) {
  QTableWidgetItem* target = makeTableItem(table, rowNo, colNo, text);
  target->setData(Qt::UserRole, data);
  return target;
}

QTableWidgetItem* UIUtil::makeTableItem(QTableWidget* table, int rowNo, int colNo, const QString& text) {
  QTableWidgetItem* target = new QTableWidgetItem;
  table->setItem(rowNo, colNo, target);
  target->setData(Qt::UserRole, 1);
  target->setText(text);
  return target;
}
/////
bool SettingManager::loadSetting() {
  std::ifstream inputFs(PARAMETER_FILE.c_str());
  if (inputFs.fail()) return false;

  std::string eachLine;
  std::vector<std::string> contents;

  while (getline(inputFs, eachLine)) {
    contents.push_back(eachLine);
  }

  logLevel_ = LOG_NO;
  logDir_ = "Log";

  for (int index = 0; index < contents.size(); index++) {
    QString each = QString::fromStdString(contents[index]);
    int startPos = each.indexOf(":");
    if (startPos == std::string::npos) continue;

    QString value = each.mid(startPos + 1);
    value = value.trimmed();
    //
    if (each.startsWith("database")) {
      dataBase_ = value.toStdString();

    } else if (each.startsWith("robotModelName")) {
      robotModelName_ = value.toStdString();

    } else if (each.startsWith("logLevel")) {
      logLevel_ = value.toInt();

    } else if (each.startsWith("logDir")) {
      logDir_ = value.toStdString();

    } else if (each.startsWith("application")) {
      QStringList elems = value.split("|");
      if (elems.size() < 2) continue;
      appMap_[elems[0].toStdString()] = elems[1].toStdString();
    }
  }

  return true;
}

bool SettingManager::saveSetting() {
  std::ofstream outputFs(PARAMETER_FILE.c_str());

  outputFs << "database : " << dataBase_ << std::endl;
  outputFs << "robotModelName : " << robotModelName_ << std::endl;
  outputFs << "logLevel : " << logLevel_ << std::endl;
  outputFs << "logDir : " << logDir_ << std::endl;
  std::map<std::string, std::string>::const_iterator itKey;
  for (itKey = appMap_.begin(); itKey != appMap_.end(); itKey++) {
    outputFs << "application : " << itKey->first << "|" << itKey->second << std::endl;
  }

  return true;
}

void SettingManager::clearExtList() {
  appMap_.clear();
}

void SettingManager::setTargetApp(std::string strExt, std::string strApp) {
  appMap_[strExt] = strApp;
}

std::string SettingManager::getTargetApp(std::string strExt) {
  map<std::string, std::string>::iterator itr = appMap_.find(strExt);
  std::string result = "";
  if (itr != appMap_.end()) {
    result = appMap_[strExt];
  }
  return result;
}

std::vector<std::string> SettingManager::getExtList() {
  std::vector<std::string> result;

  std::map<std::string, std::string>::const_iterator itKey;
  for (itKey = appMap_.begin(); itKey != appMap_.end(); itKey++) {
    result.push_back(itKey->first);
  }
  return result;
}
/////
ArgumentEstimator* EstimatorFactory::createArgEstimator(TaskModelParam* targetParam) {
  ArgumentEstimator* handler = new Calculator();
  //ArgumentEstimator* handler = new PythonWrapper();
  handler->initialize(targetParam);
  return handler;
}

void EstimatorFactory::deleteArgEstimator(ArgumentEstimator* handler) {
  handler->finalize();
  delete handler;
}

}
