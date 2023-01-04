from . import *
from lib.utility.dict_as_member import DottedNotation
from helper.identity_generator import generateOpId
from lib.service.ssh import Ssh
from lib.service.remote_log_reader import RemoteLogReader
from lib.utility.progress_bar import progress_bar_init
from lib.utility.progress_bar import progress_task
import pyVim
from pyVmomi import Vim
from pyHbr import task
from pyHbr import hostd
from pyVmomi import Vmodl
import pyHbr


@log.class_logger
@pretty_print
class Esxs(Actors):

    def __init__(self, items):
        self.initialize(items)
        for item in items:
            if item.enabled:
                self.append(Esx.build(item))
            else:
                self.append(NullObject.build(item))

    def remove(self, item):
        Actors.remove(self, item)


@log.class_logger
@pretty_print
class Esx(Actor):

    @staticmethod
    def build(esx):
        return Esx(esx)

    def __init__(self, esx):
        self.initialize(esx)
        self._hbr = None
        self.host = None
        self.HostDatastore = None
        self.HostDevice = None
        self.HostMachine = None
        self.HostReplication = None
        self.HostSnapshot = None
        self.HostTask = None
        self.HostDisk = None
        self.Host = None
        self.RemoteFile = None
        self.PsfFile = None
        self.fullPath = None
        self.serviceInstance = None
        self.credentials = DottedNotation(self.credentials)
        self.datastore = DottedNotation(self.datastore)
        self.tools = DottedNotation(self.tools)
        self.authenticationString = \
            getAuthenticationString(self.credentials.user,
                                   self.credentials.password,
                                   self.ip)

    def _setHbr(self, hbr):
        self._hbr = hbr

    @log.low_class_function
    def configure(self):
        self._ssh = Ssh(self.ip,
                       self.credentials.user,
                       self.credentials.password,
                       self.logger.esx,
                       self.logger.esxLog)
        self.logger.info('logs:%s' % (self.logs))
        self.filesDirectory = '%s/%s' % (self.filesDirectory, self.name)
        command = "/bin/mkdir -vp %s" % self.filesDirectory
        (code, stdout) = runCommand(command)
        self.logsDirectory = '%s/%s' % (self.logsDirectory, self.name)
        command = "/bin/mkdir -vp %s" % self.logsDirectory
        (code, stdout) = runCommand(command)
        self._remoteLogReader = RemoteLogReader(self._ssh, 
                                                self.logsDirectory,
                                                logFiles=self.logs,
                                                color=log.GREEN35)
        self.host = pyHbr.hostd.Hostd(self.ip,
                                self.credentials.user,
                                self.credentials.password)
        self.logger.info('host:%s' % (self.host))
        self.hostServiceInstance = self.host.GetServiceInstance()
        self.logger.info('hostServiceInstance:%s' % (self.hostServiceInstance))
        self.authorizationManager = self.host.GetAuthorizationManager()
        self.logger.info('authorizationManager:%s'
                         % (self.authorizationManager))
        self.logger.info('self.datastore.name:%s'
                         % (self.datastore.name))
        self.remoteStorage = self.host.RemoteStorage(self.datastore.name)
        self.logger.info('remoteStorage:%s' % (self.remoteStorage))
        self.serviceInstance = self.host.GetServiceInstance()
        self.logger.info('serviceInstance:%s' % (self.serviceInstance))
        content = self.serviceInstance.RetrieveContent()
        datacenters = content.GetRootFolder().GetChildEntity()
        self.datacenter = datacenters[0]
        self.logger.info('datacenter:%s' % (self.datacenter))
        self.HostDatastore.configure()
        self.HostDevice.configure()
        self.HostMachine.configure()
        self.HostReplication.configure()
        self.HostSnapshot.configure()
        self.HostTask.configure()
        self.HostDisk.configure()
        self.Host.configure()
        self.RemoteFile.configure()
        self.PsfFile.configure()
        self.logger.debug('configured Esx:%s' % self)

    @log.low_class_function
    def check(self):
        # TODO, check for this
        # datastore.accessible
        self.logger.debug('self.tools:%s' % (self.tools))
        command = "ls %s" % self.tools.disk_tool
        run_ssh(self._ssh, command)

        command = "ls %s" % self.tools.disk_create
        run_ssh(self._ssh, command)

        command = ('vsish -e get /system/modules/hbr_filter/general'
                   ' | grep version:')
        run_ssh(self._ssh, command)

        command = "esxcli storage filesystem list -i"
        run_ssh(self._ssh, command)

        command = "readlink -f '/vmfs/volumes/datastore1/'"
        run_ssh(self._ssh, command, ignoreError=True)

        command = "/sbin/vmkload_mod hbr_filter"
        run_ssh(self._ssh, command, ignoreError=True)

        command = "/sbin/vmkload_mod -l"
        filters = run_ssh(self._ssh, command)
        if 'hbr_filter' in filters:
            self.logger.info('Found hbr_filter')
        else:
            self.logger.critical('hbr_filter NOT FOUND')

        self.operationId = generateOpId(index=self.index)

        command = 'logger -t Hostd %s' % self.operationId
        run_ssh(self._ssh, command)

        command = 'logger -t vmkernel %s' % self.operationId
        run_ssh(self._ssh, command)

        self.HostMachine.list()
        try:
            self.logger.debug('remoteStorage.MakeDirectory: %s'
                              % self.testDirectory)
            makeDirResult = self.remoteStorage.\
                MakeDirectory(self.testDirectory)
            self.logger.debug('makeDirResult: %s' % makeDirResult)
        except Vim.Fault.FileAlreadyExists, e:
            self.logger.ignored('Folder already exists: %s\n%s'
                                % (self.testDirectory, str(e)))

        self.datastorePath = str(self.remoteStorage) + ' ' + self.testDirectory
        self.fullPath = pyVim.path.DsPathToFsPath(self.datastorePath)
        self.logger.info('datastorePath:%s' % (self.datastorePath))
        self.logger.info('fullPath:%s' % (self.fullPath))

    @log.low_class_function
    def create(self):
        pass

    def start(self):
        pass

    def stop(self):
        pass

    @log.low_class_function
    def cleanup(self):
        if self.fullPath:
            command = 'rm -fr %s' % self.fullPath
            run_ssh(self._ssh, command, silent=False, ignoreError=False)
        self._ssh.cleanup()

    def waitForTask(self, task, label='', ignoreError=False):
        progress_bar_init(label=label)
        taskResult = None
        try:
            taskResult = pyVim.task.WaitForTask(
                                                task,
                                                raiseOnError=True,
                                                si=self.serviceInstance,
                                                pc=None,
                                                onProgressUpdate=progress_task)
        except Exception, e:
            if ignoreError:
                self.logger.ignored('waitForTask Exception: %s ' % (str(e)))
            else:
                self.logger.critical('waitForTask Exception: %s ' % (str(e)))

        return taskResult

    def ssh(self, command, ignoreError=False):
        (rc, stdout, stderr) = self._ssh.run(command, ignoreError=ignoreError)
        stdout.rstrip()
        if rc == 0:
            if stdout:
                self.logger.remote_output('esx: %s' % (stdout))
            return stdout
        else:
            if ignoreError:
                self.logger.ignored('esx error for: %s' % (stderr))
            else:
                self.logger.error('esx error for: %s' % (stderr))

    def getSourcesPath(self):
        return self.datastore.machineSources

    def findMachineById(self, machineId):
        for vm in self.host.GetVmList():
            try:
                cfg = vm.GetConfig()
                print cfg
                if cfg and cfg.GetId() == machineId:
                    return vm
            except Vmodl.Fault.ManagedObjectNotFound:
                pass
        return None

