# encoding: utf-8

from task_tool.task_design import *

class SetTaskFrameCmd(Cmd):
    """ """
    _cmd_name = 'setTaskFrame'
    _disp_name = 'setTaskFrame'
    _signature = [('xyz', ParamType.VECTOR3),
                      ('rpy', ParamType.VECTOR3)]

    def __init__(self, frame):
        super(SetTaskFrameCmd, self).__init__()
        self.frame = frame
        self.base = None

    def arguments(self):
        xyz,rpy = transform_exp([comp(self.frame)])
        return [xyz, rpy]

class MoveLCmd(Cmd):
    """ """
    _cmd_name = 'moveArm'
    _disp_name = 'Arm'
    _signature = [('xyz', ParamType.VECTOR3),
                      ('rpy', ParamType.VECTOR3),
                      ('tm', ParamType.DBL),
                      ('grasp_xyz', ParamType.VECTOR3),
                      ('grasp_rpy', ParamType.VECTOR3),
                      ('armID', ParamType.INT)]

    def __init__(self, goal):
        super(MoveLCmd, self).__init__()
        self.goal = goal

    def arguments(self):
        tf,tm = self.goal
        xyz,rpy = transform_exp([comp(tf)], xyz=None)
        return [xyz, rpy, tm, 'xyz(graspF)', 'rpy(graspF)', 'handID']

class InsertionRandomCmd(Cmd):
    """ """
    _cmd_name = 'insertionRandom'
    _disp_name = 'insertion'
    _signature = [('xyz', ParamType.VECTOR3),
                      ('rpy', ParamType.VECTOR3)]

    def __init__(self):
        super(InsertionRandomCmd, self).__init__()

    def arguments(self):
        return ['xyz(graspF)', 'rpy(graspF)']

class GripperCmd(Cmd):
    """ """
    _cmd_name = 'moveGripper'
    _disp_name = 'Gripper'
    _signature = [('width', ParamType.DBL),
                      ('tm', ParamType.DBL),
                      ('gripperID', ParamType.INT)]

    def __init__(self, width, gripper, tm=Double(0.2), model_actions=[]):
        super(GripperCmd, self).__init__()
        self.width = width
        self.tm = tm
        self.gripperID = gripper
        self._model_actions = []
        for model_action in model_actions:
            self._model_actions.append(ModelAction(*model_action))

    def arguments(self):
        return [comp(self.width), comp(self.tm), comp(self.gripperID)]

    def model_actions(self):
        return self._model_actions

class GraspCmd(GripperCmd):
    """ """
    def __init__(self, width, gripper, target):
        super(GraspCmd, self).__init__(width, gripper, model_actions=[('attach', target, gripper)])

class ReleaseCmd(GripperCmd):
    """ """
    def __init__(self, width, gripper, target):
        width = EPlus(width, Double(0.03))
        super(ReleaseCmd, self).__init__(width, gripper, model_actions=[('detach', target, gripper)])