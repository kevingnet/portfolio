from . import *
from lib.utility import log
from lib.utility.log import clear_tabs
from lib.service.events import ee


logger = log.default_logger


@ee.on("onMachineIp")
def handler_onMachineIp(machineName):
    Controller.onMachineIp(machineName)


@log.class_logger
class Controller():
    actors = None

    @classmethod
    def setActors(cls, actors):
        cls.actors = actors

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.configure')
    def configure(cls):
        execute(configure, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.check')
    def check(cls):
        execute(check, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.create')
    def create(cls):
        execute(create, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.start')
    def start(cls):
        execute(start, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.stop')
    def stop(cls):
        execute(stop, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.cleanup')
    def cleanup(cls):
        executeCleanup(cleanup, cls.actors)

    @classmethod
    @log.high_class_static_function('lib.framework.Controller.onMachineIp')
    def onMachineIp(cls, machineName):
        for pm in cls.actors.ProtectedMachines:
            if pm.enabled and pm.name == machineName:
                pm.onMachineIp()


def _clearFunctionTabs():
    clear_tabs()


def execute(function, actors):
    _clearFunctionTabs()
    function(actors.Esxs, 'Esxs')
    function(actors.Hbr, 'Hbr')
    function(actors.ProtectedMachines, 'ProtectedMachines')
    function(actors.ReplicationGroups, 'ReplicationGroups')


def executeCleanup(function, actors):
    _clearFunctionTabs()
    function(actors.ProtectedMachines, 'ProtectedMachines')
    function(actors.ReplicationGroups, 'ReplicationGroups')
    function(actors.Esxs, 'Esxs')
    function(actors.Hbr, 'Hbr')


def configure(items, text):
    if not items:
        return
    message('Configuring', items, text)
    items.configure()


def check(items, text):
    if not items:
        return
    message('Checking', items, text)
    items.check()


def create(items, text):
    if not items:
        return
    message('Creating dependent objects for', items, text)
    items.create()


def start(items, text):
    if not items:
        return
    message('Starting', items, text)
    items.start()


def stop(items, text):
    if not items:
        return
    message('Stopping', items, text)
    items.stop()


def cleanup(items, text):
    if not items:
        return
    message('Unloading', items, text)
    if items and items.cleanup:
        items.cleanup()


def onMachineIp(items, text):
    if not items:
        return
    message('onMachineIp', items, text)
    items.onMachineIp()


def message(operation, items, text):
    if isinstance(items, list):
        logger.note('%s %s, count:%d' % (operation, text, len(items)))
    else:
        logger.note('%s %s' % (operation, text))
