# -*- coding: utf-8 -*-

from controller_base import ControllerBase

import numpy as np
import cnoid.TeachingPlugin
import transforms3d._gohlketransforms as tf

from tp_chore import robot

class BasicCommandController(ControllerBase):
    def __init__(self, robotItemName, toolLinkMap):
        super(BasicCommandController, self).__init__(robotItemName, toolLinkMap)
        self.tp = cnoid.TeachingPlugin.TPInterface.instance()
        self.tp.setRobotName(self.robotItemName())

    def moveArm(self, xyz, rpy, tm, grasp_xyz, grasp_rpy, armID, isReal):
        rpy = np.radians(rpy)
        grasp_rpy = np.radians(grasp_rpy)

        jointPath = self.tp.getJointPath(self.getToolLinkName(armID))
        print(xyz, rpy, tm, grasp_xyz, grasp_rpy)
        f = (xyz, t.quaternion_from_euler(*rpy, axes='rxyz'))
        fg = (grasp_xyz, t.quaternion_from_euler(*grasp_rpy, axes='rxyz'))

        if isReal:
            m = np.dot(pose2mat(f), pose2mat(fg))
            xyz2 = t.translation_from_matrix(m)
            q = t.quaternion_from_matrix(m)
            goal = (xyz2, q)
            ret = server.movel(goal, tm, goal_tolerance=0.003)
            print(ret)
            return ret
        else:
            m = np.dot(np.dot(pose2mat(self.task_frame), pose2mat(f)), pose2mat(fg))
            xyz2 = t.translation_from_matrix(m)
            rpy2 = t.euler_from_matrix(m)
            traj = self.tp.interpolate(jointPath, xyz2, rpy2, tm)
            self.tp.followTrajectory(traj)
            return True

    def moveGripper(self, width, tm, handID, isReal):
        if isReal:
            return server.moveg(width, 5.0)
        else:
            return True

    def setTaskFrame(self, xyz, rpy, isReal):
        rpy = np.radians(rpy)
        self.task_frame = (xyz, t.quaternion_from_euler(*rpy, axes='rxyz'))
        print('task frame: ', self.task_frame)

        if isReal:
            server.init_task(env={'task_frame' : self.task_frame})
        return True
