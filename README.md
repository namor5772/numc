# numc

## Numc
Numc is a console C++ program that solves the Numbers round from the Letters and Numbers TV show. Given six integers and a target, it searches for expressions using each input at most once with `+`, `-`, `*`, and `/`. Division is integer-only (no remainder), and subtraction is only accepted when the result is non-negative. The program prints all unique solutions while accounting for commutativity and associativity, and it reports when none exist.

Example (Windows path shown; on macOS/Linux use `./numc`):

```text
x64/Release/numc.exe 100 75 25 10 7 5 666
...
1 259708 48 ((100+75+10)*(25-7)/5) = 666
2 3456920 12 ((((100-7)*(25+10))+75)/5) = 666
2 SOLUTIONS FOUND USING 6 NUMBERS, TESTED 30965760 CASES
```

Each solution line is `<solution#> <case#> <hits> <expression> = <target>`, where `<case#>` is the enumeration index at which the solution was first found and `<hits>` counts how many equivalent forms collapsed into it. A block of results is printed for every expression size from 1 to 6 numbers ("NO SOLUTIONS FOUND..." when a size has none). The search is deterministic integer arithmetic, so output is identical on all platforms.

Inputs are positional: six numbers followed by the target (7 args total). There is no argument validation, so always provide all seven values — fewer args crash.

## Building and Running
Cross-platform since commit `32ad537`: MSVC-only calls were replaced with portable ones (`snprintf` etc.), and a `Makefile` was added for non-Windows builds.

**Windows** (Visual Studio Community 2026 solution; MSVC + Windows SDK):
- Open `numc.sln` and build `Release|x64` (or `Debug|x64`), or from a Developer Command Prompt:

```text
msbuild numc.sln /p:Configuration=Release /p:Platform=x64
x64/Release/numc.exe 100 75 25 10 7 5 666
```

**macOS/Linux** (Apple clang or g++ both work):

```text
make
./numc 100 75 25 10 7 5 666
```

The Windows binary under `x64/Release/` is tracked in git; the Unix `./numc` binary is gitignored and rebuilt locally.

## GUI
`numc.py` is a Tkinter front end for the solver: enter the six numbers and the target (defaults `100 75 25 10 7 5` → `666`), press [Calculate] or Enter, and the results appear in a scrollable text box. It validates all seven fields as 32-bit integers, then runs the solver on a background thread (10-minute timeout) with a live elapsed-time status line, so the window never freezes mid-search.

Run it with `python numc.py` (Windows) or `python3 numc.py` (macOS/Linux) after building the solver for your platform — it shells out to `x64/Release/numc.exe` on Windows and `./numc` elsewhere, and shows a build hint if the binary is missing. It uses only the Python standard library (Homebrew Python on macOS may need `brew install python-tk`).

## Desktop shortcuts
`numc.ico` (a red "666" calculator) is the icon used by the desktop shortcuts on both machines.

**Windows**: a regular desktop shortcut runs `numc.py` with `numc.ico`; the GUI also sets it as its window icon.

**macOS** (Mac mini): `numc_launcher.applescript` is the source for a small launcher app compiled with `osacompile -o ~/Applications/Numc.app numc_launcher.applescript`. Double-clicking it starts the GUI detached under `/opt/homebrew/bin/python3`; if the GUI is already running it brings the window to the front instead of starting a second copy (first use of that path asks a one-time Automation consent for System Events). The Desktop "Numc" shortcut is a Finder alias to that app. Notes for rebuilding it:

- The app lives in `~/Applications`, not on the Desktop, to avoid macOS folder-access prompts; the script hardcodes this machine's repo path, so edit `repoDir` if the repo moves.
- Icon: convert `numc.ico` → `.icns` (`sips` → iconset → `iconutil`), copy it over `Contents/Resources/applet.icns`, then `xattr -cr`, `codesign -f -s -`, and set the Finder icon via `NSWorkspace setIcon:forFile:` **last** (that order matters — `xattr -cr`/`codesign` strip or reject a pasted-on icon).
- On an iCloud-synced Desktop the alias itself must also get the icon pasted on (plus `SetFile -a C` to force the custom-icon flag), or it renders generic — and iCloud strips it again after launches. The "Refresh Icons" tool in the `Claude_Python_Testbed` repo (`desktop_launchers/refresh_launcher_icons.command`) re-applies all launcher icons including Numc's; its master image `icon_numc_master.png` lives there.

## Structure and Logic
All solver code lives in `numc.cpp` (~700 lines) — it is the only translation unit in the build (both `numc.vcxproj` and the `Makefile` compile just this file). `numc.h` is an empty placeholder. `numd.cpp` is a stale near-duplicate that is **not** part of the build; make changes in `numc.cpp` only. Visual Studio artifacts are `numc.sln`, `numc.vcxproj`, and `numc.vcxproj.filters`; `numc.py`, `numc.ico`, and `numc_launcher.applescript` are the GUI/launcher layer described above.

### Search enumeration
`main()` reads the six numbers into `av[6]` and the target into `ag`, then runs one block per expression size: `CheckBlock1()` handles the trivial one-number case, and five `CheckBlock(xn, xk, xm, xq, xcn, xci, xt)` calls handle sizes 2–6 with hard-coded parameters per size (operator count, bracketing-table slice, template trim). Each block brute-forces the cross product of:

- **permutations** of the input numbers (`GenerateNextPermutation`, stepping the global `a[]`),
- **operator combinations** over `+ - * /` (`GenerateNextCombination`, stepping `b[]`) — `4^(k-1)` combos for `k` numbers,
- **bracketing shapes** from the `c[42][12]` table — the slice used per size has 1, 2, 5, 14, 42 rows for 2–6 numbers (the Catalan numbers, i.e. every distinct way to parenthesize).

So the 6-number block tests `720 × 4^5 × 42 = 30,965,760` cases, matching the count the program prints; the 5-number block's `2,580,480` likewise.

Each `c[42][12]` row is a postfix (RPN) template string: `'a'` means "push the next number of the current permutation" and a digit means "apply operator `b[digit - ci]`", where `ci` trims the template for smaller expression sizes. Treat the table as a fixed constant — the `CheckBlock` parameters assume its exact layout.

### Evaluation
`TryPermCombBracTriplet()` evaluates the current template on a fixed int stack (`si[]`), applying `Op()` for each operator. `Op()` enforces the game rules: division must be exact, subtraction must not go negative, and any invalid intermediate result becomes the sentinel `ERINT = -6666`, which poisons everything downstream so the case is rejected.

### Canonicalization and deduplication
On a target match, `CreateSortedExpressionString_Enhanced()` builds a *canonical* string so algebraically equivalent expressions collapse to one solution: it flattens chains of `+/-` and `*//` into term lists, sorts the terms by signed value with a string hash (`strHash`) as tie-break, and re-parenthesizes. This canonicalization is the subtle part of the codebase — most deduplication bugs live here. Unique strings are stored in `fs[5000][40]` with hit counters and discovery indices in the parallel `fsn[]`/`fsi1[]`/`fsii[]` arrays (hard caps, no bounds checks), and each block prints its stored solutions at the end.

### Global state
Functions communicate through fixed-size globals rather than parameters, and there is no dynamic allocation anywhere (keep it that way — see `AGENTS.md`): `av[]`/`ag` are the inputs, `a[]`/`b[]` the current permutation/operator combo, `br[]`/`si[]`/`so[]`/`ssp[]` the expression-building stacks, `sbr[][]`/`ssi[][]`/`sso[][]` etc. the parallel stacks used during canonical string construction (index `TMP = 6` is the scratch slot), and `fs[]` and friends the solution store.

## Contributor Docs
- `AGENTS.md` — repository guidelines (style, build, testing conventions).
- `CLAUDE.md` — orientation for Claude Code sessions working in this repo.
