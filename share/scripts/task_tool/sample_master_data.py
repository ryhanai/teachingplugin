# encoding: utf-8

from task_tool.task_design import *

db = MasterManager()
db.add('green_tea_bottle_with_cap', 'greentea350hrp.wrl', 'greentea350.jpg',
           [Feature('cap_frame', tf=[0,0,0.095,0,0,0])])
db.add('tray', 'box_trayHrp.wrl')
db.add('green_tea_cap', 'greentea350_capHrp.wrl')
db.add('MA266_pully', 'MA266_x2Hrp.wrl')
db.add('board', 'boardHrp.wrl')
db.add('SEBZ16-150_rail', 'SEBZ16-150_railHrp.wrl')
db.add('bolt', 'CBM3-10Hrp.wrl')
db.add('screw feeder', 'screw_feederHrp.wrl')
db.add('fixture', 'fixture01Hrp.wrl')
db.add('SEBZ16_block', 'SEBZ16-270_blockHrp.wrl')
db.add('ycb_driver_bit', 'driver_tipHrp.wrl', 'ycb_driver_bit.jpg')
db.add('ycb_main_wing', 'ycb_main_wingHrp.wrl', 'ycb_main_wing.jpg')
db.add('ycb_tool_station', 'ycb_tool_stationHrp.wrl', 'ycb_tool_station.jpg',
           [Feature('hole0', tf=[-0.05,0,0,0,0,0]),
            Feature('hole1', tf=[-0.01,0,0,0,0,0]),
            Feature('hole2', tf=[0.01,0,0,0,0,0]),
            Feature('hole3', tf=[0.03,0,0,0,0,0])]
           )
db.add('ycb_airplane_body', 'YCBAirplane_before_boltingHrp.wrl', 'ycb_airplane_body.jpg')
db.add('ycb_screw', 'boltHrp.wrl', 'ycb_screw.jpg')
db.add('green_tea_bottle', 'greentea350_bottleHrp.wrl', 'greentea350.jpg',
           [Feature('cap_frame', tf=[0,0,0.095,0,0,0])])
db.add('FP', 'frameHrp.wrl')
