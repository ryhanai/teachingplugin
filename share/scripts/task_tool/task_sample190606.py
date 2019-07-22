#!/usr/bin/python3
# encoding: utf-8

import numpy as np
from task_model.task_design import *
from task_model.basic_commands import *
from task_model.sample_master_data import *

# We do not know what kind of motion patterns are needed for general assembly tasks,
# how to organize the motion patterns, how to parameterize the motions patterns.

# Some of the motion patterns used for assembling are represented as a kind of pick-and-place.
# We first design a class for pick motion with typical parameterization.
# Then add some parameters and motion for placing to extend the pick-pattern to various pick-and-place or
# pick-and-assemble motions.

class TaskPick(Task):
    """ """
    def __init__(self, name, comment, master_manager):
        super(TaskPick, self).__init__(name, comment, master_manager)

    def define_model(self):
        self.add_model(name='picked_obj', master_name='green_tea_bottle_with_cap', tf=[0.4, 0.2, 0.872, 0, 0, 0])
        self.add_model(name='target_obj', master_name='FP', tf=[0.4, -0.2, 0.872, 0, 0, 0])

    def define_params(self):
        # parameters for picking
        self.add_param(ParamModel('pickF', self.menv['picked_obj']))
        self.add_param(ParamTF('approachTF', [0, 0 ,0.05, 0, 0, 0], hide=True))
        self.add_param(ParamTF('retractTF', [0, 0 ,0.05, 0, 0, 0], hide=True))

        # parameters for grasp
        self.add_param(ParamTF('graspF', [0, 0 ,0.18, 0, -90, 0]))
        self.add_param(ParamDouble('finger_interval', 0.013))
        self.add_param(ParamInt('handID', 0, 'hand(0=left,1=right)'))

    def define_motion(self):
        # picking motion
        self.add_command(GripperCmd(width=EPlus(self.param('finger_interval'),Double(0.04)), gripper=self.param('handID')))
        self.add_command(MoveLCmd((self.param('approachTF'),1.0), base=self.param('pickF')))
        self.add_command(MoveLCmd((np.zeros(6),0.5), base=self.param('pickF')))
        self.add_command(GraspCmd(width=self.param('finger_interval'), gripper=self.param('handID'), target='picked_obj'))
        self.add_command(MoveLCmd((self.param('retractTF'),1.0), base=self.param('pickF')))

class TaskHold(Task):
    """ """
    def __init__(self, name, comment, master_manager):
        super(TaskHold, self).__init__(name, comment, master_manager)

    def define_model(self):
        self.add_model(name='picked_obj', master_name='green_tea_bottle_with_cap', tf=[0.4, 0.2, 0.872, 0, 0, 0])

    def define_params(self):
        # parameters for picking
        self.add_param(ParamModel('holdF', self.menv['picked_obj']))
        self.add_param(ParamTF('approachTF', [0, 0 ,0.05, 0, 0, 0], hide=True))

        # parameters for grasp
        self.add_param(ParamTF('graspF', [0, 0 ,0.18, 0, -90, 0]))
        self.add_param(ParamDouble('finger_interval', 0.013))
        self.add_param(ParamInt('handID', 0, 'hand(0=left,1=right)'))

    def define_motion(self):
        # picking motion
        self.add_command(GripperCmd(width=EPlus(self.param('finger_interval'),Double(0.04)), gripper=self.param('handID')))
        self.add_command(MoveLCmd((self.param('approachTF'),1.0), base=self.param('holdF')))
        self.add_command(MoveLCmd((np.zeros(6),0.5), base=self.param('holdF')))
        self.add_command(GraspCmd(width=self.param('finger_interval'), gripper=self.param('handID'), target='picked_obj'))

def meta_data_sample():
    t = Task(name='meta data sample', comment='', master_manager=db)
    t.add_metadata_file('fasten_by_slipping.mp4')
    t.add_metadata_file('finger_attachments.stl')
    t.add_metadata_image('greentea350.jpg')
    return t

def hold():
    t = TaskHold(name='hold',
                     comment='fix an object during some manipulation by another hand',
                     master_manager=db)
    return t

def pick_place():
    t = TaskPick(name='pick and place',
                     comment='This is a base motion pattern for various extended patterns',
                     master_manager=db)

    # 追加のパラメータを定義する
    t.add_param(ParamModel('placeF', t.menv['target_obj']))
    approachF = ParamTF('approachF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    retractF = ParamTF('retractF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    t.add_params([approachF, retractF])

    # place動作を定義する（placeFをベースとした軌道として定義している）
    place_motion = []
    place_motion.append(MoveLCmd((approachF,1.0), base=t.param('placeF')))
    place_motion.append(MoveLCmd((np.zeros(6),0.5), base=t.param('placeF')))
    place_motion.append(ReleaseCmd(width=t.param('finger_interval'), gripper=t.param('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((retractF,1.0), base=t.param('placeF')))

    t.add_commands(place_motion)
    return t

def pick_screw():
    t = TaskPick(name='pick and screw', comment='pick an object, place it onto another object and screw it.', master_manager=db)

    # モデルマスタを差替える
    t.replace_master('picked_obj', 'green_tea_cap')

    # 追加のパラメータを定義する
    t.add_param(ParamModel('placeF', t.menv['target_obj']))
    approachF = ParamTF('approachF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    retractF = ParamTF('retractF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    t.add_params([approachF, retractF])

    # パラメータ調整
    t.param('graspF').value = [0, 0, 0.1, 0, -90, 0]

    # place動作を定義する（placeFをベースとした軌道として定義している）
    place_motion = []
    place_motion.append(MoveLCmd((approachF,1.0), base=t.param('placeF')))
    place_motion.append(MoveLCmd((np.zeros(6),0.5), base=t.param('placeF')))
    place_motion.append(MoveLCmd((np.array([0,0,0,0,0,-45]),0.5), base=t.param('placeF')))
    place_motion.append(ReleaseCmd(width=t.param('finger_interval'), gripper=t.param('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((np.zeros(6),0.5), base=t.param('placeF')))
    place_motion.append(GraspCmd(width=t.param('finger_interval'), gripper=t.param('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((np.array([0,0,0,0,0,-45]),0.5), base=t.param('placeF')))
    place_motion.append(ReleaseCmd(width=t.param('finger_interval'), gripper=t.param('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((retractF,1.0), base=t.param('placeF')))

    t.add_commands(place_motion)
    return t


import sys
#import optparse

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('task name need to be specified')
        print('pick_place, pick_screw, hold, etc.')
    else:
        tsk = eval(sys.argv[1])()
        print(yaml.dump([tsk.compile()]))
