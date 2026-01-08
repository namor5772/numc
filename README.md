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
This repo is a Visual Studio solution created with Visual Studio Community 2019. Recommended environment is Windows with MSVC and the Windows SDK installed.

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
Core implementation lives in `numc.cpp` with declarations in `numc.h`. The `numd.cpp` file is a near-duplicate variant. Visual Studio artifacts are in `numc.vcxproj` and `numc.vcxproj.filters`.

The solver is a brute-force search across:
- permutations of the six inputs (`GenerateNextPermutation`)
- combinations of operators (`GenerateNextCombination`)
- bracket patterns encoded in the `c[42][12]` table

For each permutation/combination/bracketing triplet, `TryPermCombBracTriplet` evaluates the expression using a simple stack, applying `Op` for each operator with integer-only rules. When a result matches the target, `CreateSortedExpressionString_Enhanced` builds a canonicalized expression string (ordering operands for commutative operations) so equivalent expressions collapse into one. The `fs[]` arrays store unique solutions and counts, which are printed at the end of each block of tests in `CheckBlock`.
