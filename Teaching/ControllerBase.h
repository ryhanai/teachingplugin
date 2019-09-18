#ifndef TEACHING_CONTROLLERBASE_H_INCLUDED
#define TEACHING_CONTROLLERBASE_H_INCLUDED

#include <cnoid/BodyItem>
#include <cnoid/Link>
#include <cnoid/EigenUtil>
#include <boost/variant.hpp>

#include "CommandDefTypes.h"

using namespace cnoid;

namespace teaching {

typedef boost::variant<double, int, std::string, cnoid::Vector2, cnoid::Vector3, cnoid::VectorXd, cnoid::Matrix3> CompositeParamType;
//typedef boost::variant<double, int, std::string, cnoid::VectorXd> CompositeParamType;

class CommandParseErrorException {
public:
  CommandParseErrorException(const std::string& message) { setMessage(message); }
  virtual ~CommandParseErrorException() { }
  const std::string& message() const { return message_; }
  void setMessage(const std::string& message) { message_ = message; }
private:
  std::string message_;
};

class ControllerBase {
public:
  virtual std::vector<CommandDefParam*> getCommandDefList() = 0;
  inline void setRootName(std::string value) { this->rootName = value; }

  virtual bool executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params, bool isReal=false) = 0;

  virtual void initialize() = 0; // R.Hanai
  virtual cnoid::Link* getToolLink(int toolNumber) = 0;

protected:
  std::string rootName;
};

}
#endif
