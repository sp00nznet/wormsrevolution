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

## Phase 4: Boot (DONE — runtime clears the unregistered-function class)

The build is minutes of work; the runtime is the craft. First boot got a long,
**healthy** way in before the expected wall:

- Clean runtime bring-up: D3D12 device (RTX 5070) → FunctionDispatcher → SDL3 +
  MnK input → audio + XMA decoder / audio-worker threads → GPU / VSync threads →
  `Runtime initialized successfully`.
- Mounted `extracted/` as the guest disk; **loaded `default.xex`**; function table
  `0x82180000–0x82E6E79C`, image `0x82000000–0x831E0000`.
- `Initializing shader storage for title 58411290`; resolved XAM party/UI ordinals
  (`XamPartyGetUserList`, `XamShowPartyUI`, …) via generated thunks.
- First crash: `[FATAL] unregistered function at 0x823D0608` — the house-standard
  bring-up class (a target discovery didn't place in a function).

Cleared the whole class in two moves:

1. **Batch vtable/RTTI registration.** `extract_pe.py` can't decode this title's
   LZX variant, so a one-shot `OnPostLoadXexImage` hook (`REX_DUMP_IMAGE`, in
   `src/worms_app.h`) dumps the runtime-decompressed image straight out of guest
   memory. `find_missing_vtable_funcs.py` scanned it → **427 vtable/thunk entries**
   (83 adjustor thunks + 344 function entries) reachable only through data pointers.
   Registered all 427 + `0x823D0608` as `[entrypoint.functions]` hints.
2. **Runtime harvest for computed targets.** Some functions are reached by
   `lis/addi`-computed addresses — invisible to pointer scans. A tolerant indirect
   dispatcher (`src/dispatch_tolerance.cpp`, `-DWORMS_HARVEST=ON`) logs each unique
   unregistered target instead of fataling, so one play-through harvests them all.
   The boot→render path surfaced exactly **3** (`0x82C431D8`, `0x82A42AE8`,
   `0x82A65FA8`); registered them too. Total hints: **444**.

Rebuilt with real dispatch (`WORMS_HARVEST=OFF`): **boots crash-free** into the
render loop.

## Phase 5: Renders the intro (REACHED)

- **Renders the Team17 Digital Ltd intro logo** (`images/team17_splash.png`) and
  advances into the **intro cinematic** — full-motion `.wmv` playback decoding and
  presenting (`images/intro_movie.png`). Front-end sequence is live.
- Known issue: some `PM4_DRAW_INDX` calls report `Vertex fetch constant … invalid`
  and fail in the GPU backend — a subset of geometry isn't drawing yet (the movies
  and logo present fine). Next GPU bring-up target.

### Next up (TODO)

- [ ] GPU: chase the invalid vertex-fetch-constant draws (fetch-constant setup vs.
      backend support for this draw type — `tess_mode=1`, `edram_mode=4`).
- [ ] Front-end past the intro → title → menu (needs interactive input; watch for
      more computed-call targets on menu paths and harvest/register as they appear).
- [ ] Gameplay: 2D physics/terrain, turn loop, weapons. Local play first
      (single-player / hot-seat) to sidestep Xbox Live stubbing.
- [ ] Audio (FMOD `.fev`/`.fsb` banks + XMA), input mapping, DLC packs.
- [ ] Retire the bring-up scaffolds (`REX_DUMP_IMAGE` hook, `dispatch_tolerance`)
      once the reachable paths are complete.
