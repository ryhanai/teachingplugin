Delete From T_FIGURE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_FILE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_TASK_INST_PARAMETER Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_STATE Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_STATE_ACTION Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_ARGUMENT Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_TRANSITION Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
Delete From T_MODEL_INFO Where task_inst_id NOT IN (select distinct(task_inst_id) from T_TASK_MODEL_INST);
