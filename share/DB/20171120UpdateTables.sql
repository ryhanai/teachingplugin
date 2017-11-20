CREATE TABLE M_MODEL_PARAMETER (
	model_id			integer not null,
	model_param_id		integer not null,
	name 				text not null,
	value			text,
	primary key(model_id, model_param_id)
);

vacuum;
