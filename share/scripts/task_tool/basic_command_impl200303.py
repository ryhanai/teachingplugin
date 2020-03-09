# -*- coding: utf-8 -*-

from task_tool.controller_base import ControllerBase
from task_tool.util import *

import numpy as np
#import transforms3d._gohlketransforms as t

import cnoid.TeachingPlugin
from tp_chore import robot

class BasicCommandController(ControllerBase):
    def __init__(self, robotItemName, toolLinkMap):
        super(BasicCommandController, self).__init__(robotItemName, toolLinkMap)
        self.tp = cnoid.TeachingPlugin.TPInterface.instance()
        self.tp.setRobotName(self.robotItemName())

    def moveArm(self, goal, grasp, tm, armID, isReal):
        print(goal, grasp, tm, armID, isReal)
        jointPath = self.tp.getJointPath(self.getToolLinkName(armID))

        if isReal:
            # m = np.dot(pose2mat(f), pose2mat(fg))
            # xyz2 = t.translation_from_matrix(m)
            # q = t.quaternion_from_matrix(m)
            # goal = (xyz2, q)
            # ret = server.movel(goal, tm, goal_tolerance=0.003)
            print('[ur3dual]: real robot is not yet supported')
            return ret
        else:
            f = Frame(grasp, parent=goal)
            xyz = f.xyz()
            rpy = f.rpy()
            print(xyz, rpy)
            traj = self.tp.interpolate(jointPath, xyz, rpy, tm)
            self.tp.followTrajectory(traj)
            return True

    def moveGripper(self, width, tm, handID, isReal):
        if isReal:
            return server.moveg(width, 5.0)
        else:
            return True
