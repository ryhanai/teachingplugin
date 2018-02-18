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
