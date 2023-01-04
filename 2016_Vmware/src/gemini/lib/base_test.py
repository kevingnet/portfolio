import unittest
import sys
import ssl
from lib.base_test_helper.utility import *
from lib.base_test_helper.directories import *
from lib.utility import log
from lib.utility import configuration
from lib.utility import add_unittest_asserts
from lib.utility.configuration import consolidate_files
from lib.utility.current_class import CurrentClass
from lib.framework.factory import Factory
from lib.framework.controller import Controller
from lib.lwrdo.factory import Factory as VMTFactory
from lib.service.wait import wait
from lib.service.events import Events
from lib.utility.log import clear_tabs
from lib.base_test_helper.unhandled_exception_handler import all_exceptions_hander


@log.class_logger
class BaseTest(unittest.TestCase, configuration.Configuration):
    setattr(CurrentClass(), 'name', 'None')
    delete = None
    deleteCalled = False
    isPythonVersion2_6 = False
    if sys.version_info[:2] == (2, 6):
        isPythonVersion2_6 = True

    @log.test_class_function
    def run(self, result=None):
        self.currentResult = clearCounters(result)
        unittest.TestCase.run(self, result)

    @log.test_class_function
    def __init__(self, testFile, *args, **kwargs):
        sys.excepthook = all_exceptions_hander
        log.testCase = self
        BaseTest.delete = self._delete
        self._fixPython3SslIssue()
        self.wait = wait

        self.currentResult = None
        self.startDate = datetime.datetime.today()
        self.startTime = int(time.time())

        unittest.TestCase.__init__(self, *args, **kwargs)
        add_unittest_asserts.global_test_case = self
        add_unittest_asserts.isPythonVersion2_6 = BaseTest.isPythonVersion2_6

        self._calcTestFileInformation(testFile)
        yamlFile = consolidate_files(self.path)
        configuration.Configuration.__init__(self, yamlFile)
        self._calcTestDirectory()

        self.logger = setupLogger(self, self.config, self.testDirectory)

        printHeader(self.name, self.startDate, self.logger)

        self.preConfigure()
        self.writeConfiguration()

        self.logger.note('Build phase')
        self.actors = Factory.addActorsNames()
        self.mediators = Factory.addMediatorsNames()
        clear_tabs()
        Factory.build(self)
        Events().actors = self.actors
        Controller.setActors(self.actors)

        self.logger.note('Post build phase')
        clear_tabs()
        self.postConfigure()

        self.logger.note('Initialization phase')
        clear_tabs()
        VMTFactory.build(self.actors)

        self.logger.note('Configuration phase')
        # assert 1==0
        clear_tabs()
        self._configure()
        # assert 1==0
        self.logger.note('Post-Initialization phase')
        clear_tabs()
        self.postInitialize()

        self.logger.note('Verification phase')
        clear_tabs()
        Controller.check()
        self.logger.note('Object creation phase')
        clear_tabs()
        Controller.create()

        self.logger.note('Starting test run')
        printTestHeader(self.name)

    def preConfigure(self):
        pass

    def postConfigure(self):
        pass

    def postInitialize(self):
        pass

    def cleanup(self):
        pass

    @classmethod
    @log.test_class_function
    def tearDownClass(cls):
        clear_tabs()
        # only available in 2.7, solves the above issue
        if not BaseTest.isPythonVersion2_6:
            cls.logger.note('tearDownClass')
            BaseTest.delete()

    @classmethod
    @log.test_class_function
    def __del__(self):
        clear_tabs()
        # this method causes issues in 2.7, the logger gets deleted first
        if BaseTest.isPythonVersion2_6:
            self.logger.note('__del__')
            BaseTest.delete()

    @log.test_class_function
    def _configure(self):
        clear_tabs()
        Controller.configure()

    @log.test_class_function
    def _delete(self):
        clear_tabs()
        if BaseTest.deleteCalled:
            self.logger.info('Delete already performed, exiting...')
            return
        BaseTest.deleteCalled = True
        self.logger.note('Cleanup phase')
        Events().ee.emit("terminate", self)
        self.cleanup()
        Controller.cleanup()
        cleanupEmptyDirectories(self.config)
        printFooter(self.name, self.testDirectory,
                    self.startTime, self.currentResult, self.logger)
        Events().ee.emit("at_end")
        createColorlessLog(self.testDirectory)
        createHtmlLog(self.testDirectory)

    def _calcTestFileInformation(self, testFile):
        testFile = os.path.splitext(testFile)[0]
        if testFile.startswith('./'):
            testFile = testFile[2:]
        testName = testFile.split('/')
        self.name = testName[-1]
        self.path = testFile

    def _calcTestDirectory(self):
        self.testDirectory = os.path.join(
                self.config.framework.resultsPath,
                os.path.splitext(self.path)[0])
        if not os.path.exists(self.testDirectory):
            os.makedirs(self.testDirectory)
        makeDirectories(self.name, self.testDirectory, self.config)

    def _fixPython3SslIssue(self):
        try:
            ssl._create_default_https_context = ssl._create_unverified_context
        except Exception, e:
            self.logger.warning('Exception:%s' % str(e))
