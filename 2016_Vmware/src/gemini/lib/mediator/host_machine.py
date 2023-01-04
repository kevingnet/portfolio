from lib.utility import log
from lib.lwrdo.helper.run_ssh import run_ssh
from lib.lwrdo.helper.object_identity import getMachine
from . import *

ASCII_A_ORDINAL = ord("a")
logger = log.default_logger


@log.class_logger
@pretty_print
class HostMachines(Actors):

    def __init__(self, items, actors, config):
        self.initialize(config.HostMachines_defaults)
        for idx, item in enumerate(items):
            item = item['HostMachine']
            item = self.setChildAttributes(item, idx, HostMachine)
            if item.enabled:
                self.append(HostMachine.build(item, actors, config))
            else:
                self.append(NullObject.build(item))


@log.class_logger
@pretty_print
class HostMachine(Actor):

    @log.high_class_function
    def __init__(self, image, actors, config):
        self._esx = None
        self._hbr = None
        self._init('HostMachine', image, actors, config)

    @staticmethod
    def unregisterByMatchingNames(esx, nameSpecification):
        logger.test('Unregistering all *%s* machines' % nameSpecification)
        vmIds = esx.HostMachine.getIds(nameSpecification)
        logger.test('vmIds:%s' % vmIds)
        for vmId in vmIds:
            esx.HostMachine.powerOff(vmId)
            esx.HostMachine.unregister(vmId)

    @staticmethod
    def deleteDiskDevices(machine):
        machine = getMachine(machine)
        logger.test('Reloading disk devices in %s' % machine.name)
        machine.powerOn(waitForIp=True)
        for disk in machine.disks:
            if disk.enabled and not disk.isBaseDisk:
                command = 'rm -f %s' % _getDeviceName(disk.index)
                logger.info('command %s' % command)
                run_ssh(machine._ssh, command, ignoreError=True, silent=False)
        machine.powerOff()
        machine.powerOn(waitForIp=True)


def _getDeviceName(index):
    return '/dev/sd%s' % chr(ASCII_A_ORDINAL + index)
