# ft_irc — Checklist de Testes Manuais (HexChat)

Verificar antes da avaliação. Os testes estão ordenados por dependência — cada secção
assume que a anterior foi concluída com sucesso.

> **Nota geral:** alguns erros do servidor não são mostrados pelo HexChat — ele trata-os
> silenciosamente ou com comportamento próprio. Nesses casos a verificação deve ser feita
> via `nc -C localhost 6667`.

---

## Preparação — Arrancar o servidor

```bash
./ircserv 6667 pass
```

---

## Preparação — Ligar o HexChat

### Cliente 1 (Alice)

1. Abre o HexChat
2. Menu **HexChat → Network List** (ou Ctrl+S)
3. Clica **Add**, dá o nome `ft_irc`
4. Em **Servers**, adiciona `127.0.0.1/6667`
5. Em **Login method** escolhe `Server password`
6. Em **Password** escreve `pass`
7. Define o nick `Alice`
8. Clica **Connect**

### Cliente 2 (Bob)

Repete o processo numa segunda janela do HexChat com nick `Bob`.

### Verificar ligação

Após ligar, deves ver no HexChat:
```
* Welcome to the IRC Network Alice
```
Se não aparecer, verifica a password e a porta.

---

## 1. Ligação e registo

- [ ] **Password errada**  
  Em Network List muda a password para `errada`, tenta ligar.  
  Esperado: HexChat mostra erro e desliga.

- [ ] **Password correcta**  
  Liga com `pass`.  
  Esperado: mensagem de boas-vindas (001 Welcome).

- [ ] **Dois clientes ligados**  
  Alice e Bob ligados em simultâneo.  
  Esperado: ambos registados sem erro.

---

## 2. JOIN e canais

- [ ] **Entrar num canal — Alice**  
  No HexChat de Alice:
  ```
  /join #teste
  ```
  Esperado: Alice entra no canal e aparece com `@` na lista de membros.

- [ ] **Primeiro membro fica op**  
  Verificar que Alice tem `@` na lista de membros de `#teste`.

- [ ] **Entrar num canal — Bob**  
  No HexChat de Bob:
  ```
  /join #teste
  ```
  Esperado: Bob entra no canal sem `@`. Alice vê `Bob has joined #teste`.

- [ ] **Broadcast de entrada**  
  Verificar que Alice recebeu a notificação de entrada de Bob.

- [ ] **Canal inexistente é criado automaticamente**  
  No HexChat de Alice:
  ```
  /join #novocanalqualquer
  ```
  Esperado: canal criado, Alice fica op.

---

## 3. NICK

Pré-condição: Alice e Bob estão ambos em `#teste`.

- [ ] **Mudar nick**  
  No HexChat de Alice:
  ```
  /nick AliceNova
  ```
  Esperado: Bob vê `Alice is now known as AliceNova` no canal `#teste`.  
  Repõe o nick original antes de continuar:
  ```
  /nick Alice
  ```

- [ ] **Nick já em uso**  
  No HexChat de Bob:
  ```
  /nick Alice
  ```
  Esperado no HexChat: Bob tenta automaticamente nicks alternativos (`Bob_`, `Bob__`) até
  desistir — é comportamento do HexChat ao receber o `433`, não um bug do servidor.  
  Verificação via `nc`:
  ```
  NICK Alice
  ```
  Esperado: `:server 433 * Alice :Nickname is already in use`

- [ ] **Nick inválido**  
  No HexChat de Bob:
  ```
  /nick inva!lid
  ```
  Esperado: erro 432 Erroneous nickname.

---

## 4. PRIVMSG

Pré-condição: Alice e Bob estão ambos em `#teste`.

- [ ] **Mensagem para canal**  
  No HexChat de Alice, escreve qualquer mensagem no canal `#teste`.  
  Esperado: Bob vê a mensagem. Alice não recebe eco.

- [ ] **Mensagem directa**  
  No HexChat de Alice:
  ```
  /msg Bob ola bob
  ```
  Esperado: Bob recebe a mensagem numa janela de DM. Alice não vê eco.

- [ ] **Mensagem para nick inexistente**  
  No HexChat de Alice:
  ```
  /msg NickQueNaoExiste ola
  ```
  Esperado no HexChat: a mensagem aparece no canal activo no formato `>NickQueNaoExiste< ola`
  sem abrir janela de DM — o HexChat não mostra o erro `401`.  
  Verificação via `nc`:
  ```
  PRIVMSG NickQueNaoExiste :ola
  ```
  Esperado: `:server 401 * NickQueNaoExiste :No such nick`

---

## 5. MODE

Pré-condição: Alice e Bob estão em `#teste`. Alice é op (`@`).

- [ ] **Consultar modos do canal**  
  No HexChat de Alice:
  ```
  /mode #teste
  ```
  Esperado: resposta 324 com os modos activos.

- [ ] **Não-op tenta MODE**  
  No HexChat de Bob:
  ```
  /mode #teste +i
  ```
  Esperado: erro 482 You're not channel operator.

- [ ] **+i / -i**  
  No HexChat de Alice:
  ```
  /mode #teste +i
  /mode #teste
  ```
  Esperado: modos mostram `i`. Depois:
  ```
  /mode #teste -i
  /mode #teste
  ```
  Esperado: `i` desaparece dos modos.

- [ ] **+t / -t**  
  No HexChat de Alice:
  ```
  /mode #teste +t
  /mode #teste
  ```
  Esperado: modos mostram `t`. Depois:
  ```
  /mode #teste -t
  ```

- [ ] **+k / -k**  
  No HexChat de Alice:
  ```
  /mode #teste +k segredo
  ```
  Bob sai do canal e tenta voltar sem key:
  ```
  /part #teste
  /join #teste
  ```
  Esperado: erro 475 Cannot join channel (+k). Com key:
  ```
  /join #teste segredo
  ```
  Esperado: sucesso. Para remover:
  ```
  /mode #teste -k
  ```

- [ ] **+l / -l**  
  Com Alice e Bob dentro do canal:
  ```
  /mode #teste +l 1
  ```
  Nota: o limite não expulsa os membros já dentro — só impede novas entradas.  
  Abre um terceiro cliente e tenta entrar:
  ```
  /join #teste
  ```
  Esperado: erro 471 Cannot join channel (+l). Para remover:
  ```
  /mode #teste -l
  ```

- [ ] **+o / -o**  
  No HexChat de Alice:
  ```
  /mode #teste +o Bob
  ```
  Esperado: Bob fica com `@` na lista de membros. Para tirar:
  ```
  /mode #teste -o Bob
  ```
  Esperado: Bob perde `@`.

---

## 6. INVITE

Pré-condição: Alice é op em `#teste`. MODE foi testado e funciona.

- [ ] **Canal +i, não-convidado tenta entrar**  
  No HexChat de Alice, activa invite-only:
  ```
  /mode #teste +i
  ```
  Bob sai do canal:
  ```
  /part #teste
  ```
  Bob tenta voltar:
  ```
  /join #teste
  ```
  Esperado: erro 473 Cannot join channel (+i).

- [ ] **Op convida**  
  No HexChat de Alice:
  ```
  /invite Bob #teste
  ```
  Esperado: Bob recebe notificação de convite. Alice recebe confirmação 341.

- [ ] **Convidado entra**  
  No HexChat de Bob:
  ```
  /join #teste
  ```
  Esperado: Bob entra com sucesso.

- [ ] **Não-op tenta convidar**  
  Activa `+i` novamente. No HexChat de Bob (sem op):
  ```
  /invite Alice #teste
  ```
  Esperado: erro 482.  
  Remove `+i` antes de continuar:
  ```
  /mode #teste -i
  ```

---

## 7. TOPIC

Pré-condição: Alice e Bob estão em `#teste`. Alice é op. MODE foi testado e funciona.

- [ ] **Canal sem topic**  
  No HexChat de Alice:
  ```
  /topic #teste
  ```
  Esperado: mensagem "No topic is set".

- [ ] **Op define topic**  
  No HexChat de Alice:
  ```
  /topic #teste :bem vindos ao canal
  ```
  Esperado: Bob vê a mudança de topic. Topic aparece no topo do canal.

- [ ] **Consultar topic**  
  No HexChat de Alice:
  ```
  /topic #teste
  ```
  Esperado: mostra "bem vindos ao canal".

- [ ] **Qualquer membro pode mudar topic sem +t**  
  No HexChat de Bob (sem op):
  ```
  /topic #teste :topic do bob
  ```
  Esperado: sucesso — sem `+t` qualquer membro pode mudar o topic.

- [ ] **Canal com +t, não-op tenta mudar topic**  
  No HexChat de Alice:
  ```
  /mode #teste +t
  ```
  No HexChat de Bob:
  ```
  /topic #teste :outro topic
  ```
  Esperado: erro 482 You're not channel operator.

- [ ] **Op muda topic com +t activo**  
  No HexChat de Alice:
  ```
  /topic #teste :topic novo
  ```
  Esperado: sucesso. Bob vê a mudança.  
  Remove `+t` antes de continuar:
  ```
  /mode #teste -t
  ```

---

## 8. KICK

Pré-condição: Alice e Bob estão em `#teste`. Alice é op.

- [ ] **Op faz kick**  
  No HexChat de Alice:
  ```
  /kick #teste Bob
  ```
  Esperado: Bob é removido do canal. Ambos vêem a mensagem de KICK.

- [ ] **Kick com razão**  
  Bob volta ao canal (`/join #teste`). No HexChat de Alice:
  ```
  /kick #teste Bob :comportamento inaceitavel
  ```
  Esperado: razão aparece na mensagem de KICK.

- [ ] **Não-op tenta kick**  
  Bob volta ao canal. No HexChat de Bob:
  ```
  /kick #teste Alice
  ```
  Esperado: erro 482 You're not channel operator.

- [ ] **Kickado pode voltar**  
  Bob (kickado) tenta voltar:
  ```
  /join #teste
  ```
  Esperado: sucesso (canal não tem `+i`).

---

## 9. QUIT

Pré-condição: Alice e Bob estão em `#teste`.

- [ ] **QUIT com razão**  
  No HexChat de Bob:
  ```
  /quit ate logo
  ```
  Esperado: Alice vê `Bob has quit (ate logo)`.

- [ ] **Nick disponível após QUIT**  
  Liga um novo cliente com nick `Bob`.  
  Esperado: registo com sucesso, sem erro 433.

- [ ] **Desconexão abrupta**  
  Bob liga-se novamente. Fecha a janela do HexChat de Bob sem `/quit`.  
  Esperado: Alice vê mensagem de quit. Servidor mantém-se estável.

---

## 10. Estabilidade

- [ ] **Canal vazio é destruído**  
  Todos saem do canal `#teste`. Outro cliente entra no mesmo canal.  
  Esperado: canal é criado de novo, novo cliente fica op.

- [ ] **Ligações e desligações em sequência**  
  Liga e desliga o Cliente 2 várias vezes seguidas.  
  Esperado: servidor mantém-se estável, Cliente 1 continua a funcionar normalmente.
