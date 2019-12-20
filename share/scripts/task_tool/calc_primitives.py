import transforms3d._gohlketransforms as tf
import numpy as np

class CalculatorMatrix:
    def __init__(self, arr):
        self._arr = arr
    def __add__(self, x):
        return self.__class__(self._arr + x._arr)
    def __mul__(self, x):
        if isinstance(x, CalculatorMatrix):
            y = np.matrix(x._arr)
            return self.__class__(np.array(np.matrix(self._arr) * y))
        if isinstance(x, list) or isinstance(x, np.ndarray):
            y = np.matrix(x).T
            return np.array((np.matrix(self._arr) * y).T)[0]
    def __repr__(self):
        return self._arr.__repr__()

def xyz(transform):
    return transform[:3]
def rpy(transform):
    return transform[3:]
def rotFromRpy(rpy):
    return CalculatorMatrix(tf.euler_matrix(*rpy, axes='rxyz')[:3,:3])
def rpyFromRot(rotation):
    return tf.euler_from_matrix(rotation._arr, axes='rxyz')
