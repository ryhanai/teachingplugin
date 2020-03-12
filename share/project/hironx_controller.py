# -*- coding: utf-8 -*-

import sys
sys.path.append('../../share/scripts')

from task_tool.basic_commands200303 import *
from task_tool.basic_command_impl200303 import *

class RecognizeCmd(Cmd):
    """ """
    _cmd_name = 'recognize'
    _disp_name = 'Recognize'
    _signature = [('result', ParamType.FRM, ParamDir.Out)]

    def __init__(self):
        super(RecognizeCmd, self).__init__()

class TestCmd(Cmd):
    """ """
    _cmd_name = 'test'
    _disp_name = 'outDoubleAndInt'
    _signature = [('outDouble', ParamType.DBL, ParamDir.Out),
                      ('outInt', ParamType.INT, ParamDir.Out),]

    def __init__(self):
        super(TestCmd, self).__init__()

class HiroNXController(BasicCommandController):
    def __init__(self, use_ros=False):
        super(HiroNXController, self).__init__(robotItemName='main_withHands',
                                                   toolLinkMap={0:'LARM_JOINT5',
                                                                    1:'RARM_JOINT5'}
                                                   )

    def commands(self):
        return [MoveLCmd, GripperCmd, RecognizeCmd, TestCmd]

    def recognize(self, result):
        result.xyzRPY = np.array([0.5, 0, 0.8, 0, 0, 0])
        return True

    def testOutArg(self, outInt, outDouble):
        print('outInt={}, outDouble={}'.format(outInt, outDouble))
        outInt.py_value = 1234
        outDouble.py_value = 3.14
        return True

controller = HiroNXController(use_ros=False)

# 2020.03.09
# The following functions are to be removed,
# after teachingPlugin is changed to call controller APIs with 'controller.' prefix.

def getToolLinkName(num):
    return controller.getToolLinkName(num)

def getCommandDefList():
    return controller.getCommandDefList()
