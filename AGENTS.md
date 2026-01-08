# Repository Guidelines

## Project Structure & Module Organization
This repository is a small C/C++ console program built with Visual Studio. Core logic lives in `numc.cpp` with declarations in `numc.h`. The `numd.cpp` file is a near-duplicate variant used for alternate builds or experiments. Visual Studio artifacts are in `numc.sln`, `numc.vcxproj`, and `numc.vcxproj.filters`. Build outputs typically land under `x64/` (e.g., `x64/Release`).

## Build, Test, and Development Commands
- Visual Studio: open `numc.sln` and build `Release|x64` (or `Debug|x64`).
- MSBuild (Developer Command Prompt):
  `msbuild numc.sln /p:Configuration=Release /p:Platform=x64`
- Run locally after build:
  `x64/Release/numc.exe 100 75 25 10 7 5 666`
  (expects 6 inputs plus a target; no argument validation is currently performed).

## Coding Style & Naming Conventions
Code uses 4-space indentation and braces on the same line as control statements. Function names are in PascalCase (`TryPermCombBracTriplet`, `CheckBlock`), while globals and locals are short lowercase (`av`, `ag`, `si`). Constants are uppercase (`ERINT`). Keep arrays fixed-size and avoid adding dynamic allocation unless needed for performance or correctness. There is no enforced formatter or linter.

## Testing Guidelines
There is no automated test suite. Validate changes by building and running the executable with known inputs (see the README example). If you modify core logic, verify that the program prints all solutions for a known target and that invalid operations (divide by zero, non-integer division) are still rejected.

## Commit & Pull Request Guidelines
Recent commits are short, capitalized summaries (e.g., `Update README.md`) and sometimes include a device/date stamp (e.g., `Binda Windows Desktop 23122023 16:34`). Follow this concise style and include dates only when meaningful. For PRs, include a brief description, build configuration used, example input/output, and note any behavior changes. Screenshots of console output are welcome for larger changes.
