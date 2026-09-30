#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temporary_dir=$(mktemp -d /tmp/vaxiom-tests.XXXXXX)
trap 'rm -rf "$temporary_dir"' EXIT HUP INT TERM

"$project_dir/vaxiom" "$project_dir/input.txt" >"$temporary_dir/original.ast"
grep -q "Declare Num" "$temporary_dir/original.ast"
grep -q "ArrayType" "$temporary_dir/original.ast"
grep -q "For I" "$temporary_dir/original.ast"
grep -q "ArrayAccess" "$temporary_dir/original.ast"
grep -q "ExpressionStatement" "$temporary_dir/original.ast"

"$project_dir/vaxiom" "$project_dir/tests/complete.vax" >"$temporary_dir/complete.ast"
grep -q "Binary \*" "$temporary_dir/complete.ast"
grep -q "Binary AND" "$temporary_dir/complete.ast"
grep -q "While" "$temporary_dir/complete.ast"
grep -q "Function Add" "$temporary_dir/complete.ast"
grep -q "Call Add" "$temporary_dir/complete.ast"

"$project_dir/lexer" "$project_dir/tests/complete.vax" "$temporary_dir/tokens.txt" >/dev/null
grep -q "STRING_LITERAL" "$temporary_dir/tokens.txt"
grep -q "GREATER_EQUAL" "$temporary_dir/tokens.txt"

if "$project_dir/vaxiom" "$project_dir/tests/invalid.vax" \
    >"$temporary_dir/invalid.out" 2>"$temporary_dir/invalid.err"; then
    echo "expected invalid.vax to fail" >&2
    exit 1
fi
grep -q "parse error" "$temporary_dir/invalid.err"

echo "All Vaxiom tests passed."
