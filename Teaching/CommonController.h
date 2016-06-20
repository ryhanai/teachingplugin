#ifndef TEACHING_COMMON_CONTROLLER_H_INCLUDED
#define TEACHING_COMMON_CONTROLLER_H_INCLUDED

namespace teaching {

typedef boost::variant<double, int, std::string, cnoid::Vector2, cnoid::Vector3, cnoid::VectorXd, cnoid::Matrix3> CompositeParamType;

class CommandParseErrorException {
public:
  CommandParseErrorException (const std::string& message) { setMessage (message); }
  virtual ~CommandParseErrorException () { }
  const std::string& message () const { return message_; }
  void setMessage (const std::string& message) { message_ = message; }
private:
  std::string message_;
};

class CommonController {
public:
    virtual bool attachModelItem(cnoid::BodyItemPtr object, int target) = 0;
    virtual bool detachModelItem(cnoid::BodyItemPtr object, int target) = 0;
    virtual std::vector<CommandDefParam*> getCommandDefList() = 0;

    virtual bool executeCommand (const std::string& commandName, const std::vector<CompositeParamType>& params, bool simulation = true) = 0;

    inline void setRootName(std::string value) { this->rootName = value; }

protected:
  std::string rootName;
};

}
#endif
