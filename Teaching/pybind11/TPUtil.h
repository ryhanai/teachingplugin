/**
   @author Ryo Hanai
*/

#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <cnoid/BodyItem>
#include <cnoid/MessageView>
#include <cnoid/JointPath>
#include "Interpolator.h"
#include "../ControllerBase.h"

namespace teaching
{
  double toRad (double deg);
  Vector3 toRad (Vector3 degs);
  VectorX toRad (VectorX degs);

  template<typename T>
    void printLogAux (std::stringstream& ss, const T& x)
  {
    ss << x;
    // std::cout << ss.str() << std::endl;
    MessageView::instance()->putln(ss.str());
  }

  template<typename T, typename ... Rest>
    void printLogAux (std::stringstream& ss, const T& x, const Rest& ... rest)
  {
    ss << x;
    printLogAux (ss, rest...);
  }

  template<typename T, typename ... Rest>
    void printLog (const T& x, const Rest& ... rest)
  {
    std::stringstream ss;
    printLogAux (ss, x, rest...);
  }

  typedef std::tuple<double, cnoid::VectorXd> JointTrajectoryPoint;
  typedef std::tuple<std::vector<std::string>, std::vector<JointTrajectoryPoint> > JointTrajectory;

  class CartesianInterpolator // Nextage: straight-line, spherical linear interpolation
  {
  public:
    CartesianInterpolator() { }
    void clear();

    int numSamples() const { return ts_.size(); }
    double domainLower () const { return ts_.empty() ? 0.0 : ts_.front(); }
    double domainUpper () const { return ts_.empty() ? 0.0 : ts_.back(); }
    void appendSample(double t, const cnoid::Vector3& xyz, const cnoid::Matrix3d& rotation);
    void update();
    cnoid::SE3 interpolate(double t);

  private:
    cnoid::Interpolator<cnoid::VectorXd> vInterpolator_;

    std::vector<double> ts_;
    std::vector<cnoid::Matrix3d> qsamples_;
  };

  class AttachedModel
  {
  public:
    AttachedModel() {};
    AttachedModel(AttachedModel& source)
      : handLink(source.handLink), objectLink(source.objectLink),
        posVal(source.posVal),
        object(source.object) {
    };

    cnoid::Link* handLink;
    cnoid::Link* objectLink;
    cnoid::BodyItemPtr object;
    std::vector<double> posVal;
  };


  class ControllerException
  {
  public:
    ControllerException () { }
    ControllerException (const std::string& message) { setMessage (message); }
    virtual ~ControllerException () { }
    const std::string& message () const { return message_; }
    void setMessage (const std::string& message) { message_ = message; }
  private:
    std::string message_;
  };

  class RobotNotFoundException : public ControllerException
  {
  public:
    RobotNotFoundException (const std::string& name) {
      setMessage("Robot [" + name + "] not found");
    }
  };

  class ItemNotFoundException : public ControllerException
  {
  public:
    ItemNotFoundException (const std::string& name) {
      setMessage("Item [" + name + "] not found");
    }
  };

  class CommandNotFoundException : public ControllerException
  {
  public:
    CommandNotFoundException (const std::string& name) {
      setMessage("Command [" + name + "] not found");
    };
  };

  class UnexpectedArgumentException : public ControllerException
  {
  public:
    UnexpectedArgumentException (boost::bad_get& e) {
      setMessage(e.what());
    }
  };

  class UndefinedToolNumberException : public ControllerException
  {
  public:
    UndefinedToolNumberException (const int n) {
      setMessage("tool number [" + std::to_string(n) + "] is not defined in the controller");
    }
  };

  class UndefinedToolLinkException : public ControllerException
  {
  public:
    UndefinedToolLinkException (const std::string& s){
      setMessage("tool link [" + s + "] does not exist in the robot model.");
    }
  };

  class IKFailureException : public ControllerException
  {
  public:
    IKFailureException (const std::string& message) {
      setMessage("IK failure [" + message);
    }
  };
  
  class TPInterface
  {
  public:
    static TPInterface& instance ();

    // controller-dependent
    void setToolLink (int toolNumber, std::string linkName) { toolLinks_[toolNumber] = linkName; }
    std::string getToolLinkName (int toolNumber) { return toolLinks_[toolNumber]; }
    cnoid::Link* getToolLink (int toolNumber);
    cnoid::BodyItem* getRobotItem ();
    cnoid::BodyPtr getRobotBody ();
    void setRobotName (std::string robotName) { robotName_ = robotName; }
    std::string getRobotName () { return robotName_; }
    JointPathPtr getJointPath (const std::string& endLinkName);
    cnoid::BodyItem* findItemByName (const std::string& name);


    // Model action
    bool updateAttachedModels ();
    void clearAttachedModels () { attachedModels_.clear(); }
    bool attachModelItem(cnoid::BodyItemPtr object, int target);
    bool detachModelItem(cnoid::BodyItemPtr object, int target);

    // Simulator configuration
    void setTimeStep (double seconds) { dt_ = seconds; }
    double getTimeStep () { return dt_; }


    cnoid::Interpolator<cnoid::VectorXd> ji_;
    CartesianInterpolator ci_;
    bool interpolate(const VectorXd& qStart, const VectorXd& qGoal, double duration,
                     JointTrajectory& traj);
    JointTrajectory interpolate(const VectorXd& qStart, const VectorXd& qGoal, double duration);
    
    bool interpolate(std::vector<std::string>& jointNames,
                    const VectorXd& qGoal, double duration,
                     JointTrajectory& traj);
    bool interpolate(JointPathPtr jointPath,
                    const VectorXd& qGoal, double duration,
                     JointTrajectory& traj);
    JointTrajectory interpolate(JointPathPtr jointPath,
                                const Vector3& xyz, const Vector3& rpy, double duration);

    bool followTrajectory(const JointTrajectory& traj);
    cnoid::VectorXd getCurrentJointAngles();
    cnoid::VectorXd getStandardPose();
    
    void commandListUpdate();

  private:
    TPInterface ();
    std::vector<AttachedModel*> attachedModels_;

    // controller-dependent
    std::string robotName_;
    std::map<int, std::string> toolLinks_;
    double dt_ = 0.05;

  };

  typedef std::shared_ptr<TPInterface> TPInterfacePtr;
}
