# -*- coding: utf-8 -*-

#
# $ cd ~/Program/choreonoid/ext/teachingPlugin/share/project
# $ ~/Program/choreonoid/build/bin/choreonoid teaching_plugin.cnoid --python tm_tool.py
#

import os, sys
from os import path
import transforms3d._gohlketransforms as tf

sys.path.append('../scripts/')

from tp_chore import robot
from task_tool.task_sample190606 import *

# from cnoid.TeachingPlugin import *
# from controller_framework import *

##
## Choreonoid内のロボットに対するインタフェース
##
r = robot.SimHIRONX()

# 使用例
# r.getJointAngles()
# r.setJointAngle('HEAD_JOINT0', 0.5)


##
## Pythonによるタスク設計ツールの利用
##
task = pick_place() # 基本パターンを読出し

# 使用例
# print(yaml.dump(task.compile())) # YAML形式で出力


##
## pybindを利用してteachingPluginの関数やメソッドを使う
##
# tp = TPInterface.instance() # まだ上手く動いていない

# tp.setRobotName('main_withHands')
# tp.getJointPath('')
# tp.interpolate(jointPath, xyz, rpy, tm)
# tp.followTrajectory(jointPath)




# task中のモデルの位置が、GUI上のモデルの位置に更新される
# task.updateModel('picked_obj', 'plastic bottle'))

##########
# 必要であればGUIでモデル（*.wrl）を読込み

# GUIでロボットの腕を動かす
# getTF(getFrame(‘pickF’), getLink(‘LARM_JOINT5’))

# task.insert_command(where, MoveLCmd(rec.frame()))
# task.compile_and_load() # generate YAML file, and import it to DB

# 動作を作成するときのフレームを設定
# 値をGUIから取得するために必要なのはリンク
# MotionCommandを生成するために必要な情報は？


# task.addMoveLCmd(target, base)

# GUI（or r.moveGripper()）でグリッパを動かす
# task.setParamValue(‘finger_interval’, r.getGripperWidth())

# def getTF(target_link_name, base):
#     target = r.body.getLink(target_link_name, base_frame_name).position # 3x4 matrix
#     base = task.model(‘picked_obj).tf # 3x4 matrix
#     return mat2rpy(np.dot(base.inverse(), target))

# class Task:
#     addMoveLCmd():
#         f = getFrame(target=‘LARM_JOINT5’, base=‘picked_obj’)
#         task.add_command(MoveLCmd(f, base=task.param(‘pickF’)))

# r.setGripperWidth(…) # alternatively, use GUI
# width = r.getGripperWidth()
# g1 = GripperMotion(width)

# def getTF(target, base):
#     tf_target = r.getLink(target).position
#     tf_base = task.env[base].init_value.tf
#     tf_target = np.vstack((tf_target, [0,0,0,1]))
#     tf_base = np.vstack((tf_base, [0,0,0,1]))


# def test():
#     global m
#     m = task.model('picked_obj')
#     m.setBodyItem(getItemByName('pick and place|picked_obj'))
