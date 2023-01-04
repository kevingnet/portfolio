from . import *
from lib.lwrdo.helper.identity_generator import generateOpId
from lib.lwrdo.esx import Esx
from lib.framework.esx import Esx as fwEsx
from lib.utility import log
from pyVmomi import Vmodl


@log.class_logger
@pretty_print
class HbrHosts(Actor):

    @staticmethod
    def build(hosts, hbr):
        return HbrHosts(hosts, hbr)

    @log.proxy_class_function
    def __init__(self, hosts, hbr):
        self.initialize(hosts)
        self._hbr = hbr

    def configure(self):
        pass

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

    @log.proxy_class_function
    def add(self, esx):
        if not isinstance(esx, Esx) and not isinstance(esx, fwEsx):
            self.logger.error('Call function with an Esx instance')
        if isinstance(esx, fwEsx):
            esx = esx._delegate
        if self.exist(esx.ip):
            self.logger.info('Host already exist: %s removing' % esx.ip)
            self._hbr.control.removeHost(esx.ip)
        operationId = generateOpId()
        self._hbr.control.addEsxHost(esx.authenticationString, operationId)

    @log.proxy_class_function
    def exist(self, host):
        if isinstance(host, Esx) or isinstance(host, fwEsx):
            host = host.ip
        if self.find(host) is not None:
            self.logger.info('found host: %s' % host)
            return True
        self.logger.info('host NOT found: %s' % host)
        return False

    @log.proxy_class_function
    def find(self, host):
        if isinstance(host, Esx) or isinstance(host, fwEsx):
            host = host.ip
        self.logger.info('Searching for host: %s' % host)
        hostToFind = None
        for h in self._hbr.storageManager.GetEnabledHosts():
            self.logger.verbose('found host: %s' % h.id)
            if h.id == host:
                hostToFind = h
                break
        return hostToFind

    @log.proxy_class_function
    def get(self, host):
        if isinstance(host, Esx) or isinstance(host, fwEsx):
            host = host.ip
        self.logger.info('Getting host: %s' % host)
        for h in self._hbr.storageManager.GetEnabledHosts():
            self.logger.info('found host: %s' % h.id)
            if h.id == host:
                return h
        return None

    @log.proxy_class_function
    def remove(self, host):
        if isinstance(host, Esx) or isinstance(host, fwEsx):
            host = host.ip
        self.logger.info('Removing host: %s' % host)
        for idx, esx in enumerate(self._hbr._esxs):
            if host == esx.ip:
                hostToRemove = self.get(esx.ip)
                if hostToRemove:
                    self._hbr.control.removeHost(esx.ip)
                    try:
                        hostToRemove.Remove()
                    except Vmodl.fault.ManagedObjectNotFound:
                        pass
                    esx.cleanup()
                    esx.enabled = False
                    #self._hbr._esxs[idx] = fwNullObject.build(esx)
                    del esx
                else:
                    self.logger.critical('Could not find host: %s' % host)

    @log.proxy_class_function
    def count(self):
        self.logger.info('Counting hosts')
        count = 0
        for h in self._hbr.storageManager.GetEnabledHosts():
            count += 1
        self.logger.info('count: %d' % count)
        return count

