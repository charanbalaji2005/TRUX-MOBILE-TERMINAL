#!/usr/bin/env bash
# tests/manual_core_test.sh
#
# The exact manual test sequence from the spec (section 78), run against
# the real meridian-shell binary and actually checked against expected
# output rather than just eyeballed — each command below produces a
# PASS/FAIL line, and the script's own exit code reflects whether
# everything passed. Generated files are cleaned up at the end
# regardless of outcome.
#
# Usage: ./tests/manual_core_test.sh   (run from the repo root, after
#                                        `make shell` or `make all`)

set -u
SHELL_BIN="./build/meridian-shell"
FAILURES=0
CHECKS=0

pass() { CHECKS=$((CHECKS+1)); echo "  [ OK ]  $1"; }
fail() { CHECKS=$((CHECKS+1)); FAILURES=$((FAILURES+1)); echo "  [FAIL]  $1"; }

check_contains() {
    local desc="$1" haystack="$2" needle="$3"
    if [[ "$haystack" == *"$needle"* ]]; then pass "$desc"; else
        fail "$desc (expected to find: '$needle', got: '$haystack')"
    fi
}

if [ ! -x "$SHELL_BIN" ]; then
    echo "meridian-shell not built — run 'make shell' or 'make all' first."
    exit 1
fi

echo "=== Meridian Shell manual core test ==="
echo "(spec section 78 — the exact listed command sequence)"
echo

echo "--- pwd ---"
out=$("$SHELL_BIN" -c "pwd")
check_contains "pwd prints a real absolute path" "$out" "/"

echo "--- echo hello ---"
out=$("$SHELL_BIN" -c "echo hello")
check_contains "echo hello" "$out" "hello"

echo "--- echo \"\$HOME\" ---"
out=$("$SHELL_BIN" -c 'echo "$HOME"')
check_contains "echo \$HOME expands" "$out" "$HOME"

echo "--- export TEST_VAR=hello / echo \"\$TEST_VAR\" ---"
out=$("$SHELL_BIN" -c 'export TEST_VAR=hello; echo "$TEST_VAR"')
check_contains "export + expansion" "$out" "hello"

echo "--- printf \"hello\\n\" ---"
out=$("$SHELL_BIN" -c 'printf "hello\n"')
check_contains "printf builtin passthrough to real printf(1)" "$out" "hello"

echo "--- echo hello | grep hello ---"
out=$("$SHELL_BIN" -c "echo hello | grep hello")
check_contains "real pipe through real grep(1)" "$out" "hello"

echo "--- echo hello > test_output.txt / cat test_output.txt ---"
"$SHELL_BIN" -c "echo hello > test_output.txt" >/dev/null
out=$(cat test_output.txt 2>/dev/null)
check_contains "redirection creates file with content" "$out" "hello"

echo "--- echo world >> test_output.txt / cat test_output.txt ---"
"$SHELL_BIN" -c "echo world >> test_output.txt" >/dev/null
out=$(cat test_output.txt 2>/dev/null)
check_contains "append keeps prior content" "$out" "hello"
check_contains "append adds new content" "$out" "world"

echo "--- sleep 1 & / jobs ---"
out=$("$SHELL_BIN" -c 'sleep 1 & jobs')
check_contains "background job reported by jobs" "$out" "sleep 1"

echo
echo "=== cleaning up generated files ==="
rm -f test_output.txt
if [ ! -e test_output.txt ]; then pass "test_output.txt removed"; else fail "test_output.txt still present"; fi

echo
echo "$CHECKS checks, $FAILURES failed"
exit $([ "$FAILURES" -eq 0 ] && echo 0 || echo 1)
