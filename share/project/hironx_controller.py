# -*- coding: utf-8 -*-

import sys
sys.path.append('../../share/scripts')

from task_tool.basic_commands200303 import *
from task_tool.basic_command_impl200303 import *

class HiroNXController(BasicCommandController):
    def __init__(self, use_ros=False):
        super(HiroNXController, self).__init__(robotItemName='main_withHands',
                                                   toolLinkMap={0:'LARM_JOINT5',
                                                                    1:'RARM_JOINT5'}
                                                   )

    def commands(self):
        return [MoveLCmd, GripperCmd]

controller = HiroNXController(use_ros=False)

# 2020.03.09
# The following functions are to be removed,
# after teachingPlugin is changed to call controller APIs with 'controller.' prefix.

def getToolLinkName(num):
    return controller.getToolLinkName(num)

def getCommandDefList():
    return controller.getCommandDefList()
