#!/usr/bin/python3
# encoding: utf-8

import numpy as np
import yaml

from collections import OrderedDict
import functools
import operator
import copy
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
        return OrderedDict([('name', quoted(e.name)),
                            ('master_name', quoted(e.master_name)),
                            ('type', e.model_type),
                            ('pos', e.tf)])
    elif isinstance(e, ParamModel):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            #('values', e.value.tf.copy()), # このフィールドは移行後に不要になる（モデルが持っているので）
                            ('values', copy.copy(e.value.tf)),
                            ('model_name', e.value.name),
                            ('model_param_id', -1),
                            ('hide', e.hide)])
        return code
    elif isinstance(e, ParamTF):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            ('values', e.value),
                            ('hide', e.hide)])
        return code
    elif isinstance(e, Param):
        code = OrderedDict([('param_type', e.param_type.comp()),
                            ('name', quoted(e.name)),
                            ('disp_name', quoted(e.disp_name)),
                            ('values', comp(e.value)),
                            ('hide', e.hide)])
        return code

class Value(object):
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
    modelはdisp_nameを持たない。nameを表示にも使用している。
    model_masterのfeatures.valueのquotation（ここには”式”を書けるため文字列。yamlで確実に文字列と解釈するためquotationをつけている）
    task parameterがmodelにbindされている場合、frame値が重複して保持されている（これも移行途中のため残す）

    現在の仕様ではModelはparentを持てない（self.parentは常にNone。modelはすべてのparentがworldの扱い）

    """
    def __init__(self, tf, name, master_name, model_type='Work'):
        super(Model, self).__init__(tf)
        self.name = name
        self.master_name = master_name
        self.model_type = model_type
        self.body_item = None

    def updateFrame(self):
        self.tf = self.body_item.body.getRootLink().position

    def getBodyItem(self):
        return self.body_item

    def setBodyItem(self, body_item):
        """
        task中のmodel位置がbody_itemの位置に更新される（body_itemがあるときはframeを共有したほうが良さそう
        """
        self.body_item = body_item
        self.updateFrame()

    def setMaster(self, master_name):
        self.master_name = master_name

class EPlus(object):
    """ """
    def __init__(self, le, re):
        self.le = le
        self.re = re

class ParamType(Enum):
    UNDEF=0,
    INT=1,
    DBL=2,
    TF=3,
    FRM=4,
    VECTOR3=11,
    BOOL=21

    def comp(param_type):
        output = {ParamType.FRM:3, ParamType.TF:3, ParamType.DBL:2, ParamType.INT:1}
        return output[param_type]

class Param(object):
    """ """
    last_number = 0

    def get_number(self):
        n = Param.last_number
        Param.last_number += 1
        return n

    def __init__(self, name, value, disp_name, type_prefix, hide=False):
        # self.name = '%c%02d' % (type_prefix, self.get_number())
        self.name = name
        if disp_name:
            self.disp_name = disp_name
        else:
            self.disp_name = name
        self.value = value
        self.param_type = ParamType.UNDEF
        #self.unit = ''
        self.hide = hide


class ParamModel(Param):
    """
    This parameter is bound to some 3D model
    value: name of model
    """
    def __init__(self, name, value, disp_name=None, hide=False):
        super(ParamModel, self).__init__(name, value, disp_name, type_prefix = 'F', hide=hide)
        self.param_type = ParamType.FRM

class ParamTF(Param):
    """ """
    def __init__(self, name, value, disp_name=None, hide=False):
        super(ParamTF, self).__init__(name, value, disp_name, type_prefix = 'T', hide=hide)
        self.param_type = ParamType.TF

class ParamDouble(Param):
    """ """
    def __init__(self, name, value, disp_name=None, hide=False):
        super(ParamDouble, self).__init__(name, Double(value), disp_name, type_prefix = 'D', hide=hide)
        self.param_type = ParamType.DBL

class ParamInt(Param):
    """ """
    def __init__(self, name, value, disp_name=None, hide=False):
        super(ParamInt, self).__init__(name, Integer(value), disp_name, type_prefix = 'I', hide=hide)
        self.param_type = ParamType.INT

class State(object):
    """
    - state id is automatically numbered
    - positions of each state will be automatically computed in the future
    """

    _type = 0

    def __init__(self, pos=[0,0]):
        self.id = 0
        self.condition = ''
        self.pos = pos

    def set_id(self, state_id):
        self.id = state_id

    @classmethod
    def node_type(cls):
        return cls._type

    def compile(self):
        return OrderedDict([('id', self.id),
                            ('type', self.__class__.node_type()),
                            ('pos', self.pos)])

class InitialState(State):
    """ """
    _type = 1

    def __init__(self, pos=[0,0]):
        super(InitialState, self).__init__(pos)

    def compile(self):
        code = super(InitialState, self).compile()
        return code

class FinalState(State):
    """ """
    _type = 2

    def __init__(self, pos=[0,0]):
        super(FinalState, self).__init__(pos)

    def compile(self):
        code = super(FinalState, self).compile()
        return code

class Cmd(State):
    """ """
    _cmd_name = 'command'
    _disp_name = 'Command'
    _type = 5
    _signature = [('xyz', ParamType.VECTOR3),
                      ('rpy', ParamType.VECTOR3),
                      ('tm', ParamType.DBL),
                      ('armID', ParamType.INT)]

    def __init__(self, pos=[0,0]):
        super(Cmd, self).__init__(pos)

    def model_actions(self):
        return []

    @classmethod
    def signature(cls):
        return cls._signature

    @classmethod
    def params(cls):
        return list(zip(*cls._signature))[0]

    @classmethod
    def compile_signature(cls):
        code = OrderedDict([('name', quoted(cls._cmd_name)),
                            ('dispName', quoted(cls._disp_name)),
                            ('retType', quoted(Cmd.compile_type(ParamType.BOOL)))])
        args = []
        for v,t in cls.signature():
            t2 = Cmd.compile_type(t)
            if type(t2) == tuple:
                t3,l = t2
                args.append(OrderedDict([('name', quoted(v)),
                                             ('type', quoted(t3)),
                                             ('length', l)]))
            else:
                args.append(OrderedDict([('name', quoted(v)),
                                             ('type', quoted(t2))]))
        code['args'] = args
        return code

    @staticmethod
    def compile_type(t):
        output = {ParamType.VECTOR3: ('double',3),
                      ParamType.DBL: 'double',
                      ParamType.INT: 'int',
                      ParamType.BOOL: 'boolean',
                      }
        return output[t]

    def compile(self):
        code = super(Cmd, self).compile()
        code['cmd_name'] = quoted(self._cmd_name)
        code['disp_name'] = quoted(self._disp_name)
        args = []
        for var,exp in zip(self.params(), self.arguments()):
            args.append(OrderedDict([('name',quoted(var)), ('value',quoted(exp))]))
        code['arguments'] = args
        actions = []
        for model_action in self.model_actions():
            actions.append(model_action.compile())
        if actions != []:
            code['model_actions'] = actions
        return code

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
    def __init__(self, name, file_name, image_file_name=None, features=[]):
        self.name = name
        self.file_name = file_name
        self.image_file_name = image_file_name
        self.features = features

    def compile(self):
        code = OrderedDict([('name', quoted(self.name)),
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

class Task(object):
    """ """
    def __init__(self, name, comment, master_manager):
        self.name = name
        self.comment = comment
        self.mm = master_manager

        self.max_id = 1
        self.initialize = ''
        self.env = OrderedDict([])
        self.menv = OrderedDict([])
        self.transitions = []
        self.states = []
        self.metadata_images = []
        self.metadata_files = []
        self.define_task_model()

    def define_task_model(self):
        self.define_model()
        self.define_params()
        self.define_motion()

    def add_model(self, name, master_name, tf):
        self.menv[name] = Model(tf, name, master_name)

    def models(self):
        return self.menv

    def model(self, name):
        return self.menv[name]

    def replace_master(self, name, master_name):
        self.menv[name].setMaster(master_name)

    def add_param(self, param):
        self.env[param.name] = param

    def del_param(self, name):
        del self.env[name]

    def add_params(self, params):
        for param in params:
            self.add_param(param)

    def add_command(self, command):
        self.states.append(command)

    def add_commands(self, commands):
        self.states.extend(commands)

    def reset_commands(self):
        self.states = []

    def add_metadata_file(self, file_name):
        self.metadata_files.append(MetaDataFile(file_name))

    def add_metadata_image(self, image_file_name):
        self.metadata_images.append(MetaDataImage(image_file_name))

    def params(self):
        return self.env

    def param(self, name):
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


class MasterManager:
    """ """
    def __init__(self):
        self.masters = {}

    def add(self, name, file_name, image_file_name=None, features=[]):
        self.masters[name] = ModelMaster(name,
                                             file_name,
                                             image_file_name=image_file_name,
                                             features=features)

    def get(self, name):
        return self.masters[name]

    def get_master(self, master_name):
        return self.get(master_name)


