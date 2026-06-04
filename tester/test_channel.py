"""
test_channel.py — JOIN, modos de acesso (+i, +k, +l), broadcast
"""

from helpers import IRCClient, wait, assert_contains, assert_not_contains, unick, WAIT_SHORT, WAIT_MEDIUM

def test_channel_join_creates_channel_and_sends_353_366():
    c = IRCClient()
    c.register(unick("Join"))
    c.send("JOIN #basic")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "353", "JOIN devia enviar 353 Names")
    assert_contains(out, "366", "JOIN devia enviar 366 End of Names")
    c.close()

def test_channel_join_first_member_becomes_op():
    nick = unick("FOp")
    chan = "#optest" + str(int(__import__('time').time()))[-4:]
    c = IRCClient()
    c.register(nick)
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "MODE " + chan + " +o " + nick, "primeiro membro devia receber op via MODE")
    c.close()

def test_channel_join_invalid_name_returns_403():
    c = IRCClient()
    c.register(unick("Bad"))
    c.send("JOIN nochanprefix")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "403", "canal sem # devia devolver 403")
    c.close()

def test_channel_join_broadcast_to_existing_members():
    chan = "#bcast" + str(int(__import__('time').time()))[-4:]
    c1 = IRCClient()
    c1.register(unick("Fst"))
    c2 = IRCClient()
    c2.register(unick("Snd"))

    c1.send("JOIN " + chan)
    wait(WAIT_SHORT)
    c1.recv()

    c2.send("JOIN " + chan)
    wait(WAIT_SHORT)

    out = c1.recv()
    assert_contains(out, "JOIN", "membro existente devia receber JOIN do novo membro")

    c1.close()
    c2.close()

def test_channel_join_already_member_returns_443():
    chan = "#already" + str(int(__import__('time').time()))[-4:]
    c = IRCClient()
    c.register(unick("Alr"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    c.recv()
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "443", "JOIN de canal onde já está devia devolver 443")
    c.close()

def test_channel_join_invite_only_without_invite_returns_473():
    chan = "#inv1" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("IOp1"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +i")
    wait(WAIT_SHORT)
    op.recv()

    c = IRCClient()
    c.register(unick("NoIn"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "473", "JOIN em canal +i sem convite devia devolver 473")

    op.close()
    c.close()

def test_channel_join_invite_only_with_invite_succeeds():
    chan = "#inv2" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("IOp2"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +i")
    wait(WAIT_SHORT)
    op.recv()

    guest_nick = unick("Inv")
    guest = IRCClient()
    guest.register(guest_nick)

    op.send("INVITE " + guest_nick + " " + chan)
    wait(WAIT_SHORT)
    op.recv()
    guest.recv()

    guest.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = guest.recv()
    assert_contains(out, "JOIN", "cliente convidado devia conseguir entrar no canal +i")
    assert_not_contains(out, "473", "cliente convidado não devia receber 473")

    op.close()
    guest.close()

def test_channel_join_with_wrong_key_returns_475():
    chan = "#key1" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("KOp1"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +k segredo")
    wait(WAIT_SHORT)
    op.recv()

    c = IRCClient()
    c.register(unick("WKey"))
    c.send("JOIN " + chan + " errada")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "475", "key errada devia devolver 475")

    op.close()
    c.close()

def test_channel_join_with_correct_key_succeeds():
    chan = "#key2" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("KOp2"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +k segredo")
    wait(WAIT_SHORT)
    op.recv()

    c = IRCClient()
    c.register(unick("CKey"))
    c.send("JOIN " + chan + " segredo")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "JOIN", "key correcta devia permitir entrada")
    assert_not_contains(out, "475", "key correcta não devia devolver 475")

    op.close()
    c.close()

def test_channel_join_without_key_returns_475():
    chan = "#key3" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("KOp3"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +k segredo")
    wait(WAIT_SHORT)
    op.recv()

    c = IRCClient()
    c.register(unick("NKey"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "475", "JOIN sem key num canal +k devia devolver 475")

    op.close()
    c.close()

def test_channel_join_channel_full_returns_471():
    chan = "#lim" + str(int(__import__('time').time()))[-4:]
    op = IRCClient()
    op.register(unick("LOp"))
    op.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan + " +l 1")
    wait(WAIT_SHORT)
    op.recv()

    c = IRCClient()
    c.register(unick("Over"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "471", "canal cheio devia devolver 471")

    op.close()
    c.close()

if __name__ == "__main__":
    import run_all, os, sys
    sys.path.insert(0, os.path.dirname(__file__))
    run_all.discover_test_files = lambda: [__file__]
    run_all.main()