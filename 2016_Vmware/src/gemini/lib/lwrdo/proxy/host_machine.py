from . import *
from lib.service.vim_cmd import VimCmd
from lib.utility import log
from lib.framework.machine import Machine as fwMachine
from lib.framework.protected_machine import ProtectedMachine as fwProtectedMachine

@log.class_logger
@pretty_print
class HostMachine(Actor):

    @staticmethod
    def build(esx):
        return HostMachine(esx)

    def __init__(self, esx):
        self.initialize(esx.HostMachine)
        self._esx = esx._delegate
        self._ssh = None

    def configure(self):
        self._ssh = self._esx._ssh

    def check(self):
        pass

    def create(self):
        pass

    def start(self):
        pass

    def stop(self):
        pass

    def cleanup(self):
        pass

    def _getId(self, parameter):
        vimId = None
        if isinstance(parameter, Machine):
            vimId = parameter.id
        elif isinstance(parameter, fwMachine):
            vimId = parameter._delegate.id
        elif isinstance(parameter, fwProtectedMachine):
            vimId = parameter._delegate.id
        elif isinstance(parameter, ProtectedMachine):
            vimId = parameter.id
        elif isinstance(parameter, Replica):
            vimId = parameter.machine.id
        elif isinstance(parameter, Disk):
            vimId = parameter.machine.id
        elif isinstance(parameter, ReplicaDisk):
            vimId = parameter.machine.id
        elif isinstance(parameter, str):
            vimId = parameter
        elif isinstance(parameter, int):
            vimId = str(parameter)
        if not vimId:
            self.logger.critical('Could not obtain id from: %s'
                                 % str(parameter))
        return vimId

    @log.proxy_class_function
    def list(self):
        return VimCmd.machineList(self._ssh)

    @log.proxy_class_function
    def getId(self, machine):
        pathName = None
        if isinstance(machine, str):
            pathName = machine
        else:
            pathName = machine.pathName
        path = pathName.split(' ')
        if len(path) == 1:
            path = path[0]
        else:
            path = path[1]
        path = "'%s'" % path
        self.logger.debug('get id for: %s' % path)
        return VimCmd.machineGetId(self._ssh, path)

    @log.proxy_class_function
    def getIds(self, namePattern):
        data = VimCmd.machineGetId(self._ssh, namePattern)
        data = str(data)
        data = data.splitlines()
        idList = []
        for d in data:
            id = 0
            try:
                id = int(d)
            except:
                pass
            if id != 0:
                idList.append(id)
        return idList

    @log.proxy_class_function
    def getGuestInformation(self, machine, silent=False):
        vimId = self._getId(machine)
        return VimCmd.machineGetGuestInformation(self._ssh, vimId,
                                                 silent=silent)

    @log.proxy_class_function
    def getIp(self, machine):
        data = self.getGuestInformation(machine)
        return data['ipAddress']

    @log.proxy_class_function
    def register(self, machine, ignoreError=False):
        return VimCmd.machineRegister(self._ssh, machine.fileName,
                                      ignoreError=ignoreError)

    @log.proxy_class_function
    def registerVmx(self, vmxFile, ignoreError=False):
        return VimCmd.machineRegister(self._ssh, vmxFile,
                                      ignoreError=ignoreError)

    @log.proxy_class_function
    def unregister(self, machine, ignoreError=True):
        vimId = None
        if isinstance(machine, str):
            vimId = machine
        else:
            vimId = self._getId(machine)
        return VimCmd.machineUnregister(self._ssh, vimId,
                                        ignoreError=ignoreError)

    @log.proxy_class_function
    def powerOn(self, machine, ignoreError=False):
        vimId = self._getId(machine)
        return VimCmd.machinePowerOn(self._ssh, vimId,
                                     ignoreError=ignoreError)

    @log.proxy_class_function
    def powerOff(self, machine, ignoreError=True):
        vimId = self._getId(machine)
        return VimCmd.machinePowerOff(self._ssh, vimId,
                                      ignoreError=ignoreError)

    @log.proxy_class_function
    def powerOnById(self, machineId, ignoreError=True):
        return VimCmd.machinePowerOn(self._ssh, machineId,
                                     ignoreError=ignoreError)

    @log.proxy_class_function
    def powerOffById(self, machineId, ignoreError=True):
        return VimCmd.machinePowerOff(self._ssh, machineId,
                                      ignoreError=ignoreError)

"""
machineGetHostContrains
machineList
machineCreateDummy
machineDestroy
machineUpgrade
machineReload
machineAnswerMessage
machineSetScreenResolution
machineAcquireMksTicket
machineConvertToTemplate
machineConvertToMachine
machinePowerGetState
machinePowerHibernate
machinePowerReboot
machinePowerReset
machinePowerShutdown
machinePowerSuspend
machinePowerResume
machineGetSnapshotsInformation
machineListTasks
machineListDatastores
machineListNetworks
machineGetSummary
machineGetRuntimeInformation
machineGetConfiguration
machineGetCapability
machineGetSpaceNeededForConsolidation
machineGetCpuIdMask
machineGetConfigurationOption
machineListDisabledMethods
machineGetEnvironment
machineGetFileLayout
machineGetFileLayoutEx
machineGetHeartBeatStatus
machineGetManagedIdentityStatus
machineQueryCompatibility
toolsCancelInstall
toolsInstall
toolsUpgrade
"""