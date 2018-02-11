ALTER TABLE T_TASK_INST_PARAMETER ADD COLUMN type integer not null default 0;
ALTER TABLE T_TASK_INST_PARAMETER ADD COLUMN model_id integer;
ALTER TABLE T_TASK_INST_PARAMETER ADD COLUMN model_param_id integer;

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
