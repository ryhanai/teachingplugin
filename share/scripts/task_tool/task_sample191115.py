#!/usr/bin/python3
# encoding: utf-8

import sys
sys.path.append('..')

import numpy as np
from task_tool.task_design import *
from task_tool.basic_commands191115 import *
from task_tool.sample_master_data import *

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
        self.add_model(name='picked_obj', master_name='ycb_screw', tf=[-0.05, -0.35, 0.775, 0, 0, 0])
        self.add_model(name='target_obj', master_name='FP', tf=[-0.027, -0.138, 0.838, 0, 0, 0])

    def define_params(self):
        # parameters for picking
        self.add_param(ParamModel('pickF', self.menv['picked_obj']))
        self.add_param(ParamTF('approachTF', [0, 0 ,0.12, 0, 0, 0], hide=True))
        self.add_param(ParamTF('retractTF', [0, 0 ,0.12, 0, 0, 0], hide=True))

        # parameters for grasp
        self.add_param(ParamTF('graspF', [0, 0 ,0.30, -90, 90, 0]))
        self.add_param(ParamDouble('finger_interval', 0.015))
        self.add_param(ParamInt('handID', 1, 'hand(0=left,1=right)'))

    def define_motion(self):
        # picking motion
        self.add_command(SetTaskFrameCmd(self.param('pickF')))
        self.add_command(GripperCmd(width=EPlus(self.param('finger_interval'),Double(0.04)), gripper=self.param('handID')))
        self.add_command(MoveLCmd((self.param('approachTF'),1.0)))
        self.add_command(MoveLCmd((np.zeros(6),0.5)))
        self.add_command(GraspCmd(width=self.param('finger_interval'), gripper=self.param('handID'), target='picked_obj'))
        self.add_command(MoveLCmd((self.param('retractTF'),1.0)))

def meta_data_sample():
    t = Task(name='meta data sample', comment='', master_manager=db)
    t.add_metadata_file('fasten_by_slipping.mp4')
    t.add_metadata_file('finger_attachments.stl')
    t.add_metadata_image('greentea350.jpg')
    return t

def pick_place():
    t = TaskPick(name='pick and place',
                     comment='This is a base motion pattern for various extended patterns',
                     master_manager=db)

    # 追加のパラメータを定義する
    t.add_param(ParamModel('placeF', t.menv['target_obj']))
    approachF = ParamTF('approachF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    retractF = ParamTF('retractF1', [0, 0 ,0.05, 0, 0, 0], hide=True)
    waypointF = ParamTF('waypointF1', [-0.03, -0.03 ,0.06, 0, 0, 0], hide=False)
    t.add_params([approachF, retractF, waypointF])

    # place動作を定義する（placeFをベースとした軌道として定義している）
    place_motion = []
    place_motion.append(SetTaskFrameCmd(t.param('placeF')))
    place_motion.append(MoveLCmd((waypointF,1.5))) # waypoint to manually avoid collision
    place_motion.append(MoveLCmd((approachF,1.0)))
    place_motion.append(InsertionRandomCmd())
    place_motion.append(ReleaseCmd(width=t.param('finger_interval'), gripper=t.param('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((retractF,1.0)))

    t.add_commands(place_motion)
    return t

#import optparse

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('task name need to be specified')
        print('pick_place, etc.')
    else:
        tsk = eval(sys.argv[1])()
        print(yaml.dump([tsk.compile()]))
