# numc

## Numc
Numc is a console C/C++ program that solves the Numbers round from the Numbers and Letters TV show. Given six integers and a target, it searches for expressions using each input at most once with `+`, `-`, `*`, and `/`. Division is integer-only (no remainder), and subtraction is only accepted when the result is non-negative. The program prints all unique solutions while accounting for commutativity and associativity, and it reports when none exist.

Example:

```text
x64/Release/numc.exe 100 75 25 10 7 5 666
((100+75+10)*(25-7)/5) = 666
((((100-7)*(25+10))+75)/5) = 666
...
```

Inputs are positional: six numbers followed by the target (7 args total). There is no argument validation, so always provide all seven values.

## Building and Running
This repo is a Visual Studio solution created with Visual Studio Community 2026. Recommended environment is Windows with MSVC and the Windows SDK installed.

Build in Visual Studio:
- Open `numc.sln`
- Build `Release|x64` (or `Debug|x64`)

Build via MSBuild (Developer Command Prompt):

```text
msbuild numc.sln /p:Configuration=Release /p:Platform=x64
```

Run after building:

```text
x64/Release/numc.exe 100 75 25 10 7 5 666
```

## Structure and Logic
Core implementation lives in `numc.cpp`. `numc.h` is currently an empty placeholder. The `numd.cpp` file is a near-duplicate variant kept for alternate builds or experiments; check with `diff numc.cpp numd.cpp` before making parallel edits. Visual Studio artifacts are in `numc.vcxproj` and `numc.vcxproj.filters`.

`main` searches every subset size from 1 through 6 by calling `CheckBlock1()` for the trivial one-number case and `CheckBlock(6, k, ...)` for `k = 2..6`. Each block is a brute-force search across:
- permutations of the inputs (`GenerateNextPermutation`)
- combinations of operators (`GenerateNextCombination`) over the four primitives `+ - * /`
- bracket patterns encoded in the `c[42][12]` table (the 42 distinct bracketings of 6 operands; smaller subsets index a subrange)

For each permutation/combination/bracketing triplet, `TryPermCombBracTriplet` evaluates the expression using a fixed-size stack, applying `Op` for each operator with integer-only rules (invalid results propagate as the sentinel `ERINT = -6666`). When a result matches the target, `CreateSortedExpressionString_Enhanced` builds a canonicalized expression string (ordering operands for commutative operations) so equivalent expressions collapse into one. The `fs[]` arrays store unique solutions and counts, which are printed at the end of each block of tests in `CheckBlock`.

## Contributor Docs
- `AGENTS.md` — repository guidelines (style, build, testing conventions).
- `CLAUDE.md` — orientation for Claude Code sessions working in this repo.
