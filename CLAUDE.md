# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

`numc` is a single-binary C++ console solver for the "Numbers round" from the *Countdown / Numbers and Letters* TV show. It takes 6 integers plus a target (7 positional args, no validation) and prints all unique expressions built from `+ - * /` that evaluate to the target. Division is integer-only (rejects non-integer results); subtraction must produce a non-negative result.

Usage: `x64/Release/numc.exe 100 75 25 10 7 5 666`

## Build & Run

This is a Visual Studio 2026 solution (MSVC + Windows SDK). No test suite exists — validate changes by running the binary against known inputs.

- Visual Studio: open `numc.sln`, build `Release|x64` (or `Debug|x64`).
- MSBuild (Developer Command Prompt):
  `msbuild numc.sln /p:Configuration=Release /p:Platform=x64`
- Quick sanity check after changes: run with a known target and confirm expected solutions still appear and invalid operations (divide by zero, non-integer division, negative subtraction) are still rejected.

## Architecture

All logic lives in **`numc.cpp`** (~710 lines). `numc.h` is empty. `numd.cpp` is a near-duplicate variant kept for alternate builds/experiments — when modifying core logic, decide whether the change should be mirrored into `numd.cpp` (check with `diff numc.cpp numd.cpp`; the two files currently differ only in comments/typos and minor init).

The solver is a brute-force search over three orthogonal dimensions, driven from `main` via `CheckBlock(...)` calls that enumerate subset sizes 1..6:

1. **Permutations** of the input numbers — `GenerateNextPermutation` walks lexicographic permutations of array `a[]` / `av[]`.
2. **Operator combinations** — `GenerateNextCombination` enumerates all 4^(n-1) operator tuples for the 4 primitive ops (codes 0..3 mapped by `OpMap`: `+ - * /`).
3. **Bracketings** — hardcoded in the `c[42][12]` table as postfix-style strings (digits = operator slots, `a` = operand placeholder). The table encodes all 42 distinct bracketings of 6 operands; `CheckBlock` indexes subranges of it for smaller subset sizes.

For each (permutation, operator-combo, bracketing) triple, `TryPermCombBracTriplet` evaluates the expression via a small fixed-size stack using `Op(o, x, y)` for integer arithmetic. Invalid results propagate as the sentinel `ERINT = -6666`. When the value equals target `ag`, `CreateSortedExpressionString_Enhanced` builds a **canonical** string representation: operands of commutative ops (`+`, `*`) are sorted so expressions equivalent under commutativity/associativity collapse to one string. Results are deduplicated via `strHash` + linear scan into the global `fs[5000][40]` array.

### Global state

The program relies heavily on fixed-size global arrays (no dynamic allocation by convention):

- `av[6]`, `ag` — inputs and target, set from `argv` in `main`.
- `fs[]`, `fsn[]`, `fsi1[]`, `fsii[]`, `fsp` — deduplicated solution strings, counts, and running pointer.
- `br[]`, `si[]`, `so[]`, `ssp[]` — primary expression-building stack.
- `sbr[][]`, `ssi[][]`, `sso[][]`, `ssa[][]`, `ssb[][]`, `ssx[][]` — per-level internal stacks used during canonical string construction.
- `c[42][12]` — the bracketing lookup table (treat as a fixed constant; the indexing in `CheckBlock(6, k, ..., cn, ci, xt)` assumes its exact layout).

### Conventions

- 4-space indent, braces on same line as control statements.
- PascalCase for functions (`TryPermCombBracTriplet`, `CheckBlock`), short lowercase names for locals/globals (`av`, `ag`, `si`), uppercase for constants (`ERINT`, `TMP`).
- Keep arrays fixed-size. Avoid introducing dynamic allocation unless there's a concrete need.
- No formatter/linter is configured.
