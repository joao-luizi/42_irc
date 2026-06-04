"""
test_op.py — KICK, INVITE, TOPIC, MODE (i/t/k/o/l)
"""

import time
from helpers import IRCClient, wait, assert_contains, assert_not_contains, unick, WAIT_SHORT, WAIT_MEDIUM

def _chan():
    """Canal único por chamada."""
    return "#ch" + str(int(time.time()))[-5:]

def _setup_channel(channame, op_nick, guest_nick=None):
    op = IRCClient()
    op.register(op_nick)
    op.send("JOIN " + channame)
    wait(WAIT_SHORT)
    op.recv()

    if guest_nick is None:
        return op, None

    guest = IRCClient()
    guest.register(guest_nick)
    guest.send("JOIN " + channame)
    wait(WAIT_SHORT)
    op.recv()
    guest.recv()
    return op, guest

# ---------------------------------------------------------------------------
# KICK
# ---------------------------------------------------------------------------

def test_op_kick_by_op_removes_target():
    chan = _chan()
    op, guest = _setup_channel(chan, unick("KOp1"), unick("KTgt"))

    op.send("KICK " + chan + " " + unick("KTgt") + " :fora")
    wait(WAIT_MEDIUM)

    out = guest.recv()
    assert_contains(out, "KICK", "target devia receber mensagem KICK")

    op.close()
    guest.close()

def test_op_kick_by_non_op_returns_482():
    chan = _chan()
    op_nick  = unick("KOp2")
    gst_nick = unick("KGs2")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    guest.send("KICK " + chan + " " + op_nick + " :tentativa")
    wait(WAIT_SHORT)
    out = guest.recv()
    assert_contains(out, "482", "não-op a fazer KICK devia devolver 482")

    op.close()
    guest.close()

def test_op_kick_target_not_in_channel_returns_441():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("KOp3"))

    outside_nick = unick("Kout")
    outside = IRCClient()
    outside.register(outside_nick)
    wait(WAIT_SHORT)
    outside.recv()

    op.send("KICK " + chan + " " + outside_nick + " :nao esta")
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "441", "KICK de alguém fora do canal devia devolver 441")

    op.close()
    outside.close()

def test_op_kick_nonexistent_channel_returns_403():
    c = IRCClient()
    c.register(unick("KNoc"))
    c.send("KICK #inexistente alguem :bye")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "403", "KICK em canal inexistente devia devolver 403")
    c.close()

def test_op_kick_broadcast_to_channel():
    chan = _chan()
    op_nick  = unick("KBOp")
    gst_nick = unick("KBGs")
    obs_nick = unick("KBOb")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    observer = IRCClient()
    observer.register(obs_nick)
    observer.send("JOIN " + chan)
    wait(WAIT_SHORT)
    op.recv()
    guest.recv()
    observer.recv()

    op.send("KICK " + chan + " " + gst_nick + " :saiu")
    wait(WAIT_MEDIUM)

    out = observer.recv()
    assert_contains(out, "KICK", "observador devia receber broadcast do KICK")

    op.close()
    guest.close()
    observer.close()

# ---------------------------------------------------------------------------
# INVITE
# ---------------------------------------------------------------------------

def test_op_invite_sends_invite_to_target():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("IOp1"))

    tgt_nick = unick("ITg1")
    target = IRCClient()
    target.register(tgt_nick)
    wait(WAIT_SHORT)
    target.recv()

    op.send("INVITE " + tgt_nick + " " + chan)
    wait(WAIT_SHORT)

    out = target.recv()
    assert_contains(out, "INVITE", "target devia receber mensagem INVITE")

    op.close()
    target.close()

def test_op_invite_confirms_to_inviter():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("IOp2"))

    tgt_nick = unick("ITg2")
    target = IRCClient()
    target.register(tgt_nick)
    wait(WAIT_SHORT)
    target.recv()

    op.send("INVITE " + tgt_nick + " " + chan)
    wait(WAIT_SHORT)

    out = op.recv()
    assert_contains(out, "341", "inviter devia receber 341 RPL_INVITING")

    op.close()
    target.close()

def test_op_invite_by_non_op_returns_482():
    chan = _chan()
    op_nick  = unick("IOp3")
    gst_nick = unick("IGs3")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    tgt_nick = unick("ITg3")
    target = IRCClient()
    target.register(tgt_nick)
    wait(WAIT_SHORT)
    target.recv()

    guest.send("INVITE " + tgt_nick + " " + chan)
    wait(WAIT_SHORT)
    out = guest.recv()
    assert_contains(out, "482", "não-op a fazer INVITE devia devolver 482")

    op.close()
    guest.close()
    target.close()

def test_op_invite_already_on_channel_returns_443():
    chan = _chan()
    op_nick  = unick("IOp4")
    gst_nick = unick("IGs4")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    op.send("INVITE " + gst_nick + " " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "443", "INVITE para membro já no canal devia devolver 443")

    op.close()
    guest.close()

# ---------------------------------------------------------------------------
# TOPIC
# ---------------------------------------------------------------------------

def test_op_topic_view_no_topic_returns_331():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("TOp1"))

    op.send("TOPIC " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "331", "sem topic devia devolver 331")

    op.close()

def test_op_topic_set_and_view():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("TOp2"))

    op.send("TOPIC " + chan + " :o meu topic")
    wait(WAIT_SHORT)
    op.recv()

    op.send("TOPIC " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "332", "topic definido devia devolver 332")
    assert_contains(out, "o meu topic", "topic devia conter o texto definido")

    op.close()

def test_op_topic_restricted_non_op_returns_482():
    chan = _chan()
    op_nick  = unick("TOp3")
    gst_nick = unick("TGs3")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    op.send("MODE " + chan + " +t")
    wait(WAIT_MEDIUM)
    op.recv()
    guest.recv()

    guest.send("TOPIC " + chan + " :tentativa")
    wait(WAIT_SHORT)
    out = guest.recv()
    assert_contains(out, "482", "não-op a mudar topic com +t devia devolver 482")

    op.close()
    guest.close()

def test_op_topic_change_broadcast_to_channel():
    chan = _chan()
    op_nick  = unick("TOp4")
    gst_nick = unick("TGs4")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    op.send("TOPIC " + chan + " :novo topic")
    wait(WAIT_MEDIUM)

    out = guest.recv()
    assert_contains(out, "novo topic", "mudança de topic devia ser broadcast para o canal")

    op.close()
    guest.close()

# ---------------------------------------------------------------------------
# MODE
# ---------------------------------------------------------------------------

def test_op_mode_query_returns_324():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("MOp1"))

    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "324", "MODE query devia devolver 324")

    op.close()

def test_op_mode_invite_only_toggle():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("MOp2"))

    op.send("MODE " + chan + " +i")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    modes = out.strip().split()[-1]
    assert_contains(out, "i", "+i devia aparecer nos modos")

    op.send("MODE " + chan + " -i")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    modes = out.strip().split()[-1]
    assert_not_contains(modes, "i", "-i devia remover 'i' dos modos")

    op.close()

def test_op_mode_topic_restricted_toggle():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("MOp3"))

    op.send("MODE " + chan + " +t")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "t", "+t devia aparecer nos modos")

    op.send("MODE " + chan + " -t")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_not_contains(out, "+t", "-t devia remover 't' dos modos")

    op.close()

def test_op_mode_key_set_and_remove():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("MOp4"))

    op.send("MODE " + chan + " +k chave")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "k", "+k devia aparecer nos modos")

    op.send("MODE " + chan + " -k")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_not_contains(out, "k", "-k devia remover 'k' dos modos")

    op.close()

def test_op_mode_user_limit_set_and_remove():
    chan = _chan()
    op, _ = _setup_channel(chan, unick("MOp5"))

    op.send("MODE " + chan + " +l 5")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "l", "+l devia aparecer nos modos")

    op.send("MODE " + chan + " -l")
    wait(WAIT_SHORT)
    op.recv()
    op.send("MODE " + chan)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_not_contains(out, "l", "-l devia remover 'l' dos modos")

    op.close()

def test_op_mode_give_op_to_member():
    chan = _chan()
    op_nick  = unick("MOp6")
    gst_nick = unick("MGs6")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    op.send("MODE " + chan + " +o " + gst_nick)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "+o", "dar op devia broadcast MODE +o")

    op.close()
    guest.close()

def test_op_mode_take_op_from_member():
    chan = _chan()
    op_nick  = unick("MOp7")
    gst_nick = unick("MGs7")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    op.send("MODE " + chan + " +o " + gst_nick)
    wait(WAIT_SHORT)
    op.recv()
    guest.recv()

    op.send("MODE " + chan + " -o " + gst_nick)
    wait(WAIT_SHORT)
    out = op.recv()
    assert_contains(out, "-o", "tirar op devia broadcast MODE -o")

    op.close()
    guest.close()

def test_op_mode_by_non_op_returns_482():
    chan = _chan()
    op_nick  = unick("MOp8")
    gst_nick = unick("MGs8")
    op, guest = _setup_channel(chan, op_nick, gst_nick)

    guest.send("MODE " + chan + " +i")
    wait(WAIT_SHORT)
    out = guest.recv()
    assert_contains(out, "482", "não-op a fazer MODE devia devolver 482")

    op.close()
    guest.close()

if __name__ == "__main__":
    import run_all, os, sys
    sys.path.insert(0, os.path.dirname(__file__))
    run_all.discover_test_files = lambda: [__file__]
    run_all.main()