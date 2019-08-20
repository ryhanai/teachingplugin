import numpy as np
from cnoid.Base import *
from cnoid.BodyPlugin import *
import transforms3d._gohlketransforms as tf
#from tp_util.sampling import *

import cnoid.Body as bd
import cnoid.Util as ut

def getItemByName(name):
    return RootItem.instance.findItem(name)

class SimRobot(object):
    """ """

    def __init__(self, robot_item_name):
        self.robot_item = getItemByName(robot_item_name)
        self.body = self.robot_item.body

    def setIKBase(self, base_link_name):
        self.base_link_name = base_link_name

    def setIKTarget(self, target_link_name):
        self.target_link_name = target_link_name

    def getJoint(self, name):
        for i in range(self.body.numJoints):
            if self.body.joint(i).name == name:
                return self.body.joint(i)
        raise Exception('joint "%s" not found'%name)

    # def getLink(self, name):
    #     for i in range(self.body.numLinks()):
    #         if self.body.link(i).name() == name:
    #             return self.body.link(i)
    #     raise Exception('link "%s" not found'%name)

    def getLink(self, name):
        return self.body.link(name)

    def getTF(self, base, target):
        tf_base = self.getLink(base).position
        tf_target = self.getLink(target).position
        tf_target = np.vstack((tf_target, [0,0,0,1]))
        tf_base = np.vstack((tf_base, [0,0,0,1]))
        return np.dot(tf.inverse_matrix(tf_base), tf_target)

    def setJointAngle(self, name, q):
        self.getJoint(name).q = q
        self.robot_item.notifyKinematicStateChange(True)

    def getJointAngle(self, name):
        return self.getJoint(name).q

    def getJointAngles(self):
        return np.array([self.body.joint(i).q for i in range(self.body.numJoints)])

    def getCurrentPose(self, link_name):
        return self.getLink(link_name).position()

    def setJointAngles(self, jv):
        if len(jv) != self.body.numJoints:
            raise Exception('vector length does not match the number of joints')
        for i in range(len(jv)):
            self.body.joint(i).q = jv[i]
        self.robot_item.notifyKinematicStateChange(True)

    def moveCartesian(self, frame):
        lbase = self.getLink(self.base_link_name)
        ltarget = self.getLink(self.target_link_name)
        path = bd.getCustomJointPath(self.body, lbase, ltarget)
        #if path.calcInverseKinematics(frame[:3,3], ltarget.calcRfromAttitude(frame[:3,:3])) == True:
        if path.calcInverseKinematics(frame) == True:
            self.robot_item.notifyKinematicStateChange(True)
        else:
            raise Exception('IK failure')

    def moveTCP(self, tf_w):
        self.moveCartesian(np.dot(np.dot(tf_w, self.tf_w2tcp), self.tf_tcp2palm))

    def goInitial(self):
        self.setJointAngles(self.q0)


from tp_chore.ur5param import *

class SimUR5(SimRobot):
    def __init__(self):
        super(SimUR5, self).__init__(ur5param['robot_item'])
        self.setIKBase(ur5param['ikbase'])
        self.setIKTarget(ur5param['iktarget'])
        self.q0 = np.array(ur5param['init_pose'])
        self.tf_palm2tcp = pe2mat(*ur5param['palm2tcp'])
        self.tf_tcp2palm = t.inverse_matrix(self.tf_palm2tcp)
        self.tf_w2tcp = t.euler_matrix(*ur5param['world2tcp'])

    def moveHand(self, q):
        if q < -0.40 or q > 0.40:
            return
        else:
            self.setJointAngle('right_knuckle_joint', q)
            self.setJointAngle('right_inner_knuckle_joint', q)
            self.setJointAngle('right_finger_tip_joint', -q)
            self.setJointAngle('left_knuckle_joint', q)
            self.setJointAngle('left_inner_knuckle_joint', q)
            self.setJointAngle('left_finger_tip_joint', -q)

class SimHIRONX(SimRobot):
    def __init__(self):
        super(SimHIRONX, self).__init__('World/main_withHands')
        self.setIKBase('WAIST')
        self.setIKTarget('RARM_JOINT5')
