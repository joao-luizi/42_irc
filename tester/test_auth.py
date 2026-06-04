"""
test_auth.py — PASS, NICK, USER, fluxo de registo
"""

from helpers import IRCClient, wait, assert_contains, assert_not_contains, unick, WAIT_SHORT, WAIT_MEDIUM

def test_auth_pass_wrong_password_disconnects():
    c = IRCClient()
    c.send("PASS errada")
    wait(WAIT_MEDIUM)
    out = c.recv()
    assert_contains(out, "464", "password errada devia devolver 464")
    c.close()

def test_auth_pass_empty_is_rejected():
    c = IRCClient()
    c.send("PASS ")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "461", "PASS sem argumento devia devolver 461")
    c.close()

def test_auth_pass_after_registered_returns_462():
    c = IRCClient()
    c.register(unick("U462"))
    c.send("PASS pass")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "462", "PASS depois de registado devia devolver 462")
    c.close()

def test_auth_pass_too_many_params_returns_461():
    c = IRCClient()
    c.send("PASS pass extra")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "461", "PASS com params a mais devia devolver 461")
    c.close()

def test_auth_nick_in_use_returns_433():
    nick = unick("Same")
    c1 = IRCClient()
    c1.register(nick)

    c2 = IRCClient()
    c2.send("PASS pass")
    c2.send("NICK " + nick)
    wait(WAIT_SHORT)
    out = c2.recv()
    assert_contains(out, "433", "NICK em uso devia devolver 433")

    c1.close()
    c2.close()

def test_auth_nick_invalid_chars_returns_432():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK inva!lid")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "432", "nick inválido devia devolver 432")
    c.close()

def test_auth_nick_starts_with_digit_returns_432():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK 1nick")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "432", "nick a começar com dígito devia devolver 432")
    c.close()

def test_auth_nick_too_long_returns_432():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK " + ("a" * 20))
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "432", "nick demasiado longo devia devolver 432")
    c.close()

def test_auth_nick_no_param_returns_431():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "431", "NICK sem argumento devia devolver 431")
    c.close()

def test_auth_nick_change_broadcasts_to_channel():
    old = unick("Old")
    obs = unick("Obs")
    new = unick("New")
    chan = "#nkch" + str(int(__import__('time').time()))[-4:]

    c1 = IRCClient()
    c1.register(old)
    c2 = IRCClient()
    c2.register(obs)

    c1.send("JOIN " + chan)
    c2.send("JOIN " + chan)
    wait(WAIT_MEDIUM)
    c1.recv()
    c2.recv()

    c1.send("NICK " + new)
    wait(WAIT_SHORT)

    out = c2.recv()
    assert_contains(out, new, "mudança de nick devia ser broadcast para o canal")

    c1.close()
    c2.close()

def test_auth_user_missing_params_returns_461():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK " + unick("UTest"))
    c.send("USER only_one_param")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "461", "USER com params insuficientes devia devolver 461")
    c.close()

def test_auth_user_duplicate_returns_462():
    c = IRCClient()
    c.register(unick("Dup"))
    c.send("USER dup 0 * :Dup User")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "462", "USER duplicado devia devolver 462")
    c.close()

def test_auth_full_registration_sends_001():
    c = IRCClient()
    nick = unick("Reg")
    c.send("PASS pass")
    c.send("NICK " + nick)
    c.send("USER reg 0 * :Reg User")
    wait(WAIT_MEDIUM)
    out = c.recv()
    assert_contains(out, "001", "registo completo devia enviar 001 Welcome")
    c.close()

def test_auth_commands_before_registration_return_451():
    c = IRCClient()
    c.send("PASS pass")
    c.send("NICK " + unick("Pre"))
    c.send("JOIN #test")
    wait(WAIT_SHORT)
    out = c.recv()
    assert_contains(out, "451", "JOIN antes de registado devia devolver 451")
    c.close()

if __name__ == "__main__":
    import run_all, os, sys
    sys.path.insert(0, os.path.dirname(__file__))
    run_all.discover_test_files = lambda: [__file__]
    run_all.main()