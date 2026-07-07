# Worms Revolution — Development Progress

Toolchain: **ReXGlue SDK v0.8.0** (self-contained; no XenonRecomp).

## Phase 1: Extraction & Triage (DONE)

- Source package: a **Games on Demand** container (title ID `58411290`, Team17, 2012),
  shipped as an STFS package `58411290/000D0000/8F1CFE70…` plus three DLC packs
  under `00000002/` (AllTheFunOfTheFair, MissionsOnMars, QuestPack).
- `extract_stfs.py` → **170 files** to `extracted/` (default.xex, `data.zip`, audio banks,
  movies, achievement art). STFS nested a dir; flattened back up.
- `xex_info.py` triage:
  - Image base `0x82000000` (standard — runtime handles this class cleanly).
  - Image size `0x11E0000` (**17.9 MB** — a full retail disc title, ~2× a typical XBLA game).
  - Imports: `xam.xex`, `xboxkrnl.exe` only. No XNET/Live import wall.

## Phase 2: Scaffold & Codegen (DONE)

- `rexglue init` → `project/` (manifest, CMake, `src/main.cpp`, `src/worms_app.h`).
- `rexglue codegen`:
  - First pass: **13 `UnresolvedCall`** errors — tail-call branch targets landing
    outside any discovered function (three clusters: `0x8240Exxx`, `0x829Bxxxx`, singles).
  - Added all 13 as `[entrypoint.functions]` hints (entry-only; discovery sizes them).
  - Second pass: **clean**. A handful of non-fatal "unresolved conditional branch"
    Write-phase notes remain (intra-function boundary heuristics — harmless).
  - Output: **110 `.cpp` files, ~225 MB, 88,816 recompiled functions.**

## Phase 3: Build (DONE)

- Dropped in the toolkit's `stubs.cpp` (the `XUsbcam*` bundle — the near-universal
  first link blocker) and wired it into `CMakeLists.txt` up front.
- `cmake --preset win-amd64-release` against the v0.8.0 SDK install: **configured**,
  found ReXGlue SDK 0.8.0. Clang 21 / Ninja / VS2022 toolchain.
- Compiled the 110 recomp translation units and **linked clean on the first try** —
  the pre-emptive `XUsbcam*` stub was the only gap: **`worms.exe` (74 MB)**.

## Phase 4: First boot (REACHED — first bring-up crash)

The build is minutes of work; the runtime is the craft. First boot gets a long,
**healthy** way in before the expected unregistered-function wall:

- Clean runtime bring-up: D3D12 device (RTX 5070) → FunctionDispatcher → SDL3 +
  MnK input → audio + XMA decoder / audio-worker threads → GPU / VSync threads →
  `Runtime initialized successfully`.
- Mounted `extracted/` as the guest disk; **loaded `default.xex`**; function table
  `0x82180000–0x82E6E79C`, image `0x82000000–0x831E0000`.
- `Initializing shader storage for title 58411290`; resolved XAM party/UI ordinals
  (`XamPartyGetUserList`, `XamShowPartyUI`, …) via generated thunks.
- **First crash:** `[FATAL] Call to invalid or unregistered function at
  0x823D0608`. This is the house-standard bring-up class — a branch/pointer target
  discovery didn't place in a function.

### Next up (TODO)

- [ ] Resolve the unregistered-function class. Single-register `0x823D0608` to
      advance, but the durable fix is the batch move: dump the runtime-decompressed
      image via a one-shot `OnPostLoadXexImage` hook, run
      `find_missing_vtable_funcs.py`, and inject the whole vtable/thunk cluster as
      `[entrypoint.functions]` hints (see the Lumines writeup).
- [ ] `data.zip` is the bulk of game content — confirm mount + path resolution.
- [ ] Front-end: Team17 / publisher movies (`.wmv`) → title → menu.
- [ ] Gameplay: 2D physics/terrain, turn loop, weapons. Local play first
      (single-player / hot-seat) to sidestep Xbox Live stubbing.
- [ ] Audio (FMOD `.fev`/`.fsb` banks + XMA), input mapping, DLC packs.
