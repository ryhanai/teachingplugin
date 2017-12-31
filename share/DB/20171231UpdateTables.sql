drop table T_VIA_POINT;
drop table T_FLOW_VIA_POINT;

ALTER TABLE T_TRANSITION RENAME TO tmp_T_TRANSITION;
CREATE TABLE T_TRANSITION (
	task_inst_id	integer not null,
	trans_id		integer not null,
	source_id		integer not null,
	target_id		integer not null,
	source_index		integer,
	primary key(task_inst_id, trans_id)
);
INSERT INTO T_TRANSITION(task_inst_id, trans_id, source_id, target_id, source_index)
SELECT task_inst_id, trans_id, source_id, target_id, 0
FROM tmp_T_TRANSITION;
drop table tmp_T_TRANSITION;

ALTER TABLE T_FLOW_TRANSITION RENAME TO tmp_T_FLOW_TRANSITION;
CREATE TABLE T_FLOW_TRANSITION (
	flow_id			integer not null,
	trans_id		integer not null,
	source_id		integer not null,
	target_id		integer not null,
	source_index		integer,
	primary key(flow_id, trans_id)
);
INSERT INTO T_FLOW_TRANSITION(flow_id, trans_id, source_id, target_id, source_index)
SELECT flow_id, trans_id, source_id, target_id, 0
FROM tmp_T_FLOW_TRANSITION;
drop table tmp_T_FLOW_TRANSITION;

vacuum;
