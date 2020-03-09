# -*- coding: utf-8 -*-

import sys
sys.path.append('../../share/scripts')

from task_tool.task_sample191115 import *
from task_tool.util import *
from task_tool.calc_primitives import *

#
# load a sample task
#

tsk = pick_place()

def load_task(tsk):
    print(yaml.dump([tsk.compile()]))
