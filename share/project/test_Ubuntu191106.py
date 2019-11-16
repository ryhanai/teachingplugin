# -*- coding: utf-8 -*-

def getCommandDefList():
    return """
- name : 'moveArm'
  dispName : 'Arm'
  retType : 'boolean'
  args :
    - name : 'xyz'
      type : 'double'
      length : 3
    - name : 'rpy'
      type : 'double'
      length : 3
    - name : 'tm'
      type : 'double'
    - name : 'grasp_xyz'
      type : 'double'
      length : 3
    - name : 'grasp_rpy'
      type : 'double'
      length : 3
    - name : 'armID'
      type : 'int'
- name : 'moveGripper'
  dispName : 'Gripper'
  retType : 'boolean'
  args :
    - name : 'width'
      type : 'double'
    - name : 'tm'
      type : 'double'
    - name : 'gripperID'
      type : 'int'
- name : 'setTaskFrame'
  dispName : 'setTaskFrame'
  retType : 'boolean'
  args :
    - name : 'xyz'
      type : 'double'
      length : 3
    - name : 'rpy'
      type : 'double'
      length : 3
- name : 'insertionRandom'
  dispName : 'insertion'
  retType : 'boolean'
  args :
    - name : 'xyz'
      type : 'double'
      length : 3
    - name : 'rpy'
      type : 'double'
      length : 3

"""


import cnoid.TeachingPlugin
import transforms3d._gohlketransforms as tf

from screw_insertion import SkillServer

class ControllerBase(object):
    def __init__(self):
        "ロボットアイテム名の設定、リンクチェインマッピングの定義等"
        pass
    def getToolLink(toolNumber):
        "1つのロボットアイテムが複数のリンクチェインを持つことがある。"
        "また、リンク名はロボットごとに定義され、タスクとは独立なので、"
        "タスク中でリンクチェインを指定する記述から対象とするロボット"
        "におけるリンクチェインへのマッピングを定義しなければならない。"
        pass
    def getCommandDefList(self):
        pass
    def executeCommand(self, commandName, params, isReal):
        return getattr(self, commandName)(*params)

class HiroNXController(ControllerBase):
    def __init__(self):
        self.tp = cnoid.TeachingPlugin.TPInterface.instance()
        self.tp.setRobotName('main_withHands')
        self.toolLinks = {}
        self.toolLinks[0] = 'RARM_JOINT5'
    def moveArm(self, goal, tm, armID):
        jointPath = self.tp.getJointPath(self.toolLinks[armID])
        xyz = goal[:,3]
        rpy = tf.euler_from_matrix(goal[:,:3], axes='rxyz')
        traj = self.tp.interpolate(jointPath, xyz, rpy, tm)
        print(traj)


import os, sys, yaml, time
from os import path
import numpy as np
from rspt_dev.util import *

sys.path.append("../../share/scripts")

from tp_chore import robot

# def setJointPositions(tp, jointNames, positions):
#     b = tp.getRobotBody()
#     for i,n in enumerate(jointNames):
#         for j in range(b.getNumJoints()):
#             if b.joint(j).name == n:
#                 b.joint(j).q = positions[i]

tp = cnoid.TeachingPlugin.TPInterface.instance()
tp.setRobotName('main_withHands')
toolLinks = {}

# HIRO
# toolLinks[0] = 'LARM_JOINT5'
# toolLinks[1] = 'RARM_JOINT5'

# ur3dual
toolLinks[0] = 'larm_wrist_3_joint'
toolLinks[1] = 'rarm_wrist_3_joint'
# rarm_ee_link

controller = HiroNXController()
server = SkillServer()

def executeCommand(encoded_cmd):
    global cmd, traj, task_frame
    cmd = yaml.load(encoded_cmd, Loader=yaml.SafeLoader)
    r = robot.SimHIRONX()
    #controller.executeCommand(cmd['commandName'], cmd['args'], cmd['isReal'])
    cmdName = cmd[0]['commandName']
    args = cmd[0]['args']
    isReal = cmd[0]['isReal']
    print(cmdName)

    try:
        if cmdName == 'setTaskFrame':
            xyz = args[0]['VectorXd']
            rpy = np.radians(args[1]['VectorXd'])
            task_frame = (xyz, t.quaternion_from_euler(*rpy, axes='rxyz'))
            print('task frame: ', task_frame)
            if isReal:
                server.init_task(env={'task_frame' : task_frame})
            return True
        elif cmdName == 'moveArm':
            xyz = args[0]['VectorXd']
            rpy = np.radians(args[1]['VectorXd'])
            tm = args[2]
            grasp_xyz = args[3]['VectorXd']
            grasp_rpy = np.radians(args[4]['VectorXd'])
            armID = args[5]
            jointPath = tp.getJointPath(toolLinks[armID])
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
                m = np.dot(np.dot(pose2mat(task_frame), pose2mat(f)), pose2mat(fg))
                xyz2 = t.translation_from_matrix(m)
                rpy2 = t.euler_from_matrix(m)
                traj = tp.interpolate(jointPath, xyz2, rpy2, tm)
                tp.followTrajectory(traj)
                return True
        elif cmdName == 'insertionRandom':
            if isReal:
                return server.insertion()
            else:
                return True
        elif cmdName == 'moveGripper':
            width = args[0]
            tm = args[1]
            handID = args[2]

            if isReal:
                ret = server.moveg(width, 5.0)
                return ret
            else:
                return True
    except Exception as e:
        print('Exception: ', e)
        return False
