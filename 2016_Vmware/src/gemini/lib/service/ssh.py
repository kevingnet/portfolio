#!/usr/bin/env python

from __future__ import generators
from lib.utility import log
import os
import paramiko
import socket
import time


@log.class_logger
class Ssh():

    @log.service_class_function
    def __init__(self, hostname, username, password,
                 logFunction, logLogFunction,
                 keepAlive=True, retryWaitSeconds=3):
        self.logFunction = logFunction
        self.logLogFunction = logLogFunction
        self._host = hostname
        self._user = username
        self._password = password
        self._port = 22
        self._byteCount = 100
        self._connectionTimeoutSeconds = 5
        self.log = log.default_logger
        self.log.info('host/port: %s:%d - user: %s'
                      % (self._host, self._port, self._user))
        self._transport = None
        self._look_for_keys = False
        self._compress = True
        self._keepAlive = keepAlive
        self._retryWaitSeconds = retryWaitSeconds
        self._init()
        self.session_number = 1

    @log.service_class_function
    def cleanup(self):
        if self._transport:
            self.log.debug('%s:disconnecting host' % (self._host))
            try:
                self._transport.close()
            except:
                pass
            self._transport = None

    def _init(self):
        self._transport = paramiko.Transport((self._host, self._port))

    def _connectIf(self):
        """
        connect if needed
        """
        #self.log.vv('%s:_connectIf' % (self._host))
        if self._transport.is_active() is False:
            try:
                self.log.debug('%s:connecting host' % (self._host))
                self.log.verbose('username %s' % (self._user))
                self.log.verbose('_transport %s' % str(self._transport))
                self._transport.connect(username=self._user,
                                        password=self._password)
                self.log.vv('_transport %s' % str(self._transport))
            except paramiko.AuthenticationException, e:
                self.log.error('%s:_connectIf AuthenticationException'
                               % (self._host, str(e)))
            except paramiko.BadHostKeyException, e:
                self.log.error('%s:_connectIf BadHostKeyException'
                               % (self._host, str(e)))
            except paramiko.SSHException, e:
                self.log.error('%s:_connectIf SSHException'
                               % (self._host, str(e)))
            except socket.error, e:
                self.log.error('%s:_connectIf socket.error'
                               % (self._host, str(e)))
        #else:
            #self.log.vv('%s:_connectIf: Already connected, OK' % (self._host))

    def disconnect(self):
        self.log.debug('%s:disconnecting host' % (self._host))
        self._transport.close()

    def _disconnectIfNotKeepAlive(self):
        """
        disconnect if _keepAlive is False
        """
        #self.log.vv('%s:_disconnectIfNotKeepAlive' % (self._host))
        if self._keepAlive:
            pass
            #self.log.verbose('%s:keeping host connection alive' % (self._host))
        else:
            self.log.debug('%s:disconnecting host' % (self._host))
            self._transport.close()

    def _getResult(self, session, silent, ignoreError=False):
        """
        process results of a command, will 'create' an error during failures
        """
        code = session.recv_exit_status()
        stdout = []
        stderr = []
        try:
            while session.recv_ready():
                stdout.append(session.recv(self._byteCount))
            stdout = "".join(stdout).rstrip()

            while session.recv_stderr_ready():
                stderr.append(session.recv_stderr(self._byteCount))
            stderr = "".join(stderr).rstrip()
        except Exception, e:
            self.log.warning('%s:_getResult Exception %s'
                             % (self._host, str(e)))
        if code == 0:
            if not silent:
                self.log.vv('%s:result OK' % (self._host))
        else:
            stderr.rstrip()
            if not stderr:
                stderr = ''
            errStr = '%d' % code
            try:
                errStr = os.strerror(code).rstrip()
            except:
                pass
            #self.log.vv('stderr %s' % (stderr))
            #self.log.vv('errStr %s' % (errStr))
            if ignoreError:
                self.log.ignored('%s: ERROR: %s:%s'
                                 % (self._host, stderr, errStr))
                stdout = '%s\r%s' % (stdout, stderr)
            else:
                self.log.error('%s: ERROR: %s:%s'
                               % (self._host, stderr, errStr))
        return (code, stdout, stderr)

    def _run(self, command, silent=False, get_pty=False):
        """
        run command and return session object
        """
        session = None
        if not silent:
            self.log.debug('%s:run: %s' % (self._host, command))
        self._connectIf()
        try:
            session = self._transport.open_channel('session')
        except Exception, e:
            self.log.ignored('%s:_run Exception %s'
                           % (self._host, str(e)))
        if not session:
            self.cleanup()
            self._init()
            session = self._transport.open_channel('session')
        try:
            session = self._transport.open_channel('session')
        except Exception, e:
            self.log.error('%s:_run Exception %s'
                           % (self._host, str(e)))
        #self.log.vv('session %s' % str(session))
        self.session_number += 1
        #self.log.vv('session_number %d' % self.session_number)
        try:
            session.exec_command(command)
        except paramiko.SSHException, e:
            self.log.error('%s:_run SSHException %s'
                           % (self._host, str(e)))
        #self.log.vv('session %s' % str(session))
        return session

    def run(self, command, ignoreError=False, silent=False, get_pty=False):
        """
        run command and return results (code, stdout, stderr)
        """
        session = self._run(command, silent, get_pty)
        self._disconnectIfNotKeepAlive()
        return self._getResult(session, silent, ignoreError)

    def runRetry(self, command, times, ignoreError=False, silent=False):
        """
        run command, retry 'times' every two seconds
        """
        if not silent:
            self.log.debug('%s:runRetry: %s' % (self._host, command))
        left = times
        succeeded = False
        session = None
        code = -1
        while ((not succeeded) and (left >= 0)):
            session = self._run(command, silent, ignoreError)
            code = session.recv_exit_status()
            if code == 0:
                succeeded = True
            else:
                left -= 1
                time.sleep(self._retryWaitSeconds)
        result = (None, None, None)
        if code == 0:
            result = self._getResult(session, silent, ignoreError)
        else:
            self.log.critical('%s: ERROR: Command retried %d times failed'
                              % (self._host, times))
        self._disconnectIfNotKeepAlive()
        return result

    def execute(self, command, silent=False):
        """
        run command, return only an error code, but will
        'create' an error during failure
        """
        if not silent:
            self.log.debug('%s:execute: %s' % (self._host, command))
        self._connectIf()
        handle = self._transport.open_session()
        try:
            handle.exec_command(command)
        except paramiko.SSHException, e:
            self.log.error('%s:execute SSHException'
                           % (self._host, str(e)))
        self._disconnectIfNotKeepAlive()
        code = handle.recv_exit_status()
        if code == 0:
            if not silent:
                self.log.debug('%s:execute OK' % (self._host))
        else:
            self.log.error('%s:execute ERROR: %s'
                           % (self._host, os.strerror(code).rstrip()))
        return code

    def get(self, remoteFile, localPath, autoCreateDirectories=True):
        """
        get a remote 'file' into a local 'path' or 'file'
        Use cases:
        1) localPath is a directory. MUST have a trailing '/' : /foo/bar/
        2) localPath is a file. MUST NOT have a trailing '/', obviously
        Return 0 on Success, -1 on Failure
        TODO: determine is exceptions are needed
        """
        self.log.debug('%s:get: remote:"%s" -> local:"%s"'
                       % (self._host, remoteFile, localPath))

        dirName = os.path.dirname(localPath)
        # make sure we have a directory to copy into
        if not os.path.exists(dirName):
            if autoCreateDirectories:
                os.makedirs(dirName)
            else:
                self.log.error('Directory %s not found.' % (dirName))
                return -1

        if os.path.isdir(localPath):
            localPath = localPath + '/' + os.path.basename(remoteFile)

        self._connectIf()
        try:
            sftp = paramiko.SFTPClient.from_transport(self._transport)
            sftp.get(remoteFile, localPath)
            sftp.close()
        except Exception, e:
            self.log.error('%s:get Exception %s' % (self._host, str(e)))
        self._disconnectIfNotKeepAlive()
        return 0

    def put(self, localFile, remotePath):
        """
        put a local 'file' into a remote 'path' or 'file'
        Return 0 on Success, -1 on Failure
        TODO: determine is exceptions are needed
        """
        if not os.path.exists(localFile):
            self.log.error('Local file: %s - does not exist' % localFile)
            return -1
        self.log.debug('%s:put: local:"%s" -> remote:"%s"'
                       % (self._host, localFile, remotePath))
        self._connectIf()
        try:
            sftp = paramiko.SFTPClient.from_transport(self._transport)
            sftp.put(localFile, remotePath)
            sftp.close()
        except Exception, e:
            self.log.error('%s:put Exception %s' % (self._host, str(e)))
        self._disconnectIfNotKeepAlive()

    def fileExists(self, fileName):
        command = "[ ! -f filename ] && echo 'File not found!'"
        session = self._run(command)
        self._disconnectIfNotKeepAlive()
        (code, stdout, stderr) = self._getResult(session, silent)
        if stdout == 'File not found!':
            return False
        else:
            return True


def findCmd(cmd):
    """Finds the path to the specified command

    Requires 'which' command on the path

    @param cmd               [in] The cmd to find
    """
    cmd = 'which ' + cmd

    (rc, stdout) = runCommandGetOutput(cmd)

    if stdout == None or stdout == '':
        raise RuntimeError(cmd + " is not on the host's $PATH")
    elif not '/' in stdout:
        raise RuntimeError("'which " + cmd
                           + "' did not return a path: " + stdout)
    else:
        return stdout.rstrip("\n")


def lsRemoteDir(host, dir):
    """Helper function to list a remote directory and return the output
    as a list of file names

    @param host          [in] The host to run the command on
    @param dir           [in] Full path to directory to list
    @return contents    [out] List.  The dir contents as a list.
    """
    # List the dir, each item on its own line
    lsCmd = "ls -1 " + dir

    (rc, output) = runStafCommandGetOutput(host, lsCmd)
    if rc != 0:
        printInfo("Command: " + lsCmd + " returned non-zero return " +
                  "code: " + str(rc))
        return None

    contents = []
    for line in output:
        file = line.strip()
        contents.append(file)

    return contents

