# AGENTS.md - AMS2 Linux Haptics

Companion tool for Automobilista 2 (AMS2) sim-racing game. Reads shared memory telemetry.

## Agent instructions

- If unsure, ask for clarification
- If a local change is requested, ONLY do that. If the change requires broader edits, ask for confirmation
- DO NOT, under ANY circunstances, make code changes when the user makes a non-coding request (such as "show me all LSP flags")
  - If, while investigating LSP, you notice that the build is broken, TELL the user
- DO NOT delete existing comments
- Improvement plans are welcome - as you make changes, try to leave the codebase better than you found it. But you MUST ask for confirmation first

## Build Commands

Assume the project is already configured at build/. If that's not the case, STOP and tell the user.

```bash
# Configure (assume this was already run)
mkdir -p build && cmake -B build

# Build
cmake --build build/

# clang-format before committing
clang-format -i src/*.{c,h}
```

A clean build is NEVER needed - if you believe the build system is misconfigured and a clean build is needed, STOP and tell the user.

## Code Style

### Language
- C23

### Style
- Variables, functions and types are generally lowercase with underscores (e.g., `read_proc_memory`, `ams2_shmem`)
  - Structs that were imported externally (e.g. `ams2_shmem.h`) are exceptions, and follow the original style

### Project Structure

- `CMakeLists.txt`, `.clang-format` at root
- `src/`
