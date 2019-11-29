#!/usr/bin/env python

from rspt_dev.util import *
import rospy
import actionlib
# import robotiq_msgs.msg

import sys, time, math
from math import pi, radians, degrees
import numpy as np
import tf
import tf2_ros

from geometry_msgs.msg import TransformStamped, Pose, WrenchStamped, TwistStamped, Point, Quaternion
from vtac_examples.srv import set_zero
from sensor_msgs.msg import JointState

from control_msgs.msg import (
    FollowJointTrajectoryAction,
    FollowJointTrajectoryGoal,
    )
from trajectory_msgs.msg import (
    JointTrajectoryPoint,
    )

class Robot(object):
    def __init__(self):
        rospy.init_node('robot_client')
        print('rospy.init_node succeeded')
        self._tf_listener = tf.TransformListener()

    def set_base_frame(self, frame_id, val, parent_frame_id):
        self._base_frame_id = frame_id
        self.set_frame(frame_id=frame_id, val=val, parent_frame_id=parent_frame_id)
        # self._base_frame = self.get_frame(frame_id)
        self._base_frame = val

    def set_active_arm(self, arm):
        warn('set_active_arm is not yet implemented')

    def activate_admittance_control(self):
        self._admc_active = True

    def deactivate_admittance_control(self):
        self._admc_active = False

    def openg(self):
        self.moveg(0.08)

    def closeg(self):
        self.moveg(0.0)

    def moveg(self, width, timeout=5.0, force=5.0, velocity=0.1):
        self._arm.moveg(width, timeout, force, velocity)

    def movej(self, joint_values):
        pass

    def getl(self):
        pass

    def movel(self, target_frame, wait=True):
        self._arm.movel(target_frame, self._base_frame, wait)

    def movel_rel(self, target_frame, base_frame=([0,0,0],[0,0,0,1]), wait=True):
        u = target_frame[0]
        Tworld_base = pose2mat(base_frame)
        u_world = np.dot(Tworld_base[:3,:3], u)
        print('U_WORLD: ', u_world)

    def go_home():
        warn('go_home is not yet implemented')

    def set_frame(self, frame_id, val, parent_frame_id='world'):
        broadcaster = tf2_ros.StaticTransformBroadcaster()
        msg = TransformStamped()
        msg.header.stamp = rospy.Time.now()
        msg.header.frame_id = parent_frame_id
        msg.child_frame_id = frame_id

        trans,quat = val
        msg.transform.translation.x = trans[0]
        msg.transform.translation.y = trans[1]
        msg.transform.translation.z = trans[2]
        msg.transform.rotation.x = quat[0]
        msg.transform.rotation.y = quat[1]
        msg.transform.rotation.z = quat[2]
        msg.transform.rotation.w = quat[3]
        broadcaster.sendTransform(msg)

    def set_frame_RPY(self, frame_id, xyz, RPY, parent_frame_id='world'):
        quat = t.quaternion_from_euler(RPY[0], RPY[1], RPY[2], axes='rxyz') # choreonoid style euler
        self.set_frame(frame_id, xyz, quat, parent_frame_id=parent_frame_id)

    def get_frame(self, frame_id, parent_frame_id='world'):
        now = rospy.Time(0)
        while not rospy.is_shutdown():
            try:
                now = rospy.Time.now()
                self._tf_listener.waitForTransform(parent_frame_id, frame_id, now, rospy.Duration(3.0))
                trans,quat = self._tf_listener.lookupTransform(parent_frame_id, frame_id, now)
                return trans,quat
            except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException) as e:
                rospy.logwarn(e)

    def get_frame_RPY(self, frame_id, parent_frame_id='world'):
        trans,quat = self.get_frame(parent_frame_id, frame_id)
        return trans,t.euler_from_quaternion(quat, axes='rxyz')

class UR5eHande(Robot):
    def __init__(self):
        super(UR5eHande, self).__init__()

        self._arm = Arm('d_bot',
                            '/gripper/gripper_action_controller',
                             'HandE',
                             '/d_bot_controller/follow_joint_trajectory',
                             '/d_bot_controller/ft_sensor/wrench_calibrated',
                             '/d_bot_controller/ft_sensor/set_zero',
                             use_URScript=True
                             )

        # transform from object to d_bot_wrist_3_link
        self._arm._p0 = [0, 0, 0.25]
        self._arm._q0 = t.quaternion_from_euler(-math.pi/2, math.pi/2, 0, axes='rxyz')
        # update TCP position
        # self.set_frame_RPY('d_bot_tool0', [0,0.25,0], [-1.57,0,0],
        #                        parent_frame_id='d_bot_wrist_3_link')


class Arm:
    def __init__(self, name,
                     gripper_action,
                     gripper_type,
                     arm_trajectory_action,
                     wrench_topic,
                     set_zero_service,
                     use_URScript
                     ):

        self._p0 = [0, 0, 0.28]
        self._q0 = t.quaternion_from_euler(-math.pi/2, -math.pi/2, 0, axes='rxyz')

        self._gripper_type = gripper_type

        self._use_URScript = use_URScript
        if (self._use_URScript):
            self._gripper = actionlib.SimpleActionClient(gripper_action, robotiq_msgs.msg.CModelCommandAction)
        else:
            self._gripper = actionlib.SimpleActionClient(gripper_action, FollowJointTrajectoryAction)

        self._gripper.wait_for_server()

        self.__name = name
        self.__wrench = np.zeros(6)

        # self.traj_client = actionlib.SimpleActionClient(
        #     arm_trajectory_action,
        #     FollowJointTrajectoryAction
        #     )
        # self.goal = FollowJointTrajectoryGoal()
        # self.goal.goal_time_tolerance = rospy.Time(0.5)
        # server_up = self.traj_client.wait_for_server(timeout=rospy.Duration(10.0))
        # if not server_up:
        #     rospy.logerr("Timed out waiting for Joint Trajectory Action Server to connect.")
        #     return False

        rospy.Subscriber(wrench_topic, WrenchStamped, self.wrench_callback, queue_size=1)
        self.set_zero_service_name = set_zero_service

        self._pub_equilibrium = rospy.Publisher('/admittance_control/equilibrium_desired', Point, queue_size=1)
        self._pub_equilibrium_pose = rospy.Publisher('/admittance_control/equilibrium_pose_desired', Pose, queue_size=1)

    @property
    def name(self):
        return self.__name

    def wrench_callback(self, msg):
        wrench_filter_factor = 0.1
        force = msg.wrench.force
        torque = msg.wrench.torque
        wv = np.array([force.x, force.y, force.z, torque.x, torque.y, torque.z])
        self.__wrench = (1 - wrench_filter_factor) * self.__wrench + wrench_filter_factor * wv
        return True

    @property
    def wrench(self):
        return self.__wrench

    def set_zero(self):
        rospy.wait_for_service(self.set_zero_service_name)
        set_zero_prxy = rospy.ServiceProxy(self.set_zero_service_name, set_zero)
        try:
            resp = set_zero_prxy()
        except rospy.ServiceException as e:
            print("Service did not process request: " + str(e))

    # @property
    # def state(self):
    #     return self._state

    # @property
    # def current_pose(self):
    #     return self._moveit_client.get_current_pose().pose

    def moveg(self, position=0.0, timeout=5.0, force=5.0, velocity=0.1):
        '''
        position:e goal opening in meter
        timeout: timeout in seconds
        '''

        if self._use_URScript:
            goal = robotiq_msgs.msg.CModelCommandGoal()
            goal.force = force
            goal.velocity = velocity
            if self._gripper_type == 'HandE':
                goal.position = position/2.0
            elif self._gripper_type == 'AdaptiveGripper':
                goal.position = position
            else:
                print('unknown gripper_type: ', self._gripper_type)
                return
        else:
            goal = FollowJointTrajectoryGoal()
            goal.trajectory.joint_names = ['rhand_robotiq_85_left_knuckle_joint']
            time = 1.0
            point = JointTrajectoryPoint()
            point.time_from_start = rospy.Duration(time)
            joint_position = -8.448133 * position + 0.75585477 # gripper IK
            point.positions = [joint_position]
            goal.trajectory.points.append(point)
            goal.trajectory.header.stamp = rospy.Time.now()

        try:
            self._gripper.send_goal(goal)
            self._gripper.wait_for_result(rospy.Duration(timeout))
            result = self._gripper.get_result()
        except rospy.ROSInterruptException:
            rospy.loginfo("INTERRUPTED", file=sys.stderr)

    def movel(self, target_frame, wait=False):
        msg = Point(*target_frame[0])
        pub.publish(msg)

    def movel(self, target_frame, base_frame, wait):
        Tworld_base = pose2mat(base_frame)
        Tbase_tcp = pose2mat(target_frame)
        Ttcp_wrist = pose2mat((self._p0, self._q0))
        Tworld_wrist = np.dot(np.dot(Tworld_base, Tbase_tcp), Ttcp_wrist)
        p = t.translation_from_matrix(Tworld_wrist)
        q = t.quaternion_from_matrix(Tworld_wrist)

        pose = Pose()
        pose.position.x = p[0]
        pose.position.y = p[1]
        pose.position.z = p[2]
        pose.orientation.x = q[0]
        pose.orientation.y = q[1]
        pose.orientation.z = q[2]
        pose.orientation.w = q[3]

        # self._moveit_client.set_pose_target(pose)
        # self._moveit_client.go()


class UR3dual(Robot):
    def __init__(self):
        super(UR3dual, self).__init__()

        self._use_URScript = False
        if self._use_URScript:
            gripper_action = '/right_arm/gripper/gripper_action_controller'
        else:
            gripper_action = '/right_hand/joint_trajectory_controller/follow_joint_trajectory'
        self._rarm = Arm('rarm',
                             gripper_action,
                             'AdaptiveGripper',
                             '/right_arm/follow_joint_trajectory',
                             '/right_arm/ft_sensor/wrench_calibrated',
                             '/right_arm/ft_sensor/set_zero',
                             use_URScript=self._use_URScript
                             )
        self._rarm.set_zero()

        self._joint_state = {}
        rospy.Subscriber('/joint_states', JointState, self.js_callback, queue_size=1)

        self.set_active_arm('rarm')
        # self.state_dim = 18

    def getl(self, in_task_frame=True):
        base_frame_id = self._base_frame_id if in_task_frame else 'world'
        if self._arm.name == 'rarm':
            return self.get_frame('rarm_tool1', base_frame_id)
        elif self._arm.name == 'larm':
            return self.get_frame('larm_tool1', base_frame_id)
        else:
            error('illegal arm name: {}'.format(self._arm.name))

    def movel(self, target_frame, wait=False):
        target_is_specified_in_task_frame = True
        if target_is_specified_in_task_frame:
            #Tworld_base = pose2mat(self.get_frame('task_frame', 'world'))
            Tworld_base = pose2mat(self._base_frame)
            Tbase_tcp = pose2mat(target_frame)
            Tworld_tcp = np.dot(Tworld_base, Tbase_tcp)
            p = t.translation_from_matrix(Tworld_tcp)
            q = t.quaternion_from_matrix(Tworld_tcp)
        else:
            p,q = target_frame

        position_only = True
        if position_only:
            msg = Point(*p)
            self._arm._pub_equilibrium.publish(msg)
        else:
            msg = Pose(Point(*p), Quaternion(*q))
            self._arm._pub_equilibrium_pose.publish(msg)

    def js_callback(self, msg):
        for n,p,v in zip(msg.name, msg.position, msg.velocity):
            self._joint_state[n] = [p, v]

    def set_active_arm(self, arm):
        if arm == 'rarm':
            self._arm = self._rarm
        elif arm == 'larm':
            self._arm = self._larm
        else:
            error('unknown arm: ', arm)

    def set_zero(self):
        self._rarm.set_zero()
        # self._larm.set_zero()

    def get_state(self):

        try:
            jv = np.array([self._joint_state['rarm_shoulder_pan_joint'],
                               self._joint_state['rarm_shoulder_lift_joint'],
                               self._joint_state['rarm_elbow_joint'],
                               self._joint_state['rarm_wrist_1_joint'],
                               self._joint_state['rarm_wrist_2_joint'],
                               self._joint_state['rarm_wrist_3_joint']])
        except:
            jv = np.zeros(6)

        p = self.getl()

        return RobotState(jv, p, self._arm.wrench)



