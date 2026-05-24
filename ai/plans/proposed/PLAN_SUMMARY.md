# Implementation Plan Summary

**Project:** Mangrove Plugin Refactoring (JUCE → Native Formats)  
**Created:** May 2026  
**Total Duration:** 20–30 weeks  
**Status:** 🔄 Active Execution — Phase 5 in progress  
**Elapsed:** ~4 weeks | **On Track:** Yes

---

## Documents Created

### 1. **IMPLEMENTATION_PLAN.md** (Main Document — in `proposed/`)
The master plan document covering:
- Project overview and phase dependencies
- Directory structure (post-implementation)
- Development guidelines and standards
- Integration checkpoints
- Branching strategy
- Resource requirements
- Success criteria
- Phase descriptions with links

**Use this first.** It's the navigation hub for all other documents.

---

### 2. **PHASE_1_DSP_EXTRACTION.md** (2–3 weeks)
**Objective:** Extract audio processing logic into standalone `CompressorChain` class

**Produces:**
- `Source/DSP/compressor_chain.h` (API definition)
- `Source/DSP/compressor_chain.cpp` (implementation)
- `Tests/dsp_tests.cpp` (unit tests)

**Key Tasks:**
1. Create CMake build structure
2. Define CompressorChain public API (14 parameters)
3. Implement constructor & initialization
4. Port input stage (gain, saturation)
5. Port level compressor (dual feedback modes)
6. Port density compressor (fast limiter)
7. Implement metering
8. Write comprehensive tests
9. Code review & documentation

**Acceptance:** Audio output matches original JUCE version (RMS < 0.01 dB)

---

### 3. **PHASE_2_IIR_REPLACEMENT.md** (1–2 weeks)
**Objective:** Implement custom Butterworth high-pass filter to eliminate JUCE dependency

**Produces:**
- `Source/DSP/iir_filter.h` (2nd-order Butterworth filter)
- `Source/DSP/iir_filter.cpp` (implementation)
- `Tests/iir_tests.cpp` (frequency response tests)
- Updated `CompressorChain` to use custom filter

**Key Tasks:**
1. Understand Butterworth filter mathematics
2. Create IIRFilter header (API)
3. Implement coefficient calculation (normalized frequency method)
4. Write validation tests (frequency response accuracy)
5. Integrate into CompressorChain
6. Regression testing
7. Code review & documentation

**Acceptance:** Frequency response accurate within ±1 dB; output matches Phase 1

---

### 4. **PHASE_3_SAMPLE_RATE_FIX.md** (Not Yet Created, ~1–2 weeks)
**Objective:** Replace hardcoded 44.1 kHz with dynamic sample-rate handling

**Will Cover:**
- Identify all sample-rate dependencies
- Implement dynamic coefficient recalculation
- Test at multiple sample rates (44.1, 48, 96, 192 kHz)
- Verify output quality across all rates

---

### 5. **PHASE_4_VST3_WRAPPER.md** (Not Yet Created, ~3–4 weeks)
**Objective:** Wrap CompressorChain in IPlug2 VST 3 framework

**Will Cover:**
- IPlug2 project setup
- Parameter mapping (14 JUCE params → VST 3)
- Audio processing callback
- Binary state serialization
- Cross-platform build configuration (Windows/macOS/Linux)

---

### 6. **PHASE_5_GUI_IMPLEMENTATION.md** (Not Yet Created, ~2–3 weeks)
**Objective:** Build parameter UI using IGraphics (sliders, toggles, meters)

**Will Cover:**
- IGraphics widget setup
- Slider/toggle layout matching original
- Real-time meter display (input RMS, compression reduction)
- Event handling and parameter binding
- Visual styling

---

### 7. **PHASE_6_SERIALIZATION.md** (Not Yet Created, ~1–2 weeks)
**Objective:** Implement preset save/load system

**Will Cover:**
- Binary format (fast, compact)
- JSON format (human-readable)
- Round-trip testing (save → load → verify)
- Version compatibility

---

### 8. **PHASE_7_AUDIOUNIT.md** (Not Yet Created, ~4–6 weeks)
**Objective:** Create native macOS AudioUnit v3 plugin

**Will Cover:**
- AUv3 framework integration
- Swift UI implementation
- Parameter bridging
- Testing in Logic Pro/Final Cut Pro

---

### 9. **PHASE_8_LADSPA.md** (Not Yet Created, ~1–2 weeks)
**Objective:** Create Linux LADSPA plugin wrapper

**Will Cover:**
- LADSPA SDK integration
- Port descriptor setup (14 parameters + 2 audio buffers)
- Minimal plugin (host provides parameter sliders)

---

### 10. **PHASE_9_TESTING.md** (Not Yet Created, ~3–4 weeks)
**Objective:** Comprehensive regression and cross-platform testing

**Will Cover:**
- Unit test suite
- Regression tests (golden audio comparison)
- Cross-DAW testing (Reaper, Studio One, Logic, Ableton)
- Performance profiling
- Memory leak detection
- Edge case testing

---

### 11. **PHASE_10_PACKAGING.md** (Not Yet Created, ~2–3 weeks)
**Objective:** Create distribution packages and release infrastructure

**Will Cover:**
- Windows installer (NSIS)
- macOS installer + code signing + notarization
- Linux package (deb, rpm, Flatpak)
- Version management
- User documentation

---

### 12. **OPTIONAL_PLATFORMS.md** (Decision Guide)
**Supplementary Document** — Not required for MVP, but describes additional formats:

- **VST 2** (Recommended: 2 weeks, high backwards compatibility)
- **Standalone App** (Recommended: 3 weeks, increases accessibility)
- **CLAP** (Consider: 3–4 weeks, future-proof)
- **iOS AUv3** (Consider: 3–4 weeks, niche market)
- **Web Audio** (Not recommended: 6–8 weeks, low performance)
- **Decision matrix** and cost-benefit analysis

---

## How to Use These Documents

### For Project Managers:
1. Read **IMPLEMENTATION_PLAN.md** (overview + timeline)
2. Reference **OPTIONAL_PLATFORMS.md** for scope decisions
3. Track phase status in IMPLEMENTATION_PLAN.md table
4. Schedule resource allocation based on phase durations

### For Developers:
1. Start with **IMPLEMENTATION_PLAN.md** (understand overall structure)
2. Read relevant **PHASE_X.md** document for detailed tasks
3. Execute tasks in order (dependencies matter)
4. Check off items as you complete them
5. Update IMPLEMENTATION_PLAN.md status when phase is complete

### For New Team Members:
1. Read **IMPLEMENTATION_PLAN.md** (first hour)
2. Read your assigned phase document completely (2–4 hours)
3. Set up local development environment
4. Begin Task 1 of your phase
5. Ask questions if blocked

---

## Quick Facts

### Core Phases (Required for MVP)
| Phase | Duration | Status | Completed |
|-------|----------|--------|-----------|
| 1. DSP Extraction | 2–3 weeks | ✅ Complete | May 6 |
| 2. IIR Filter | 1–2 weeks | ✅ Complete | May 13 |
| 3. Sample Rate Fix | 1–2 weeks | ✅ Complete | May 18 |
| 4. VST 3 Wrapper | 3–4 weeks | ✅ Complete | May 21 |
| 5. GUI | 2–3 weeks | 🔄 In Progress | ETA May 30 |
| 6. Serialization | 1–2 weeks | 📋 Planned | — |
| 7. AudioUnit | 4–6 weeks | 📋 Planned | — |
| 8. LADSPA | 1–2 weeks | 📋 Planned | — |
| 9. Testing | 3–4 weeks | 📋 Planned | — |
| 10. Packaging | 2–3 weeks | 📋 Planned | — |
| **TOTAL** | **20–30 weeks** | **~4 weeks elapsed** | **On track** |

### Critical Path (Minimum to Release)
```
Phase 1 (2–3w)
  → Phase 2 (1–2w)
    → Phase 3 (1–2w)
      → Phase 4 (3–4w)
        → Phase 5 (2–3w)
          → Phase 6 (1–2w)
            → Phase 9 (3–4w)
              → Phase 10 (2–3w)
```

**Critical Path Duration:** ~17–25 weeks (for VST 3 on all platforms)

### Optional Additions (Post-Release)
- Phase 11: VST 2 (2 weeks)
- Phase 12: Standalone App (3 weeks)
- Phase 13: CLAP (3–4 weeks)
- Phase 14: iOS AUv3 (3–4 weeks)

---

## Document Structure

Each phase document follows this format:

1. **Objective** — What is being built
2. **Duration** — Estimated weeks
3. **Acceptance Criteria** — How to know it's done
4. **Detailed Tasks** — Step-by-step instructions
   - Task description
   - Implementation code/pseudo-code
   - Acceptance criteria for that task
5. **Integration Testing** — How to verify it works
6. **Deliverables Checklist** — Files created
7. **Success Criteria** — Phase completion requirements
8. **Timeline** — Week-by-week breakdown
9. **Common Pitfalls** — Problems to avoid

---

## Key Success Factors

### Technical
✅ Audio output must match original within ±0.01 dB RMS  
✅ No JUCE dependencies in DSP core  
✅ Lock-free parameter updates (no mutex in audio thread)  
✅ Zero compiler warnings on all platforms  
✅ 90%+ unit test code coverage  

### Process
✅ Test-driven development (write tests before code)  
✅ Frequent integration checks (don't diverge long)  
✅ Clear task breakdown (no ambiguous assignments)  
✅ Version control discipline (logical commits, clean history)  
✅ Documentation-first (comment non-obvious algorithms)  

### Timeline
✅ Phase dependencies respected (no jumping ahead)  
✅ Realistic estimates (padding for unknowns included)  
✅ Regular status updates (weekly minimum)  
✅ Early blocker identification (escalate by mid-week)  

---

## Development Environment Setup

### Prerequisites (Before Phase 1)
- **C++ Compiler:** GCC 7+, Clang 5+, MSVC 2017+
- **CMake:** 3.15+ (download from cmake.org)
- **Git:** For version control
- **Text Editor/IDE:** VS Code, CLion, Xcode, Visual Studio
- **Test Framework:** Catch2 (header-only, included)

### IPlug2 Setup (For Phase 4)
```bash
# Clone IPlug2 as submodule
git submodule add https://github.com/iPlug2/iPlug2.git external/iplug2

# Install dependencies (macOS example)
brew install freetype fontconfig
```

### DAW Setup (For Testing)
- **Reaper** (Windows/macOS/Linux) — primary test platform
- **Studio One** (Windows/macOS) — optional, for validation
- **Logic Pro** (macOS) — for AUv3 testing
- **Ardour** (Linux) — for LADSPA testing

---

## Common Questions

### Q: Do I need to complete phases in order?
**A:** Yes. Each phase depends on previous work. You can't start Phase 4 (VST 3 wrapper) without Phase 1–3 (DSP extraction, filters, sample-rate fix).

### Q: Can phases run in parallel?
**A:** Partially. Phase 7 (AudioUnit), 8 (LADSPA), and 9 (Testing) can be done in parallel once Phases 1–6 are complete. But 1–6 must be sequential.

### Q: How long is this really going to take?
**A:** With full-time dedicated developer: **20–30 weeks**
- Best case (experienced team, no blockers): 20 weeks
- Typical case (normal developer, some issues): 25 weeks
- Worst case (learning curve, integration issues): 30 weeks

### Q: Can I use IPlug2 to save effort?
**A:** Absolutely. IPlug2 saves ~4 weeks of boilerplate. Strongly recommended for Phase 4–5.

### Q: What if I only want VST 3, not AU/LADSPA?
**A:** You can skip Phases 7 and 8. MVP with just VST 3: ~15 weeks. Then add AU and LADSPA as optional Phase 11–12.

### Q: What about licensing?
**A:** Code is plugin-agnostic. VST 3 SDK is free (JUCE/Steinberg license). IPlug2 is open-source. Standalone uses PortAudio (open-source). No paid tools required.

### Q: Who should assign these phases to engineers?
**A:** Assignment suggestions:
- **Phases 1–3:** Senior engineer (DSP understanding required)
- **Phases 4–6:** Mid-level engineer (VST 3 + GUI experience)
- **Phase 7:** macOS specialist (Swift + Audio framework)
- **Phase 8:** Linux specialist (LADSPA, jack)
- **Phase 9:** QA/test engineer
- **Phase 10:** Release engineer

---

## Next Actions

### For First Time Reading This Plan:

1. ✅ You are here: Reading PLAN_SUMMARY.md (5 mins)
2. ➡️ **Next:** Read [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md) completely (30 mins)
3. ➡️ **Then:** Decide which platforms to target (10 mins)
   - Core: VST 3, AUv3, LADSPA (recommended)
   - Optional: VST 2, Standalone (see OPTIONAL_PLATFORMS.md)
4. ➡️ **Then:** Set up development environment (1–2 hours)
5. ➡️ **Finally:** Begin Phase 1 ([PHASE_1_DSP_EXTRACTION.md](./PHASE_1_DSP_EXTRACTION.md))

### For Project Kickoff:

1. **Review** IMPLEMENTATION_PLAN.md with team
2. **Confirm** target platforms (VST 3 minimum, AU/LADSPA optional)
3. **Assign** Phases 1–3 to senior engineer
4. **Set up** Git repository + CI/CD
5. **Allocate** resources for Phases 4–10
6. **Schedule** weekly status meetings
7. **Begin** Phase 1

---

## Document Locations

Documents are organized by phase status under `ai/plans/`:

```
ai/plans/
├── STATE.md                     ← Current execution state (read this first!)
├── proposed/                    ← Strategy docs + in-progress phases
│   ├── PLAN_SUMMARY.md          ← You are here
│   ├── IMPLEMENTATION_PLAN.md   ← Overall strategy & dependencies
│   ├── PHASE_5_GUI_IMPLEMENTATION.md ← In progress (GUI)
│   ├── OPTIONAL_PLATFORMS.md    ← VST 2, Standalone, CLAP, iOS, Web
│   └── REFACTORING_ANALYSIS.md  ← Original analysis document
├── done/                        ← Completed phases
│   ├── PHASE_1_DSP_EXTRACTION.md    ✅
│   ├── PHASE_2_IIR_REPLACEMENT.md   ✅
│   ├── PHASE_3_SAMPLE_RATE_FIX.md   ✅
│   └── PHASE_4_VST3_WRAPPER.md      ✅
└── todo/                        ← Planned but not started
    ├── PHASE_6_SERIALIZATION.md
    ├── PHASE_7_AUDIOUNIT.md
    ├── PHASE_8_LADSPA.md
    ├── PHASE_9_TESTING.md
    └── PHASE_10_PACKAGING.md
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | May 18, 2026 | Initial plan created |

---

## Contact & Questions

When you have questions about this plan:

1. **Ambiguity in task description?** → Issue with `[PHASE_N]` tag
2. **Blocker on current phase?** → Document in phase .md, flag for review
3. **Time estimate wrong?** → Update phase document + note reason
4. **Need clarification?** → Refer to IMPLEMENTATION_PLAN.md dependencies

---

**Ready to start Phase 1?** → Open [PHASE_1_DSP_EXTRACTION.md](./PHASE_1_DSP_EXTRACTION.md)

Good luck! 🎵
