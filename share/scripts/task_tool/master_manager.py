#!/usr/bin/python3
# encoding: utf-8

from task_design import *

class MasterManager:
    """ """
    def __init__(self):
        self.masters = {}

    def add(self, master_id, name, file_name, image_file_name=None, features=[]):
        self.masters[name] = ModelMaster(master_id, name, file_name,
                                             image_file_name=image_file_name, features=features)

    def get(self, name):
        return self.masters[name]

    def master_id(self, master_name):
        return self.get(master_name).master_id

    def get_master(self, master_name):
        return self.get(master_name)
