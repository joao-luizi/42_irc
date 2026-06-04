# ft_irc — Test Suite

Conjunto de testes automatizados para servidores IRC compatíveis com RFC 1459.  
Testa comportamento protocolar puro — qualquer implementação de ft_irc passa estes testes.

---

## Requisitos

- Python 3
- Servidor a correr antes de executar os testes

---

## Configuração

Por defeito o tester liga a `127.0.0.1:6667` com password `pass`.  
Para usar valores diferentes, passa variáveis de ambiente:

```bash
IRC_HOST=127.0.0.1 IRC_PORT=6667 IRC_PASS=outra python3 tester/run_all.py
```

---

## Correr os testes

Os testes precisam de dois terminais: um para o servidor e outro para o tester.

**Terminal 1 — arrancar o servidor:**
```bash
./ircserv 6667 pass
```

**Terminal 2 — correr todos os testes:**
```bash
python3 tester/run_all.py
```

**Terminal 2 — correr apenas uma secção:**
```bash
python3 tester/test_auth.py
python3 tester/test_channel.py
python3 tester/test_msg.py
python3 tester/test_op.py
python3 tester/test_quit.py
```

**Terminal 2 — correr com detalhe nas falhas:**
```bash
VERBOSE=1 python3 tester/run_all.py
VERBOSE=1 python3 tester/test_op.py
```

---

## Correr debaixo do Valgrind

**Terminal 1 — arrancar o servidor com Valgrind:**
```bash
valgrind --leak-check=full --track-origins=yes ./ircserv 6667 pass
```

**Terminal 2 — correr os testes em modo lento:**
```bash
SLOW=1 python3 tester/run_all.py
```

O `SLOW=1` multiplica todos os timeouts por 5 para compensar a lentidão do Valgrind.  
Após os testes, Ctrl+C no Terminal 1 para ver o relatório de memória.

Para guardar o relatório do Valgrind em ficheiro:
```bash
# Terminal 1
valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log ./ircserv 6667 pass

# Terminal 2
SLOW=1 python3 tester/run_all.py

# Após Ctrl+C no Terminal 1
cat valgrind.log
```

---

## Estrutura

| Ficheiro | O que testa |
|---|---|
| `test_auth.py` | PASS, NICK, USER, fluxo de registo, códigos 431/432/433/451/461/462/464 |
| `test_channel.py` | JOIN, modos +i/+k/+l, broadcast de entrada, códigos 403/443/471/473/475 |
| `test_msg.py` | PRIVMSG para canal e nick directo, códigos 401/403/411/412 |
| `test_op.py` | KICK, INVITE, TOPIC, MODE (i/t/k/o/l), códigos 331/332/341/482 |
| `test_quit.py` | QUIT com e sem razão, broadcast, cleanup de nick, desconexão abrupta |

---

## Output

```
=== test_auth.py ===
[AUTH] full registration sends 001 .................. PASS
[AUTH] nick in use returns 433 ...................... PASS
[AUTH] nick no param returns 431 .................... FAIL
      NICK sem argumento devia devolver 431

=====================================================
 Results: 49 passed, 1 failed
=====================================================
```

---

## Notas

- Cada teste cria os seus próprios clientes e fecha-os no fim — os testes são independentes entre si.
- Os nicks gerados são únicos por run para evitar colisões em runs consecutivos sem reiniciar o servidor.
- Os canais são únicos por teste para evitar interferência entre testes que correm em sequência.
