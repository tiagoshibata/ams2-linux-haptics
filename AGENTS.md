# AGENTS.md - AMS2 Linux Haptics

Companion tool for Automobilista 2 (AMS2) sim-racing game. Reads shared memory telemetry.

## Agent instructions

- If unsure, ask for clarification
- If a local change is requested, ONLY do that. If the change requires broader edits, ask for confirmation
- Improvements are welcome - as you plan and make changes, try to leave the code better than you found it. But you must ask for confirmation first

## Build Commands

```bash
# Configure
mkdir -p build && cmake -B build

# Build
cmake --build build/

# clang-format before committing
clang-format -i src/*.{c,h}
```

A clean build is NEVER needed - if the build system is misconfigured and you believe a clean build is needed, stop and throw an error.

## Code Style

### Language
- C23

### Style
- Variables, functions and types are generally lowercase with underscores (e.g., `read_proc_memory`, `ams2_shmem`)
- Structs that were imported externally (e.g. `ams2_shmem.h`) might have exceptions
- Use `#pragma once` in headers

### Project Structure

- `CMakeLists.txt`, `.clang-format` at root
- `src/` has source files
