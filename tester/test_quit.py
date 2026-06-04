"""
test_quit.py — QUIT, broadcast para canal, cleanup do servidor, desconexão abrupta
"""

import time
from helpers import IRCClient, wait, assert_contains, assert_not_contains, unick, WAIT_SHORT, WAIT_MEDIUM, WAIT_LONG

def _chan():
    return "#qt" + str(int(time.time()))[-5:]

def test_quit_broadcast_to_channel_members():
    chan = _chan()
    c1 = IRCClient()
    c1.register(unick("QUsr"))
    c2 = IRCClient()
    c2.register(unick("QObs"))

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("QUIT :ate logo")
    wait(WAIT_MEDIUM)

    out = c2.recv()
    assert_contains(out, "QUIT", "membros do canal deviam receber broadcast do QUIT")

    c1.close()
    c2.close()

def test_quit_reason_included_in_broadcast():
    chan = _chan()
    c1 = IRCClient()
    c1.register(unick("QRsn"))
    c2 = IRCClient()
    c2.register(unick("QOb2"))

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("QUIT :razao especifica")
    wait(WAIT_MEDIUM)

    out = c2.recv()
    assert_contains(out, "razao especifica", "razão do QUIT devia aparecer no broadcast")

    c1.close()
    c2.close()

def test_quit_nick_no_longer_in_use():
    nick = unick("Free")
    c1 = IRCClient()
    c1.register(nick)
    c1.send("QUIT :sair")
    wait(WAIT_LONG)
    c1.close()

    c2 = IRCClient()
    c2.send("PASS pass")
    c2.send("NICK " + nick)
    c2.send("USER free 0 * :Free Nick")
    wait(WAIT_MEDIUM)
    out = c2.recv()
    assert_contains(out, "001", "nick devia estar livre depois do QUIT")
    assert_not_contains(out, "433", "nick não devia estar em uso depois do QUIT")
    c2.close()

def test_quit_without_reason_uses_default():
    chan = _chan()
    c1 = IRCClient()
    c1.register(unick("QNRs"))
    c2 = IRCClient()
    c2.register(unick("QOb3"))

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("QUIT")
    wait(WAIT_MEDIUM)

    out = c2.recv()
    assert_contains(out, "QUIT", "QUIT sem razão devia broadcast com razão default")

    c1.close()
    c2.close()

def test_quit_abrupt_disconnect_nick_freed():
    nick = unick("Abrt")
    c1 = IRCClient()
    c1.register(nick)
    c1.close()
    wait(WAIT_LONG)

    c2 = IRCClient()
    c2.send("PASS pass")
    c2.send("NICK " + nick)
    c2.send("USER abrupt 0 * :Abrupt Nick")
    wait(WAIT_MEDIUM)
    out = c2.recv()
    assert_contains(out, "001", "nick devia estar livre após desconexão abrupta")
    assert_not_contains(out, "433", "nick não devia estar em uso após desconexão abrupta")
    c2.close()

def test_quit_abrupt_disconnect_server_stable():
    """Servidor deve continuar a aceitar ligações após desconexão abrupta."""
    for i in range(3):
        nick = unick("St%d" % i)
        c = IRCClient()
        c.register(nick)
        c.close()
        wait(WAIT_SHORT)

    wait(WAIT_MEDIUM)

    chan = _chan()
    c = IRCClient()
    c.register(unick("Aftr"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "353", "servidor devia estar estável após desconexões abruptas")
    c.close()

if __name__ == "__main__":
    import run_all, os, sys
    sys.path.insert(0, os.path.dirname(__file__))
    run_all.discover_test_files = lambda: [__file__]
    run_all.main()