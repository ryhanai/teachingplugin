/*!
  @author Ryo Hanai
*/
#include <cnoid/PyUtil>
#include "TPUtil.h"
#include <pybind11/stl.h>

using namespace std;
using namespace cnoid;
using namespace teaching;

namespace py = pybind11;

PYBIND11_MODULE(TeachingPlugin, m)
{
  m.doc() = "teachingPlugin Controller module";

  py::module::import ("cnoid.Util");

  py::class_<TPInterface>(m, "TPInterface")
    .def_static("instance", &TPInterface::instance, py::return_value_policy::reference)
    .def("getRobotBody", &TPInterface::getRobotBody)
    .def("setRobotName", &TPInterface::setRobotName)
    .def("getRobotName", &TPInterface::getRobotName)
    .def("getJointPath", &TPInterface::getJointPath)
    .def("updateAttachedModels", &TPInterface::updateAttachedModels)
    .def("clearAttachedModels", &TPInterface::clearAttachedModels)
    .def("attachModelItem", &TPInterface::attachModelItem)
    .def("detachModelItem", &TPInterface::detachModelItem)
    .def("interpolate", (JointTrajectory(TPInterface::*)(const VectorXd&, const VectorXd&, double))&TPInterface::interpolate)
    //.def("interpolate", (bool(TPInterface::*)(const VectorXd&, const VectorXd&, double, JointTrajectory&))&TPInterface::interpolate)
    // .def("interpolate", (bool(TPInterface::*)(std::vector<std::string>& jointNames, const VectorXd& qGoal, double duration, JointTrajectory& traj))&TPInterface::interpolate)
    // .def("interpolate", (bool (TPInterface::*)(JointPathPtr jointPath, const VectorXd& qGoal, double duration, JointTrajectory& traj))&TPInterface::interpolate)
    .def("interpolate", (JointTrajectory(TPInterface::*)(JointPathPtr jointPath, const Vector3& xyz, const Vector3& rpy, double duration))&TPInterface::interpolate)
    .def("followTrajectory", (bool(TPInterface::*)(const JointTrajectory& traj))&TPInterface::followTrajectory)
    ;
  
}

