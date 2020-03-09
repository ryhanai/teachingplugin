#include "PythonControllerWrapper.h"

#include <cnoid/PluginManager>
#include <cnoid/PythonExecutor>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>

#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"
#include "LoggerUtil.h"
#include "gettext.h"

using namespace cnoid;

namespace teaching {

PythonControllerWrapper* PythonControllerWrapper::instance () {
  static PythonControllerWrapper* c = new PythonControllerWrapper();
  return c;
}

std::vector<CommandDefParam*> PythonControllerWrapper::getCommandDefList() {
  DDEBUG("PythonControllerWrapper::getCommandDefList");
  std::vector<CommandDefParam*> result;

  PythonExecutor executor;
  if (executor.eval("getCommandDefList()") == false) return result;

  std::string returnValue = executor.returnValue().cast<std::string>();

  YAMLReader pyaml;
  if(pyaml.parse(returnValue)==false) {
    QString errMsg =  QString::fromStdString(_("Command information is invalid.")).append("\n").append(QString::fromStdString(pyaml.errorMessage()));
    QMessageBox::warning(0, "PythonController", errMsg);
    return result;
  }

  Listing* cmdList = pyaml.document()->toListing();
  QString errMessage = "";
  for (int index = 0; index < cmdList->size(); index++) {
    cnoid::ValueNode* eachCmd = cmdList->at(index);
    Mapping* cmdMap = eachCmd->toMapping();

    QString cmdName = "";
    try {
      cmdName = QString::fromStdString(cmdMap->get("name").toString());
    } catch (...) {
      errMessage.append(_("Failed to read the 'name' of the command. index=") + QString::number(index)).append("\n");
      continue;
    }
    //
    bool existInvalidKey = false;
    for(auto it = cmdMap->begin(); it != cmdMap->end(); ++it){
        string key = it->first;
        if (key == "name" || key == "dispName" || key == "retType" || key == "args") continue;
        errMessage.append(_("YAML key is invalid. key=") + QString::fromStdString(key)).append(" (").append(cmdName).append(")").append("\n");
        existInvalidKey = true;
    }
    if (existInvalidKey) continue;

    QString cmdDispName = "";
    try {
      cmdDispName = QString::fromStdString(cmdMap->get("dispName").toString());
    } catch (...) {
      errMessage.append(_("Failed to read the 'dispName' of the command. (")).append(cmdName).append(")").append("\n");
      continue;
    }

    QString retType = "";
    try {
      retType = QString::fromStdString(cmdMap->get("retType").toString());
    } catch (...) {
      errMessage.append(("Failed to read the 'retType' of the command. (")).append(cmdName).append(")").append("\n");
      continue;
    }

    CommandDefParam* cmdDef = new CommandDefParam(cmdName, cmdDispName, retType);
    result.push_back(cmdDef);

    Listing* argsList = cmdMap->findListing("args");
    if (argsList) {
      for (int idxArg = 0; idxArg < argsList->size(); idxArg++) {
        Mapping* argMap = argsList->at(idxArg)->toMapping();

        QString argName = "";
        try {
          argName = QString::fromStdString(argMap->get("name").toString());
        } catch (...) {
          errMessage.append(_("Failed to read the 'name' of the argument. (")).append(cmdName).append(")").append("\n");
          break;
        }

        bool existInvalidKeyArg = false;
        for(auto it = argMap->begin(); it != argMap->end(); ++it){
            string key = it->first;
            if (key == "name" || key == "type" || key == "length") continue;
            errMessage.append(_("The YAML key in the argument definition is invalid. key=") + QString::fromStdString(key)).append(" (").append(cmdName).append("-").append(argName).append(")").append("\n");
            existInvalidKeyArg = true;
        }
        if (existInvalidKeyArg) break;

        QString argType = "";
        try {
          argType = QString::fromStdString(argMap->get("type").toString());
        } catch (...) {
          errMessage.append(_("Failed to read the 'type' of the argument. (")).append(cmdName).append("-").append(argName).append(")").append("\n");
          continue;
        }

        int aLength = 0;
        try {
          QString argLength = QString::fromStdString(argMap->get("length").toString());
          aLength = argLength.toInt();
        } catch (...) {
        }
        ArgumentDefParam* arg = new ArgumentDefParam(argName.toStdString(), argType.toStdString(), aLength);
        cmdDef->addArgument(arg);
      }
    }
  }

  if(0<errMessage.length()) {
    result.clear();
    QMessageBox::warning(0, "PythonController", errMessage);
    return result;
  }

  DDEBUG("PythonControllerWrapper::getCommandDefList End");
  return result;
}

bool PythonControllerWrapper::executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params, bool isReal) {
  DDEBUG("PythonControllerWrapper::executeCommand");
  PythonExecutor executor;

	Listing* archive = new Listing();
  archive->setDoubleFormat("%.9g");
  MappingPtr argsNode = archive->newMapping();

  argsNode->write("commandName", commandName, DOUBLE_QUOTED);
  argsNode->write("isReal", isReal);
  Listing* paramNode = argsNode->createListing("args");

  CommandDefParam* def = TaskExecutor::instance()->getCommandDef(commandName);
  if (def->getArgList().size() != params.size()) return false;
  std::vector<ArgumentDefParam*> argList = def->getArgList();

  for (int index = 0; index < params.size(); index++) {
    CompositeParamType param = params[index];
    MappingPtr eachNode = paramNode->newMapping();
    eachNode->write("name", argList[index]->getName(), DOUBLE_QUOTED);

    if(param.type() == typeid(double)) {
      eachNode->write("value", boost::get<double>(param));

    } else if (param.type() == typeid(int)) {
      eachNode->write("value", boost::get<int>(param));

    } else if (param.type() == typeid(std::string)) {
      eachNode->write("value", boost::get<string>(param), DOUBLE_QUOTED);

    } else if (param.type() == typeid(cnoid::Vector2)) {
      cnoid::Vector2 val = boost::get<cnoid::Vector2>(param);
      Listing* valNode = eachNode->createListing("value");
      valNode->append(val[0]);
      valNode->append(val[1]);

    } else if (param.type() == typeid(cnoid::Vector3)) {
      cnoid::Vector3 val = boost::get<cnoid::Vector3>(param);

      Listing* valNode = eachNode->createListing("value");
      valNode->append(val[0]);
      valNode->append(val[1]);
      valNode->append(val[2]);

    } else if (param.type() == typeid(cnoid::VectorXd)) {
      cnoid::VectorXd val = boost::get<cnoid::VectorXd>(param);

      Listing* valNode = eachNode->createListing("value");
      for (int index = 0; index < val.size(); index++) {
        valNode->append(val[index]);
      }

    } else if (param.type() == typeid(cnoid::Matrix3)) {
      cnoid::Matrix3 val = boost::get<cnoid::Matrix3>(param);

      Listing* valNode = eachNode->createListing("value");
      for (int idxRow = 0; idxRow < 3; idxRow++) {
        for (int idxCol = 0; idxCol < 3; idxCol++) {
          valNode->append(val(idxCol,idxRow));
        }
      }

    } else if (param.type() == typeid(RelativeTrajectory)) {
      RelativeTrajectory val = boost::get<RelativeTrajectory>(param);

      Listing* valNode = eachNode->createListing("value");

      MappingPtr trajNode = valNode->newMapping();
      trajNode->write("baseObject", val.base_object_name, DOUBLE_QUOTED);
      trajNode->write("baseLink", val.base_link_name, DOUBLE_QUOTED);
      trajNode->write("targetObject", val.target_object_name, DOUBLE_QUOTED);
      trajNode->write("targetLink", val.target_link_name, DOUBLE_QUOTED);

      vector<TrajectoryPoint> viaList = val.points;
      if( 0<viaList.size()) {
        Listing* viasNode = trajNode->createListing("via_points");
        for (TrajectoryPoint viaParam : viaList) {
          MappingPtr viaNode = viasNode->newMapping();
          viaNode->write("time", viaParam.time_from_start);
          Listing* posList = viaNode->createFlowStyleListing("pos");
          posList->append(viaParam.link_position.translation().x());
          posList->append(viaParam.link_position.translation().y());
          posList->append(viaParam.link_position.translation().z());
          posList->append(viaParam.link_position.rotation().x());
          posList->append(viaParam.link_position.rotation().y());
          posList->append(viaParam.link_position.rotation().z());
          posList->append(viaParam.link_position.rotation().w());
        }
      }
    }
  }

  std::stringstream ss;
  YAMLWriter writer(ss);
  writer.setKeyOrderPreservationMode(true);
  writer.putNode(archive);
  DDEBUG_V("Convert Yaml %s", ss.str().c_str());

  if (executor.eval("executeCommand( \"\"\"" + ss.str() + "\"\"\")") == false) {
    DDEBUG("PythonControllerWrapper::executeCommand Error");
    return false;
  }
  DDEBUG("PythonControllerWrapper::executeCommand End");

  return true;
}

  bool PythonControllerWrapper::executeCommand(const std::string& commandName,
                                               TaskModelParamPtr taskParam,
                                               ElementStmParamPtr targetParam,
                                               bool isReal) {
    DDEBUG("PythonControllerWrapper::executeCommand");
    setGlobalParam(taskParam);

    std::stringstream pythonScriptStream;
    pythonScriptStream << "controller." << commandName << "(";
    vector<ArgumentParamPtr> argList = targetParam->getArgList();
    for (int idxArg = 0; idxArg < argList.size(); idxArg++) {
      DDEBUG_V("index : %d, %d", idxArg, argList.size());

      ArgumentParamPtr arg = argList[idxArg];
      QString valueDesc = arg->getValueDesc();
      DDEBUG_V("valueDesc : %s", valueDesc.toStdString().c_str());
      if (idxArg > 0) {
        pythonScriptStream << ", ";
      }
      pythonScriptStream << valueDesc.toStdString();
    }
    if(isReal) {
      pythonScriptStream << ", True";
    } else {
      pythonScriptStream << ", False";
    }
    pythonScriptStream << ")";

    DDEBUG_V("python script : %s", pythonScriptStream.str().c_str());
    PythonExecutor executor;
    if (executor.eval(pythonScriptStream.str()) == false) {
      DDEBUG("PythonControllerWrapper::executeCommand Error");
      return false;
    }

    DDEBUG("PythonControllerWrapper::executeCommand End");
    return true;
  }

  void PythonControllerWrapper::setGlobalParam(TaskModelParamPtr targetParam) {
    if (targetParam == NULL) return;
    DDEBUG("PythonControllerWrapper::setGlobalParam");

    std::stringstream script;
    script << "def setGlobalParam():" << std::endl;

    for (ParameterParamPtr param : targetParam->getActiveParameterList()) {
      QString paramName = param->getRName();
      std::stringstream scriptParam;

      if (param->getParamType() == PARAM_TYPE_FRAME) {
        vector<double> paramList;
        if(param->getType() == PARAM_KIND_MODEL) {
          int modelId = param->getModelId();
          vector<ModelParamPtr> modelList = targetParam->getActiveModelList();
          std::vector<ModelParamPtr>::iterator model = std::find_if(modelList.begin(), modelList.end(), ModelParamComparator(modelId));
          if (model == modelList.end()) continue;

          std::stringstream scriptModel;
          scriptModel << "Frame(xyzRPY=[";
          scriptModel << (*model)->getPosX() << ",";
          scriptModel << (*model)->getPosY() << ",";
          scriptModel << (*model)->getPosZ() << ",";
          scriptModel << (*model)->getRotRx() << ",";
          scriptModel << (*model)->getRotRy() << ",";
          scriptModel << (*model)->getRotRz();
          scriptModel << "])";

          int feature_id = param->getModelParamId();
          if (feature_id != NULL_ID) {
            ModelMasterParamPtr master = (*model)->getModelMaster();
            if (!master) continue;
            vector<ModelParameterParamPtr> masterParamList = master->getModelParameterList();
            vector<ModelParameterParamPtr>::iterator masterParamItr = find_if(masterParamList.begin(), masterParamList.end(), ModelMasterParamComparator(feature_id));
            if (masterParamItr == masterParamList.end()) continue;
            QString desc = (*masterParamItr)->getValueDesc();
            //desc = desc.replace("origin", QString::fromStdString(scriptModel.str()));
            scriptParam << "  " << paramName.toStdString() << " = ";
            // scriptParam << "list(map(sum, zip(" << desc.toStdString() << ", " << scriptModel.str() << ")))" << std::endl;
            scriptParam << "Frame(xyzRPY=" << desc.toStdString() << ", parent=" << scriptModel.str() << ")" << std::endl;

          } else {
            scriptParam << "  " << paramName.toStdString() << " = ";
            scriptParam << scriptModel.str() << std::endl;
          }

        } else {
          scriptParam << "  " << paramName.toStdString() << " = Frame(xyzRPY=[";
          for (int idxElem = 0; idxElem < 6; idxElem++) {
            QString each = QString::fromStdString(param->getValues(idxElem));
            scriptParam << each.toDouble() << ",";
          }
          scriptParam << "])" << std::endl;

          for (unsigned int index = 0; index < paramList.size(); index++) {
            scriptParam << paramList.at(index) << ",";
          }
        }

      } else {
        QString paramStr = QString::fromStdString(param->getValues(0));
        scriptParam << "  " << paramName.toStdString() << " = " << paramStr.toDouble() << std::endl;
      }
      DDEBUG_V("name : %s, %s", paramName.toStdString().c_str(), scriptParam.str().c_str());
      /////
      script << "  global " << paramName.toStdString() << std::endl;
      script << scriptParam.str();
    }
    script << "setGlobalParam()";

    string strCon = script.str();
    DDEBUG_V("%s", strCon.c_str());
    PythonExecutor executor;
    if (executor.execCode(strCon)==false) {
      DDEBUG("PythonControllerWrapper::setGlobalParam Error");
      return;
    }
  }
  


void PythonControllerWrapper::initialize() {
}

cnoid::Link* PythonControllerWrapper::getToolLink(int toolNumber) {
  PythonExecutor executor;
  if (executor.eval("getToolLinkName( " + QString::number(toolNumber).toStdString() + ")") == false) {
    QMessageBox::warning(0, "PythonController", _("Python command (getToolLinkName) execution failed."));
    return 0;
  }
  std::string linkName = executor.returnValue().cast<std::string>();
  string robotName = SettingManager::getInstance().getRobotModelName();
  BodyItem* robotModel = ChoreonoidUtil::searchParentModel(robotName);
  if (!robotModel) return 0;
  Link* targetLink = robotModel->body()->link(linkName);
  if (!targetLink) {
    QMessageBox::warning(0, "PythonController", _("Failed to get link information."));
    return 0;
  }
  return targetLink;
}

}
