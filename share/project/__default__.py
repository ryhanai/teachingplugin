import transformations as tf
from numpy import *
def translation_from_matrix(m):
    return tf.translation_from_matrix(m)
def translation_matrix(v):
    return tf.translation_matrix(v)
def euler_from_matrix(m):
    return rad2deg(tf.euler_from_matrix(m))
def euler_matrix(e):
    return tf.euler_matrix(*deg2rad(e))
def f(xyzrpy):
    return dot(translation_matrix(xyzrpy[:3]), euler_matrix(xyzrpy[3:]))
