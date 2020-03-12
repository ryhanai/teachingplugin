# -*- coding: utf-8 -*-

import numpy as np
import transforms3d as t3
import copy
# from abc import ABC, abstractmethod

class TPValue(object):
    def __init__(self, py_value):
        self._py_value = py_value

    @property
    def py_value(self):
        return self._py_value

    @py_value.setter
    def py_value(self, v):
        self._py_value = v

    def __repr__(self):
        return 'TPValue(py_value={})'.format(self.py_value)

    def to_tp(self):
        return self.py_value

class Frame(TPValue):
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
        self._parent = parent

        if tf:
            self.transform = tf.transform
        else:
            if type(xyzRPY) == list:
                xyzRPY = np.array(xyzRPY)

            if angle_unit == 'degree':
                xyzRPY[3:] = np.radians(xyzRPY[3:])
            elif angle_unit != 'radian':
                raise ValueError('invalid angle_unit: {}'.format(angle_unit))

            self.xyzRPY = xyzRPY

        self.update()

    def update(self):
        if self._parent:
            self.world_transform = np.dot(self._parent.world_transform, self.transform)
        else:
            self.world_transform = self.transform

    def __repr__(self):
        return 'Frame(xyz={}, rpy={})'.format(self.xyz(), self.rpy())

    @property
    def xyzRPY(self, angle_unit='degree'):
        return np.concatenate([self.xyz(), self.rpy()])

    @xyzRPY.setter
    def xyzRPY(self, values):
        T = values[:3]
        RPY = values[3:]
        R = t3.euler.euler2mat(*RPY[::-1], axes='rzyx')
        self.transform = t3.affines.compose(T, R, np.ones(3))
        self.update()

    def xyz(self):
        return self.world_transform[:-1,-1]

    def rpy(self):
        return t3.euler.mat2euler(self.world_transform[:3,:3], axes='rzyx')[::-1]

    def quaternion(self):
        return t3.quaternions.mat2quat(self.world_transform[:3,:3])

    def to_tp(self):
        xyz = self.xyz()
        RPY = np.radians(self.rpy())
        return np.concatenate([xyz, RPY])
