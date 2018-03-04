CREATE TABLE M_MODEL_PARAMETER (
	model_id			integer not null,
	model_param_id		integer not null,
	name 				text not null,
	value			text,
	primary key(model_id, model_param_id)
);

drop table T_VIA_POINT;
drop table T_FLOW_VIA_POINT;

ALTER TABLE T_TRANSITION RENAME TO tmp_T_TRANSITION;

CREATE TABLE T_TRANSITION (
	task_inst_id	integer not null,
	trans_id		integer not null,
	source_id		integer not null,
	source_index	integer not null,
	target_id		integer not null,
	target_index	integer not null,
	primary key(task_inst_id, trans_id)
);

INSERT INTO T_TRANSITION(task_inst_id, trans_id, source_id, source_index, target_id, target_index)
SELECT task_inst_id, trans_id, source_id, 0, target_id, 0
FROM tmp_T_TRANSITION;

drop table tmp_T_TRANSITION;

ALTER TABLE T_FLOW_TRANSITION RENAME TO tmp_T_FLOW_TRANSITION;

CREATE TABLE T_FLOW_TRANSITION (
	flow_id			integer not null,
	trans_id		integer not null,
	source_id		integer not null,
	source_index	integer not null,
	target_id		integer not null,
	target_index	integer not null,
	primary key(flow_id, trans_id)
);

INSERT INTO T_FLOW_TRANSITION(flow_id, trans_id, source_id, source_index, target_id, target_index)
SELECT flow_id, trans_id, source_id, 0, target_id, 0
FROM tmp_T_FLOW_TRANSITION;

drop table tmp_T_FLOW_TRANSITION;


ALTER TABLE M_MODEL ADD COLUMN hash text;

ALTER TABLE T_TASK_INST_PARAMETER RENAME TO tmp_T_TASK_INST_PARAMETER;

CREATE TABLE T_TASK_INST_PARAMETER (
	task_inst_id	integer not null,
	task_param_id	integer not null,
	elem_num		integer not null,
	name			text,
	rname			text,
	unit			text,
	value			text,
	hide			integer,
	type			integer not null,
	model_id		integer,
	model_param_id	integer,
	primary key(task_inst_id, task_param_id)
);

INSERT INTO T_TASK_INST_PARAMETER(task_inst_id, task_param_id, elem_num, name, rname, unit, value, hide, type, model_id, model_param_id)
SELECT task_inst_id, task_param_id, elem_num, name, rname, unit, value, hide, type, 0, 0
FROM tmp_T_TASK_INST_PARAMETER;
drop table tmp_T_TASK_INST_PARAMETER;


ALTER TABLE T_MODEL_INFO RENAME TO tmp_T_MODEL_INFO;

CREATE TABLE T_MODEL_INFO (
	task_inst_id	integer not null,
	model_id		integer not null,
	model_master_id integer not null,
	model_type		integer not null,
	rname 			text not null,
	pos_x			real not null,
	pos_y			real not null,
	pos_z			real not null,
	rot_x			real not null,
	rot_y			real not null,
	rot_z			real not null,
	primary key(task_inst_id, model_id)
);

INSERT INTO T_MODEL_INFO(task_inst_id, model_id, model_master_id, model_type, rname, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z)
SELECT task_inst_id, model_id, model_master_id, model_type, rname, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z
FROM tmp_T_MODEL_INFO;

drop table tmp_T_MODEL_INFO;


ALTER TABLE M_MODEL ADD COLUMN image_data blob;
ALTER TABLE M_MODEL ADD COLUMN image_file_name text;


CREATE TABLE T_FLOW_MODEL_PARAM (
	flow_id			integer not null,
	model_id		integer not null,
	master_id		integer not null,
	master_param_id	integer not null,
	pos_x			real not null,
	pos_y			real not null,
	primary key(flow_id, model_id)
);

ALTER TABLE T_FLOW_TRANSITION ADD COLUMN type integer not null default 0;

CREATE TABLE T_FLOW_PARAMETER (
	flow_id			integer  not null,
	param_id		integer  not null,
	name			text	 not null,
	value			text	 not null,
	pos_x			real	 not null,
	pos_y			real	 not null,
	primary key(flow_id, param_id)
);

vacuum;
