from task_tool.basic_commands191115 import *
from task_tool.basic_command_impl import *


class UR3dualController(BasicCommandController):
    def __init__(self, use_ros=False):
        super(UR3dualController, self).__init__(robotItemName='main_withHands',
                                                    toolLinkMap={0:'larm_wrist_3_joint', 1:'rarm_wrist_3_joint'})
        if use_ros:
            from screw_insertion import SkillServer
            self.server = SkillServer()

    def commands(self):
        return [MoveLCmd, GripperCmd, SetTaskFrameCmd, InsertionRandomCmd]

    #
    # Implementation of extended commands
    #
    def insertionRandom(self, isReal):
        if isReal:
            return self.server.insertion()
        else:
            return True


class RobotState:
    def __init__(self, joint_positions, tool_frame, wrist_wrench):
        self._jv = joint_positions
        self._frame = tool_frame
        self._wrench = wrist_wrench

    def x(self):
        return self._frame[0][0]
    def y(self):
        return self._frame[0][1]
    def z(self):
        return self._frame[0][2]
    def xyz(self):
        return self._frame[0]

    def fz(self):
        return self._wrench[2]
    def fabs(self):
        return norm(self._wrench[:2], ord=2)

    def jv(self):
        return self._jv


# task_frames = [
#     (np.array([0.0, -0.1, 0.18]), t.quaternion_about_axis(0.3, (1,-1,0))),
#     (np.array([-0.05, -0.15, 0.17]), t.quaternion_about_axis(0.3, (1,-1,0))),
#     (np.array([-0.06, -0.16, 0.16]), t.quaternion_about_axis(0.3, (1,-1,0)))
#     ]


h = 1.0
ori = [0.06203120640983931, -0.007827257799592313, 0.10777645012885509, 0.9922071861583662]
hole_frames = [([-0.04999383727752055, -0.2500331583231785, h], ori),
                   ([-0.04999383727752055, -0.2500331583231785, h-0.02], ori),
                   ([-0.04999383727752055, -0.2500331583231785, h-0.04], ori)]


from numpy.linalg import norm

class SkillServer:
    def __init__(self):
        self.r = UR3dual()
        pass

    def init_task(self, env={}):
        self._env = env
        self.r.set_base_frame(frame_id='task_frame', val=self._env['task_frame'], parent_frame_id='world')

    def insertion1_trajectory(self):
        n = 10
        traj = []
        R = 0.01
        for i in range(n):
            x = R * (0.5+i/n) * math.cos(4 * math.pi * i / n)
            y = R * (0.5+i/n) * math.sin(4 * math.pi * i / n)
            z = 0.017
            q = [0,0,0,1]
            tm = 1.0
            traj.append((([x,y,z],q),tm))
        return traj

    def insertion(self):
        self.r.set_zero()

        grasp_offset = 0.3
        # move down
        ret_code = self.movel(([0,0,grasp_offset+0.015],[0,0,0,1]), duration=2.0,
                                conds=[lambda s: (True,'success') if s.fz() < -5 else (False,None)])

        for i in range(5):
            s = self.r.get_state()
            xyz = np.random.normal(0, 0.005, 3)
            xyz[2] = grasp_offset + 0.005
            ret_code = self.movel((xyz,[0,0,0,1]),
                                      conds=[lambda s: (True,'success') if s.z() < grasp_offset+0.015 else (False,None)])
            print('ret = ', ret_code)
            if ret_code == 'success':
                return True

        return False

        # move spirally
        # for frame, duration in self.insertion1_trajectory():
        #     ret = self.movel(frame, duration,
        #                       conds=[lambda s: (True,'success') if s.z() < 0.018 else (False,None),
        #                                  lambda s: (True,'failure') if s.fabs() > 8 else (False,None)])
        #     print('ret = ', ret)
        #     if ret == 'success':
        #         return True

        return ret_code == 'success'

    def movel(self, target_frame, duration=1.0, conds=[], goal_tolerance=0.002, timeout=5.0):
        print('goal = ', target_frame)
        tm_start = time.time()
        self.r.movel(target_frame, wait=False)
        while True:
            rospy.sleep(0.125)
            state = self.r.get_state()
            print('z = ', state.z())
            tm_current = time.time()
            for cond in conds:
                print('fz=', state.fz())
                satisfied,result = cond(state)
                if satisfied:
                    return result
            error = norm(state.xyz() - np.array(target_frame[0]), ord=2)
            print('e = ', error)
            if error < goal_tolerance:
                return 'reached'
            if tm_current - tm_start > timeout:
                return 'timeout'

    def moveg(self, width, timeout):
        return self.r.moveg(width, timeout)


def run_task(n=0):
    """
    Each call to the 'server' correspond to the service call to ROS action server.
    """

    server.init_task(env={'task_frame': hole_frames[n]})
    # pick motion
    # server.openg()
    # server.movel()
    # server.movel() # approach
    # server.closeg()
    # server.movel() # retreat
    # transport motion
    server.movel(([0,0,0.05],[0,0,0,1]))
    server.movel(([0,0,0.03],[0,0,0,1]))
    server.insertion() # special skill
    server.movel(([0,0,0.01],[0,0,0,1]), conds=[lambda s: (True,'success') if (s.fz() > 4 and s.z() < 0.01) else (False,None)])
    server.movel(([0,0,0.04],[0,0,0,1]), duration=2.0)


def init():
    server = SkillServer()
    # server.init_task(env={'task_frame': (hole_frames[0][0],[0,0,0,1])})
    return server

if __name__ == '__main__':
    server = test()
    print(server)
