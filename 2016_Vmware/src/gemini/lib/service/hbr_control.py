import sys
import os.path
import os
from lib.utility import log
from lib.utility import utility
from lib.framework.esx import Esx as fwEsx
from lib.third_party.humanize import pretty_print

ENABLE_VMTREE = True
if ENABLE_VMTREE:
    from   pyVmomi import Hbr
    from   pyVmomi import Vmodl
    from   pyVmomi import Vim
    import pyVim.host
    import pyVim.vm
    import pyHbr.hostd
    from pyHbr.pretty import PrettyPrint
    from   pyHbr.hbrsrv import Hbrsrv
    import pyHbr.hbrsrv
    from   pyHbr.hbrsrv import Hbrsrv
    from pyHbr.pretty import PrettyPrint
    from   pyHbr.hbrsrv import MakeImage

logger = log.default_logger


@log.class_logger
@pretty_print
class HbrControl(object):

    @staticmethod
    def build(hbr):
        return HbrControl(hbr)

    def __init__(self, hbr):
        try:
            self.script = '%s/apps/hbr/hbrsrv/scripts/hbrsrvctl.py' % \
                (os.environ['VMTREE'])
        except KeyError:
            self.script = '/apps/hbr/hbrsrv/scripts/hbrsrvctl.py'
            pass
        logger.info('script:%s' % self.script)
        self.connection = None
        self.replicationManager = None
        self.keyFile = hbr.authentication['key']
        self.certificateFile = hbr.authentication['certificate']
        self.ip = hbr.ip
        self.user = hbr.credentials.user
        self.password = hbr.credentials.password
        self.port = hbr.port
        self.commandPrefix = ("./py.sh -S %s --vmodl '%s' "
                              "--keyfile=%s --certfile=%s " \
                              % (self.script, self.ip,
                                 self.keyFile, self.certificateFile))

    def setConnection(self, connection):
        self.connection = connection
        self.replicationManager = self.connection.GetReplicationManager()

    @log.service_class_function
    def configure(self):
        pass

    @log.service_class_function
    def cleanup(self):
        pass

    @log.service_class_function
    def login(self):
        command = self.commandPrefix
        logger.note('command:%s' % command)
        try:
            output = os.system(command + ' status')
        except Exception, e:
            logger.error('Exception:%s' % str(e))

        logger.info('output:%s' % output)

    @log.service_class_function
    def stats(self, operationId):
        serverStats = self.replicationManager.GetServerStats()
        details = self.replicationManager.GetServerDetails()
        name = "server uuid: " + details.instanceUUID + " server name: " \
                 + details.instanceName

        pyHbr.pretty.PrintServerStats(name, serverStats)

    @log.service_class_function
    def details(self, operationId):
        details = self.replicationManager.GetServerDetails()
        results = []
        longest = 0

        for p in details._GetPropertyList():
            if p.name != "dynamicType" and p.name != "dynamicProperty":
                if len(p.name) > longest:
                    longest = len(p.name)
                results.append((p.name, getattr(details, p.name)))

        if not results:
            self.logger.error("ERROR: No attributes on", details)

        for name, val in results:
            self.logger.info("%*s: %s" % (longest, name, val))

    @log.service_class_function
    def removeHost(self, hostName):
        found = 0;

        storageMgr = self.connection.GetStorageManager()

        for host in storageMgr.GetEnabledHosts():
            if (host.GetId() == hostName):
                host.Remove()
                found = 1

        if not found:
            self.logger.debug("Host %s not found." % hostName)
        else:
            self.logger.debug("Removed host %s." % hostName)

    def CreateIdentSpec(self, id, datastoreUUID, pathname):
        identSpec = Hbr.Replica.IdentSpec()
        identSpec.id = id
        identSpec.datastoreUUID = datastoreUUID
        identSpec.pathname = pathname
        return identSpec

    @log.service_class_function
    def getImageInformation(self, group):
        image = group.GetActiveImage()
        self.logger.debug("Got image: %s", image)
        if image:
            pyHbr.pretty.PrintImage(group, image)
        return image

    @log.service_class_function
    def createImage(self, group, path, esx, isTestBubble=False):
        if isinstance(esx, fwEsx):
            esx = esx._delegate
        snapshotPIT = False
        disableNics = False
        keepOnlyReplicatedDisks = False
        useInstance = None

        task = pyHbr.hbrsrv.MakeImage(group, path, snapshotPIT, disableNics,
                                      keepOnlyReplicatedDisks, useInstance,
                                      isTestBubble)
        self.connection.WaitForTask(task)

        self.logger.debug("Made image %s", str(task.info.result))
        return task.info.result

    def commitToImage(self, group, consolidate=False):
        image = group.GetActiveImage()
        task = group.CommitToImage(image, consolidate)
        self.connection.WaitForTask(task)
        return task.info.result

    def removeImage(self, group):
        image = group.GetActiveImage()
        task = group.RevertImage(image)
        self.connection.WaitForTask(task)
        self.logger.debug("task result %s", str(task.info.result))
        return task.info.result

    @log.service_class_function
    def addEsxHost(self, host, operationId=None, extraWhatever=[]):
        """
        host = '$user:$password\@$host'
        """
        useThumb = True
        (host, user, password) = pyHbr.util.ParseHostUserPass(host)
        hostd = pyHbr.hostd.Hostd(host, user, password)
        self.connection.EnableHost(hostd, useThumb, extraWhatever)

        if useThumb:
            self.logger.debug("Added hbrsrv thumbprint %s to %s" \
                             % (self.connection.Thumbprint(),
                                hostd.Hostname()))
            self.logger.debug("Added Thumbprint HostInfo %s for %s" \
                              % (hostd.Thumbprint(), hostd.Hostname()))
        else:
            self.logger.debug("Added User/Pass HostInfo %s" % host)
