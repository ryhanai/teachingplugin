CREATE TABLE T_FLOW (
	flow_id				integer primary key,
	name 				text not null,
	comment				text not null,
	created_date		text not null,
	last_updated_date	text not null
);
CREATE TABLE T_FLOW_STATE (
	state_id		integer primary key,
	flow_id			integer not null,
	type			integer not null,
	task_inst_id	integer,
	pos_x			real not null,
	pos_y			real not null,
	condition		text
);
CREATE TABLE T_FLOW_TRANSITION (
	trans_id		integer primary key,
	flow_id			integer not null,
	source_id		integer not null,
	target_id		integer not null,
	condition		text
);
CREATE TABLE T_FLOW_VIA_POINT (
	trans_id		integer,
	seq				integer,
	pos_x			real not null,
	pos_y			real not null,
	primary key(trans_id, seq)
);
CREATE TABLE T_TASK_MODEL_INST (
	task_inst_id		integer primary key,
	name 				text not null,
	comment				text not null,
	flow_id				integer,
	created_date		text not null,
	last_updated_date	text not null
);
CREATE TABLE T_MODEL_INFO (
	model_id		integer primary key,
	task_inst_id	integer not null,
	model_type		integer not null,
	name 			text not null,
	rname 			text not null,
	file_name		text,
	model_data		blob,
	pos_x			real not null,
	pos_y			real not null,
	pos_z			real not null,
	rot_x			real not null,
	rot_y			real not null,
	rot_z			real not null
);
CREATE TABLE T_MODEL_DETAIL (
	model_detail_id		integer primary key,
	model_id			integer not null,
	file_name			text,
	model_data			blob
);
CREATE TABLE T_FIGURE (
	figure_id		integer primary key,
	task_inst_id	integer not null,
	name 			text not null,
	data			blob
);
CREATE TABLE T_FILE (
	file_id			integer primary key,
	task_inst_id	integer not null,
	name 			text not null,
	file_data		blob
);
CREATE TABLE T_TASK_INST_PARAMETER (
	task_param_id	integer primary key,
	task_inst_id	integer not null,
	type			integer not null,
	model_name		text,
	elem_num		integer not null,
	elem_types		text,
	name			text,
	rname			text,
	unit			text,
	value			text
);
CREATE TABLE T_STATE (
	state_id		integer primary key,
	task_inst_id	integer not null,
	type			integer not null,
	cmd_name		text,
	pos_x			real not null,
	pos_y			real not null,
	condition		text
);
CREATE TABLE T_STATE_ACTION (
	state_action_id		integer primary key,
	state_id			integer not null,
	seq					integer not null,
	action				text not null,
	model				text not null,
	target				text
);
CREATE TABLE T_ARGUMENT (
	arg_id		integer primary key,
	state_id	integer not null,
	seq			integer not null,
	name		text,
	value		text
);
CREATE TABLE T_TRANSITION (
	trans_id		integer primary key,
	task_inst_id	integer not null,
	source_id		integer not null,
	target_id		integer not null,
	condition		text
);
CREATE TABLE T_VIA_POINT (
	trans_id		integer,
	seq				integer,
	pos_x			real not null,
	pos_y			real not null,
	primary key(trans_id, seq)
);

