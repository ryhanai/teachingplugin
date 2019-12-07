CREATE TABLE T_TRAJECTORY (
  task_inst_id  integer not null,
  trajectory_id integer not null,
  name          text not null,
  base_object   text not null,
  base_link     text not null,
  target_object text not null,
  target_link   text not null,
  primary key(task_inst_id, trajectory_id)
);

CREATE TABLE T_VIA_POINT (
  task_inst_id  integer not null,
  trajectory_id integer not null,
  via_id        integer not null,
  seq           integer not null,
  pos_x         real not null,
  pos_y         real not null,
  pos_z         real not null,
  rot_x         real not null,
  rot_y         real not null,
  rot_z         real not null,
  trans         text not null,
  start_time    real not null,
  primary key(task_inst_id, trajectory_id, via_id)
);
