# Claude Project Context: Mangrove Plugin Refactoring

**Project:** Mangrove Compressor Plugin — JUCE to Native Formats  
**Owner:** Timo Weggen (timo.weggen@gmail.com)  
**Status:** 🔄 Phase 5 (GUI Implementation) — In Progress  
**Last Updated:** May 24, 2026

---

## Quick Start

### What is this project?
Refactoring the Mangrove compression plugin from JUCE-based code into native plugin formats (VST 3, AudioUnit, LADSPA). The goal is to remove JUCE dependencies and provide multi-platform support.

### Current state?
- **Phases 1–4:** Complete (DSP core, IIR filters, sample rate handling, VST 3 wrapper)
- **Phase 5:** In progress (GUI implementation)
- **Timeline:** 4 weeks elapsed, ~6 weeks remaining (on schedule)

### How do I get oriented?
1. Read: `ai/plans/STATE.md` (current snapshot — 5 min) ⭐ Start here
2. Review: `ai/plans/proposed/IMPLEMENTATION_PLAN.md` (overall strategy — 10 min)
3. Check: `ai/plans/proposed/PHASE_5_GUI_IMPLEMENTATION.md` (current work — 10 min)
4. Reference completed work: `ai/plans/done/PHASE_*.md` (completed phases)
5. Build: `mkdir -p build_phase5 && cd build_phase5 && cmake .. && cmake --build .`

---

## Project Structure

```
mangrove-refactored/
├── Source/
│   ├── DSP/                       ✅ Complete (Phases 1–3)
│   │   ├── compressor_chain.h     (207 lines — API)
│   │   ├── compressor_chain.cpp   (368 lines — impl)
│   │   ├── iir_filter.h           (custom Butterworth)
│   │   └── iir_filter.cpp
│   ├── VST3/                      ✅ Complete (Phase 4)
│   │   ├── MangrovePlugin.h
│   │   └── MangrovePlugin.cpp
│   └── Plugin/                    🔄 In Progress (Phase 5)
│       ├── MangroveUI.h
│       ├── MangroveUI.cpp         (GUI implementation)
│       ├── MangrovePlugin.h
│       └── MangrovePlugin.cpp
├── Tests/
│   ├── dsp_tests.cpp              (40 passing tests)
│   └── CMakeLists.txt
├── CMakeLists.txt                 (root build config)
├── CLAUDE.md                      ← You are here
├── ai/plans/
│   ├── STATE.md                   (current execution state)
│   └── proposed/
│       ├── IMPLEMENTATION_PLAN.md  (main strategy doc)
│       ├── plan_summary.md         (quick reference)
│       ├── PHASE_1_DSP_EXTRACTION.md
│       ├── PHASE_2_IIR_REPLACEMENT.md
│       ├── PHASE_3_SAMPLE_RATE_FIX.md
│       ├── PHASE_4_VST3_WRAPPER.md
│       ├── PHASE_5_GUI_IMPLEMENTATION.md ← Current work
│       ├── PHASE_6_SERIALIZATION.md
│       ├── PHASE_7_AUDIOUNIT.md
│       ├── PHASE_8_LADSPA.md
│       ├── PHASE_9_TESTING.md
│       ├── PHASE_10_PACKAGING.md
│       └── OPTIONAL_PLATFORMS.md
├── build/                         (debug build)
└── build_phase5/                  (latest Phase 5 build)
```

---

## The Audio Processing Core

### CompressorChain API

**File:** `Source/DSP/compressor_chain.h`

The main audio engine is the `CompressorChain` class — a lock-free, dependency-free C++17 library.

```cpp
// Initialize with sample rate
void initialize(float sampleRate);

// Process audio (per-block, stereo)
void process(const float* inL, const float* inR, 
             float* outL, float* outR, int numSamples);

// 14 parameters (atomic, lock-free)
setInputGain(float dB);
setInputSaturation(float amount);
setLevelThreshold(float dB);
setLevelRatio(float ratio);
setLevelAttack(float ms);
setLevelRelease(float ms);
... (8 more)

// Metering
void getMeterData(float& inputGain, float& levelReduction, 
                  float& densityReduction);
```

**Key Properties:**
- **Latency:** 0 samples (per-sample processing)
- **Thread Safety:** Lock-free parameter updates (std::atomic)
- **Sample Rate:** Dynamic recalculation (44.1, 48, 96, 192 kHz)
- **No Dependencies:** Zero external libs in DSP core

### Signal Chain

```
Input Stereo
  ↓
[Input Stage]
  - Gain (dB→linear)
  - Saturation (soft clipping)
  - HPF (custom Butterworth, Phase 2)
  ↓
[Level Compressor]
  - Sidechain detection (RMS)
  - Envelope follower
  - Compression curve (hard-knee or vari-mu)
  - Attack/release ramping
  - Optional tube saturation
  ↓
[Density Compressor]
  - Peak detection (instantaneous)
  - Fast envelope (transient limiter)
  - Soft or hard ceiling
  ↓
Output Stereo
```

---

## Building & Testing

### Build VST 3 (Phase 5)
```bash
cd build_phase5
cmake ..
cmake --build . --config Release

# Test plugin
./Builds/MangroveIPlug_VST3  # Binary location (platform-dependent)
```

### Test DSP Core
```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest --verbose  # Should show 40/40 passing
```

### Verify Builds
- ✅ **Windows 11 VST 3** — Compiles cleanly, runs in DAWs
- ✅ **macOS VST 3** — Compiles cleanly, runs in DAWs
- ✅ **macOS AU v3** — Also available via IPlug2
- ⏳ **Linux VST 3** — Not yet tested

---

## Current Work: Phase 5 (GUI Implementation)

**Objective:** Implement parameter UI using IGraphics framework

**Status:** ~60% complete (as of May 24)

### Key Files
- `Source/Plugin/MangroveUI.h` — GUI class definition
- `Source/Plugin/MangroveUI.cpp` — GUI implementation (in progress)
- `Source/Plugin/config.h` — IGraphics configuration

### Recent Changes
- `e0f94c8` (May 21): Initial IPlug2 GUI wrapper setup
- `9645a2f` (May 24): Added "Fast" toggle for 0-sample attack reaction

### Next Steps
1. **Complete parameter binding** (sliders ↔ CompressorChain)
2. **Real-time meter display** (input RMS, compression reduction)
3. **UI responsiveness testing** (CPU load verification)
4. **Cross-platform testing** (Windows/macOS)
5. **DAW integration** (Reaper, Studio One)

### Common Issues & Solutions
- **Meter lag:** Meters update via `MeterData` struct, call `getMeterData()` at UI refresh (~50 Hz)
- **Parameter binding:** Use `setParameter()` → `setInputGain()` mapping
- **Thread safety:** All parameter updates are atomic, no locking needed

---

## Development Guidelines

### Code Style
- **Classes:** `CamelCase`
- **Methods:** `camelCase`
- **Members:** `_leadingUnderscore` for private
- **Comments:** Only for non-obvious WHY (not WHAT)

### Testing Requirements
- Unit tests required for new components
- DSP changes need regression tests (golden audio comparison)
- All tests must pass before merge

### Commit Guidelines
- Incremental commits per task (not one mega-commit)
- Prefix: `Phase N, Task N.M: Description`
- Example: `Phase 5, Task 5.2: Bind slider to parameter setters`

### No Breaking Changes
- DSP API (`compressor_chain.h`) is frozen — additions only
- Existing parameter semantics must be preserved
- Audio output must match original within ±0.01 dB RMS

---

## Common Tasks

### Add a New Parameter
1. Add to CompressorChain API: `void setNewParam(float value);`
2. Add internal state: `std::atomic<float> _newParam{default};`
3. Update `process()` to use parameter
4. Add test case in `Tests/dsp_tests.cpp`
5. Add UI slider in `MangroveUI.cpp`

### Debug Audio Issues
1. Check `getMeterData()` — are meters showing expected values?
2. Verify parameter ranges — check min/max in `compressor_chain.h`
3. Look for saturation — check `Source/DSP/compressor_chain.cpp` lines 155–222
4. Check sample rate — ensure `initialize()` called at plugin load

### Performance Optimization
- Profile with: `cmake --build . --config Release && perf/profiling`
- Lock-free design already optimized (no locks in audio thread)
- Avoid std::pow() in inner loops (pre-calculate coefficients)

---

## Testing Strategy

### Unit Tests (Phase 1)
- File: `Tests/dsp_tests.cpp`
- Status: 40/40 passing ✅
- Coverage: Initialization, compression curves, edge cases, meters

### Regression Tests (Phase 9)
- Compare audio output to original JUCE version
- Tolerance: ±0.01 dB RMS
- Golden audio files: `Tests/fixtures/`

### Cross-DAW Testing (Phase 9)
- Reaper (all platforms)
- Studio One (Windows/macOS)
- Logic Pro (macOS, AU v3)
- Ableton Live (not yet specified)

### Performance Tests (Phase 10)
- CPU usage <5% per instance (estimated)
- Memory: No allocations in audio path
- Latency: 0 samples guaranteed

---

## Architecture Decisions

### Why IPlug2?
- Reduces boilerplate (~4 weeks saved)
- Single codebase for VST 3, AU, LADSPA
- Built-in GUI framework (IGraphics)
- Active community support

### Why Lock-Free?
- Audio thread must never block
- std::atomic<float> is cheap (1–2 clock cycles)
- Parameter updates from UI happen without audio dropouts

### Why Per-Sample Processing?
- Exact coefficient recalculation per sample
- No block-based latency
- Matches original JUCE behavior exactly

### Why Custom IIR Filter?
- Zero JUCE dependencies (Phase 2 goal)
- Butterworth provides smooth response for compression sidechain
- Minimal CPU cost vs. high-order filters

---

## Phase Breakdown (Remaining Work)

### Phase 5: GUI Implementation (Current, ETA May 30)
- **Tasks:** Parameter binding, meter display, responsiveness
- **Blocker:** None identified
- **Estimate:** 2–3 weeks (1 week actual so far)

### Phase 6: Serialization (Start ~May 31)
- **Tasks:** Binary + JSON preset save/load
- **Blocker:** None identified
- **Estimate:** 1–2 weeks

### Phase 7: AudioUnit v3 (Start ~June 7)
- **Tasks:** AUv3 wrapper, Swift UI, Logic Pro testing
- **Blocker:** macOS 13+ required
- **Estimate:** 4–6 weeks

### Phase 8: LADSPA (Start ~Mid-June)
- **Tasks:** LADSPA wrapper, Jack testing
- **Blocker:** None identified
- **Estimate:** 1–2 weeks

### Phases 9–10: Testing & Packaging
- **Timeline:** Mid-June through mid-August
- **Deliverables:** Installers, code signing, documentation

---

## Important URLs & References

### Documentation
- Current state: `ai/plans/STATE.md` ⭐ Start here
- Main plan: `ai/plans/proposed/IMPLEMENTATION_PLAN.md`
- Current phase: `ai/plans/proposed/PHASE_5_GUI_IMPLEMENTATION.md`
- Completed phases: `ai/plans/done/PHASE_*.md` (reference)
- Planned phases: `ai/plans/todo/PHASE_*.md` (next work)

### Code References
- DSP API: `Source/DSP/compressor_chain.h` (207 lines, well-documented)
- Implementation: `Source/DSP/compressor_chain.cpp` (368 lines)
- Tests: `Tests/dsp_tests.cpp` (40 test cases)

### External Resources
- IPlug2: https://github.com/iPlug2/iPlug2
- VST 3 SDK: https://github.com/steinbergmedia/vst3sdk
- Original JUCE code: `Source/Original/PluginProcessor.cpp` (reference only)

---

## Troubleshooting

### Build Fails
1. Check CMake version: `cmake --version` (need 3.15+)
2. Check C++ standard: `-DCMAKE_CXX_STANDARD=17`
3. Clean build: `rm -rf build_phase5 && mkdir build_phase5 && cd build_phase5 && cmake .. && cmake --build .`

### Tests Fail
1. Verify compressor_chain.cpp is compiled: `ls -la build_phase5/CMakeFiles/compressor_chain.dir/`
2. Check linking: `cmake --build . --verbose | grep -i error`

### Plugin Not Recognized by DAW
1. Ensure VST 3 SDK is found: Check CMake output for VST paths
2. Verify binary location: `find build_phase5 -name "*.vst3"`
3. Check DAW scan: May need to point DAW to build output directory

### Audio Issues
1. Check parameters are being set: Add debug output to `process()`
2. Verify meter values: Call `getMeterData()` and check ranges
3. Look for saturation: Check input stage (lines 155–222 in compressor_chain.cpp)

---

## Contact & Questions

### Documentation Issues
- Ambiguous task description? → Refer to `IMPLEMENTATION_PLAN.md` dependencies section
- Missing context? → Read corresponding `PHASE_N.md` document
- Build failure? → Check CMakeLists.txt and compiler flags

### For Next Developer
1. This file (CLAUDE.md) is your entry point
2. Read `ai/plans/STATE.md` for current execution status
3. Read `ai/plans/proposed/PHASE_5_GUI_IMPLEMENTATION.md` for detailed tasks
4. Start from most recent commit and work forward

---

## Version History

| Date | Changes |
|------|---------|
| May 24, 2026 | CLAUDE.md created with full project context |
| May 21, 2026 | Phase 4 complete, Phase 5 begun |
| May 18, 2026 | Phase 3 complete (sample rate handling) |
| May 13, 2026 | Phase 2 complete (IIR filters) |
| May 6, 2026 | Phase 1 complete (DSP extraction) |

---

**Status:** 🟢 Green (on track, no blockers)  
**Next Action:** Continue Phase 5 GUI implementation, aiming for May 30 completion  
**Maintained by:** Timo Weggen (timo.weggen@gmail.com)
