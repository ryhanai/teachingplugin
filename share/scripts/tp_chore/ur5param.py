# -*- coding: utf-8 -*-

import math

ur5param = {'palm2tcp' : ([0,0.24,0], [math.pi/2, 0, 0]),
            'world2tcp' : [0, 0, 0],
            'init_pose' : [ 0.075, -1.97222205, -1.78023584, -0.95993109, 1.57079633, -1.50098316,
                            -0.4, 0., -0.4, 0.4, -0.4, 0., -0.4, 0.4],
            'ikbase' : 'base_link',
            'iktarget' : 'robotiq_140_base_joint',
            'robot_item' : 'World/main_withHands'}
