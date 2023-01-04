from lib.utility import log
from lib.lwrdo.helper.run_ssh import run_ssh
from lib.lwrdo.helper.object_identity import getMachine
from lib.lwrdo.helper.object_identity import getDisk
from lib.service.checksum_disk import checksum_disk
from lib.service.checksum_disk import checksum_disk_page
from lib.service.checksum_disk import checksum_disk_pages
from lib.third_party.humanize import pretty_print
import random
from random import shuffle
from lib.service.wait import wait

logger = log.default_logger
EQUALITY = 0
INEQUALITY = 1
TERABYTE = 1099511627776
TWO_TERABYTES = TERABYTE * 2
ASCII_A_ORDINAL = ord("a")


@pretty_print
class DiskData(dict):
    def __getitem__(self, i):
        item = None
        try:
            item = self.__dict__[i]
        except:
            pass
        if not item:
            logger.warning('DiskData __getitem__ item: %s Not Found'
                           % str(item))

    @staticmethod
    def build(disk, pageSize, writeCount,
              checksumDisk, checksumPages, skips=None):
        return DiskData(disk, pageSize, writeCount,
                        checksumDisk, checksumPages, skips)

    def __init__(self, disk, pageSize, writeCount,
                 checksumDisk, checksumPages, skips=None):
        self.disk = disk
        self.deviceName = _getDeviceName(disk.index)
        self.pageSize = pageSize
        self.writeCount = writeCount
        self.skips = skips
        self.checksumDisk = checksumDisk
        self.checksumPages = checksumPages
        if self.disk.size >= TWO_TERABYTES:
            self.pageSize = 16384
            self.checksumDisk = False
            self.checksumPages = False
            logger.info('Large disk, size=%d Changing page size to %d'
                        ' and skipping checksums'
                        % (self.disk.size, self.pageSize))
        logger.debug('DiskData disk:%s device:%s pageSize %d'
                     % (self.disk.name, self.deviceName, self.pageSize))

    def setWritePageData(self, name, pages):
        logger.debug('setWritePageData %s for disk %s pages %s'
                     % (name, self.disk.name, str(pages)))
        setattr(self, name, pages)
        if not self.skips:
            self.skips = []
        for page in pages:
            self.skips.append(page)

    def getWritePageData(self, name):
        logger.debug('getWritePageData %s for disk %s'
                     % (name, self.disk.name))
        data = None
        if hasattr(self, name):
            data = getattr(self, name)
            logger.debug('Page data %s' % str(data))
        return data

    def setWriteDiskChecksum(self, name, checksum):
        logger.debug('setWriteDiskChecksum %s for disk %s checksum %s'
                     % (name, self.disk.name, str(checksum)))
        setattr(self, '%s.disk' % name, checksum)

    def getWriteDiskChecksum(self, name):
        logger.debug('getWriteDiskChecksum %s for disk %s'
                     % (name, self.disk.name))
        data = None
        if hasattr(self, '%s.disk' % name):
            data = getattr(self, '%s.disk' % name)
            logger.debug('Checksum %s' % data)
        return data


# why are checksums optional if the verification fails in not used?
@log.class_logger
class TestDataWriter():
    random.seed()

    @staticmethod
    def build(machine,
              pageSizes,
              writeCounts,
              checksumDisk=True,
              checksumPages=True,
              skipPagesLists=None):
        return TestDataWriter(machine,
                              pageSizes,
                              writeCounts,
                              checksumDisk,
                              checksumPages,
                              skipPagesLists)

    @log.service_class_function
    def __init__(self,
                 machine,
                 pageSizes,
                 writeCounts,
                 checksumDisk=True,
                 checksumPages=True,
                 skipPagesLists=None):
        machine = getMachine(machine)
        self.machine = machine
        self.checksumDisk = checksumDisk
        self.checksumPages = checksumPages
        pageSizes = _parameterToListMatchingDiskCount(machine, pageSizes)
        writeCounts = _parameterToListMatchingDiskCount(machine, writeCounts)

        self.writeCounts = writeCounts
        self.disksData = []
        for idx, disk in enumerate(self.machine.disks):
            if not disk.enabled or disk.isBaseDisk:
                continue
            disk = getDisk(disk)
            skips = None
            writeCount = None
            if skipPagesLists:
                skips = skipPagesLists[idx]
            if writeCounts:
                writeCount = writeCounts[idx]
            dd = DiskData.build(disk,
                                pageSizes[idx],
                                writeCount,
                                checksumDisk,
                                checksumPages,
                                skips)
            self.disksData.append(dd)

    @log.service_class_function
    def cleanup(self):
        for dd in self.disksData:
            command = 'rm -f %s' % dd.deviceName
            logger.info('command:%s' % command)
            run_ssh(dd.disk.machine._ssh, command,
                    ignoreError=True, silent=False)

    @log.service_class_function
    def write(self, writeName):
        for dd in self.disksData:
            pages = TestDataWriter.writeForDisk(dd.disk,
                                                writeCount=dd.writeCount,
                                                deviceName=dd.deviceName,
                                                pageSize=dd.pageSize,
                                                skipPagesList=dd.skips)
            dd.setWritePageData(writeName, pages)
            dd.setWriteDiskChecksum(writeName, None)
        if self._shouldChecksum():
            wasRunning = self.machine.isRunning()
            if wasRunning:
                logger.info('Powering off: %s' % self.machine.name)
                self.machine.powerOff()
            self.checksum(writeName)
            if wasRunning:
                logger.info('Powering on: %s' % self.machine.name)
                self.machine.powerOn(waitForIp=True)

    def _getDiskFromIndex(self, disks, index):
        for disk in disks:
            if disk.index == index:
                return getDisk(disk)
        return None

    @log.service_class_function
    def verify(self, disks, writeName, comparison=EQUALITY):
        for idx, dd in enumerate(self.disksData):
            disk = self._getDiskFromIndex(disks, dd.disk.index)
            if dd.checksumDisk:
                diskChecksum = dd.getWriteDiskChecksum(writeName)
                TestDataWriter.verifyDisk(disk,
                                          diskChecksum,
                                          comparison=comparison)
            if dd.checksumPages:
                pages = dd.getWritePageData(writeName)
                TestDataWriter.verifyPages(disk,
                                           pages,
                                           dd.pageSize,
                                           comparison=comparison)

    @staticmethod
    def writeForDisk(disk,
                     writeCount=3,
                     deviceName=None,
                     pageSize=8192,
                     skipPagesList=None):
        machine = disk.machine
        # calculate device name, if, based on index, could be based on bus
        if not deviceName:
            deviceName = _getDeviceName(disk.index)
        logger.info('deviceName:%s' % deviceName)
        _connectDeviceIf(disk, deviceName)

        # get random pages, skip some if lists passed in
        pages = _generatePageList(disk,
                                  writeCount=writeCount,
                                  pageSize=pageSize,
                                  skipPagesList=skipPagesList)
        # write data to pages
        for page, ignore in pages:
            command = 'dd if=/dev/urandom count=1 of=%s bs=%d seek=%d' \
                    % (deviceName, pageSize, page)
            logger.info('command:%s' % command)
            #wait(500)
            run_ssh(machine._ssh, command, ignoreError=False, silent=False)
        if len(pages) < writeCount:
            logger.error('Invalid number of writes. Pages:%d was less than'
                         ' writeCount:%d' % (len(pages), writeCount))
        return pages

    @log.service_class_function
    def checksum(self, writeName):
        for dd in self.disksData:
            diskChecksum = None
            if dd.checksumDisk:
                diskChecksum = checksum_disk(dd.disk)
            dd.setWriteDiskChecksum(writeName, diskChecksum)
            pages = dd.getWritePageData(writeName)
            if dd.checksumPages:
                pages = checksum_disk_pages(dd.disk, pages, dd.pageSize)
            dd.setWritePageData(writeName, pages)

    def _shouldChecksum(self):
        for dd in self.disksData:
            if dd.checksumDisk or dd.checksumPages:
                return True
        return False

    @staticmethod
    def verifyDisk(disk,
                   diskChecksum,
                   comparison=EQUALITY):
        comparison = _fixEqualityParameter(comparison)
        machine = disk.machine
        latestDiskChecksum = machine._esx.HostDisk.checksum(disk)
        logger.note('Verifying disk test data for disk %s' % disk.name)
        logger.info('diskChecksum:%s' % diskChecksum)
        logger.info('latestDiskChecksum:%s' % latestDiskChecksum)
        if comparison == EQUALITY:
            if diskChecksum != latestDiskChecksum:
                logger.warning('Disk checksums mismatch diskChecksum:%s '
                               '!= latestDiskChecksum:%s' %
                               (diskChecksum, latestDiskChecksum))
            else:
                logger.info('Disk checksums matched disk:%s '
                            '== mirror:%s' %
                            (diskChecksum, latestDiskChecksum))
        else:
            if diskChecksum == latestDiskChecksum:
                logger.warning('Disk checksums matched diskChecksum:%s '
                               '== latestDiskChecksum:%s (not expected)' %
                               (diskChecksum, latestDiskChecksum))
            else:
                logger.info('Disk checksums did not match (was expected) '
                            'disk:%s != checksums:%s' %
                            (diskChecksum, latestDiskChecksum))

    @staticmethod
    def verifyPages(disk,
                    pages,
                    pageSize,
                    comparison=EQUALITY):
        comparison = _fixEqualityParameter(comparison)
        logger.note('Verifying disk page data for disk %s' % disk.name)
        failures = 0
        for page, checksum in pages:
            page = int(page)
            pageChecksum = checksum_disk_page(disk, page, pageSize)
            logger.debug('page:%d checksum:%s mirrorChecksum:%s'
                         % (page, checksum, pageChecksum))
            if comparison == EQUALITY:
                if checksum != pageChecksum:
                    failures += 1
                    logger.warning('Page checksums mismatch checksum:%s '
                                   '!= pageChecksum:%s' %
                                   (checksum, pageChecksum))
                else:
                    logger.info('Page checksums matched protected:%s '
                                '== mirror:%s' %
                                (checksum, pageChecksum))
            else:
                if checksum == pageChecksum:
                    failures += 1
                    logger.warning('Page checksums matched checksum:%s '
                                   '== pageChecksum:%s (not expected)' %
                                   (checksum, pageChecksum))
                else:
                    logger.info('Page checksums did not match protected:%s '
                                '== mirror:%s (was expected)' %
                                (checksum, pageChecksum))
            if failures:
                logger.warning('failures:%d' % (failures))


def _generatePageList(disk,
                      writeCount=3,
                      pageSize=8192,
                      skipPagesList=None):
    logger.info('writeCount:%d disk.size:%d pageSize:%d'
                % (writeCount, disk.size, pageSize))
    # Verify that pageCount is > numWrites
    pageCount = int(disk.size / pageSize) - 1
    if pageCount <= writeCount:
        logger.error('Invalid parameter. pageCount:%r was <= than'
                     ' writeCount:%r' % (pageCount, writeCount))
    # get random pages, skip some if lists passed in
    pages = _generatePages(writeCount,
                           pageSize,
                           disk.size,
                           skipPagesList)
    logger.info('pages:%s' % str(pages))
    return pages


def _connectDeviceIf(disk, deviceName):
    """
    changing a disk size will screw up things in the machine, do...
    echo 1 > /sys/block/sdc/device/delete
    echo "scsi add-single-device 0 0 3 0" > /proc/scsi/scsi
    echo "- - -" > /sys/class/scsi_host/host0/scan
    """
    machine = disk.machine
    if not machine._ssh:
        logger.info('machine %s possibly not initialized, exiting...'
                    % disk.machine.name)

    logger.info('deviceName:%s' % deviceName)
    #wait(500)
    # rescan scsi bus, we do this here because the machine is powered on
    # TODO: move function to right place
    command = ('echo "- - -" > /sys/class/scsi_host/host0/scan')
    run_ssh(machine._ssh, command,
            ignoreError=False, silent=False)
    run_ssh(machine._ssh, 'ls -Al /dev/sd*',
            ignoreError=True, silent=False)
    # make sure the device name is an actual device
    command = ("file %s |awk '{print $2 $3}'" % deviceName)
    deviceType = run_ssh(machine._ssh, command,
                         ignoreError=False, silent=False)
    if deviceType != 'blockspecial':
        logger.error('Invalid device. type:%s should be "blockspecial"'
                     % (deviceType))


def _generatePages(writeCount,
                   pageSize,
                   diskSize,
                   skipPagesList=None):
    """
    generate 'valid' pages, based on disk size and skip lists
    """
    allPagesList = _getAllPagesList(diskSize, pageSize)
    skips = []
    if skipPagesList:
        if _validatePages(allPagesList, skipPagesList):
            skips.extend(skipPagesList)
        else:
            logger.error('Invalid page found in skipPagesList')
    # subtract the skip list
    remainingPages = _getRemainingPages(allPagesList, skips)
    logger.debug('All pages count: %d' % len(remainingPages))
    if writeCount > len(remainingPages):
        # not enough pages, error out
        logger.error('Invalid write count:%d, maximum is:%d' %
                     (writeCount, len(remainingPages)))
    # randomize whole list
    shuffle(remainingPages)
    # return only the requested number of pages
    return remainingPages[0:writeCount]


def _getAllPagesList(diskSize, pageSize):
    """
    generage pages up to disk size, by page size
    """
    pages = []
    page = 0
    pages.append(page)
    nextOffset = 0
    page += 1
    while (1):
        nextOffset += (pageSize * 2)
        if nextOffset > diskSize:
            break
        pages.append(page)
        page += 1
    return pages


def _getRemainingPages(allPagesList, skipPages):
    """
    subtract skip pages, so we don't clobber previous data
    """
    pages = []
    skips = []
    if skipPages:
        try:
            for skip, checksum in skipPages:
                skips.append(skip)
        except:
            pass
    for page in allPagesList:
        if page not in skips:
            pages.append((page, None))
    return pages


def _validatePages(allPagesList, pages):
    """
    all pages are calculated by the page size.
    """
    logger.vv('_validatePages allPagesList count:%d' % len(allPagesList))
    logger.vv('_validatePages pages:%s' % str(pages))
    for page, checksum in pages:
        if page not in allPagesList:
            logger.warning('invalid page:%s' % str(page))
            return False
    return True


def _getDeviceName(index):
    return '/dev/sd%s' % chr(ASCII_A_ORDINAL + index)


def _fixEqualityParameter(param):
    if isinstance(param, str):
        if param == '==':
            param = EQUALITY
        else:
            param = INEQUALITY
    return param


def _parameterToListMatchingDiskCount(machine, param):
    if isinstance(param, int):
        l = []
        for disk in machine.disks:
            if not disk.enabled or disk.isBaseDisk:
                l.append(0)
            else:
                l.append(param)
        return l
    return param


def _listToTuple(l):
    tuples = []
    for i in l:
        tuples.append((i, ''))
    return tuples
