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
SELECT task_inst_id, trans_id, source_id, source_index, target_id, 0
FROM tmp_T_TRANSITION;

UPDATE T_TRANSITION set source_index = 0;

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
SELECT flow_id, trans_id, source_id, source_index, target_id, 0
FROM tmp_T_FLOW_TRANSITION;

UPDATE T_FLOW_TRANSITION set source_index = 0;

drop table tmp_T_FLOW_TRANSITION;


ALTER TABLE M_MODEL ADD COLUMN hash text;
