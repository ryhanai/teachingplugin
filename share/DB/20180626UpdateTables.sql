ALTER TABLE T_FLOW RENAME TO tmp_T_FLOW;

CREATE TABLE T_FLOW (
	flow_id				integer primary key,
	name 				text not null,
	comment				text,
	created_date		text not null,
	last_updated_date	text not null
);

INSERT INTO T_FLOW(flow_id, name, comment, created_date, last_updated_date)
SELECT flow_id, name, comment, created_date, last_updated_date
FROM tmp_T_FLOW;

drop table tmp_T_FLOW;



ALTER TABLE T_FLOW_MODEL_PARAM RENAME TO tmp_T_FLOW_MODEL_PARAM;

CREATE TABLE T_FLOW_MODEL_PARAM (
	flow_id			integer not null,
	model_id		integer not null,
	master_id		integer not null,
	pos_x			real not null,
	pos_y			real not null,
	primary key(flow_id, model_id)
);

INSERT INTO T_FLOW_MODEL_PARAM(flow_id, model_id, master_id, pos_x, pos_y)
SELECT flow_id, model_id, master_id, pos_x, pos_y
FROM tmp_T_FLOW_MODEL_PARAM;

drop table tmp_T_FLOW_MODEL_PARAM;




ALTER TABLE T_TASK_MODEL_INST RENAME TO tmp_T_TASK_MODEL_INST;

CREATE TABLE T_TASK_MODEL_INST (
	task_inst_id		integer primary key,
	name 				text not null,
	comment				text,
	flow_id				integer,
	created_date		text not null,
	last_updated_date	text not null,
	exec_env			text
);


INSERT INTO T_TASK_MODEL_INST(task_inst_id, name, comment, flow_id, created_date, last_updated_date, exec_env)
SELECT task_inst_id, name, comment, flow_id, created_date, last_updated_date, exec_env
FROM tmp_T_TASK_MODEL_INST;

drop table tmp_T_TASK_MODEL_INST;



alter table T_MODEL_INFO add hide integer;
UPDATE T_MODEL_INFO SET hide = 0;


Insert into T_FLOW (flow_id, name, comment, created_date, last_updated_date) VALUES ( -1, 'test', '', datetime('now', 'localtime'), datetime('now', 'localtime') );

Delete From T_TASK_MODEL_INST Where flow_id NOT IN (select distinct(flow_id) from T_FLOW);
Delete From T_FLOW Where flow_id = -1;


Delete From T_FIGURE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_FILE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_TASK_INST_PARAMETER Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_STATE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_STATE_ACTION Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_ARGUMENT Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_TRANSITION Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_MODEL_INFO Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);

