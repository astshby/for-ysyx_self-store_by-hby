# Repository Guidelines

## Project Structure & Module Organization

This workbench combines several teaching projects. `nemu/` contains the emulator (`src/`, `include/`, `configs/`, and tools). `abstract-machine/` provides AM platform code, `klib`, and shared build scripts. Programs and regressions live in `am-kernels/`, particularly `kernels/`, `benchmarks/`, and `tests/`; keep experiments under `tests/self/`. `npc/` holds the developing processor simulator (`csrc/` harnesses and `vsrc/` RTL). `nvboard/` supplies the virtual board, and `fceux-am/` contains the AM NES emulator.

## Build, Test, and Development Commands

Set `YSYX_HOME`, `NEMU_HOME`, and `AM_HOME` to this checkout and its matching subdirectories. Root `make` is coordination/tracing only; build within a subproject.

- `make -C nemu menuconfig`: choose the ISA, engine, and debug features.
- `make -C nemu`: build NEMU; add `run IMG=/path/image.bin` to execute an image.
- `make -C am-kernels/tests/cpu-tests ARCH=riscv32-nemu run`: run CPU regressions.
- `make -C am-kernels/tests/am-tests ARCH=riscv32-nemu run mainargs=t`: run an AM test task.
- `make -C am-kernels/kernels/hello ARCH=native run`: perform a quick host smoke test.
- `make -C nemu clean`: remove NEMU build output; application directories also provide `make clean`.

Use `minirv-nemu` or `minirv-npc` for miniature-ISA exercises. NPC's Makefile and AM NPC run rule are currently scaffolds; complete them before relying on `make sim` or `ARCH=*-npc run`.

## Coding Style & Naming Conventions

Match neighboring code: two-space C/C++ indentation, braces on the declaration line, `snake_case` functions and variables, and uppercase macros/config symbols. Preserve license blocks and keep headers self-contained. Use descriptive lowercase RTL signal names and conventional module filenames. Make recipes require tabs. No repository-wide formatter is configured, so avoid unrelated reformatting.

## Testing Guidelines

Add focused C regressions to the appropriate `am-kernels/tests/` suite; CPU tests are discovered from `tests/*.c`. Run the smallest relevant case first, then the full `cpu-tests` suite for ISA or emulator changes. Test AM/device changes on `native` and the target architecture when supported. Do not commit generated `build/`, `Makefile.*`, binaries, logs, traces, disassemblies, or waveforms unless they are intentional fixtures.

## Source Reading & Learning Guidance

When explaining ysyx source, trace the relevant flow end to end: entry point, call chain, state/data transitions, and exit or error path. Distinguish host mechanisms such as NEMU from guest program behavior, and cite concrete files, symbols, and observed evidence. Start with a compact conceptual map before drilling into implementation details. Preserve the course's learning intent: explain reasoning and debugging methods, suggest the next verification step, and do not modify exercise code unless explicitly requested. Keep guidance aligned with the current lab stage and the long-term goal of completing ysyx without skipping foundational work.

## Commit & Pull Request Guidelines

History uses short subjects such as `modified my info` and scoped summaries like `README: fix broken URL`; infrastructure changes also use `fix:`, `feat:`, and `refactor`. Prefer a concise, imperative subject naming the affected subsystem, and keep commits focused. Pull requests should explain behavior, list exact test commands/results, and link the relevant issue or lab task. Add screenshots or waveforms only when visual or RTL behavior needs them. Never alter root tracing rules marked `DO NOT modify`.
