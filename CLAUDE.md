# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A single-file C++ console program that solves the Numbers round from the "Letters and Numbers" TV show: given six integers and a target, it brute-forces all expressions using `+ - * /` (each input used at most once) and prints every unique solution. Division must be exact (integer, no remainder) and subtraction must not go negative; invalid intermediate results are marked with the sentinel `ERINT = -6666`.

## Build and run

Visual Studio: open `numc.sln`, build `Release|x64`.

MSBuild is not on PATH in a plain shell. From PowerShell:

```powershell
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
& $msbuild numc.sln /p:Configuration=Release /p:Platform=x64
```

Run (exactly 7 positional args — six numbers then the target; there is **no argument validation**, fewer args crash):

```powershell
.\x64\Release\numc.exe 100 75 25 10 7 5 666
```

`numc.py` (repo root) is a Tkinter GUI wrapper around the exe: `python numc.py`. It validates the seven inputs before shelling out to `x64/Release/numc.exe` on a background thread, so it needs the Release|x64 build to exist. Pure stdlib, no dependencies. `numc.ico` is its icon (also used by a desktop shortcut on the owner's machine).

## Testing

There is no test suite. Validate changes by building and running the known-good example above; it finishes in a few seconds and must end with:

```
1 259708 48 ((100+75+10)*(25-7)/5) = 666
2 3456920 12 ((((100-7)*(25+10))+75)/5) = 666
2 SOLUTIONS FOUND USING 6 NUMBERS, TESTED 30965760 CASES
```

(Blocks for 1–5 numbers print "NO SOLUTIONS" for this input.) If you touch core logic, also confirm divide-by-zero, non-integer division, and negative subtraction are still rejected.

## Architecture

All code lives in `numc.cpp` (~630 lines). `numc.h` is **empty**, and `numd.cpp` is a stale near-duplicate (comment typos, missing zero-init) that is **not part of the build** — the .vcxproj compiles only `numc.cpp`. Make changes in `numc.cpp`.

The search enumerates permutations × operator-combinations × bracketings:

- `main()` runs one block per expression size: `CheckBlock1()` (trivial 1-number match), then `CheckBlock(n,k,m,q,cn,ci,t)` for 2–6 numbers with hard-coded parameters per size (the 6-number block tests 30,965,760 cases).
- `c[42][12]` is the table of all 42 bracketing shapes as postfix (RPN) template strings: `'a'` = push the next number of the current permutation, a digit = apply operator `b[digit - ci]`. `ci` trims the template for smaller expression sizes.
- `GenerateNextPermutation()` / `GenerateNextCombination()` step the global `a[]` / `b[]` arrays; `TryPermCombBracTriplet()` evaluates the postfix string on an int stack (`si[]`), applying `Op()` which enforces the game rules.
- On a target match, `CreateSortedExpressionString_Enhanced()` builds a *canonical* string so algebraically equivalent expressions collapse to one: it flattens chains of `+/-` and `*//` into term lists, sorts terms by signed value with a string hash (`strHash`) as tie-break, and re-parenthesizes. This canonicalization is the subtle part of the codebase — most dedup bugs live here.
- Unique strings are stored in `fs[]` (hard cap 5000 solutions × 40 chars, no bounds checks) with hit counters, and printed at the end of each block as `<solution#> <case#> <hits> <expr> = <target>`.

### Global state

Functions communicate through fixed-size globals, not parameters (no dynamic allocation anywhere — keep it that way per AGENTS.md):

- `av[6]`, `ag` — input numbers and target, set from `argv` in `main`.
- `a[]`, `b[]` — current permutation and operator combination; `n,k,m,q,cn,ci` — per-block enumeration sizing, set globally before the loops run.
- `br[]`, `si[]`, `so[]`, `ssp[]` — expression-building stack (string, value, operator, internal pointer).
- `sbr[][]`, `ssi[][]`, `sso[][]`, `ssa[][]`, `ssb[][]`, `ssx[][]` — parallel internal stacks used during canonical string construction; index `TMP = 6` is the scratch slot.
- `fs[]`, `fsn[]`, `fsi1[]`, `fsii[]` — deduplicated solution strings with hit counts and discovery indices.
- `c[42][12]` — the bracketing table; treat as a fixed constant, the `CheckBlock` parameters assume its exact layout.

## Repo quirks

- Build outputs under `x64/Release/` (including `numc.exe`) are **tracked in git** even though `.gitignore` lists `x64/` — every rebuild dirties the working tree. The owner has historically committed rebuilt binaries (e.g. "Recomiled" commits), but don't sweep artifact churn into unrelated commits.
- Style (per AGENTS.md): 4-space indent, braces on the same line, PascalCase functions (`TryPermCombBracTriplet`), short lowercase globals/locals (`av`, `ag`, `si`), uppercase constants (`ERINT`). No formatter or linter.
- Commits are short capitalized summaries, sometimes with a device/date stamp.
