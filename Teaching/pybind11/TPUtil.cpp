/**
   @author Ryo Hanai
*/

#ifdef _WIN32
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>

#include <QCoreApplication>
#include <cnoid/ItemList>
#include <cnoid/RootItem>
#include <cnoid/ValueTree> // for Listing

#include "TPUtil.h"

namespace teaching
{
  double toRad (double deg) {
    return deg * M_PI/180.0;
  }
  // template<typename T>
  // T toRad (T degs)
  // {
  //   T rads(degs.array() * M_PI/180.0);
  //   return rads;
  // }
  Vector3 toRad (Vector3 degs)
  {
    Vector3 rads(degs.array() * M_PI/180.0);
    return rads;
  }

  VectorX toRad (VectorX degs)
  {
    VectorX rads(degs.array() * M_PI/180.0);
    return rads;
  }

  void CartesianInterpolator::clear()
  {
    vInterpolator_.clear();
    ts_.clear();
    qsamples_.clear();
  }

  void CartesianInterpolator::appendSample(double t, const Vector3& xyz, const Matrix3d& rotation)
  {
    VectorXd p(3);
    p.head<3>() = xyz;
    vInterpolator_.appendSample(t, p);
    ts_.push_back(t);
    qsamples_.push_back(rotation);
  }

  void CartesianInterpolator::update ()
  {
    vInterpolator_.update();
  }

  SE3 CartesianInterpolator::interpolate (double t)
  {
    VectorXd pd = vInterpolator_.interpolate(t);
    Vector3 p = pd.head<3>();
    double tdiff = ts_[1] - ts_[0];
    cnoid::Matrix3d mat0 = qsamples_[0];
    cnoid::Matrix3d mat1 = qsamples_[1];
    Quat q1(mat0);
    Quat q2(mat1);

    Quat q_slerp = q1.slerp(t / tdiff, q2);
    SE3 tf(p, q_slerp);
    return tf;
  }

  TPInterface& TPInterface::instance ()
  {
    static TPInterface tpif;
    return tpif;
  }

  TPInterface::TPInterface ()
  {
    clearAttachedModels();
  }

  bool TPInterface::attachModelItem (BodyItemPtr object, int target)
  {
    BodyPtr robotBody = getRobotItem()->body();

    printLog("ATTACH = ", object->name());
    
    try {
      Link* handLink = getToolLink(target);
      Link* objectLink = object->body()->link(0);
      Position relTrans = handLink->position().inverse()*objectLink->position();
      AttachedModel* model = new AttachedModel();
      model->handLink = handLink;
      model->objectLink = objectLink;
      for (int index = 0; index < 12; index++) {
        model->posVal.push_back(relTrans.data()[index]);
      }
      model->object = object;
      attachedModels_.push_back(model);

    } catch(...) {
      printLog("[attachModeItem] unknown link ID: ", target);
      return false;
    }

    return true;
  }

  bool TPInterface::detachModelItem (BodyItemPtr object, int target)
  {
    BodyPtr robotBody = getRobotItem()->body();

    try {
      Link* handLink = getToolLink(target);
      Link* objectLink = object->body()->link(0);

      for (std::vector<AttachedModel*>::iterator it = attachedModels_.begin(); it != attachedModels_.end();) {
        if ((*it)->handLink == handLink && (*it)->objectLink == objectLink) {
          it = attachedModels_.erase(it);
          printLog("detachModelItem");
        }
        else {
          ++it;
        }
      }
    }
    catch (...) {
      printLog("[detachModeItem] unknown link ID: ", target);
      return false;
    }

    return true;
  }

  cnoid::Link* TPInterface::getToolLink(int toolNumber)
  {
    BodyPtr robotBody = getRobotBody();
    std::string link_name = "";
    try {
      link_name = getToolLinkName(toolNumber);
    } catch (...) {
      throw UndefinedToolNumberException(toolNumber);
    }

    Link* link = robotBody->link(link_name);
    if (link) {
      return link;
    } else {
      throw UndefinedToolLinkException(link_name);
    }
  }

  BodyItem* TPInterface::getRobotItem ()
  {
    BodyItem* robotItem = findItemByName(robotName_);
    if (robotItem) {
      return robotItem;
    } else {
      throw RobotNotFoundException(robotName_);
    }
  }

  BodyItem* TPInterface::findItemByName (const std::string& name)
  {
    ItemList<BodyItem> bodyItems;
    bodyItems.extractChildItems(RootItem::instance());
    for (size_t i = 0; i < bodyItems.size(); i++) {
      BodyItem* item = bodyItems.get(i);
      if (item->name() == name) { return item; }
    }

    return NULL;
  }

  BodyPtr TPInterface::getRobotBody ()
  {
    BodyItem* robotItem = getRobotItem();
    BodyPtr robotBody = robotItem->body();
    return robotBody;
  }

  JointPathPtr TPInterface::getJointPath (const std::string& endLinkName)
  {
    BodyPtr body = getRobotBody();
    Link* base = body->rootLink();
    Link* tool = body->link(endLinkName);
    JointPathPtr joint_path = getCustomJointPath(body, base, tool);
    return joint_path;
  }

  bool TPInterface::updateAttachedModels ()
  {
    for (unsigned int index = 0; index<attachedModels_.size(); index++) {
      AttachedModel* model = attachedModels_[index];
      Link* hand = model->handLink;
      Link* object = model->objectLink;

      std::vector<double> vecPos = model->posVal;
      Position objHandTrans;
      for (int index = 0; index < 12; index++) {
        objHandTrans.data()[index] = vecPos[index];
      }

      BodyItemPtr objItem = model->object;

      object->R() = hand->R() * objHandTrans.linear();
      object->p() = hand->p() + hand->R() * objHandTrans.translation();
      objItem->notifyKinematicStateChange(true);
    }

    return true;
  }

  JointTrajectory TPInterface::interpolate(const VectorXd& qStart, const VectorXd& qGoal, double duration)
  {
    JointTrajectory traj;

    ji_.clear();
    ji_.appendSample(0, qStart);
    ji_.appendSample(duration, qGoal);
    ji_.update();

    for (double time = 0.0; time < duration+dt_; time += dt_) {
      if (time > duration) { time = duration; }
      VectorXd q = ji_.interpolate(time);
      auto wp = std::make_tuple(time, q);
      std::get<1>(traj).push_back(wp);
    }

    return traj;
  }

  // bool TPInterface::interpolate(std::vector<std::string>& jointNames,
  //                               const VectorXd& qGoal, double duration,
  //                               JointTrajectory& traj
  //                               )
  // {
  //   BodyPtr body = getRobotBody();

  //   VectorXd qStart;
  //   qStart.resize(jointNames.size());
  //   for (int i = 0; i < jointNames.size(); i++) {
  //     std::get<0>(traj).push_back(jointNames[i]);
  //     Link* link = body->link(jointNames[i]);
  //     if (!link) {
  //       printLog("Joint ", jointNames[i], " does not exist. The controller and the robot model may not be specified correctly.");
  //       return false;
  //     }
  //     qStart[i] = body->joint(link->jointId())->q();
  //   }

  //   return interpolate(qStart, qGoal, duration, traj);
  // }

  // bool TPInterface::interpolate (JointPathPtr jointPath,
  //                                const VectorXd& qGoal, double duration,
  //                                JointTrajectory& traj)
  // {
  //   std::vector<std::string> joint_names;
  //   VectorXd qStart;
  //   qStart.resize(jointPath->numJoints());
  //   for (int i = 0; i < jointPath->numJoints(); i++) {
  //     std::get<0>(traj).push_back(jointPath->joint(i)->name());
  //     qStart[i] = jointPath->joint(i)->q();
  //   }

  //   return interpolate(qStart, qGoal, duration, traj);
  // }

  JointTrajectory TPInterface::interpolate (JointPathPtr jointPath,
                                            const Vector3& xyz, const Vector3& rpy, double duration)
  {
    JointTrajectory traj;

    for (int i = 0; i < jointPath->numJoints(); i++) {
      std::get<0>(traj).push_back(jointPath->joint(i)->name());
    }
    std::get<1>(traj).clear();

    Link* tool = jointPath->endLink();
    ci_.clear();
    ci_.appendSample(0, tool->p(), tool->attitude());
    ci_.appendSample(duration, xyz, rotFromRpy(rpy));
    ci_.update();

    // double duration = ci.domainUpper();

    for (double time = 0.0; time < duration+dt_; time += dt_) {
      if (time > duration) { time = duration; }

      SE3 tf = ci_.interpolate(time);
      if (jointPath->calcInverseKinematics(tf.translation(),
                                           tool->calcRfromAttitude(tf.rotation().toRotationMatrix()))) {
        // robotItem->notifyKinematicStateChange(true);
        VectorXd q;
        q.resize(jointPath->numJoints());
        for (int i = 0; i < jointPath->numJoints(); i++) {
          q[i] = jointPath->joint(i)->q();
        }
        auto wp = std::make_tuple(time, q);
        std::get<1>(traj).push_back(wp);
      } else {
        // throwing IK failure exception might be better
        printLog("IK failure");
        return traj;
      }
    }

    return traj;
  }

  bool TPInterface::followTrajectory(const JointTrajectory& traj)
  {
    BodyItem* robotItem = getRobotItem();
    BodyPtr body = getRobotBody();

#ifdef _WIN32
    double last_tm = 0.0;
#else
    auto start_tm = std::chrono::system_clock::now();
#endif

    std::vector<std::string> joint_names = std::get<0>(traj);

    for (auto wp : std::get<1>(traj)) {
      double tm = std::get<0>(wp);
      VectorXd q = std::get<1>(wp);

      for (int i = 0; i < q.size(); i++) {
        Link* link = body->link(joint_names[i]);
        if (!link) {
          printLog("joint ", joint_names[i], " not found");

        }
        body->joint(link->jointId())->q() = q[i];
      }

      robotItem->notifyKinematicStateChange(true);
      //QCoreApplication::sendPostedEvents();
      QCoreApplication::processEvents();
      //SceneView::instance()->sceneWidget()->update();
      updateAttachedModels();

#ifdef _WIN32
      double dt = tm - last_tm;
      last_tm = tm;
      Sleep((int)(dt*1000));
#else
      auto abs_time = start_tm + std::chrono::milliseconds((int)(tm*1000));
      std::this_thread::sleep_until(abs_time);
#endif
    }

    printLog("followTrajectory finished");

    return true;
  }

  VectorXd TPInterface::getCurrentJointAngles ()
  {
    BodyPtr body = getRobotBody();
    VectorXd q;
    int n = body->numJoints();
    q.resize(n);
    for (int i = 0; i < n; i++) { q[i] = body->joint(i)->q(); }
    return q;
  }

  VectorXd TPInterface::getStandardPose ()
  {
    BodyPtr body = getRobotBody();
    VectorXd q;
    q.resize(body->numJoints());

    const Listing& pose = *body->info()->findListing("standardPose");
    if (!pose.isValid()) {
      printLog("standard pose is not valid");
    } else if (pose.size() != body->numJoints()) {
      printLog("getStandardPose: number of joints does not match");
    } else {
      for (int jointIndex = 0; jointIndex < pose.size(); jointIndex++) {
        q[jointIndex] = radian(pose[jointIndex].toDouble());
      }
    }

    // exception would be better if standardPose is not defined
    return q;
  }


}
