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
  if(executor.eval("getCommandDefList()")) {
    std::string returnValue = executor.returnValue().cast<std::string>();

    YAMLReader pyaml;
    if(pyaml.parse(returnValue)==false) {
      DDEBUG("Parse Error");
      return result;
    }

    Listing* cmdList = pyaml.document()->toListing();
    for (int index = 0; index < cmdList->size(); index++) {
      cnoid::ValueNode* eachCmd = cmdList->at(index);
      Mapping* cmdMap = eachCmd->toMapping();

      QString cmdName = "";
      QString cmdDispName = "";
      QString retType = "";

      try {
        cmdName = QString::fromStdString(cmdMap->get("name").toString());
      } catch (...) {
        QString errMessage = "Failed to read the name of the command.";
        DDEBUG(errMessage.toStdString().c_str());
        continue;
      }
      try {
        cmdDispName = QString::fromStdString(cmdMap->get("dispName").toString());
      } catch (...) {
        QString errMessage = "Failed to read the dispName of the command.";
        DDEBUG(errMessage.toStdString().c_str());
        continue;
      }
      try {
        retType = QString::fromStdString(cmdMap->get("retType").toString());
      } catch (...) {
        QString errMessage = "Failed to read the retType of the command.";
        DDEBUG(errMessage.toStdString().c_str());
        continue;
      }

      CommandDefParam* cmdDef = new CommandDefParam(cmdName, cmdDispName, retType);
      result.push_back(cmdDef);

      Listing* argsList = cmdMap->findListing("args");
      if (argsList) {
        for (int idxArg = 0; idxArg < argsList->size(); idxArg++) {
          Mapping* argMap = argsList->at(idxArg)->toMapping();
          QString argName = "";
          QString argType = "";
          int aLength = 0;
          try {
            argName = QString::fromStdString(argMap->get("name").toString());
          } catch (...) {
            continue;
          }
          try {
            argType = QString::fromStdString(argMap->get("type").toString());
          } catch (...) {
            continue;
          }
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

  //for(CompositeParamType param : params) {
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
    }
  }

  std::stringstream ss;
  YAMLWriter writer(ss);
  writer.setKeyOrderPreservationMode(true);
  writer.putNode(archive);
  DDEBUG_V("Convert Yaml %s", ss.str().c_str());

  if (executor.eval("executeCommand \"\"\"" + ss.str() + "\"\"\")") == false) {
    DDEBUG("PythonControllerWrapper::executeCommand Error");
    return false;
  }
  DDEBUG("PythonControllerWrapper::executeCommand End");

  return true;
}

void PythonControllerWrapper::initialize() {
}

cnoid::Link* PythonControllerWrapper::getToolLink(int toolNumber) {
  PythonExecutor executor;
  if (executor.eval("getToolLinkName " + QString::number(toolNumber).toStdString()) == false) {
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
