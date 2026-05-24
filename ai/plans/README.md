# Mangrove Plugin Refactoring Plans

**Status:** 🔄 Phase 5 (GUI Implementation) in progress  
**Elapsed:** ~4 weeks of ~20–30 week project  
**On Track:** ✅ Yes

---

## Quick Navigation

### ⭐ Start Here
- **`STATE.md`** — Current execution state snapshot (5 min read)
  - What's done, what's in progress, what's next
  - Build status, timeline, known issues
  - Action items for next phase

### 📋 Strategy Documents (in `proposed/`)
- **`IMPLEMENTATION_PLAN.md`** — Master plan with phase dependencies (10 min)
- **`PLAN_SUMMARY.md`** — Quick reference guide
- **`OPTIONAL_PLATFORMS.md`** — Additional format options (VST 2, Standalone, CLAP, etc.)
- **`REFACTORING_ANALYSIS.md`** — Original analysis document

### ✅ Completed Phases (in `done/`)
- **`PHASE_1_DSP_EXTRACTION.md`** — DSP core library (2–3 weeks) ✅
- **`PHASE_2_IIR_REPLACEMENT.md`** — Custom filter (1–2 weeks) ✅
- **`PHASE_3_SAMPLE_RATE_FIX.md`** — Dynamic sample rates (1–2 weeks) ✅
- **`PHASE_4_VST3_WRAPPER.md`** — VST 3 plugin (3–4 weeks) ✅

### 🔄 In Progress (in `proposed/`)
- **`PHASE_5_GUI_IMPLEMENTATION.md`** — GUI/metering (2–3 weeks, ~60% done)

### 📅 Planned Phases (in `todo/`)
- **`PHASE_6_SERIALIZATION.md`** — Preset save/load (1–2 weeks)
- **`PHASE_7_AUDIOUNIT.md`** — macOS AU v3 (4–6 weeks)
- **`PHASE_8_LADSPA.md`** — Linux LADSPA (1–2 weeks)
- **`PHASE_9_TESTING.md`** — Regression & validation (3–4 weeks)
- **`PHASE_10_PACKAGING.md`** — Distribution setup (2–3 weeks)

---

## For Different Roles

### 🚀 New to the Project?
1. Read `STATE.md` (current status)
2. Read `proposed/IMPLEMENTATION_PLAN.md` (strategy)
3. Read root `CLAUDE.md` (full project context)

### 👨‍💻 Developer Working on Phase 5
1. Check `STATE.md` for current blockers
2. Read `proposed/PHASE_5_GUI_IMPLEMENTATION.md` for detailed tasks
3. Reference `done/PHASE_4_VST3_WRAPPER.md` for VST 3 architecture
4. Build: `cd build_phase5 && cmake --build .`

### 📊 Project Manager
1. Read `STATE.md` for timeline and velocity
2. Review `proposed/IMPLEMENTATION_PLAN.md` for phase dependencies
3. Check `PHASE_X_STATUS.md` files (in root, updated per phase)

### 🧪 QA/Tester (Phase 9+)
1. Read `todo/PHASE_9_TESTING.md` when ready
2. Check `done/PHASE_1_DSP_EXTRACTION.md` for test patterns
3. Build against `Tests/dsp_tests.cpp` baseline

---

## Phase Status at a Glance

| Phase | Duration | Status | Progress | Est. Date |
|-------|----------|--------|----------|-----------|
| 1. DSP | 2–3w | ✅ Done | 100% | May 6 |
| 2. IIR | 1–2w | ✅ Done | 100% | May 13 |
| 3. Sample Rate | 1–2w | ✅ Done | 100% | May 18 |
| 4. VST 3 | 3–4w | ✅ Done | 100% | May 21 |
| 5. GUI | 2–3w | 🔄 In Progress | 60% | May 30 |
| 6. Serialization | 1–2w | 📋 Planned | 0% | Jun 7 |
| 7. AudioUnit | 4–6w | 📋 Planned | 0% | Jun 21 |
| 8. LADSPA | 1–2w | 📋 Planned | 0% | Jun 28 |
| 9. Testing | 3–4w | 📋 Planned | 0% | Jul 19 |
| 10. Packaging | 2–3w | 📋 Planned | 0% | Aug 9 |

---

## Key Files by Topic

### Audio Processing
- `done/PHASE_1_DSP_EXTRACTION.md` — DSP architecture
- `done/PHASE_2_IIR_REPLACEMENT.md` — Filter implementation
- `Source/DSP/compressor_chain.h` — API reference

### Plugin Wrappers
- `done/PHASE_4_VST3_WRAPPER.md` — VST 3 integration
- `proposed/PHASE_5_GUI_IMPLEMENTATION.md` — UI framework (IGraphics)
- `todo/PHASE_7_AUDIOUNIT.md` — macOS AU v3
- `todo/PHASE_8_LADSPA.md` — Linux support

### Testing & Quality
- `done/PHASE_1_DSP_EXTRACTION.md` → See "Test Coverage" section
- `todo/PHASE_9_TESTING.md` — Comprehensive validation
- `Tests/dsp_tests.cpp` — 40 unit tests (all passing)

### Distribution
- `todo/PHASE_10_PACKAGING.md` — Installers & release
- `proposed/OPTIONAL_PLATFORMS.md` — Extra formats to consider

---

## File Organization Rules

- **`proposed/`** → Strategy documents (master plans) + in-progress phases
- **`done/`** → Completed phases (reference, locked)
- **`todo/`** → Planned but not started phases (next work)
- **`STATE.md`** → Current execution snapshot (read first!)

**Update Strategy:**
- When starting a phase: Move from `todo/` to `proposed/`
- When completing a phase: Move from `proposed/` to `done/`, create status summary
- When planning: Documents stay in `proposed/` until phase truly starts

---

## Common Tasks

### "What should I work on next?"
1. Read `STATE.md` — What's the current blocker?
2. Check relevant `PHASE_N.md` — What task is next?
3. Start building/coding per detailed task description

### "Where do I find the DSP source code?"
→ `Source/DSP/compressor_chain.h` (API) + `compressor_chain.cpp` (impl)

### "How do I test my changes?"
→ `Tests/dsp_tests.cpp` (40 unit tests) or DAW test (Phase 5+)

### "What does the GUI framework do?"
→ `proposed/PHASE_5_GUI_IMPLEMENTATION.md` (IGraphics details)

### "What's the build process?"
→ `proposed/IMPLEMENTATION_PLAN.md` (section: "Development Environment Setup")

---

## Timeline Velocity

**Actual vs. Planned:**
- Phase 1: 2–3w planned → ~1w actual ✅ 1–2w ahead
- Phase 2: 1–2w planned → ~1w actual ✅ On schedule
- Phase 3: 1–2w planned → ~1w actual ✅ On schedule
- Phase 4: 3–4w planned → ~2.5w actual ✅ 1w ahead
- Phase 5: 2–3w planned → ~2w (est.) 🔄 On track

**Projected Completion:** Mid-August 2026 (±1–2 weeks)

---

## How to Read Phase Documents

Each `PHASE_N.md` document follows this structure:

1. **Objective** — What is being built
2. **Duration** — Estimated time (usually accurate)
3. **Deliverables** — Files created (concrete list)
4. **Detailed Tasks** — Step-by-step work (with code samples)
5. **Integration Testing** — How to verify it works
6. **Success Criteria** — Definition of "done"
7. **Common Pitfalls** — Problems to avoid

**Read the whole document before starting.** Task descriptions reference earlier sections.

---

## Resources

### Internal
- `CLAUDE.md` (root) — Full project context for Claude instances
- `PHASE_1_STATUS.md` (root) — Phase 1 completion report (detailed)
- `.claude/projects/.../memory/` — Persistent memory files

### External
- IPlug2: https://github.com/iPlug2/iPlug2
- VST 3 SDK: https://github.com/steinbergmedia/vst3sdk
- CMake: https://cmake.org/

---

**Last Updated:** May 24, 2026  
**Maintained by:** Timo Weggen (timo.weggen@gmail.com)
