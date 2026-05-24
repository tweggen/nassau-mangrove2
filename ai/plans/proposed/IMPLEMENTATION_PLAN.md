# Mangrove Plugin Refactoring: Complete Implementation Plan

**Project Duration:** 20–30 weeks  
**Target:** Multi-platform native plugins (VST 3, AudioUnit, LADSPA)  
**Status:** 🔄 Active Execution — Phase 5 (GUI Implementation) in progress  
**Started:** May 2026 | **Current:** May 24, 2026 | **Elapsed:** ~4 weeks

---

## Project Overview

This document defines a phased approach to refactor the Mangrove compression plugin from JUCE-dependent code to native plugin formats. Each phase is self-contained, produces deliverables, and is designed to be executable by agents or developers independently.

### Core Phases

| Phase | Deliverable | Duration | Status | Completed |
|-------|-------------|----------|--------|-----------|
| **[1. DSP Extraction](./done/PHASE_1_DSP_EXTRACTION.md)** | `CompressorChain` library (platform-independent) | 2–3 weeks | ✅ Complete | ~May 6 |
| **[2. IIR Filter Replacement](./done/PHASE_2_IIR_REPLACEMENT.md)** | Custom Butterworth filter implementation | 1–2 weeks | ✅ Complete | ~May 13 |
| **[3. Sample Rate Handling](./done/PHASE_3_SAMPLE_RATE_FIX.md)** | Dynamic sample-rate coefficient recalculation | 1–2 weeks | ✅ Complete | ~May 18 |
| **[4. VST 3 Wrapper (IPlug2)](./done/PHASE_4_VST3_WRAPPER.md)** | Cross-platform VST 3 plugin (Windows/macOS/Linux) | 3–4 weeks | ✅ Complete | ~May 21 |
| **[5. GUI Implementation](./PHASE_5_GUI_IMPLEMENTATION.md)** | IGraphics-based parameter UI + metering | 2–3 weeks | 🔄 In Progress | ETA May 30 |
| **[6. Serialization](./todo/PHASE_6_SERIALIZATION.md)** | Binary + JSON preset system | 1–2 weeks | 📋 Planned | — |
| **[7. AudioUnit (AUv3)](./todo/PHASE_7_AUDIOUNIT.md)** | Native macOS AU plugin | 4–6 weeks | 📋 Planned | — |
| **[8. LADSPA (Bonus)](./todo/PHASE_8_LADSPA.md)** | Linux LADSPA plugin wrapper | 1–2 weeks | 📋 Planned | — |
| **[9. Testing & Validation](./todo/PHASE_9_TESTING.md)** | Regression tests, DSP verification, cross-DAW testing | 3–4 weeks | 📋 Planned | — |
| **[10. Packaging & Release](./todo/PHASE_10_PACKAGING.md)** | Builds, codesigning, distribution setup | 2–3 weeks | 📋 Planned | — |

---

## Phase Dependencies

```
Phase 1 (DSP Extraction)
  └─> Phase 2 (IIR Filter)
       └─> Phase 3 (Sample Rate)
            └─> Phase 4 (VST 3 Wrapper) ──> Phase 5 (GUI) ──> Phase 6 (Serialization)
            │
            └─> Phase 7 (AudioUnit)
            │
            └─> Phase 8 (LADSPA)
                │
                └─> Phase 9 (Testing)
                     └─> Phase 10 (Packaging)
```

**Critical Path:** Phases 1–3 → Phase 4 (VST 3 is primary target)  
**Optional Extensions:** Phase 7 (AUv3) and Phase 8 (LADSPA) can run in parallel after Phase 6

---

## Directory Structure (Post-Implementation)

```
mangrove-refactored/
├── CMakeLists.txt                 # Build configuration
├── Source/
│   ├── DSP/
│   │   ├── compressor_chain.h     # Main CompressorChain class
│   │   ├── compressor_chain.cpp
│   │   ├── iir_filter.h           # Custom IIR implementation
│   │   └── iir_filter.cpp
│   ├── VST3/
│   │   ├── MangroveVST3.h
│   │   ├── MangroveVST3.cpp
│   │   └── gui/
│   │       ├── gui_main.cpp       # IGraphics setup
│   │       └── gui_controls.cpp   # Slider/meter widgets
│   ├── AU/
│   │   ├── MangroveAUv3.swift
│   │   └── MangroveAUv3UI.swift
│   └── LADSPA/
│       ├── MangroveLADSPA.cpp
│       └── MangroveLADSPA.h
├── Tests/
│   ├── dsp_tests.cpp              # Unit tests for DSP
│   ├── iir_tests.cpp
│   └── fixtures/
│       ├── test_audio.wav
│       └── golden_output.wav
├── Presets/
│   ├── defaults.json
│   └── examples/
└── Builds/                         # Build outputs
    ├── VST3/
    ├── AU/
    └── LADSPA/
```

---

## Development Guidelines for Agents

### Code Quality Standards

- **No external dependencies** in DSP library (compressor_chain.[h/cpp])
- **C++11 compatible** minimum (C++17 preferred for std::atomic)
- **Lock-free audio path:** Parameters use `std::atomic<float>`, no locks in `process()`
- **Unit tests required:** Every new component must have corresponding test
- **Documentation:** Function headers + algorithm comments for non-obvious math

### Naming Conventions

- **Classes:** `CamelCase` (e.g., `CompressorChain`, `IIRFilter`)
- **Methods:** `camelCase` (e.g., `processBlock()`, `setInputGain()`)
- **Constants:** `SCREAMING_SNAKE_CASE` (e.g., `MAX_FILTER_ORDER`)
- **Member variables:** `_leadingUnderscore` for private (e.g., `_sampleRate`)
- **Parameters:** `camelCase` (e.g., `sampleRate`, `inputGain`)

### Testing Requirements

Each phase includes:
- **Unit tests** (isolated component behavior)
- **Integration tests** (multi-component interaction)
- **Regression tests** (output matches original)
- **Edge case tests** (silence, extreme parameters, sample-rate changes)

### Performance Targets

- **Audio processing:** Deterministic, no allocation in hot path
- **Parameter updates:** Lock-free, <1 sample latency
- **GUI updates:** 20 Hz meter refresh (non-blocking)
- **Memory:** ~10 KB per instance (state + filters)

---

## Common Tasks Across All Phases

### Code Review Checklist (Before Phase Completion)

- [ ] All functions have docstring headers
- [ ] All magic numbers documented or named as constants
- [ ] No JUCE dependencies in DSP files
- [ ] Memory leaks checked (Valgrind/Address Sanitizer)
- [ ] Compiler warnings cleaned (Level 4/all)
- [ ] Thread safety verified (atomic operations, no mutex)
- [ ] Sample-rate and time handling uses `double`, not hardcoded values
- [ ] Tests pass on all target platforms (Windows, macOS, Linux)

### Build & Environment Setup

Each phase begins with:
1. Create source files in correct location (see directory structure)
2. Add CMake targets / build configurations
3. Write initial unit tests (TDD approach)
4. Verify builds without warnings on all platforms

### Documentation

Each phase produces:
- **README.md** (what was built, how to use it)
- **TESTING.md** (test procedures, validation criteria)
- **CHANGELOG.md** (changes from original JUCE version)

---

## Integration Checkpoints

### After Phase 3 (Sample Rate Fix)
**Verify:** CompressorChain operates identically to JUCE version across 44.1/48/96 kHz
- Golden audio file comparison (impulse, sweep, complex signal)
- RMS error < 0.1 dB across all sample rates

### After Phase 4 (VST 3)
**Verify:** Plugin loads, processes audio in Reaper/Studio One
- Audio output bit-exact (or within float rounding error)
- Parameters automatable, respond in real-time
- No audio glitches, clicks, or dropouts

### After Phase 6 (Serialization)
**Verify:** Save preset → reload → verify all 14 parameters match
- Test JSON and binary formats
- Cross-version compatibility

### After Phase 9 (Testing)
**Verify:** Regression test suite passes on Windows, macOS, Linux
- CPU usage < 10% per instance (reasonable load)
- Latency < 1 sample for parameter changes

---

## Branching Strategy

```
main (original JUCE code)
  ├── dsp-extraction (Phase 1–3: Core DSP)
  │    └── iir-filter (Phase 2)
  │    └── sample-rate-fix (Phase 3)
  │
  ├── vst3-integration (Phase 4–6: VST 3 plugin)
  │    └── iplug2-setup (Phase 4)
  │    └── gui-implementation (Phase 5)
  │    └── serialization (Phase 6)
  │
  ├── au-integration (Phase 7: AudioUnit)
  │
  ├── ladspa-integration (Phase 8: LADSPA)
  │
  └── release-v2.0 (Phase 10: Final release)
```

Each phase merges into `main` after testing.

---

## Resource Requirements

### Tools & Software
- **C++ Compiler:** GCC 7+, Clang 5+, MSVC 2017+
- **CMake:** 3.15+
- **Test Framework:** Catch2 (header-only, included)
- **IPlug2:** Git submodule (or downloaded separately)
- **JUCE (reference only):** For comparison, not linked

### Hardware
- **Development Machine:** macOS + Windows VM (or parallel machines)
- **Test DAWs:** Reaper (required), Studio One (recommended), Logic Pro (macOS)
- **Audio Interface:** Optional (testing with dummy audio device sufficient)

### Team Allocation
- **Phase 1–3:** 1 senior engineer (architecture + DSP understanding)
- **Phase 4–6:** 1 engineer (VST 3 + GUI experience)
- **Phase 7–8:** 1 engineer (platform-specific: AU for macOS person, LADSPA for Linux)
- **Phase 9–10:** 1 QA engineer + release manager

---

## Success Criteria

### Per-Phase Success
Each phase is **complete** when:
1. All planned tasks finished and code merged
2. Unit tests pass with 100% coverage of new code
3. No compiler warnings (Level 4)
4. Code review approved
5. Acceptance criteria (in phase document) met

### End-to-End Success
Full project **complete** when:
1. VST 3 plugin builds and runs on Windows + macOS
2. Presets save/load without data loss
3. Regression test suite passes (output ≤ 0.1 dB RMS error)
4. Cross-DAW testing: Plugin works in Reaper, Studio One, Logic
5. No memory leaks detected (Valgrind/ASan)
6. Codesigned and notarized (macOS)
7. Installer packages created (Windows/macOS)
8. Documentation complete

---

## Phase Descriptions (Quick Reference)

### Phase 1: DSP Extraction
**Goal:** Create `CompressorChain` class with no JUCE dependencies  
**Input:** Original `PluginProcessor.cpp` (850 lines of processBlock)  
**Output:** `compressor_chain.h/.cpp` (reusable DSP library)  
[→ Read [PHASE_1_DSP_EXTRACTION.md](./PHASE_1_DSP_EXTRACTION.md)]

### Phase 2: IIR Filter Replacement
**Goal:** Implement custom Butterworth high-pass filter  
**Input:** JUCE IIRFilter usage in compressor_chain  
**Output:** Custom `iir_filter.h/.cpp` (no JUCE dependency)  
[→ Read [PHASE_2_IIR_REPLACEMENT.md](./PHASE_2_IIR_REPLACEMENT.md)]

### Phase 3: Sample Rate Handling
**Goal:** Replace hardcoded 44.1 kHz with dynamic sample-rate calculation  
**Input:** CompressorChain with fixed time coefficients  
**Output:** Dynamic coefficient recalculation, tested at 44.1/48/96 kHz  
[→ Read [PHASE_3_SAMPLE_RATE_FIX.md](./PHASE_3_SAMPLE_RATE_FIX.md)]

### Phase 4: VST 3 Wrapper
**Goal:** Wrap CompressorChain in IPlug2 VST 3 framework  
**Input:** CompressorChain library  
**Output:** Loadable `.vst3` plugin (Windows/macOS/Linux)  
[→ Read [PHASE_4_VST3_WRAPPER.md](./PHASE_4_VST3_WRAPPER.md)]

### Phase 5: GUI Implementation
**Goal:** Build parameter UI with IGraphics (sliders, toggles, meters)  
**Input:** VST 3 wrapper from Phase 4  
**Output:** Interactive plugin UI matching original appearance  
[→ Read [PHASE_5_GUI_IMPLEMENTATION.md](./PHASE_5_GUI_IMPLEMENTATION.md)]

### Phase 6: Serialization
**Goal:** Implement preset save/load (binary + JSON formats)  
**Input:** VST 3 plugin with parameters  
**Output:** Preset system compatible with original JUCE presets (if possible)  
[→ Read [PHASE_6_SERIALIZATION.md](./PHASE_6_SERIALIZATION.md)]

### Phase 7: AudioUnit (AUv3)
**Goal:** Wrap CompressorChain in AudioUnit v3 framework (macOS)  
**Input:** CompressorChain library, GUI design  
**Output:** Loadable `.auv3` plugin for Logic Pro, Final Cut, etc.  
[→ Read [PHASE_7_AUDIOUNIT.md](./PHASE_7_AUDIOUNIT.md)]

### Phase 8: LADSPA (Linux)
**Goal:** Wrap CompressorChain in LADSPA framework  
**Input:** CompressorChain library  
**Output:** Loadable `.so` plugin for Linux DAWs (JACK, Ardour, etc.)  
[→ Read [PHASE_8_LADSPA.md](./PHASE_8_LADSPA.md)]

### Phase 9: Testing & Validation
**Goal:** Comprehensive regression and cross-platform testing  
**Input:** All plugins (VST 3, AU, LADSPA)  
**Output:** Test suite, validated build artifacts  
[→ Read [PHASE_9_TESTING.md](./PHASE_9_TESTING.md)]

### Phase 10: Packaging & Release
**Goal:** Create distribution packages, codesigning, documentation  
**Input:** Validated plugins and test results  
**Output:** Installer packages, releases, user documentation  
[→ Read [PHASE_10_PACKAGING.md](./PHASE_10_PACKAGING.md)]

---

## Communication & Handoffs

When passing work between agents or team members:

1. **Check phase prerequisites:** Verify all dependencies are complete
2. **Review acceptance criteria:** Ensure previous phase delivered all requirements
3. **Read phase document:** Detailed tasks and context in dedicated file
4. **Run tests:** Execute phase tests before starting new phase
5. **Document blockers:** If stuck, note in phase document for next person
6. **Merge & notify:** Merge completed phase, update status in IMPLEMENTATION_PLAN.md

---

## Known Risks & Mitigation

| Risk | Phase(s) | Mitigation |
|------|----------|-----------|
| DSP behavior mismatch | 1–3 | Golden file comparison; perceptual testing |
| IIR filter accuracy | 2 | Frequency response validation; match original Q/gain |
| Plugin crashes on load | 4–7 | Minimal plugin first; add features incrementally |
| Parameter automation jitter | 4–5 | Lock-free queues; test automation in Reaper |
| Preset incompatibility | 6 | Versioning strategy; test round-trip save/load |
| macOS notarization delay | 10 | Start codesigning early; test sandbox restrictions |
| LADSPA GUI limitations | 8 | Document host-drawn parameter expectation |

---

## Rollback Strategy

If a phase fails critical acceptance criteria:

1. **Identify root cause** (code bug, design issue, external tool problem)
2. **Decide fix location:**
   - Code bug? → Fix in phase, retest
   - Design issue? → Escalate, modify phase plan
   - Tool problem? → Workaround or switch tool
3. **If unfixable:** Revert phase branch, document lesson learned
4. **Resume:** Previous phase becomes current; schedule retry after root cause fix

---

## Getting Started

**For the first agent picking up Phase 1:**

1. Read this file (IMPLEMENTATION_PLAN.md) — you're here ✓
2. Read [PHASE_1_DSP_EXTRACTION.md](./PHASE_1_DSP_EXTRACTION.md) in detail
3. Set up CMake build structure
4. Begin Task 1: Create `compressor_chain.h` header
5. Check off tasks as completed; document blockers
6. Run tests; commit; open PR with phase completion summary

**For agents picking up later phases:**

1. Verify all prior phases are merged into `main`
2. Read relevant phase document
3. Review acceptance criteria from previous phase
4. Check out new branch from `main`
5. Begin Phase N

---

## Status Tracking

**Legend:**
- 📋 Planned (not started)
- 🔄 In Progress (currently being worked on)
- ✅ Completed (merged to main)
- ❌ Blocked (awaiting resolution)

Current status: All phases **📋 Planned**

To update status:
```
Edit IMPLEMENTATION_PLAN.md, update table above
Include in commit message: "Update phase status: Phase X → 🔄 In Progress"
```

---

## Questions & Escalations

If an agent encounters:

- **Ambiguity in phase description?** → Open issue with `[PHASE_N]` tag
- **Tool setup problem?** → Document in phase .md, flag for next agent
- **Design disagreement?** → Note in CHANGELOG.md, proceed with current plan
- **Test failure?** → Reproduce locally, attach minimal test case to issue
- **Time estimate slipping?** → Update phase .md with revised estimate + reason

---

## Appendix: File Manifest

Files created by this implementation plan:

```
Existing (to be refactored):
  Source/PluginProcessor.h
  Source/PluginProcessor.cpp
  Source/PluginEditor.h
  Source/PluginEditor.cpp

New files created per phase:
  Phase 1: Source/DSP/compressor_chain.h, .cpp
  Phase 2: Source/DSP/iir_filter.h, .cpp
  Phase 3: (modifications to Phase 1–2 files)
  Phase 4: Source/VST3/MangroveVST3.h, .cpp
  Phase 5: Source/VST3/gui_main.cpp, gui_controls.cpp
  Phase 6: Source/DSP/preset_serializer.h, .cpp
  Phase 7: Source/AU/MangroveAUv3.swift, .swift
  Phase 8: Source/LADSPA/MangroveLADSPA.h, .cpp
  Phase 9: Tests/*.cpp
  Phase 10: (installer scripts, packaging config)

Build configuration:
  CMakeLists.txt (root)
  CMakeLists.txt (Source/)
  CMakeLists.txt (Tests/)
```

---

**Next Step:** Select Phase 1 and open [PHASE_1_DSP_EXTRACTION.md](./PHASE_1_DSP_EXTRACTION.md) to begin implementation.
