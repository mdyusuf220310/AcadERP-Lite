#!/bin/bash

set -u

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$PROJECT_DIR/src/acaderp"
TEST_DIR="$PROJECT_DIR/tests"

passed=0
failed=0

if [ ! -x "$EXECUTABLE" ]; then
    echo "ERROR: Executable not found. Run make first."
    exit 1
fi

for i in $(seq -w 1 26); do
    input="$TEST_DIR/T${i}.in"
    expected="$TEST_DIR/T${i}.out"
    actual="$TEST_DIR/T${i}.actual"

    "$EXECUTABLE" < "$input" > "$actual"

    if diff -u "$expected" "$actual" > /dev/null; then
        echo "T${i}: PASS"
        passed=$((passed + 1))
    else
        echo "T${i}: FAIL"
        diff -u "$expected" "$actual"
        failed=$((failed + 1))
    fi

    rm -f "$actual"
  rm -f "$PROJECT_DIR/tests/T25.save"
done

echo "------------------------"
echo "Passed: $passed"
echo "Failed: $failed"

if [ "$failed" -ne 0 ]; then
    exit 1
fi
