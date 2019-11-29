# -*- coding: utf-8 -*-

import sys
sys.path.append('../../share/scripts')

from ur3dual_controller import *
from task_tool.task_sample191115 import *

controller = UR3dualController(use_ros=True)

#
# teachingPluginからcontroller.executeCommand(...)
# と呼出されるようにすれば以下の大域関数定義は不要になる
#

def toolLinkMap():
    return controller.toolLinkMap()

def robotItemName():
    return controller.robotItemName()

def getCommandDefList():
    return controller.getCommandDefList()

def executeCommand(encoded_cmd):
    return controller.executeCommand(encoded_cmd)

#
# load a sample task
#

tsk = pick_place()

def load_task(tsk):
    print(yaml.dump([tsk.compile()]))
