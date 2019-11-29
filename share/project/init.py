import sys
sys.path.append('../../share/scripts')

from ur3dual_controller import *
from task_tool.task_sample191115 import *

tsk = pick_place()

def load_task(tsk):
    print(yaml.dump([tsk.compile()]))
