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
  //DDEBUG("PythonControllerWrapper::getCommandDefList");
  std::vector<CommandDefParam*> result;

  PythonExecutor executor;
  if (executor.eval("controller.getCommandDefList()") == false) return result;

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
            if (key == "name" || key == "type" || key == "length" || key == "direction") continue;
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

        int direction = 0;
        try {
          QString strDir = QString::fromStdString(argMap->get("direction").toString());
          if (strDir == "out") direction = 1;
          else if (strDir == "inout") direction = 2;
        } catch (...) {
        }

        ArgumentDefParam* arg = new ArgumentDefParam(argName.toStdString(), argType.toStdString(), aLength, direction);
        cmdDef->addArgument(arg);
      }
    }
  }

  if(0<errMessage.length()) {
    result.clear();
    QMessageBox::warning(0, "PythonController", errMessage);
    return result;
  }

  return result;
}

bool PythonControllerWrapper::executeCommand(const std::string& commandName,
                                              TaskModelParamPtr taskParam,
                                              ElementStmParamPtr targetParam,
                                              bool isReal) {
  DDEBUG("PythonControllerWrapper::executeCommand");

  CommandDefParam* def = TaskExecutor::instance()->getCommandDef(commandName);
  std::vector<ArgumentDefParam*> argDefList = def->getArgList();
  vector<ArgumentParamPtr> argList = targetParam->getArgList();
  if (argDefList.size() != argList.size()) return false;

  TeachingUtil::setGlobalParam(taskParam);

  std::stringstream pythonScriptStream;
  std::stringstream varScriptStream;
  std::stringstream outScriptStream;

  pythonScriptStream << "controller." << commandName << "(";
  for (int idxArg = 0; idxArg < argList.size(); idxArg++) {
    DDEBUG_V("index : %d, %d", idxArg, argList.size());

    ArgumentParamPtr arg = argList[idxArg];
    ArgumentDefParam* argDef = argDefList[idxArg];
    QString valueDesc = arg->getValueDesc();
    DDEBUG_V("valueDesc : %s, Direction : %d", valueDesc.toStdString().c_str(), argDef->getDirection());
    if (idxArg > 0) {
      pythonScriptStream << ", ";
    }

    if (argDef->getDirection() != 0 
          && (argDef->getType() == "int" || argDef->getType() == "double")) {
      varScriptStream << "    global " << valueDesc.toStdString() << std::endl;
      varScriptStream << "    _" << valueDesc.toStdString() << " = TPValue(" << valueDesc.toStdString() << ")" << std::endl;
      outScriptStream << "    " << valueDesc.toStdString() << " = _" << valueDesc.toStdString() << ".to_tp()" << std::endl;

      pythonScriptStream << "_" << valueDesc.toStdString();
    } else {
      pythonScriptStream << valueDesc.toStdString();
    }
  }
  if(isReal) {
    pythonScriptStream << ", True";
  } else {
    pythonScriptStream << ", False";
  }
  pythonScriptStream << ")";

  if (outScriptStream.str().length() == 0) {
    DDEBUG_V("python script : %s", pythonScriptStream.str().c_str());
    PythonExecutor executor;
    if (executor.eval(pythonScriptStream.str()) == false) {
      DDEBUG("PythonControllerWrapper::executeCommand Error");
      return false;
    }

  } else {
    std::stringstream scriptStream;
    scriptStream << "def stub():" << std::endl;
    scriptStream << varScriptStream.str();
    scriptStream << "    ret = " << pythonScriptStream.str() << std::endl;
    scriptStream << outScriptStream.str();
    scriptStream << "    return ret" << std::endl;
    scriptStream << "stub()";

    DDEBUG_V("python script(Out) : %s", scriptStream.str().c_str());
    PythonExecutor executor;
    if (executor.execCode(scriptStream.str()) == false) {
      DDEBUG("PythonControllerWrapper::executeCommand(Out) Error");
      return false;
    }
  }

  DDEBUG("PythonControllerWrapper::executeCommand End");
  return true;
}

void PythonControllerWrapper::initialize() {
}

cnoid::Link* PythonControllerWrapper::getToolLink(int toolNumber) {
  PythonExecutor executor;
  if (executor.eval("controller.getToolLinkName( " + QString::number(toolNumber).toStdString() + ")") == false) {
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
