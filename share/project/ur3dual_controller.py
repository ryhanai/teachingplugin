# -*- coding: utf-8 -*-

from task_tool.basic_commands191115 import *
from task_tool.basic_command_impl import *

# for real robot
# from screw_insertion import SkillServer
# server = SkillServer()

class UR3dualController(BasicCommandController):
    def __init__(self):
        super(UR3dualController, self).__init__(robotItemName='main_withHands',
                                                    toolLinkMap={0:'larm_wrist_3_joint', 1:'rarm_wrist_3_joint'})

    def commands(self):
        return [MoveLCmd, GripperCmd, SetTaskFrameCmd, InsertionRandomCmd]

    #
    # Implementation of extended commands
    #
    def insertionRandom(self, isReal):
        if isReal:
            return server.insertion()
        else:
            return True


controller = UR3dualController()

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
