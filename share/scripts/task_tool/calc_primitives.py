import transforms3d._gohlketransforms as tf
import numpy as np

def xyz(transform):
    return transform[:3]
def rpy(transform):
    return transform[3:]
def rotFromRpy(rpy):
    return tf.euler_matrix(*rpy, axes='rxyz')[:3,:3]
def rpyFromRot(rotation):
    return tf.euler_from_matrix(rotation, axes='rxyz')
