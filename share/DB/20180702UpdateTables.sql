ALTER TABLE T_TASK_INST_PARAMETER ADD COLUMN param_type integer not null default 2;

UPDATE T_TASK_INST_PARAMETER SET param_type=4 where 1<elem_num;


ALTER TABLE T_TASK_INST_PARAMETER RENAME TO tmp_T_TASK_INST_PARAMETER;
CREATE TABLE T_TASK_INST_PARAMETER (
	task_inst_id	integer not null,
	task_param_id	integer not null,
	name			text,
	rname			text,
	unit			text,
	value			text,
	hide			integer,
	type			integer not null,	/* 0:Normal, 1:Model */
	model_id		integer,
	model_param_id	integer,
	param_type		integer not null,	/* 1:int, 2:double, 3:string, 4:frame */
	primary key(task_inst_id, task_param_id)
);
INSERT INTO T_TASK_INST_PARAMETER(task_inst_id, task_param_id, name, rname, unit, value, hide, type, model_id, model_param_id, param_type)
SELECT task_inst_id, task_param_id, name, rname, unit, value, hide, type, model_id, model_param_id, param_type
FROM tmp_T_TASK_INST_PARAMETER;
drop table tmp_T_TASK_INST_PARAMETER;

ALTER TABLE T_FLOW_PARAMETER ADD COLUMN param_type integer not null default 2;
