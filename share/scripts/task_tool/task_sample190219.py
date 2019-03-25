#!/usr/bin/python3
# encoding: utf-8

import numpy as np
from master_manager import *
from task_design import *


db = MasterManager()
db.add(1, 'green_tea_bottle_with_cap', 'greentea350hrp.wrl',
           'greentea350.jpg', [Feature('cap_frame', tf=[0,0,0.12,0,0,0])])
db.add(2, 'tray', 'box_trayHrp.wrl')
db.add(3, 'green_tea_cap', 'greentea350_capHrp.wrl')
db.add(5, 'MA266_pully', 'MA266_x2Hrp.wrl')
db.add(6, 'board', 'boardHrp.wrl')
db.add(7, 'SEBZ16-150_rail', 'SEBZ16-150_railHrp.wrl')
db.add(9, 'bolt', 'CBM3-10Hrp.wrl')
db.add(10, 'screw feeder', 'screw_feederHrp.wrl')
db.add(13, 'fixture', 'fixture01Hrp.wrl')
db.add(14, 'SEBZ16_block', 'SEBZ16-270_blockHrp.wrl')
db.add(17, 'ycb_driver_bit', 'driver_tipHrp.wrl')
db.add(32, 'ycb_main_wing', 'ycb_main_wingHrp.wrl')
db.add(33, 'ycb_tool_station', 'ycb_tool_stationHrp.wrl')
db.add(34, 'ycb_airplane_body', 'YCBAirplane_before_boltingHrp.wrl')
db.add(35, 'ycb_screw', 'boltHrp.wrl')
db.add(36, 'green_tea_bottle', 'greentea350_bottleHrp.wrl')


## define task specific commands

class GoInitialCmd(Cmd):
    def __init__(self, tm=5.0, pos=[0,0]):
        super().__init__('goInitial', 'Initial Pose', pos)
        self.tm = tm

    def compile(self):
        data = super().compile()
        args = []
        args.append(OrderedDict([('seq',1), ('name',quoted('tm')), ('value',quoted(self.tm))]))
        data['arguments'] = args
        return data

class FollowTrajectoryCmd(Cmd):
    """ """
    def __init__(self, waypoints, base):
        super().__init__('followTrajectory', 'ArmTraj')
        self.waypoints = waypoints
        self.base = base

    def compile(self):
        return []

class MoveLCmd(Cmd):
    """ """
    def __init__(self, goal, base):
        super().__init__('moveArm', 'Arm',)
        self.goal = goal
        self.base = base

    def compile(self):
        """
        把持姿勢の変数graspFとハンド選択の変数handIDは決め打ちで、すべてのcommandにつけられる.
        """
        tf,tm = self.goal
        xyz,rpy = transform_exp([comp(self.base), comp(tf), 'graspF'], xyz=None)
        code = super().compile()
        args = []
        args.append(OrderedDict([('name',quoted('xyz')), ('value',quoted(xyz))]))
        args.append(OrderedDict([('name',quoted('rpy')), ('value',quoted(rpy))]))
        args.append(OrderedDict([('name',quoted('tm')), ('value',quoted(tm))]))
        args.append(OrderedDict([('name',quoted('armID')), ('value',quoted('handID'))]))
        code['arguments'] = args
        return code

class GripperCmd(Cmd):
    """ """
    def __init__(self, width, gripper, tm=Double(0.2), disp_name='Gripper', model_actions=[], pos=[0,0]):
        super().__init__('moveGripper', disp_name, pos)
        self.width = width
        self.tm = tm
        self.gripperID = gripper
        self.model_actions = []
        for model_action in model_actions:
            self.model_actions.append(ModelAction(*model_action))

    def compile(self):
        code = super().compile()
        args = []
        args.append(OrderedDict([('name',quoted('width')), ('value',quoted(comp(self.width)))]))
        args.append(OrderedDict([('name',quoted('tm')), ('value',quoted(comp(self.tm)))]))
        args.append(OrderedDict([('name',quoted('gripperID')), ('value',quoted(comp(self.gripperID)))]))
        code['arguments'] = args
        if self.model_actions != []:
            actions = []
            for model_action in self.model_actions:
                actions.append(model_action.compile())
            code['model_actions'] = actions
        return code

class GraspCmd(GripperCmd):
    """ """
    def __init__(self, width, gripper, target):
        super().__init__(width, gripper, model_actions=[('attach', target, gripper)])

class ReleaseCmd(GripperCmd):
    """ """
    def __init__(self, width, gripper, target):
        width = EPlus(width, Double(0.04))
        super().__init__(gripper, width, model_actions=[('detach', target, gripper)])


class TaskPick(Task):
    """
    Pickタスク。
    これにPlace時の動作とその動作を制御するパラメータを追加することで、
    Pick and PlaceやPick and Assembleパターンを作る。
    """
    def __init__(self, name, comment, master_manager):
        super().__init__(name, comment, master_manager)

    def define_task_model(self):
        self.define_model()
        self.define_params()
        self.define_motion()

    def define_model(self):
        """デフォルトモデルが設定されている"""
        self.add_model(name='picked_obj', master_name='green_tea_bottle_with_cap', tf=[0.4, 0.2, 0.872, 0, 0, 0])

    def replace_model(self, name, master_name):
        """モデルを差替える (not yet implemented)"""
        pass

    def define_params(self):
        # parameters for picking
        self.add_param(ParamModel('pickF', self.menv['picked_obj']))
        self.add_param(ParamTF('approach_pos_pick', [0, 0 ,0.05, 0, 0, 0]))
        self.add_param(ParamTF('retract_pos_pick', [0, 0 ,0.05, 0, 0, 0]))

        # parameters for grasp
        self.add_param(ParamTF('graspF', [0, 0 ,0.18, 0, -90, 0]))
        self.add_param(ParamDouble('finger_interval', 0.013))
        self.add_param(ParamInt('handID', 0, 'hand(0=left,1=right)'))

    def define_motion(self):
        # picking motion
        self.add_command(GripperCmd(width=EPlus(self.getp('finger_interval'),Double(0.04)), gripper=self.getp('handID')))
        self.add_command(MoveLCmd((self.getp('approach_pos_pick'),1.0), base=self.getp('pickF')))
        self.add_command(MoveLCmd((np.zeros(6),0.5), base=self.getp('pickF')))
        self.add_command(GraspCmd(width=self.getp('finger_interval'), gripper=self.getp('handID'), target='picked_obj'))
        self.add_command(MoveLCmd((self.getp('retract_pos_pick'),1.0), base=self.getp('pickF')))

    def set_place_motion(self, commands):
        self.add_commands(commands)

def pick_place():
    # Pickタスクをつくる
    tsk = TaskPick(name='pick and place',
                       comment='This is a base motion pattern for various extended patterns',
                       master_manager=db)

    # Place動作を制御するためのパラメータを追加する
    tsk.add_param(ParamTF('placeF', [0.4, 0 ,0.872, 0, 0, 0]))
    approachF = ParamTF('approachF1', [0, 0 ,0.05, 0, 0, 0])
    retractF = ParamTF('retractF1', [0, 0 ,0.05, 0, 0, 0])
    tsk.add_params([approachF, retractF])

    # Place動作を定義する
    # （placeFを基準とした軌道として定義している）
    place_motion = []
    place_motion.append(MoveLCmd((approachF,1.0), base=tsk.getp('placeF')))
    place_motion.append(MoveLCmd((np.zeros(6),0.5), base=tsk.getp('placeF')))
    place_motion.append(ReleaseCmd(width=tsk.getp('finger_interval'), gripper=tsk.getp('handID'), target='picked_obj'))
    place_motion.append(MoveLCmd((retractF,1.0), base=tsk.getp('placeF')))
    tsk.set_place_motion(place_motion)

    tsk.add_metadata_file('fasten_by_slipping.mp4')
    tsk.add_metadata_file('finger_attachments.stl')
    tsk.add_metadata_image('greentea350.jpg')
    return tsk


if __name__ == '__main__':
    tsk = pick_place()
    print(yaml.dump([tsk.compile()]))

