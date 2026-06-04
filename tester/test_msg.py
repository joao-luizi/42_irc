"""
test_msg.py — PRIVMSG para canal e nick directo
"""

from helpers import IRCClient, wait, assert_contains, assert_not_contains, unick, WAIT_SHORT, WAIT_MEDIUM

def test_msg_privmsg_channel_delivered_to_others():
    chan = "#msg1" + str(int(__import__('time').time()))[-4:]
    c1 = IRCClient()
    c1.register(unick("Snd"))
    c2 = IRCClient()
    c2.register(unick("Rcv"))

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("PRIVMSG " + chan + " :ola mundo")
    wait(WAIT_SHORT)

    out = c2.recv()
    assert_contains(out, "ola mundo", "mensagem devia ser entregue ao outro membro do canal")

    c1.close()
    c2.close()

def test_msg_privmsg_channel_not_received_by_sender():
    chan = "#msg2" + str(int(__import__('time').time()))[-4:]
    c1 = IRCClient()
    c1.register(unick("SndE"))
    c2 = IRCClient()
    c2.register(unick("RcvE"))

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("PRIVMSG " + chan + " :sem echo")
    wait(WAIT_SHORT)

    out = c1.recv()
    assert_not_contains(out, "sem echo", "sender não devia receber eco da própria mensagem")

    c1.close()
    c2.close()

def test_msg_privmsg_nonexistent_channel_returns_403():
    c = IRCClient()
    c.register(unick("MsgN"))
    c.send("PRIVMSG #inexistente :teste")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "403", "PRIVMSG para canal inexistente devia devolver 403")
    c.close()

def test_msg_privmsg_no_text_returns_412():
    chan = "#ntxt" + str(int(__import__('time').time()))[-4:]
    c = IRCClient()
    c.register(unick("NTxt"))
    c.send("JOIN " + chan)
    wait(WAIT_SHORT)
    c.recv()
    c.send("PRIVMSG " + chan + " :")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "412", "PRIVMSG sem texto devia devolver 412")
    c.close()

def test_msg_privmsg_no_target_returns_411():
    c = IRCClient()
    c.register(unick("NTgt"))
    c.send("PRIVMSG")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "411", "PRIVMSG sem destinatário devia devolver 411")
    c.close()

def test_msg_privmsg_nick_delivered():
    n1 = unick("DMS")
    n2 = unick("DMR")
    c1 = IRCClient()
    c1.register(n1)
    c2 = IRCClient()
    c2.register(n2)
    wait(WAIT_SHORT)
    c2.recv()

    c1.send("PRIVMSG " + n2 + " :mensagem directa")
    wait(WAIT_SHORT)

    out = c2.recv()
    assert_contains(out, "mensagem directa", "DM devia ser entregue ao destinatário")

    c1.close()
    c2.close()

def test_msg_privmsg_nick_not_received_by_sender():
    n1 = unick("DS2")
    n2 = unick("DR2")
    c1 = IRCClient()
    c1.register(n1)
    c2 = IRCClient()
    c2.register(n2)
    wait(WAIT_SHORT)
    c2.recv()

    c1.send("PRIVMSG " + n2 + " :sem eco dm")
    wait(WAIT_SHORT)

    out = c1.recv()
    assert_not_contains(out, "sem eco dm", "DM não devia dar eco ao sender")

    c1.close()
    c2.close()

def test_msg_privmsg_unknown_nick_returns_401():
    c = IRCClient()
    c.register(unick("MUnk"))
    c.send("PRIVMSG NickInexistente :teste")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "401", "PRIVMSG para nick inexistente devia devolver 401")
    c.close()

if __name__ == "__main__":
    import run_all, os, sys
    sys.path.insert(0, os.path.dirname(__file__))
    run_all.discover_test_files = lambda: [__file__]
    run_all.main()

def test_msg_privmsg_channel_not_member_returns_442():
    chan = "#nmem" + str(int(__import__('time').time()))[-4:]
    
    # alfa cria o canal
    alfa = IRCClient()
    alfa.register(unick("Alfa"))
    alfa.send("JOIN " + chan)
    wait(WAIT_SHORT)
    alfa.recv()

    # beta tenta mandar mensagem sem estar no canal
    beta = IRCClient()
    beta.register(unick("Beta"))
    beta.send("PRIVMSG " + chan + " :ola")
    wait(WAIT_SHORT)
    out = beta.recv()
    assert_contains(out, "442", "PRIVMSG para canal onde não está devia devolver 442")

    alfa.close()
    beta.close()