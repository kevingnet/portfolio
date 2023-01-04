from . import *
from disk import Disks as lDisks
from lib.service.ssh import Ssh
from lib.service.wait import wait
from lib.utility.current_class import CurrentClass
import time
from pyVmomi import Vim
import pyVim.vm
from pyVim import path
from lib.service.events import Events

# seconds
WAIT_FOR_IP_INITIAL = 60
WAIT_FOR_IP_TIMEOUT = 60


@log.class_logger
@pretty_print
class Machines(Actors):
    setattr(CurrentClass(), 'name', 'Machines')

    def __init__(self, items):
        self.initialize(items)
        for item in items:
            self.append(Machine.build(item, None))


@log.class_logger
@pretty_print
class Machine(Actor):
    setattr(CurrentClass(), 'name', 'Machine')
    MAX_DISKS_PER_CONTROLLER = 20

    @staticmethod
    def build(machine, path, disks):
        return Machine(machine, path, disks)

    def __init__(self, machine, path, disks):
        self.initialize(machine)
        self._esx = None
        self._hbr = None
        self._hvm = None
        self._vm = None
        self._datastore = None
        self.path = None
        self.fileName = None
        self.fullPath = None
        self.id = 0
        self.disks = lDisks(disks, path, self)
        self.guestInformation = {}

    def _setEsx(self, esx):
        self._esx = esx
        self.disks._setEsx(esx)

    def _setHbr(self, hbr):
        self._hbr = hbr
        self.disks._setHbr(hbr)

    def getPath(self):
        return self.fullPath

    def has2TbDisk(self):
        for disk in self.disks:
            if disk.enabled and not disk.isBaseDisk:
                if disk.is2Tb():
                    return True
        return False

    @log.low_class_function
    def configure(self):
        self._hvm = self._esx.host.FindVmByName(self.name)
        self.logger.info('_hvm:%s' % (self._hvm))

        if not self._hvm:
            # if machine is registered
            machineId = self._esx.HostMachine.getId(self.name)
            if not machineId:
                # register machine
                self.pathName = '[%s] %s/%s/%s.vmx' \
                    % (self._esx.datastore.name,
                       self._esx.getSourcesPath(),
                       self.name, self.name)
                self.fileName = path.DsPathToFsPath(self.pathName)
                self._esx.HostMachine.register(self)
                machineId = self._esx.HostMachine.getId(self.name)
            self.logger.info('machineId:%s' % (machineId))
            #self._hvm = self._esx.findMachineById(machineId)
            self._hvm = self._esx.host.FindVmByName(self.name)
        if not self._hvm:
            self.logger.critical('Machine: %s does not exist or is not '
                                 'configured in the host' % self.name)

        vmConfig = self._hvm.GetConfig()
        self.pathName = vmConfig.files.vmPathName
        self.fileName = path.DsPathToFsPath(self.pathName)
        self.filePath = os.path.dirname(self.fileName)
        self.logger.info('pathName: %s' % (self.pathName))
        self.logger.info('fileName: %s' % (self.fileName))
        self.logger.info('filePath: %s' % (self.filePath))
        self._resolveId()
        self.logger.debug('Machine id: %s' % str(self.id))
        self.logger.info('id: %s' % (self.id))
        self._printDiskInformation()
        self._usedDiskUnitNumbers = []
        for disk in self.disks:
            if hasattr(disk, 'isBaseDisk') and disk.isBaseDisk:
                baseDiskFileName = '%s/%s.vmdk' % (self.filePath, disk.name)
                self.logger.info('baseDiskFileName: %s' % (baseDiskFileName))
                disk.path = self.filePath
                disk.fileName = baseDiskFileName
        self._attachBaseDisk()
        self._getUsedUnitNumbers()
        self._getPrimaryDiskControllerId()
        propC = self._esx.host.GetPropertyCollector()
        resPool = self._hvm.resourcePool
        self._vm = pyVim.vm.VM(self._hvm, propC, resPool, vmConfig)
        self.logger.info('_vm:%s' % (self._vm))
        self.disks.configure()

        self.guestInformation['id'] = None
        self.guestInformation['name'] = None
        self.guestInformation['family'] = None
        self.guestInformation['host'] = None
        self.guestInformation['ip'] = None
        self.guestInformation['state'] = None
        self.guestInformation['ready'] = None
        self._ssh = None

        if self.__class__.__name__ == 'Machine':
            self.logger.debug('configured Machine:%s' % self)

    @log.low_class_function
    def check(self):
        if 'initialPowerState' in self.__dict__:
            self.logger.info("initialPowerState:" + self.initialPowerState)
            if self.initialPowerState is 'PoweredOff':
                if self.isPoweredOn():
                    self._vm.PowerOff()
        #command = 'rm -f %s/*.psf' % self.filePath
        #run_ssh(self._ssh, command, ignoreError=True)
        configSpec = self._vm.GetConfig()
        self.logger.debug('IsRunnint()=' + str(self._vm.IsRunning()))
        self.logger.debug('ReportState()=' + str(self._vm.ReportState()))
        self.disks.check()

    @log.low_class_function
    def create(self):
        self.disks.create()

    @log.low_class_function
    def start(self):
        if not self._vm:
            return
        if 'initialPowerState' in self.__dict__:
            self.logger.info("initialPowerState:" + self.initialPowerState)
            if self.initialPowerState in \
                    ['PoweredOn', 'PowerOn', 'On']:
                if self.isPoweredOff():
                    self.logger.info("Powering on machine...")
                    self._vm.PowerOn()
                else:
                    self.logger.debug("Machine already powered on")
            else:
                if self._vm.IsRunning():
                    self.logger.info("Powering off machine...")
                    self._vm.PowerOff()
        self.disks.start()

    def stop(self):
        self.disks.stop()

    @log.low_class_function
    def cleanup(self):
        self.disks.cleanup()
        self._attachBaseDisk()

    @log.low_class_function
    def getNextUnitNumber(self):
        for unitNumber in range(Machine.MAX_DISKS_PER_CONTROLLER):
            if unitNumber not in self._usedDiskUnitNumbers:
                self._usedDiskUnitNumbers.append(unitNumber)
                self.logger.debug("_unitNumber %d" % unitNumber)
                return unitNumber
        self.logger.critical('Ran out of _unitNumber values. max is: %d'
                             % Machine.MAX_DISKS_PER_CONTROLLER)

    @log.low_class_function
    def _attachBaseDisk(self):
        baseDiskFileName = None
        for disk in self.disks:
            if hasattr(disk, 'isBaseDisk') and disk.isBaseDisk:
                baseDiskFileName = disk.path
        if baseDiskFileName:
            parameters = '%d %s %d %d' % (self.id, baseDiskFileName, 0, 0)
            self._esx.HostDevice.add(parameters, ignoreError=True)

    @log.low_class_function
    def _printDiskInformation(self):
        allVmDevices = self._hvm.GetConfig().GetHardware().GetDevice()
        for device in allVmDevices:
            if isinstance(device, Vim.Vm.Device.VirtualDisk):
                self.logger.info('key %s' % str(device.key))
                self.logger.info('_unitNumber %s' % str(device.unitNumber))
                self.logger.info('fileName %s' % str(device.backing.fileName))
                self.logger.info('---------------------------------------')

    @log.low_class_function
    def _getUsedUnitNumbers(self):
        if len(self._usedDiskUnitNumbers) > 0:
            return
        allVmDevices = self._hvm.GetConfig().GetHardware().GetDevice()
        for device in allVmDevices:
            if isinstance(device, Vim.Vm.Device.VirtualDisk):
                unitNumber = int(device.unitNumber)
                self._usedDiskUnitNumbers.append(unitNumber)
        self.logger.vv('_usedDiskUnitNumbers %s'
                       % str(self._usedDiskUnitNumbers))

    @log.low_class_function
    def _getPrimaryDiskControllerId(self):
        self._baseDiskKey = None
        self._baseDiskControllerKey = None
        self._baseDiskControllerBusNumber = -1
        allVmDevices = self._hvm.GetConfig().GetHardware().GetDevice()
        for device in allVmDevices:
            if isinstance(device, Vim.Vm.Device.VirtualDisk):
                self.logger.info('disk device: %s' % device)
                unitNumber = int(device.unitNumber)
                if unitNumber == 0:
                    self._baseDiskKey = device.key
                    self._baseDiskControllerKey = device.controllerKey
        if not self._baseDiskControllerKey:
            self.logger.critical('Base controller not found!')
        else:
            self.logger.info('Base controller key: %s'
                             % self._baseDiskControllerKey)
        for device in allVmDevices:
            if isinstance(device, Vim.Vm.Device.VirtualLsiLogicController):
                if device.key == self._baseDiskControllerKey:
                    self.logger.info('lsi device: %s' % device)
                    self._baseDiskControllerBusNumber = int(device.busNumber)
        if self._baseDiskControllerBusNumber == -1:
            # set to default
            self._baseDiskControllerBusNumber = 0
            self.logger.warning('Setting Base controller bus '
                                'number: %d to default'
                                % self._baseDiskControllerBusNumber)
        else:
            self.logger.info('Base controller bus number: %d'
                             % self._baseDiskControllerBusNumber)

    @log.low_class_function
    def _register(self):
        self.logger.info('Registering machine:%s' % self.name)
        self._esx.HostMachine.register(self)

    @log.low_class_function
    def _getId(self):
        self.id = int(self._esx.HostMachine.getId(self))

    @log.low_class_function
    def _resolveId(self):
        self._getId()
        if not self.id:
            self._register()
        self._getId()
        if not self.id:
            self.logger.critical('Could not get ID for machine: %s'
                                 % self.name)

    def isRunning(self):
        if not self._vm:
            return
        self.logger.debug("isRunning: %s %s"
                          % (self.name, str(self._vm.IsRunning())))
        return self._vm.IsRunning()

    def isPoweredOff(self):
        if not self._vm:
            return
        self.logger.debug("IsPoweredOff: %s %s"
                          % (self.name, str(self._vm.IsPoweredOff())))
        return self._vm.IsPoweredOff()

    def powerOff(self):
        if not self._vm:
            return
        result = None
        if self._vm.IsRunning():
            self.logger.debug("powerOff: %s" % self.name)
            if self._ssh:
                self._ssh.disconnect()
            result = self._vm.PowerOff()
        else:
            self.logger.debug("Machine was already powered off: %s"
                              % self.name)
        self._getGuestInformation()
        return result

    def powerOn(self, waitForIp=False, timeout=WAIT_FOR_IP_TIMEOUT):
        if not self._vm:
            return
        result = None
        if self._vm.IsPoweredOff():
            self.logger.debug("PowerOn: %s" % self.name)
            result = self._vm.PowerOn()
            if waitForIp:
                self._waitForIp(timeout=timeout)
        else:
            self.logger.debug("Machine was already powered on: %s"
                              % self.name)
        self._getGuestInformation()
        return result

    def _waitForIp(self, timeout=WAIT_FOR_IP_TIMEOUT):
        self.logger.note('Waiting for an ip for machine: %s '
                         'timeout:%d initial wait:%d'
                         % (self.name, timeout, WAIT_FOR_IP_INITIAL))
        interval = 2
        gotIp = False
        timeleft = timeout
        wait(WAIT_FOR_IP_INITIAL, label='booting up machine...')
        while(timeleft > 0):
            if self._gotIp():
                gotIp = True
                break
            timeleft -= interval
            time.sleep(interval)
        if not gotIp:
            self.logger.error("Could not obtain Ip Address: %s"
                              % self.name)

    def _gotIp(self):
        info = self._esx.HostMachine.getGuestInformation(self, silent=True)
        if info['ipAddress']:
            return True
        return False

    def _getGuestInformation(self):
        info = self._esx.HostMachine.getGuestInformation(self)
        self.guestInformation['id'] = info['guestId']
        self.guestInformation['name'] = info['guestFamily']
        self.guestInformation['family'] = info['guestFullName']
        self.guestInformation['host'] = info['hostName']
        self.guestInformation['ip'] = info['ipAddress']
        self.guestInformation['state'] = info['guestState']
        self.guestInformation['ready'] = info['guestOperationsReady']
        self.logger.debug("guestInformation: %s" % self.guestInformation)
        self._configureSsh()

    def _configureSsh(self):
        if self.guestInformation['ip'] == None:
            if self._ssh:
                self._ssh.disconnect()
            self._ssh = None
            self.logger.info("Machine _ssh: None")
        else:
            self._ssh = Ssh(self.guestInformation['ip'],
                       self._esx.credentials.user,
                       self._esx.credentials.password,
                       self.logger.machine,
                       self.logger.machine)
            self.logger.info("Machine _ssh: %s" % str(self._ssh))
            Events().ee.emit("onMachineIp", self.name)

