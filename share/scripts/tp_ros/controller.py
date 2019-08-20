class KinematicChain:
    def __init__(self, base_link, target_link):
        self._base_link = base_link
        self._target_link = target_link
        self._joint_path = tp.getJointPath(target_link)

    def joint_path(self):
        return self._joint_path

class SubController(object):
    def __init__(self, id, action_client):
        self._id = id
        self._client = action_client

    def id(self):
        return self._id

    def action_client(self):
        return self._client

class KinematicChainController(SubController):
    def __init__(self, id, kchain, action_client):
        super(KinematicChainController, self).__init__(id, action_client)
        self._kchain = kchain

    def send_trajectory(self, goal):
        print('send trajectory: ', self.id(), self._kchain.joint_path(), goal)
        jp = self._kchain.joint_path()
        xyz, rpy = goal
        tm = 2.0
        traj = tp.interpolate(jp, xyz, rpy, tm)
        return traj

        # perform ROS action
        # tp.follow_trajectory(jp)

class GripperController(SubController):
    def __init__(self, id, action_client, attach_link, custom_ik=None, custom_fk=None):
        super(GripperController, self).__init__(id, action_client)
        self._custom_ik = custom_ik
        self._custom_fk = custom_fk

    def send_gripper_command(self):
        print('send gripper command: ', self.id())

class ActionClient(object):
    def __init__(self, action):
        self._action = action

class TrajectoryActionClient(ActionClient):
    def __init__(self, action):
        super(TrajectoryActionClient, self).__init__(action)

class GripperCommandActionClient(ActionClient):
    def __init__(self, action):
        super(GripperCommandActionClient, self).__init__(action)

class StateUpdator:
    def __init__(self, *topics):
        self._topics = topics

class TeachingPluginController:
    def __init__(self, sub_controllers, state_updator):
        self._kchain_controllers = {}
        self._grpr_controllers = {}
        for sc in sub_controllers:
            if isinstance(sc, KinematicChainController):
                self._kchain_controllers[sc.id()] = sc
            elif isinstance(sc, GripperController):
                self._grpr_controllers[sc.id()] = sc
        self._state_updator = state_updator

    def dispatch(self, command, args, sub_controller_id):
        if command == 'moveL':
            sc = self._kchain_controllers[sub_controller_id]
            return sc.send_trajectory(args)
        elif command == 'grasp' or command == 'release':
            sc = self._grpr_controllers[sub_controller_id]
            return sc.send_gripper_command()

def ez_custom_ik():
    print(sys.__getframe().f_code.co_name)

def ez_custom_fk():
    print(sys.__getframe().f_code.co_name)


single_arm_controller = TeachingPluginController(
    sub_controllers =
    [
        KinematicChainController(
            0,
            KinematicChain('base', 'wrist_3_joint'),
            TrajectoryActionClient('/joint_trajectory/action')),
        GripperController(
            0,
            GripperCommandActionClient('/gripper/action'),
            'robotiq_140_base_joint',
            ez_custom_ik, ez_custom_fk)
    ],
    state_updator = StateUpdator('/arm/joint_states', '/gripper/joint_states')
    )

def test():
    return single_arm_controller.dispatch('moveL', [[0.5, 0.0, 0.8], [0, 0, 0]], 0)


def record_trajectory():
    """
    record the trajectory of an object
    """
    traj = []
    return traj

def record_arm_trajectory(arm='larm', base_link='WAIST'):
    target_links = {'larm' : 'LARM_JOINT5', 'rarm' : 'RARM_JOINT5'}
    traj = []
    tm = 1.0

    def save():
        traj.append((r.getTF(base_link, target_links[arm]), tm))

    # while True:
    #     change_the_pose_of_robot()
    #     save()

    return traj

def record_gripper_action(gripper='lgripper'):
    # change_the_pose_of_robot()
    save()
    return traj

def interpolate(ctraj):
    return jtraj

# client = actionlib.SimpleActionClient(
#     '/arm_controller/follow_joint_trajectory',
#     FollowJointTrajectoryAction
#     )


# ctraj = record_arm_trajectory
# jtraj = interpolate(ctraj)
# send_trajectory(jtraj, client)

def send_trajectory(jtraj, client):
    goal = FollowJointTrajectoryGoal()
    # add all the joint names
    name = 'hoge'
    goal.trajectory.joint_names.append(name)

    for positions, time in jtraj:
        point = JointTrajectoryPoint()
        # point.positions = copy(positions)
        point.positions = positions
        point.time_fromm_start = rospy.Duration(time)
        goal.trajectory.points.append(point)

    client.send_goal(goal)
    # client.send_goal(goal, feedback_cb=feedback)
    # while not rospy.is_shutdown() and not not self._get_trajectory_flag():
    #     rospy.sleep(0.05)
    # self._execute_gripper_commands()

