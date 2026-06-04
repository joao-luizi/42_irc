#!/usr/bin/env python3
"""
run_all.py — corre todos os ficheiros test_*.py encontrados nesta pasta.

Uso:
  python3 run_all.py              # modo normal
  SLOW=1 python3 run_all.py      # modo Valgrind (timings x5)
  VERBOSE=1 python3 run_all.py   # mostra detalhe de cada falha
"""

import os
import sys
import importlib
import importlib.util
import inspect
import traceback

# ---------------------------------------------------------------------------
# Config de output
# ---------------------------------------------------------------------------

VERBOSE = os.environ.get("VERBOSE", "0") == "1"
SLOW    = os.environ.get("SLOW",    "0") == "1"

RESET  = "\033[0m"
GREEN  = "\033[32m"
RED    = "\033[31m"
YELLOW = "\033[33m"
BOLD   = "\033[1m"

def green(s):  return GREEN  + s + RESET
def red(s):    return RED    + s + RESET
def yellow(s): return YELLOW + s + RESET
def bold(s):   return BOLD   + s + RESET

# ---------------------------------------------------------------------------
# Descoberta de testes
# ---------------------------------------------------------------------------

def discover_test_files():
    """Devolve lista ordenada de ficheiros test_*.py nesta pasta."""
    here = os.path.dirname(os.path.abspath(__file__))
    files = sorted(f for f in os.listdir(here)
                   if f.startswith("test_") and f.endswith(".py"))
    return [os.path.join(here, f) for f in files]

def load_tests(filepath):
    """
    Importa um ficheiro de testes e devolve lista de (nome, função).
    Funções de teste têm o formato: test_<categoria>_<descricao>
    """
    module_name = os.path.basename(filepath)[:-3]  # remove .py
    spec = importlib.util.spec_from_file_location(module_name, filepath)
    mod  = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    tests = []
    for name, obj in inspect.getmembers(mod, inspect.isfunction):
        if name.startswith("test_"):
            tests.append((name, obj))
    return tests

# ---------------------------------------------------------------------------
# Execução de um teste individual
# ---------------------------------------------------------------------------

def run_test(name, fn):
    """
    Corre fn(). Devolve (passed: bool, detail: str).
    O nome tem formato test_<CATEGORIA>_<descricao>.
    """
    # Extrai categoria e descrição do nome
    parts = name.split("_", 2)      # ["test", "auth", "pass_wrong_password"]
    category    = parts[1].upper() if len(parts) > 1 else "?"
    description = parts[2].replace("_", " ") if len(parts) > 2 else name

    label = "[%s] %s" % (category, description)
    dots  = max(1, 52 - len(label))
    line  = label + " " + ("." * dots) + " "

    try:
        fn()
        print(line + green("PASS"))
        return True, ""
    except Exception as e:
        detail = str(e)
        if VERBOSE:
            detail = traceback.format_exc()
        print(line + red("FAIL"))
        print("      " + red(detail.split("\n")[0]))
        if VERBOSE and "\n" in detail:
            for l in detail.split("\n")[1:]:
                if l.strip():
                    print("      " + l)
        return False, detail

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)

    if SLOW:
        print(yellow("[MODE=SLOW] timings x5 (Valgrind)"))
    print()

    files = discover_test_files()
    if not files:
        print(red("Nenhum ficheiro test_*.py encontrado."))
        sys.exit(1)

    passed = []
    failed = []

    for filepath in files:
        module_label = os.path.basename(filepath)
        print(bold("=== " + module_label + " ==="))

        try:
            tests = load_tests(filepath)
        except Exception as e:
            print(red("  Erro ao carregar ficheiro: " + str(e)))
            continue

        if not tests:
            print(yellow("  (sem testes encontrados)"))
            continue

        for name, fn in tests:
            ok, detail = run_test(name, fn)
            if ok:
                passed.append(name)
            else:
                failed.append((name, detail))

        print()

    # Sumário
    total = len(passed) + len(failed)
    print("=" * 53)
    summary = " Results: %d passed, %d failed" % (len(passed), len(failed))
    if failed:
        print(red(bold(summary)))
    else:
        print(green(bold(summary)))
    print("=" * 53)

    if failed:
        print()
        print(bold("FAILED tests:"))
        for name, _ in failed:
            print("  " + red(name))
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
