#!/usr/bin/python
import unittest
from lib.utility import log
from lib.base_test import BaseTest


@log.class_logger
class TestHostRemove(BaseTest):

    def __init__(self, *args, **kwargs):
        BaseTest.__init__(self, __file__, *args, **kwargs)

    @log.test_class_function
    def test_HostRemove(self):
        self.logger.test('Hbr: %s' % self.Hbr)
        self.logger.test('esxs: %s' % str(self.Esxs))

        esxToBeRemoved = self.Esxs[0]
        self.logger.test('esxToBeRemoved: %s' % str(esxToBeRemoved))
        self.assertTrue(self.Hosts.exist(esxToBeRemoved),
                        'Host %s does not exist' % esxToBeRemoved.ip)

        hostCountBefore = self.Hosts.count()
        self.logger.test('hostCountBefore: %d' % hostCountBefore)

        self.Hosts.remove(esxToBeRemoved)

        hostCountAfter = self.Hosts.count()
        self.logger.test('hostCountAfter: %d' % hostCountAfter)
        self.logger.test('esxs: %s' % str(self.Esxs))

        self.assertEqual(hostCountBefore - 1, hostCountAfter,
                         'Total number of hosts should have decreased '
                         'by one, number of hosts changed from %d to %d'
                         % (hostCountBefore, hostCountAfter))

        self.assertFalse(self.Hosts.exist(esxToBeRemoved),
                         'After removing, host %s, it should not exist'
                         % esxToBeRemoved.ip)

if __name__ == '__main__':
    unittest.main()

