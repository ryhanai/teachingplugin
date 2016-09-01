vacuum;

drop table T_TASK_MODEL;

alter table T_TASK_MODEL_INST add created_date text;
alter table T_TASK_MODEL_INST add last_updated_date text;
update T_TASK_MODEL_INST set created_date = datetime('now', 'localtime');
update T_TASK_MODEL_INST set last_updated_date = datetime('now', 'localtime');

ALTER TABLE T_TASK_MODEL_INST RENAME TO tmp_T_TASK_MODEL_INST;
CREATE TABLE T_TASK_MODEL_INST (
	task_inst_id		integer primary key,
	name 				text not null,
	comment				text,
	flow_id				integer,
	created_date		text not null,
	last_updated_date	text not null
);
INSERT INTO T_TASK_MODEL_INST(task_inst_id, name, comment, flow_id, created_date, last_updated_date)
SELECT task_inst_id, name, comment, flow_id, created_date, last_updated_date
FROM tmp_T_TASK_MODEL_INST;
drop table tmp_T_TASK_MODEL_INST;


alter table T_FLOW add created_date text;
alter table T_FLOW add last_updated_date text;
update T_FLOW set created_date = datetime('now', 'localtime');
update T_FLOW set last_updated_date = datetime('now', 'localtime');
alter table T_FLOW add comment text;
update T_FLOW set comment = "";

alter table T_TASK_PARAMETER add value text;
update T_TASK_PARAMETER set value
   = (select value from T_TASK_INST_PARAMETER where T_TASK_PARAMETER.task_param_id = T_TASK_INST_PARAMETER.task_param_id);
drop table T_TASK_INST_PARAMETER;
ALTER TABLE T_TASK_PARAMETER RENAME TO T_TASK_INST_PARAMETER;


ALTER TABLE T_TASK_INST_PARAMETER RENAME TO tmp_T_TASK_INST_PARAMETER;
CREATE TABLE T_TASK_INST_PARAMETER (
	task_param_id	integer primary key,
	task_inst_id			integer not null,
	type			integer not null,
	model_name		text,
	elem_num		integer not null,
	elem_types		text,
	name			text,
	rname			text,
	unit			text,
	value			text
);
INSERT INTO T_TASK_INST_PARAMETER(task_param_id, task_inst_id, type, model_name, elem_num, elem_types, name, rname, unit, value)
SELECT task_param_id, task_id, type, model_name, elem_num, elem_types, name, rname, unit, value
FROM tmp_T_TASK_INST_PARAMETER;
drop table tmp_T_TASK_INST_PARAMETER;

vacuum;
