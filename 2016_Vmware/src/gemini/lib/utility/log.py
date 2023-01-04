# -*- coding: utf-8 -*-
# Kevin Guerra:
# function decorators

"""Logging control and utilities.

Control of logging can be performed from the regular python logging
module.  For class-level logging, the class name is appended.

"""
import sys
import os
import logging
import inspect
from current_class import CurrentClass
from functools import wraps
import traceback

# ---------------------------------------------------------------------------
LOGGING_LEVEL_TRACE = 27
LOGGING_LEVEL_IGNORED_ERROR = 25
LOGGING_LEVEL_NOTE = 24
LOGGING_LEVEL_TEST = 23
LOGGING_LEVEL_FOOTER = 22
LOGGING_LEVEL_HEADER = 21
LOGGING_LEVEL_REMOTE_OUTPUT = 19
LOGGING_LEVEL_REMOTE_VITALS = 18
LOGGING_LEVEL_REMOTE_LOGS = 17
LOGGING_LEVEL_MACHINE = 16
LOGGING_LEVEL_ESX = 15
LOGGING_LEVEL_HBR = 14
LOGGING_LEVEL_ESX_LOG = 13
LOGGING_LEVEL_HBR_LOG = 12
LOGGING_LEVEL_FUNCTION = 11
LOGGING_LEVEL_VERBOSE = 5
LOGGING_LEVEL_VVERBOSE = 4

runningTestCase = None
logFilePath = 'gemini.log'
fullLogFilePath = 'gemini.full.log'

FRAMEWORK_WARNINGS = 0
FRAMEWORK_ERRORS = 0
FRAMEWORK_CRITICAL_ERRORS = 0

TERMINATE_TEST_ON_ERROR = True

# ---------------------------------------------------------------------------

level_dict = {
    'LOGGING_LEVEL_TRACE': 27,
    'LOGGING_LEVEL_IGNORED_ERROR': 25,
    'LOGGING_LEVEL_NOTE': 24,
    'LOGGING_LEVEL_TEST': 23,
    'LOGGING_LEVEL_FOOTER': 22,
    'LOGGING_LEVEL_HEADER': 21,
    'LOGGING_LEVEL_REMOTE_OUTPUT': 19,
    'LOGGING_LEVEL_REMOTE_VITALS': 18,
    'LOGGING_LEVEL_REMOTE_LOGS': 17,
    'LOGGING_LEVEL_MACHINE': 16,
    'LOGGING_LEVEL_ESX': 15,
    'LOGGING_LEVEL_HBR': 14,
    'LOGGING_LEVEL_ESX_LOG': 13,
    'LOGGING_LEVEL_HBR_LOG': 12,
    'LOGGING_LEVEL_FUNCTION': 11,
    'LOGGING_LEVEL_VERBOSE': 5,
    'LOGGING_LEVEL_VVERBOSE': 4,
              }


def getLevelFromString(levelName):
    res = 0
    try:
        res = level_dict[levelName]
    except:
        pass
    return res


def getLevels(levelNames):
    levels = [
            logging.WARNING,
            logging.ERROR,
            logging.CRITICAL,
              ]
    for levelName in levelNames:
        levels.append(getLevelFromString(levelName))
    return levels


class LogFilter(object):
    def __init__(self, levels):
        self.__levels = levels

    def filter(self, logRecord):
        return logRecord.levelno in self.__levels

simple_levels = [
            LOGGING_LEVEL_IGNORED_ERROR,
            LOGGING_LEVEL_FOOTER,
            LOGGING_LEVEL_HEADER,
            LOGGING_LEVEL_REMOTE_OUTPUT,
            LOGGING_LEVEL_REMOTE_VITALS,
            LOGGING_LEVEL_REMOTE_LOGS,
            LOGGING_LEVEL_MACHINE,
            LOGGING_LEVEL_ESX,
            LOGGING_LEVEL_HBR,
            LOGGING_LEVEL_ESX_LOG,
            LOGGING_LEVEL_HBR_LOG,
            LOGGING_LEVEL_FUNCTION,
            LOGGING_LEVEL_TRACE,
            LOGGING_LEVEL_VVERBOSE,
            LOGGING_LEVEL_VERBOSE,
            LOGGING_LEVEL_NOTE,
            LOGGING_LEVEL_TEST,
            logging.WARNING,
            logging.ERROR,
            logging.CRITICAL,
          ]
levels = [
            logging.DEBUG,
            logging.INFO,
         ]

simpleLogFilter = LogFilter(simple_levels)
logFilter = LogFilter(levels)


def setSimpleLevels(levels):
    simpleLogFilter = LogFilter(levels)
    simple_handler.addFilter(simpleLogFilter)


def setFileSimpleLevels(levels):
    simpleLogFilter = LogFilter(levels)
    file_handler.addFilter(logFilter)
    file_full_handler.addFilter(logFilter)
    file_simple_handler.addFilter(simpleLogFilter)

# ---------------------------------------------------------------------------

default_logger = logging.getLogger(' ')
default_logger.setLevel(logging.DEBUG)
default_logger.handlers = []

handler = logging.StreamHandler()
simple_handler = logging.StreamHandler()

handler.addFilter(logFilter)
simple_handler.addFilter(simpleLogFilter)

file_handler = None
file_simple_handler = None
file_full_handler = None
file_full_simple_handler = None
formatter = None
simple_formatter = None

logging.basicConfig()
rootlogger = default_logger
rootlogger.handlers = []
if rootlogger.level == logging.NOTSET:
    rootlogger.setLevel(LOGGING_LEVEL_VVERBOSE)

default_logger.propagate = False


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
DOUBLE_LINE = \
    ('\n=========================================================='
     '============')
TRIPLE_LINE = \
    (u'\n≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡'
     u'≡≡≡≡≡≡≡≡≡≡≡≡')
LINE = ('\n__________________________________________________________'
        '____________\n')
LINE_NR = ('---------------------------------------------------------'
           '-------------')
LINE_NR2 = ('≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡'
            '≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡')
TEST_STRING = (u'▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   T E S T   O U T P U T   '
               u'▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓')
WARNING_STRING = (u'▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  W A R N I N G  '
                  u'▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓')
ERROR_STRING = (u'███████████████████████████  E R R O R !  '
                u'████████████████████████████')
CRITICAL_STRING = (u'░░░░░░░░░░░░░░░░░░░░░░  C R I T I C A L ! ! !  '
                   u'░░░░░░░░░░░░░░░░░░░░░░░')
TRACE_STRING = (u'░░░░░░░░░░░░░░░░░░░   - - -   T R A C E   - - -   '
                u'░░░░░░░░░░░░░░░░░░░░')
NOTE_STRING = (u'░░░░░░░░░░░░░░░░░░░░   - - -   N O T E   - - -   '
               u'░░░░░░░░░░░░░░░░░░░░░')
IGNORED_ERROR_STRING = (u'░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░  DISREGARD ERROR  '
                        u'░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓░▓')


def box(textString):
    extra = u'¤ ¤ ¤'
    text = extra + ' ' + textString + ' ' + extra
    top = (u'╔═══════════════════════════════════════════════════════════'
           u'═══════╗\n')
    mid = u'║'
    bot = (u' ╚══════════════════════════════════════════════════════════'
           u'════════╝')
    middle = ' ' + mid + '\033[38;5;9m' + u'{0:^66}'.format(text) + \
        '%s\033[38;5;45m' % RESET + mid + '\n'
    text = top + middle + bot
    return text

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
RESET = '\033[0m'
BLINK = '\033[5m'
UNDER = '\033[4m'
BOLD = '\033[1m'
INVERTED = '\033[7m'

WHITE = '\033[38;5;15m'
BLACK = '\033[30m'
WHITE2 = '\033[107m'
# YELLOW = '\033[33m'
YELLOW = '\033[38;5;226m'
RED = '\033[0;31m'
RED88 = '\033[38;5;88m'
RED52 = '\033[38;5;52m'
BLUE = '\033[38;5;31m'
BLUE27 = '\033[38;5;27m'
BLUE32 = '\033[38;5;32m'
BLUE39 = '\033[38;5;39m'
BLUE74 = '\033[38;5;74m'
BLUE75 = '\033[38;5;75m'
DK_BLUE = '\033[38;5;18m'
CYAN = '\033[38;5;14m'
TEAL = '\033[38;5;38m'
LT_TEAL = '\033[38;5;67m'
GREEN = '\033[38;5;76m'
DK_GREEN = '\033[38;5;28m'
VDK_GREEN = '\033[38;5;64m'
LT_PINK = '\033[38;5;224m'
PURPLE = '\033[38;5;55m'
PURPLE20 = '\033[38;5;20m'
PURPLE55 = '\033[38;5;55m'
PURPLE54 = '\033[38;5;54m'
ORANGE = '\033[38;5;208m'
VDK_ORANGE = '\033[38;5;94m'
DK_ORANGE = '\033[38;5;130m'
ORANGE136 = '\033[38;5;202m'
ORANGE138 = '\033[38;5;130m'
ORANGE202 = '\033[38;5;202m'
ORANGE130 = '\033[38;5;130m'
GRAY237 = '\033[38;5;237m'
GRAY234 = '\033[38;5;234m'
GRAY239 = '\033[38;5;239m'
GRAY240 = '\033[38;5;240m'
GRAY244 = '\033[38;5;244m'
GRAY245 = '\033[38;5;245m'
GRAY252 = '\033[38;5;252m'
GREEN35 = '\033[38;5;35m'
BLACK16 = '\033[38;5;16m'

BOLD_BLACK = '\033[1;30m'
# BOLD_YELLOW = '\033[1;33m'
BOLD_YELLOW = '\033[1;38;5;226m'
BOLD_RED = '\033[1;31m'
BOLD_BK_RED = '\033[1;41m'

BK_WHITE = '\033[48;5;15m'
BK_RED = '\033[48;5;124m'
BK_DK_RED = '\033[48;5;88m'
BK_DK_BLUE = '\033[48;5;17m'
BK_VDK_BLUE = '\033[48;5;16m'
BK_GRAY234 = '\033[48;5;234m'
BK_GRAY235 = '\033[48;5;235m'
BK_GRAY236 = '\033[48;5;236m'
BK_GRAY237 = '\033[48;5;237m'
BK_GRAY238 = '\033[48;5;238m'
BK_GRAY239 = '\033[48;5;239m'
BK_GRAY240 = '\033[48;5;240m'
BK_GRAY242 = '\033[48;5;242m'

# ---------------------------------------------------------------------------

logging.addLevelName(LOGGING_LEVEL_IGNORED_ERROR, "   IGNORED ERROR  ")
logging.addLevelName(LOGGING_LEVEL_NOTE, "   NOTE  ")
logging.addLevelName(LOGGING_LEVEL_HEADER, "   HEADER  ")
logging.addLevelName(LOGGING_LEVEL_FOOTER, "   FOOTER  ")
logging.addLevelName(LOGGING_LEVEL_TEST, "   TEST  ")
logging.addLevelName(LOGGING_LEVEL_TRACE, "TRACE")
logging.addLevelName(LOGGING_LEVEL_FUNCTION, "FUNCTION")
logging.addLevelName(LOGGING_LEVEL_VERBOSE, "VERBOSE")
logging.addLevelName(LOGGING_LEVEL_VVERBOSE, "VV")
logging.addLevelName(LOGGING_LEVEL_REMOTE_LOGS, "REMOTE_LOGS")
logging.addLevelName(LOGGING_LEVEL_REMOTE_VITALS, "REMOTE_VITALS")
logging.addLevelName(LOGGING_LEVEL_REMOTE_OUTPUT, "REMOTE_OUTPUT")
logging.addLevelName(LOGGING_LEVEL_ESX, "ESX")
logging.addLevelName(LOGGING_LEVEL_HBR, "HBR")
logging.addLevelName(LOGGING_LEVEL_ESX_LOG, "ESX_LOG")
logging.addLevelName(LOGGING_LEVEL_HBR_LOG, "HBR_LOG")
logging.addLevelName(LOGGING_LEVEL_MACHINE, "MACHINE")

# ---------------------------------------------------------------------------
logging.addLevelName(LOGGING_LEVEL_IGNORED_ERROR,
                     "%s%s%s\n" % (LT_PINK, LINE, IGNORED_ERROR_STRING))
logging.addLevelName(LOGGING_LEVEL_NOTE,
                     "%s%s%s\n" % (WHITE, LINE, NOTE_STRING))
logging.addLevelName(LOGGING_LEVEL_TEST,
                     "%s%s%s\n" % (WHITE, LINE, TEST_STRING))
logging.addLevelName(LOGGING_LEVEL_HEADER,
                     "%s%s%s" % (BK_WHITE, BK_GRAY240,
                logging.getLevelName(LOGGING_LEVEL_HEADER)))
logging.addLevelName(LOGGING_LEVEL_FOOTER,
                     "%s%s%s" % (WHITE, BK_GRAY237,
                logging.getLevelName(LOGGING_LEVEL_FOOTER)))
logging.addLevelName(LOGGING_LEVEL_TEST,
                     "%s%s%s\n" % (CYAN, LINE, TEST_STRING))
logging.addLevelName(LOGGING_LEVEL_FUNCTION,
                     "%s%s%s" % (DK_BLUE,
                                 logging.getLevelName(LOGGING_LEVEL_FUNCTION),
                                 GRAY252))

logging.addLevelName(LOGGING_LEVEL_VVERBOSE,
                     "%s%s" % (GRAY237,
                               logging.getLevelName(LOGGING_LEVEL_VVERBOSE)))
logging.addLevelName(LOGGING_LEVEL_VERBOSE,
                     "%s%s" % (GRAY240,
                               logging.getLevelName(LOGGING_LEVEL_VERBOSE)))
logging.addLevelName(LOGGING_LEVEL_TRACE,
                     "%s%s%s\n" % (TEAL, LINE, TRACE_STRING))
logging.addLevelName(logging.DEBUG,
                     "%s%s" % (GRAY245,
                               logging.getLevelName(logging.DEBUG)))
logging.addLevelName(logging.INFO,
                     "%s%s%s%s" % (WHITE, BOLD,
                                 logging.getLevelName(logging.INFO), RESET))
logging.addLevelName(logging.WARNING,
    "%s%s%s%s\n" % (BOLD_YELLOW, LINE, WARNING_STRING, YELLOW))
logging.addLevelName(logging.ERROR,
                     "%s%s%s%s\n" % (BOLD_RED, LINE, ERROR_STRING, RED))
logging.addLevelName(logging.CRITICAL,
    "%s%s%s%s%s%s%s%s\n" %
    (YELLOW, BOLD_BK_RED, LINE, BLINK,
     CRITICAL_STRING, RESET, YELLOW, BOLD_BK_RED))

logging.addLevelName(LOGGING_LEVEL_ESX_LOG,
                     "%s%s%s" % (BK_GRAY236, GREEN35,
                                 logging.getLevelName(LOGGING_LEVEL_ESX_LOG)))
logging.addLevelName(LOGGING_LEVEL_HBR_LOG,
                     "%s%s%s" % (BK_GRAY236, BLUE39,
                                 logging.getLevelName(LOGGING_LEVEL_HBR_LOG)))
logging.addLevelName(LOGGING_LEVEL_MACHINE,
                     "%s%s%s" % (BK_DK_BLUE, PURPLE,
                                 logging.getLevelName(LOGGING_LEVEL_MACHINE)))
logging.addLevelName(LOGGING_LEVEL_ESX,
                     "%s%s%s" % (BK_DK_BLUE, GREEN35,
                                 logging.getLevelName(LOGGING_LEVEL_ESX)))
logging.addLevelName(LOGGING_LEVEL_HBR,
                     "%s%s%s" % (BK_DK_BLUE, BLUE39,
                                 logging.getLevelName(LOGGING_LEVEL_HBR)))
logging.addLevelName(LOGGING_LEVEL_REMOTE_OUTPUT,
                     "%s%s%s" % (BK_GRAY237, LT_PINK,
                    logging.getLevelName(LOGGING_LEVEL_REMOTE_OUTPUT)))
logging.addLevelName(LOGGING_LEVEL_REMOTE_LOGS,
                     "%s%s%s" % (BK_GRAY237, DK_BLUE,
                    logging.getLevelName(LOGGING_LEVEL_REMOTE_LOGS)))
logging.addLevelName(LOGGING_LEVEL_REMOTE_VITALS,
                     "%s%s%s" % (BK_GRAY235, PURPLE,
                    logging.getLevelName(LOGGING_LEVEL_REMOTE_VITALS)))

# ---------------------------------------------------------------------------
logging.Logger.ignored = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_IGNORED_ERROR, msg, *args, **kwargs)
logging.ignored = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_IGNORED_ERROR, '{0:^68}'.format(msg),
                *args, **kwargs)

logging.Logger.note = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_NOTE, '{0:^68}'.format(msg), *args, **kwargs)
logging.note = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_NOTE, msg, *args, **kwargs)
logging.Logger.header = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_HEADER, msg, *args, **kwargs)
logging.header = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_HEADER, msg, *args, **kwargs)
logging.Logger.footer = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_FOOTER, msg, *args, **kwargs)
logging.footer = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_FOOTER, msg, *args, **kwargs)
logging.Logger.test = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_TEST, msg, *args, **kwargs)
logging.test = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_TEST, msg, *args, **kwargs)
logging.Logger.function = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_FUNCTION, msg, *args, **kwargs)
logging.function = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_FUNCTION, msg, *args, **kwargs)
logging.Logger.trace = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_TRACE, msg, *args, **kwargs)
logging.trace = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_TRACE, msg, *args, **kwargs)
logging.Logger.verbose = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_VERBOSE, msg, *args, **kwargs)
logging.verbose = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_VERBOSE, msg, *args, **kwargs)
logging.Logger.vverbose = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_VVERBOSE, msg, *args, **kwargs)
logging.vv = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_VVERBOSE, msg, *args, **kwargs)
logging.Logger.vv = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_VVERBOSE, msg, *args, **kwargs)
logging.vverbose = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_VVERBOSE, msg, *args, **kwargs)
logging.Logger.remote_logs = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_REMOTE_LOGS, msg, *args, **kwargs)
logging.remote_logs = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_REMOTE_LOGS, msg, *args, **kwargs)
logging.Logger.remote_vitals = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_REMOTE_VITALS, msg, *args, **kwargs)
logging.remote_vitals = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_REMOTE_VITALS, msg, *args, **kwargs)
logging.Logger.remote_output = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_REMOTE_OUTPUT, msg, *args, **kwargs)
logging.remote_output = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_REMOTE_OUTPUT, msg, *args, **kwargs)
logging.Logger.esx = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_ESX, msg, *args, **kwargs)
logging.esx = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_ESX, msg, *args, **kwargs)
logging.Logger.hbr = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_HBR, msg, *args, **kwargs)
logging.hbr = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_HBR, msg, *args, **kwargs)
logging.Logger.machine = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_MACHINE, msg, *args, **kwargs)
logging.machine = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_MACHINE, msg, *args, **kwargs)
logging.Logger.esxLog = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_ESX_LOG, msg, *args, **kwargs)
logging.esxLog = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_ESX_LOG, msg, *args, **kwargs)
logging.Logger.hbrLog = lambda inst, msg, *args, **kwargs: \
    inst.log(LOGGING_LEVEL_HBR_LOG, msg, *args, **kwargs)
logging.hbrLog = lambda msg, *args, **kwargs: \
    logging.log(LOGGING_LEVEL_HBR_LOG, msg, *args, **kwargs)

# ---------------------------------------------------------------------------


def add_reset_at_end(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        func(*args, **kwargs)
        resetAtEnd = '%s' % (RESET)
        f1 = open(logFilePath, 'a')
        f2 = open(fullLogFilePath, 'a')
        f1.write(resetAtEnd)
        f2.write(resetAtEnd)
        f1.close()
        f2.close()
        print(resetAtEnd)
    return wrapper
default_logger.remote_logs = add_reset_at_end(default_logger.remote_logs)
default_logger.remote_vitals = add_reset_at_end(default_logger.remote_vitals)
default_logger.remote_output = add_reset_at_end(default_logger.remote_output)
default_logger.esx = add_reset_at_end(default_logger.esx)
default_logger.hbr = add_reset_at_end(default_logger.hbr)
default_logger.machine = add_reset_at_end(default_logger.machine)
default_logger.esxLog = add_reset_at_end(default_logger.esxLog)
default_logger.hbrLog = add_reset_at_end(default_logger.hbrLog)


def add_line_at_end(func, formatStr):
    @wraps(func)
    def wrapper(*args, **kwargs):
        func(*args, **kwargs)
        lineAtEnd = '%s%s%s\n' % (formatStr, LINE_NR2, RESET)
        f1 = open(logFilePath, 'a')
        f2 = open(fullLogFilePath, 'a')
        f1.write(lineAtEnd)
        f2.write(lineAtEnd)
        f1.close()
        f2.close()
        print(lineAtEnd)
    return wrapper
default_logger.test = add_line_at_end(default_logger.test, CYAN)
default_logger.note = add_line_at_end(default_logger.note, WHITE)
default_logger.trace = add_line_at_end(default_logger.trace, TEAL)
default_logger.warning = add_line_at_end(default_logger.warning, BOLD_YELLOW)
default_logger.error = add_line_at_end(default_logger.error, BOLD_RED)
default_logger.critical = add_line_at_end(default_logger.critical,
                                          '%s%s' % (YELLOW, BOLD_BK_RED))


def add_module_and_lineno(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        func(*args, **kwargs)
        frame = sys._getframe(0)
        for i in range(4):
            frame = sys._getframe(i)
            if 'log.py' in frame.f_code.co_filename:
                continue
            break
        frame_info = "%s:%s" % (frame.f_code.co_filename, frame.f_lineno)
        fileInfo = '%s%s%s\n%s\n' % (LT_TEAL, UNDER, frame_info, RESET)
        f1 = open(logFilePath, 'a')
        f2 = open(fullLogFilePath, 'a')
        f1.write(fileInfo)
        f2.write(fileInfo)
        f1.close()
        f2.close()
        print(fileInfo)
    return wrapper
default_logger.warning = add_module_and_lineno(default_logger.warning)
default_logger.error = add_module_and_lineno(default_logger.error)
default_logger.critical = add_module_and_lineno(default_logger.critical)

stackTrace = ''
exclude = ['main.py', 'case.py', 'loader.py', 'log.py']


def getStackTrace():
    global stackTrace
    global exclude
    stackTrace = ''
    for f, l, m, t in traceback.extract_stack():
        fn = os.path.basename(f)
        if fn not in exclude:
            stackTrace += '[%s:%s:@%s] %s\r\n' % (m, f, l, t)
    return stackTrace


def add_test_stack_trace(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        func(*args, **kwargs)
        global stackTrace
        if not stackTrace:
            stackTrace = getStackTrace()
        default_logger.log(LOGGING_LEVEL_TRACE, '%s' % stackTrace)
    return wrapper
default_logger.error = add_test_stack_trace(default_logger.error)
default_logger.critical = add_test_stack_trace(default_logger.critical)


def add_test_terminator(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        func(*args, **kwargs)
        if runningTestCase:
            global FRAMEWORK_ERRORS
            global stackTrace
            FRAMEWORK_ERRORS += 1
            runningTestCase.delete()
            stackTrace = ''
            runningTestCase.fail(msg='Terminating test...')
        else:
            print 'No test case, cannot terminate test'
    return wrapper
if TERMINATE_TEST_ON_ERROR:
    default_logger.error = add_test_terminator(default_logger.error)
    default_logger.critical = add_test_terminator(default_logger.critical)


def add_test_tab_reset(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        global tabs
        tabs = 0
        func(*args, **kwargs)
    return wrapper
default_logger.test = add_test_tab_reset(default_logger.test)


def add_counter_warnings(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        global_vars = func.func_globals
        sentinel = object()
        warnings = global_vars.get('FRAMEWORK_WARNINGS', sentinel)
        func(*args, **kwargs)
        warnings += 1
        global_vars['FRAMEWORK_WARNINGS'] = warnings
    return wrapper
default_logger.warning = add_counter_warnings(default_logger.warning)


def add_counter_errors(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        global_vars = func.func_globals
        sentinel = object()
        errors = global_vars.get('FRAMEWORK_ERRORS', sentinel)
        func(*args, **kwargs)
        errors += 1
        global_vars['FRAMEWORK_ERRORS'] = errors
    return wrapper
default_logger.error = add_counter_errors(default_logger.error)


def add_counter_critical_errors(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        global_vars = func.func_globals
        sentinel = object()
        errors = global_vars.get('FRAMEWORK_CRITICAL_ERRORS', sentinel)
        func(*args, **kwargs)
        errors += 1
        global_vars['FRAMEWORK_CRITICAL_ERRORS'] = errors
    return wrapper
default_logger.critical = add_counter_critical_errors(default_logger.critical)

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
_logged_classes = set()
checkedClasses = ['Disk'
                  , 'ReplicaDisk'
                  , 'ImageDisk'
                  , 'Machine'
                  , 'SpazMachine'
                  , 'ProtectedMachine']

setattr(CurrentClass(), 'checked', checkedClasses)


def class_logger(cls):
    try:
        logger = rootlogger
        cls._should_log_debug = lambda self: logger.isEnabledFor(logging.DEBUG)
        cls._should_log_function = lambda self: \
            logger.isEnabledFor(LOGGING_LEVEL_FUNCTION)
        cls._should_log_info = lambda self: logger.isEnabledFor(logging.INFO)
        cls.module = str(inspect.getmro(cls))
        cls.source = cls.__module__ + "." + cls.__name__
        cls.logger = logger
        cls.meta = cls.__name__
        _logged_classes.add(cls)
    except:
        pass
    return cls


def should_print(clazz, funcMeta):
    """
    inherited decorated functions print twice
    this function takes care of that
    """
    if clazz.__class__.__name__ in CurrentClass().checked and \
            clazz.__class__.__name__ != funcMeta:
        return False
    return True


tabs = 0


def clear_tabs():
    global tabs
    tabs = 0


def _log(func, format1, format2, name=None):
    try:
        setattr(func, 'meta', CurrentClass().name)
    except:
        pass
    @wraps(func)
    def wrapper(self, *args, **kwargs):
        shouldPrint = True
        if name:
            source = name
            logger = default_logger
        else:
            if hasattr(self.__class__, 'logger'):
                logger = self.__class__.logger
            else:
                logger = default_logger
            if hasattr(self.__class__, 'source'):
                source = self.__class__.source
            else:
                source = ''
            try:
                shouldPrint = should_print(self, func.meta)
            except:
                pass
        global tabs
        if tabs < 15:
            spaces = ''.join(['•' for s in xrange(tabs)])
        else:
            spaces = '...'
        if shouldPrint:
            logger.log(LOGGING_LEVEL_FUNCTION, '%s%sBEGIN: %s:%s()%s' %
                       (spaces, format1, source, func.__name__, RESET))
        tabs = tabs + 1
        result = func(self, *args, **kwargs)
        if shouldPrint:
            logger.log(LOGGING_LEVEL_FUNCTION, '%s%sEND: %s:%s()%s' %
                       (spaces, format2, source, func.__name__, RESET))
        tabs = tabs - 1
        if tabs < 0:
            tabs = 0
        return result
    return wrapper

# ---------------------------------------------------------------------------


def low_class_function(func):
    return _log(func,
                '%s' % ORANGE,
                '%s' % DK_ORANGE)


def high_class_function(func):
    return _log(func,
                '%s' % GREEN,
                '%s' % VDK_GREEN)


def high_class_named_function(name):
    def _high_class_function(func):
        return _log(func,
                    '%s' % (GREEN),
                    '%s' % (VDK_GREEN),
                    name=name)
    return _high_class_function


def workflow_class_function(func):
    return _log(func,
                '%s' % PURPLE55,
                '%s' % PURPLE54)


def service_class_function(func):
    return _log(func,
                '%s' % RED88,
                '%s' % RED52)


def proxy_class_function(func):
    return _log(func,
                '%s' % ORANGE202,
                '%s' % ORANGE130)


def class_function(func):
    return _log(func,
                '%s' % BOLD_BLACK,
                '%s' % BOLD_BLACK)


def test_helper_function(func):
    return _log(func,
                '%s' % BK_GRAY235,
                '%s' % BK_GRAY234)


def test_class_function(func):
    return _log(func,
                '%s%s' % (CYAN, BK_DK_BLUE),
                '%s%s' % (BLUE74, BK_DK_BLUE))


def test_function(func):
    return _log(func,
                '%s%s' % (GRAY244, BK_VDK_BLUE),
                '%s%s' % (GRAY239, BK_VDK_BLUE))


def test_setUp_function(func):
    return _log(func,
                '%s%s' % (BLUE75, BK_VDK_BLUE),
                '%s%s' % (BLUE32, BK_VDK_BLUE))


def test_tearDown_function(func):
    return _log(func,
                '%s%s' % (PURPLE20, BK_VDK_BLUE),
                '%s%s' % (DK_BLUE, BK_VDK_BLUE))


def service_class_function_named(name):
    def _service_class_function(func):
        return _log(func,
                    '%s' % (RED88),
                    '%s' % (RED52),
                    name=name)
    return _service_class_function


def low_class_static_function(name):
    def _low_class_static_function(func):
        return _log(func,
                    '%s%s' % (BOLD, ORANGE),
                    '%s%s' % (BOLD, DK_ORANGE),
                    name=name)
    return _low_class_static_function


def high_class_static_function(name):
    def _high_class_static_function(func):
        return _log(func,
                    '%s%s' % (BOLD, GREEN),
                    '%s%s' % (BOLD, VDK_GREEN),
                    name=name)
    return _high_class_static_function


def workflow_class_static_function(name):
    def _workflow_class_static_function(func):
        return _log(func,
                    '%s%s' % (BOLD, RED88),
                    '%s%s' % (BOLD, RED52),
                    name=name)
    return _workflow_class_static_function
