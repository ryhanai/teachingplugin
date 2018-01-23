DELETE from T_TASK_INST_PARAMETER Where type = 1;

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
	primary key(task_inst_id, task_param_id)
);
INSERT INTO T_TASK_INST_PARAMETER(task_inst_id, task_param_id, elem_num, name, rname, unit, value, hide)
SELECT task_inst_id, task_param_id, elem_num, name, rname, unit, value, hide
FROM tmp_T_TASK_INST_PARAMETER;
drop table tmp_T_TASK_INST_PARAMETER;

vacuum;
