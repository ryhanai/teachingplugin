#!/usr/bin/python3
# encoding: utf-8

import numpy as np
import yaml

from collections import OrderedDict
import functools
import operator
from enum import Enum


foldr = lambda func, acc, xs: functools.reduce(lambda x, y: func(y, x), xs[::-1], acc)
def transform_exp(tfchain, xyz=None):
    if xyz != None:
        XYZ = foldr(lambda x,y: '(xyz(%s) + rotFromRpy(rpy(%s)) * %s)'%(x,x,y),
                        xyz, tfchain)
    else:
        XYZ = foldr(lambda x,y: '(xyz(%s) + rotFromRpy(rpy(%s)) * %s)'%(x,x,y),
                        'xyz(%s)'%tfchain[-1], tfchain[:-1])
    RPY = 'rpyFromRot(%s)'%foldr(lambda x,y: 'rotFromRpy(rpy(%s)) * %s'%(x,y),
                                     'rotFromRpy(rpy(%s))'%tfchain[-1], tfchain[:-1])
    return XYZ, RPY

class quoted(str):
    pass

def str_representer(dumper, data):
    return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='"')
yaml.add_representer(quoted, str_representer)

def represent_ordereddict(dumper, data):
    value = []
    for item_key, item_value in data.items():
        node_key = dumper.represent_data(item_key)
        node_value = dumper.represent_data(item_value)
        value.append((node_key, node_value))
    return yaml.nodes.MappingNode(u'tag:yaml.org,2002:map', value)
yaml.add_representer(OrderedDict, represent_ordereddict)

# Exp

def comp(e):
    if type(e) == EPlus:
        return '(%s + %s)' % (comp(e.le), comp(e.re)) # 優先度に応じて不要な括弧はなくす方がすっきりする
    elif isinstance(e, Param):
        return e.name
    elif type(e) == Model:
        print('models are not allowed to be used in expressions')
        return e
    elif type(e) == Frame:
        return e.tf
    elif type(e) == np.ndarray and len(e) == 6: # TF
        return list(e)
    elif isinstance(e, Value):
        return e.value
    else:
        print('unknown expression: ', e)
        return e

def comp_def(e, mm):
    if type(e) == Model:
        return OrderedDict([('model_id', e.model_id),
                            ('name', quoted(e.name)),
                            ('master_id', mm.master_id(e.master_name)),
                            ('type', e.model_type),
                            ('pos', e.tf)])
    elif isinstance(e, ParamModel):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            ('values', e.init_value.tf.copy()), # このフィールドは移行後に不要になる（モデルが持っているので）
                            ('model_id', e.init_value.model_id),
                            ('model_param_id', -1),
                            ('hide', e.hide)])
        return code
    elif isinstance(e, ParamTF):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            ('values', e.init_value),
                            ('model_id', 0),
                            ('hide', e.hide)])
        return code
    elif isinstance(e, Param):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            ('values', comp(e.init_value)),
                            ('hide', e.hide)])
        return code

class Value:
    def __init__(self, value):
        self.value = value
class Integer(Value):
    pass
class Double(Value):
    pass

class Frame(Value):
    """ """
    def __init__(self, tf, parent=None):
        self.parent = parent
        self.tf = tf

class Model(Frame):
    """
    modelのnameは重複がない内部名なので、model_idで整数値を振る必要がない（移行途中なのでmodel_idを残す）
    modelはdisp_nameを持たない。nameを表示にも使用している。
    model_masterのfeatures.valueのquotation（ここには”式”を書けるため文字列。yamlで確実に文字列と解釈するためquotationをつけている）
    task parameterがmodelにbindされている場合、frame値が重複して保持されている（これも移行途中のため残す）

    現在の仕様ではModelはparentを持てない（self.parentは常にNone。modelはすべてのparentがworldの扱い）

    Arguments:
    model_id: 移行途中なのでmodel_idを残している。model_masterもnameが一意性を保証しているので別途idとして整数を振る必要がない。

    """
    def __init__(self, tf, name, model_id, master_name, model_type='Work'):
        super().__init__(tf)
        self.name = name
        self.model_id = model_id
        self.master_name = master_name
        self.model_type = model_type


class EPlus:
    """ """
    def __init__(self, le, re):
        self.le = le
        self.re = re

class ParamType(Enum):
    UNDEF=0,
    INT=1,
    DBL=2,
    TF=3,
    FRM=4

    def comp(param_type):
        output = {ParamType.FRM:4, ParamType.TF:4, ParamType.DBL:2, ParamType.INT:1}
        return output[param_type]

class Param:
    """ """
    last_number = 0

    def get_number(self):
        n = Param.last_number
        Param.last_number += 1
        return n

    def __init__(self, name, init_value, disp_name, type_prefix, hide=False):
        # self.name = '%c%02d' % (type_prefix, self.get_number())
        self.name = name
        if disp_name:
            self.disp_name = disp_name
        else:
            self.disp_name = name
        self.init_value = init_value
        self.param_type = ParamType.UNDEF
        #self.unit = ''
        self.hide = hide


class ParamModel(Param):
    """
    This parameter is bound to some 3D model
    init_value: name of model
    """
    def __init__(self, name, init_value, disp_name=None, hide=False):
        super().__init__(name, init_value, disp_name, type_prefix = 'F', hide=hide)
        self.param_type = ParamType.FRM


class ParamTF(Param):
    """ """
    def __init__(self, name, init_value, disp_name=None, hide=False):
        super().__init__(name, init_value, disp_name, type_prefix = 'T', hide=hide)
        self.param_type = ParamType.TF

class ParamDouble(Param):
    """ """
    def __init__(self, name, init_value, disp_name=None, hide=False):
        super().__init__(name, Double(init_value), disp_name, type_prefix = 'D', hide=hide)
        self.param_type = ParamType.DBL

class ParamInt(Param):
    """ """
    def __init__(self, name, init_value, disp_name=None, hide=False):
        super().__init__(name, Integer(init_value), disp_name, type_prefix = 'I', hide=hide)
        self.param_type = ParamType.INT

class State:
    """
    - state id is automatically numbered
    - positions of each state will be automatically computed in the future
    """
    def __init__(self, pos=[0,0]):
        self.id = 0
        self.type = 0
        self.condition = ''
        self.pos = pos

    def set_id(self, state_id):
        self.id = state_id

    def compile(self):
        return OrderedDict([('id', self.id),
                            ('type', self.type),
                            ('pos', self.pos)])

class InitialState(State):
    def __init__(self, pos=[0,0]):
        super().__init__(pos)
        self.type = 1

    def compile(self):
        code = super().compile()
        return code

class FinalState(State):
    def __init__(self, pos=[0,0]):
        super().__init__(pos)
        self.type = 2

    def compile(self):
        code = super().compile()
        return code

class Cmd(State):
    def __init__(self, cmd_name, disp_name, pos=[0,0]):
        super().__init__(pos)
        self.type = 5
        self.cmd_name = cmd_name
        self.disp_name = disp_name

    def compile(self):
        data = super().compile()
        data['cmd_name'] = quoted(self.cmd_name)
        data['disp_name'] = quoted(self.disp_name)
        return data

class ModelAction:
    def __init__(self, action, model, target):
        self.action = action
        self.model = model
        self.target = target

    def compile(self):
        return OrderedDict([('action', quoted(self.action)),
                            ('model', quoted(self.model)),
                            ('target',quoted(comp(self.target)))])

class Transition:
    def __init__(self, source_id, target_id):
        self.source_id = source_id
        self.target_id = target_id

    def compile(self):
        return OrderedDict([('source_id', self.source_id),
                            ('target_id', self.target_id),
                            #('source_index', 0), # used only by conditional nodes
                            #('target_index', 0)
                            ])

class ModelMaster:
    def __init__(self, master_id, name, file_name, image_file_name=None, features=[]):
        self.name = name
        self.master_id = master_id
        self.file_name = file_name
        self.image_file_name = image_file_name
        self.features = features

    def compile(self):
        code = OrderedDict([('id', self.master_id),
                            ('name', quoted(self.name)),
                            ('file_name', quoted(self.file_name))])
        # optional fields
        if self.image_file_name:
            code['image_file_name'] = quoted(self.image_file_name)
        if len(self.features) > 0:
            code['features'] = [x.compile() for x in self.features]
        return code

class Feature:
    """ """
    def __init__(self, name, tf):
        self.name = name
        self.tf = tf

    def compile(self):
        return OrderedDict([('name', quoted(self.name)),
                            ('value', quoted('origin + %s' % str(self.tf)))])

class MetaDataImage:
    def __init__(self, image_file_name):
        self.image_file_name = image_file_name

    def compile(self):
        return OrderedDict([('name', quoted(self.image_file_name))])

class MetaDataFile:
    def __init__(self, file_name):
        self.file_name = file_name

    def compile(self):
        return OrderedDict([('name', quoted(self.file_name))])

class Task:
    """ """
    def __init__(self, name, comment, master_manager):
        self.name = name
        self.comment = comment
        self.mm = master_manager

        self.max_id = 1
        self.initialize = ''
        self.models = []
        self.env = OrderedDict([])
        self.menv = OrderedDict([])
        self.transitions = []
        self.states = []
        self.metadata_images = []
        self.metadata_files = []
        self.define_task_model()

    def define_task_model(self):
        pass

    def add_model(self, name, master_name, tf):
        n_models = len(self.menv.items())
        self.menv[name] = Model(tf, name, n_models+1, master_name)

    def add_param(self, param):
        self.env[param.name] = param

    def add_params(self, params):
        for param in params:
            self.add_param(param)

    def add_command(self, command):
        self.states.append(command)

    def add_commands(self, commands):
        self.states.extend(commands)

    def add_metadata_file(self, file_name):
        self.metadata_files.append(MetaDataFile(file_name))

    def add_metadata_image(self, image_file_name):
        self.metadata_images.append(MetaDataImage(image_file_name))

    def getp(self, name):
        """
        display_nameは重複があるのでcodingでは使用不可
        やはりnameをユーザ指定にし、display_name省略時にはdisplay_name <- nameとする
        """
        return self.env[name]

    def auto_layout(self):
        x0,y0 = 0,0
        xspace,yspace = 120,140
        for i,st in enumerate(self.states):
            st.pos = [x0 + (i % 4) * xspace, y0 + (i // 4) * yspace]

    def compile(self):
        # add some states
        self.states.insert(0, InitialState())
        self.states.append(FinalState())
        for i,s in enumerate(self.states):
            s.id = i+1
        self.auto_layout()

        # fill transitions
        for i in range(len(self.states)-1):
            from_id = self.states[i].id
            to_id = self.states[i+1].id
            self.transitions.append(Transition(from_id, to_id))

        # output master fields used in the task
        referenced_masters = {}
        for model in self.menv.values():
            master = self.mm.get_master(model.master_name)
            referenced_masters[master.name] = master
        return OrderedDict([('taskName', quoted(self.name)),
                            ('comment', quoted(self.comment)),
                            # ('initialize', quoted(self.initialize)),
                            ('models', [comp_def(x, self.mm) for x in self.menv.values()]),
                            ('states', [x.compile() for x in self.states]),
                            ('transitions', [x.compile() for x in self.transitions]),
                            ('parameters', [comp_def(x, self.mm) for x in self.env.values()]),
                            ('files', [x.compile() for x in self.metadata_files]),
                            ('images', [x.compile() for x in self.metadata_images]),
                            ('model_master', [x.compile() for x in referenced_masters.values()])])
