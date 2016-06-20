#ifndef TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED
#define TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED

#include "TeachingTypes.h"
#include "SampleHiroController.h"

namespace teaching {

class TaskExecutor {
public:
  static TaskExecutor* instance();
  ~TaskExecutor();

  std::vector<CommandDefParam*> getCommandDefList();
  CommandDefParam* getCommandDef(const std::string& commandName);

  void setRootName(std::string value);
  bool executeCommand (const std::string& commandName, const std::vector<CompositeParamType>& params, bool simulation = true)
    throw (CommandParseErrorException);
  bool attachModelItem(cnoid::BodyItemPtr object, int target);
  bool detachModelItem(cnoid::BodyItemPtr object, int target);

private:
  TaskExecutor();
  //std::vector<teaching::CompositeParamType> params_;
  SampleHiroController* handler_;

};

}
#endif
