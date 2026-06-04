import socket
import os
import time

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

HOST     = os.environ.get("IRC_HOST", "127.0.0.1")
PORT     = int(os.environ.get("IRC_PORT", "6667"))
PASSWORD = os.environ.get("IRC_PASS", "pass")
SLOW     = os.environ.get("SLOW", "0") == "1"

WAIT_SHORT  = 1.0  if SLOW else 0.1
WAIT_MEDIUM = 2.0  if SLOW else 0.3
WAIT_LONG = 4.0 if SLOW else 0.5
RECV_TIMEOUT = 3.0 if SLOW else 1.0

# ---------------------------------------------------------------------------
# Nick único por run — evita colisões entre runs consecutivos
# ---------------------------------------------------------------------------

_RUN_ID = str(int(time.time()))[-4:]

def unick(base):
    """Nick único por run: base truncada a 6 chars + 4 dígitos do timestamp."""
    return base[:6] + _RUN_ID

# ---------------------------------------------------------------------------
# IRCClient
# ---------------------------------------------------------------------------

class IRCClient:
    def __init__(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((HOST, PORT))
        self._sock.settimeout(RECV_TIMEOUT)
        self._buf = ""

    def send(self, msg):
        if not msg.endswith("\r\n"):
            msg += "\r\n"
        self._sock.sendall(msg.encode())

    def recv(self):
        data = ""
        try:
            while True:
                chunk = self._sock.recv(4096).decode("utf-8", errors="replace")
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass
        return data

    def recv_line(self):
        try:
            while "\r\n" not in self._buf:
                chunk = self._sock.recv(4096).decode("utf-8", errors="replace")
                if not chunk:
                    break
                self._buf += chunk
        except socket.timeout:
            pass
        if "\r\n" in self._buf:
            line, self._buf = self._buf.split("\r\n", 1)
            return line
        return self._buf

    def close(self):
        try:
            self._sock.close()
        except:
            pass

    def register(self, nick, user=None, realname=None):
        if user is None:
            user = nick.lower()
        if realname is None:
            realname = nick
        self.send("PASS " + PASSWORD)
        self.send("NICK " + nick)
        self.send("USER " + user + " 0 * :" + realname)
        wait(WAIT_MEDIUM)
        self.recv()

# ---------------------------------------------------------------------------
# Timing
# ---------------------------------------------------------------------------

def wait(duration=None):
    if duration is None:
        duration = WAIT_SHORT
    time.sleep(duration)

# ---------------------------------------------------------------------------
# Assertions
# ---------------------------------------------------------------------------

class AssertionError(Exception):
    pass

def assert_contains(output, expected, msg=None):
    if expected not in output:
        label = msg or ("expected %r in output" % expected)
        raise AssertionError("%s\n  output: %r" % (label, output))

def assert_not_contains(output, unexpected, msg=None):
    if unexpected in output:
        label = msg or ("did not expect %r in output" % unexpected)
        raise AssertionError("%s\n  output: %r" % (label, output))

def registered_client(nick, user=None, realname=None):
    c = IRCClient()
    c.register(nick, user, realname)
    return c
