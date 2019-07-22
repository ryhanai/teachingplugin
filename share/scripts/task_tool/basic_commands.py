# encoding: utf-8

from task_model.task_design import *

## definition of task specific commands

class GoInitialCmd(Cmd):
    def __init__(self, tm=5.0, pos=[0,0]):
        super(GoInitialCmd, self).__init__('goInitial', 'Initial Pose', pos)
        self.tm = tm

    def compile(self):
        data = super(GoInitialCmd, self).compile()
        args = []
        args.append(OrderedDict([('seq',1), ('name',quoted('tm')), ('value',quoted(self.tm))]))
        data['arguments'] = args
        return data

class FollowTrajectoryCmd(Cmd):
    """ """
    def __init__(self, waypoints, base):
        super(FollowTrajectoryCmd, self).__init__('followTrajectory', 'ArmTraj')
        self.waypoints = waypoints
        self.base = base

    def compile(self):
        return []

class MoveLCmd(Cmd):
    """ """
    def __init__(self, goal, base):
        super(MoveLCmd, self).__init__('moveArm', 'Arm',)
        self.goal = goal
        self.base = base

    def compile(self):
        """
        把持姿勢の変数graspFとハンド選択の変数handIDは決め打ちで、すべてのcommandにつけられる.
        """
        tf,tm = self.goal
        xyz,rpy = transform_exp([comp(self.base), comp(tf), 'graspF'], xyz=None)
        code = super(MoveLCmd, self).compile()
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
        super(GripperCmd, self).__init__('moveGripper', disp_name, pos)
        self.width = width
        self.tm = tm
        self.gripperID = gripper
        self.model_actions = []
        for model_action in model_actions:
            self.model_actions.append(ModelAction(*model_action))

    def compile(self):
        code = super(GripperCmd, self).compile()
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
        super(GraspCmd, self).__init__(width, gripper, model_actions=[('attach', target, gripper)])

class ReleaseCmd(GripperCmd):
    """ """
    def __init__(self, width, gripper, target):
        width = EPlus(width, Double(0.04))
        super(ReleaseCmd, self).__init__(width, gripper, model_actions=[('detach', target, gripper)])
