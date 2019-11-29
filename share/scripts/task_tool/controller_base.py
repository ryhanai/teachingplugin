# -*- coding: utf-8 -*-

import yaml

class CommandInterfaceGenerator(object):
    """ """
    def __init__(self, commands):
        self._commands = commands

    def yaml_description(self):
        return yaml.dump([cmd.compile_signature() for cmd in self._commands])

class ControllerBase(object):
    def __init__(self, robotItemName, toolLinkMap={}):
        self.__robotItemName = robotItemName
        self.__toolLinkMap = toolLinkMap

    def commands(self):
        """ abstract """
        return []

    def robotItemName(self):
        return self.__robotItemName

    def toolLinkMap(self):
        return self.__toolLinkMap

    def toolLink(self, key):
        return self.__toolLinkMap[key]

    def getCommandDefList(self):
        gen = CommandInterfaceGenerator(self.commands())
        return gen.yaml_description()

    def executeCommand(self, encoded_cmd):
        cmd = yaml.load(encoded_cmd, Loader=yaml.SafeLoader)[0]
        cmdName = cmd['commandName']
        args = cmd['args']
        isReal = cmd['isReal']

        try:
            return self.doExecuteCommand(cmdName, args, isReal)
        except Exception as e:
            print('Exception: ', e)
            return False

    def doExecuteCommand(self, commandName, params, isReal):
        print('commandName = ', commandName)
        print('params = ', params)
        print('isReal = ', isReal)
        return getattr(self, commandName)(*params+[isReal])
