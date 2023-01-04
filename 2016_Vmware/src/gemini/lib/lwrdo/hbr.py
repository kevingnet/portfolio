from . import *
from resource import Resource
from helper.identity_generator import generateOpId
from lib.utility.dict_as_member import DottedNotation
from lib.service.hbr_control import HbrControl
from lib.service.ssh import Ssh
from lib.service.remote_log_reader import RemoteLogReader
from lib.utility.progress_bar import progress_bar_init_step
from lib.utility.progress_bar import progress_bar_stop
from lib.utility.progress_bar import run_in_background
import pyHbr
from pyVmomi import Vmodl


@log.class_logger
@pretty_print
class Hbr(Machine):

    @staticmethod
    def build(hbr):
        return Hbr(hbr)

    def __init__(self, hbr):
        self.initialize(hbr)
        self.credentials = DottedNotation(self.credentials)
        self.tools = DottedNotation(self.tools)
        self.filter = DottedNotation(self.filter)
        Machine.__init__(self, hbr, None, None)
        self._esx = None
        self._esxs = None
        self.credentials = DottedNotation(self.credentials)
        self.authentication = DottedNotation(self.authentication)
        self.authenticationString = \
            getAuthenticationString(self.credentials.user,
                                   self.credentials.password,
                                   self.ip)

    @log.low_class_function
    def configure(self):
        self._ssh = Ssh(self.ip,
                       self.credentials.user,
                       self.credentials.password,
                       self.logger.hbr,
                       self.logger.hbrLog)
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
                                                color=log.BLUE39)
        self._hvm = self._esx.host.FindVmByName(self.name)
        self.logger.info('_hvm:%s' % (self._hvm))
        self.manager = self._esx.host.GetHbrManager()
        self.logger.info('manager:%s' % (self.manager))
        self.internalSystem = self._esx.host.GetHbrInternalSystem()
        self.logger.info('internalSystem:%s' % (self.internalSystem))
        self.logger.debug('configured Hbr:%s' % self)

    def _connect(self):
        try:
            self.connection = pyHbr.hbrsrv.Hbrsrv\
                (self.ip,
                 self.credentials.user,
                 self.credentials.password,
                 vmodlPort=self.port,
                 keyFile=self.authentication.key,
                 certFile=self.authentication.certificate,
                 pushCert=True)
            self.logger.debug('connection:%s' % self.connection)
            self.replicationManager = self.connection.GetReplicationManager()
            self.logger.debug('replicationManager:%s'
                              % str(self.replicationManager))
            self.sessionManager = self.connection.GetSessionManager()
            self.logger.debug('sessionManager:%s' % str(self.sessionManager))
            self.storageManager = self.connection.GetStorageManager()
            self.logger.debug('storageManager:%s' % str(self.storageManager))
#             self.serverManager = self.connection.GetServerManager()
#             self.logger.debug('serverManager:%s' % str(self.serverManager))
        except Exception, e:
            self.logger.error('connection Exception:%s' % str(e))
        progress_bar_stop()

    @log.low_class_function
    def check(self):
        command = '/usr/bin/hbrsrv --print-default-db'
        self.database = run_ssh(self._ssh, command)
        self.database = self.database.replace('/etc/db/', '');
        self.logger.info('database: %s' % self.database)

        if self._vm is not None:
            self.logger.info('IsRunnint()=' + str(self._vm.IsRunning()))
            self.logger.info('ReportState()=' + str(self._vm.ReportState()))

    @log.low_class_function
    def create(self):
        self.createConnection()

    @log.low_class_function
    def start(self):
        self.getRemoteLogs()
        self.createConnection()

    @log.low_class_function
    def stop(self):
        self.stopServer()

    @log.low_class_function
    def cleanup(self):
        self._ssh.cleanup()

    @log.low_class_function
    def startServer(self):
        command = '/etc/init.d/hbrsrv start'
        run_ssh(self._ssh, command)
        self._remoteLogReader = RemoteLogReader(self._ssh,
                                                self.logsDirectory,
                                                logFiles=self.logs,
                                                color=log.BLUE39)
        self.getRemoteLogs()

    @log.low_class_function
    def stopServer(self):
        command = '/etc/init.d/hbrsrv stop'
        run_ssh(self._ssh, command)
        self.getRemoteLogs()

    @log.low_class_function
    def status(self):
        command = '/etc/init.d/hbrsrv status'
        run_ssh(self._ssh, command)

    @log.low_class_function
    def createImage(self, group, path, esx, isTestBubble=False):
        self.logger.debug('Creating image at: %s' % path)
        return self.control.createImage(group, path, esx,
                                        isTestBubble=isTestBubble)

    @log.low_class_function
    def commitToImage(self, group, consolidate=False):
        self.logger.debug('Committing to image')
        return self.control.commitToImage(group, consolidate=consolidate)

    @log.low_class_function
    def removeImage(self, group):
        self.logger.debug('Removing image')
        return self.control.removeImage(group)

    @log.low_class_function
    def getImageInformation(self, group):
        self.logger.debug('Getting fail over image information')
        return self.control.getImageInformation(group)

    @log.low_class_function
    def removeImage(self, groupId):
        self.logger.debug('Reverting and removing image on groupId: %s'
                          % str(groupId))
        group = None
        try:
            group = self.replicationManager.GetReplicationGroup(groupId)
        except Exception, e:
            self.logger.ignored('GetReplicationGroup Exception %s' % str(e))
        if group is None:
            self.logger.debug('No group image to remove')
            return
        image = group.GetActiveImage()
        if image:
            self.logger.debug('Removing image: %s' % str(image))
            try:
                task = group.RevertImage(image)
                self._esx.waitForTask(task)
            except Exception, e:
                self.logger.error('Exception %s' % str(e))
        else:
            self.logger.debug('No image to remove')

    @log.low_class_function
    def removeGroup(self, groupId):
        self.logger.debug('Removing groupId: %s' % str(groupId))
        group = None
        try:
            group = self.replicationManager.GetReplicationGroup(groupId)
        except Exception, e:
            self.logger.ignored('GetReplicationGroup Exception %s' % str(e))
        if group is None:
            self.logger.debug('No group to remove')
            return
        task = group.Remove()
        try:
            self._esx.waitForTask(task, label='Removing group')
        except Exception, e:
            self.logger.error('Exception %s' % str(e))

    @log.low_class_function
    def addGroup(self, groupId, vmDir, disks, addGroupArgs):
        specification = self.groupSpecificationFromArray(groupId,
                                                         vmDir,
                                                         disks,
                                                         addGroupArgs)
        self.logger.verbose('Made specification: %s' % str(specification))
        replicationGroup = self.replicationManager.\
            CreateReplicationGroup(specification)
        self.logger.verbose('replicationGroup: %s' % str(replicationGroup))

    @log.low_class_function
    def groupSpecificationFromArray(self, groupId, vmDir, disks, specWords):
        policyTiers = []
        rpo = 30  # default RPO is half an hour
        return pyHbr.hbrsrv.CreateGroupSpec(groupId,
                                            vmDir,
                                            disks,
                                            policyTiers,
                                            rpo)

    @log.low_class_function
    def updateDatabase(self, groupId, vmDir, disks):
        self.logger.debug('updateDatabase groupId: %s' % str(groupId))
        self.logger.debug('vmDir: %s' % str(vmDir))
        self.removeImage(groupId)
        self.removeGroup(groupId)
        arguments = "add '%s' '%s'" % (groupId, vmDir)
        return self.addGroup(groupId, vmDir, disks, arguments)

    def connect(self):
        self.logger.info('Creating server connection to: %s' % self.ip)
        progress_bar_init_step(length=2)
        run_in_background(self._connect())
        self.getRemoteLogs()

    def _restartHbr(self):
        self.logger.info('Restarting HBR Server...')
        command = '/etc/init.d/hbrsrv restart'
        run_ssh(self._ssh, command, estimatedTimeToComplete=2)
        self._remoteLogReader = RemoteLogReader(self._ssh,
                                                self.logsDirectory,
                                                logFiles=self.logs,
                                                color=log.BLUE39)
        self.getRemoteLogs()

    def createConnection(self):
        self.connect()
        self.control.setConnection(self.connection)

        self.logger.info('Setting thumbprint...')
        command = ('/usr/bin/hbrsrv-guestinfo.sh '
                   'set guestinfo.hbr.hms-thumbprint %s'
                   % self.authentication.thumbprint)
        run_ssh(self._ssh, command)

        self._restartHbr()
        self.connect()

        command = 'ps -ef | grep hbrsrv-bin | grep -v grep'
        psResult = run_ssh(self._ssh, command)
        if 'hbrsrv-bin' in psResult:
            self.logger.info('Found hbrsrv-bin process')
        else:
            self.logger.critical('hbrsrv-bin NOT FOUND')

        command = '/usr/bin/hbrsrv --print-lwd-ver'
        lwdVersion = run_ssh(self._ssh, command)
        if lwdVersion is not None:
            self.logger.info('LWD Protocol version: "%s"' % lwdVersion)
        else:
            self.logger.critical('LWD Protocol version NOT FOUND')

        command = 'ulimit -S -c $(( 1024 * 1024 ))'
        run_ssh(self._ssh, command)

        self.stop()
        backupFile = ('/tmp/%s.%d.%d.pre'
                      % (self.database, self._esx.index, self.index))
        command = 'cp /etc/db/%s %s' % (self.database, backupFile)
        run_ssh(self._ssh, command)

        self._ssh.get(backupFile, self.filesDirectory)
        self.cleanupDatabase()

    def cleanupDatabase(self):
        command = 'rm -f /etc/db/%s' % (self.database)
        run_ssh(self._ssh, command)

        self.startServer()
        self.connect()

        self.operationId = generateOpId()
        self.control.stats(self.operationId)

        resource = Resource()
        for esx in self._esxs:
            if esx.enabled:
                resource.HbrHosts.add(esx)
            else:
                self.logger.info('Host %s is not enabled' % esx.ip)
        self.control.details(self.operationId)

    def ssh(self, command, ignoreError=False):
        (rc, stdout, stderr) = self._ssh.run(command, ignoreError=ignoreError)
        stdout.rstrip()
        if rc == 0:
            if stdout:
                self.logger.remote_output('hbr: %s' % (stdout))
            return stdout
        else:
            if ignoreError:
                self.logger.ignored('hbr error for: %s' % (stderr))
            else:
                self.logger.error('hbr error for: %s' % (stderr))

    def getRemoteLogs(self):
        self._remoteLogReader.get()
        for esx in self._esxs:
            if esx.enabled:
                esx._remoteLogReader.get()
