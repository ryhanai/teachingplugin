# -*- coding: utf-8 -*-

import numpy as np
import transforms3d as t3
import copy

class Frame(object):
    """
    Note that Choreonoid uses Z-Y-X euler angles.
    The arguments of __init__() should not be updated to
    avoid unexpected change of task parameter values.
    """

    def __init__(self,
                     tf=None,
                     xyzRPY=np.zeros(6),
                     parent=None,
                     angle_unit='degree'):
        if tf:
            self.transform = tf.transform
        else:
            if type(xyzRPY) == list:
                xyzRPY = np.array(xyzRPY)
            if angle_unit == 'degree':
                RPY = np.radians(xyzRPY[3:])
            elif angle_unit == 'radian':
                RPY = xyzRPY[3:]
            else:
                raise ValueError('invalid angle_unit: {}'.format(angle_unit))
            T = xyzRPY[:3]
            R = t3.euler.euler2mat(*RPY[::-1], axes='rzyx')
            self.transform = t3.affines.compose(T, R, np.ones(3))

        if parent:
            self.world_transform = np.dot(parent.world_transform, self.transform)
        else:
            self.world_transform = self.transform

    def __repr__(self):
        return 'Frame(xyz={}, rpy={})'.format(self.xyz(), self.rpy())

    def xyz(self):
        return self.world_transform[:-1,-1]

    def rpy(self):
        return t3.euler.mat2euler(self.world_transform[:3,:3], axes='rzyx')[::-1]

    def quaternion(self):
        return t3.quaternions.mat2quat(self.world_transform[:3,:3])

    def to_tp(self):
        return np.array(list(self.xyz()) + list(self.rpy()))
