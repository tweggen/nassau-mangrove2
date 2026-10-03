# Mangrove Plugin Build Guide

Mangrove builds through the shared **[nassau-plugin-sdk](../nassau-plugin-sdk)**,
like every other Nassau plugin. One `nassau_add_plugin()` call emits **VST3, AU
and CLAP** from one set of sources.

## Quick start

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Bundles land in `build/out/`:

```
build/out/MangroveIPlug.vst3        VST3
build/out/MangroveIPlug.component   AU v2   (macOS only)
build/out/MangroveIPlug.clap        CLAP
```

## Where the SDK comes from

`cmake/NassauSDK.cmake` — a verbatim copy of the SDK's canonical locator
(`cmake/bootstrap/NassauSDK.cmake` there; re-copy it rather than editing this
one) — resolves the SDK in a fixed order:

1. an explicit `-DNASSAU_SDK_DIR=...` (what the `nassau-suite` superbuild passes);
2. a sibling checkout `../nassau-plugin-sdk`;
3. `FetchContent` from the SDK's git remote (slow last resort; disable with
   `-DNASSAU_SDK_ALLOW_FETCH=OFF`).

Finding no SDK is **not** an error: `NASSAU_PLUGIN_TARGETS_POSSIBLE` goes false
and the DSP core and tests still build. That is what a DSP-only CI job uses:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNASSAU_SDK_DIR=/nonexistent
```

The SDK supplies the vendored iPlug2 + VST3 SDK, the prebuilt Skia libraries for
the UI build, `nassau_add_plugin()`, and the shared project prologue (toolchain
flags, OBJC/OBJCXX enablement, the MSVC `/MT` runtime selection).

**Provisioning**, once per SDK checkout:

```sh
cd ../nassau-plugin-sdk
git submodule update --init --recursive
cmake -P cmake/ProvisionDeps.cmake     # VST3 SDK link, CLAP SDK + helpers
./scripts/fetch-skia.sh                # prebuilt Skia for the UI (macOS/arm64)
```

Without Skia the plugin still builds, headless (no editor). Force that
explicitly with `-DNASSAU_FORCE_HEADLESS=ON`.

## What this replaced

Until the SDK migration this repo carried its own copies of everything the SDK
now owns, and it is worth knowing what went where:

| was | now |
|---|---|
| `external/iplug2` + `external/vst3sdk` submodules, **9 GB on disk** | the SDK's, pinned at the identical commits (`7dfe7a96d`, `58f8da7`) |
| `Source/Plugin/CMakeLists.txt`, 212 lines declaring iPlug2 sources, include paths and per-format definitions by hand | 1 `nassau_add_plugin()` call, 30 lines with its comment |
| `Source/VST3/`, a second, older hand-rolled VST3 target with its own duplicate `MangrovePlugin.cpp` and `config.h` | removed; `Source/Plugin` is the one plugin |
| VST3 + AU (and the AU target was disabled behind `if(FALSE)`) | VST3 + AU + **CLAP**, all three building |

The old `Source/Plugin/CMakeLists.txt` justified itself with *"We do NOT use
IPlug2's own CMake because it is Xcode/VS-primary"* — true when written, and no
longer: the SDK drives iPlug2's CMake on macOS and Windows for three other
plugins.

### Historical build notes

`docs/BUILDING.md`, `docs/BUILDING_WIN11.md`, `PLUGIN_TEMPLATE_GUIDE.md` and
`VST3_FACTORY_FIX_SUMMARY.md` describe the pre-SDK layout. They are kept for the
history and are **superseded by this file**.
