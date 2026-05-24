# Phase 9: Testing & Validation

**Objective:** Comprehensive regression, cross-platform, and cross-DAW testing.

**Duration:** 3–4 weeks  
**Dependencies:** Phase 4–8 (All plugins complete)  
**Produces:** Test results, validation report  

---

## Overview

Test strategy across multiple dimensions:

1. **Unit Tests** (code level)
2. **Regression Tests** (DSP output vs. original)
3. **Integration Tests** (plugin in DAW)
4. **Cross-Platform Tests** (Windows/macOS/Linux)
5. **Cross-DAW Tests** (Reaper, Studio One, Logic, Ardour)
6. **Performance Tests** (CPU, memory, latency)
7. **Compatibility Tests** (edge cases, extreme parameters)

---

## Detailed Tasks

### Task 9.1: Unit Test Suite Review

**Duration:** 1 week

Consolidate and review all unit tests from Phases 1–8:

```bash
# Run all tests
cd build
ctest --verbose

# Generate coverage report
lcov --directory . --capture --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

**Targets:**
- Phase 1–3 (DSP core): ≥95% coverage
- Phase 4–6 (VST3): ≥80% coverage
- Phase 7–8 (AU/LADSPA): ≥70% coverage

**Test Report:**
```markdown
## Unit Test Results

| Phase | Tests | Pass | Fail | Coverage |
|-------|-------|------|------|----------|
| 1–3 DSP | 45 | 45 | 0 | 96% |
| 4–6 VST3 | 28 | 28 | 0 | 82% |
| 7–8 AU/LADSPA | 15 | 15 | 0 | 71% |
| **Total** | **88** | **88** | **0** | **83%** |
```

---

### Task 9.2: Regression Testing (DSP Output)

**Duration:** 1 week

Compare DSP output against Phase 1 golden reference at multiple sample rates:

```bash
# Generate test signals at each sample rate
python3 generate_test_signals.py --output-dir test_signals/

# Process through each plugin version
Mangrove_phase1.vst3 < input.wav > output_phase1.wav
Mangrove_final.vst3 < input.wav > output_final.wav

# Compare
python3 compare_audio.py output_phase1.wav output_final.wav
```

**Test Signals:**
1. Sine sweep (20 Hz → 20 kHz, 10 sec)
2. Pink noise (10 sec, -20 dB)
3. Drum sample (transient test, 5 sec)
4. Orchestral music (real-world, 30 sec)
5. Silence (noise floor test)

**Acceptance Criteria:**
- RMS error < 0.01 dB on all signals
- Peak error < 0.5 dB
- No audible differences in A/B comparison

**Python Script:**
```python
#!/usr/bin/env python3
import numpy as np
import scipy.io.wavfile as wav
import sys

def compare_audio(file1, file2):
    fs1, audio1 = wav.read(file1)
    fs2, audio2 = wav.read(file2)
    
    assert fs1 == fs2, "Sample rate mismatch"
    
    # Ensure same length
    length = min(len(audio1), len(audio2))
    audio1 = audio1[:length]
    audio2 = audio2[:length]
    
    # Convert to float
    if audio1.dtype != np.float32:
        audio1 = audio1.astype(np.float32) / 32768.0
    if audio2.dtype != np.float32:
        audio2 = audio2.astype(np.float32) / 32768.0
    
    # Calculate metrics
    rms_error = np.sqrt(np.mean((audio1 - audio2) ** 2))
    rms_error_db = 20 * np.log10(rms_error + 1e-10)
    
    peak_error = np.max(np.abs(audio1 - audio2))
    peak_error_db = 20 * np.log10(peak_error + 1e-10)
    
    print(f"RMS Error: {rms_error:.6f} ({rms_error_db:.2f} dB)")
    print(f"Peak Error: {peak_error:.6f} ({peak_error_db:.2f} dB)")
    
    if rms_error_db < -60:
        print("✓ PASS: Negligible difference")
    elif rms_error_db > -40:
        print("✗ FAIL: Significant difference")
        return False
    else:
        print("⚠ WARNING: Minor difference (acceptable)")
    
    return True

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: compare_audio.py <file1> <file2>")
        sys.exit(1)
    
    success = compare_audio(sys.argv[1], sys.argv[2])
    sys.exit(0 if success else 1)
```

---

### Task 9.3: Cross-Platform Build Testing

**Duration:** 1 week

Test builds on all three platforms:

**Windows:**
```bash
# Visual Studio 2022
cmake -G "Visual Studio 17 2022" -A x64 build_win
cmake --build build_win --config Release

# Verify
dir build_win\Release\*.vst3
dir build_win\Release\*.dll
```

**macOS:**
```bash
# Both Intel and Apple Silicon
cmake -G Xcode build_mac
cmake --build build_mac --config Release -- -arch x86_64 -arch arm64

# Verify
file build_mac/Release/Mangrove.vst3/Contents/MacOS/Mangrove
lipo -info build_mac/Release/Mangrove.vst3/Contents/MacOS/Mangrove
```

**Linux:**
```bash
# GCC/Clang
cmake build_linux
cmake --build build_linux --config Release

# Verify
file build_linux/libMangrove.so
ldd build_linux/libMangrove.so
```

**Acceptance Criteria:**
- Builds without errors on all platforms
- No compiler warnings
- All plugins load in test DAW
- Audio I/O works

---

### Task 9.4: Cross-DAW Testing

**Duration:** 1.5 weeks

Test on major DAWs:

| DAW | Platform | Format | Status |
|-----|----------|--------|--------|
| Reaper | Win/Mac/Linux | VST3 | Required |
| Studio One | Win/Mac | VST3 | Recommended |
| Logic Pro | Mac | AUv3 | Required |
| Ardour | Linux | LADSPA | Recommended |

**Test Procedure (per DAW):**

1. **Load plugin:**
   - Create audio track
   - Insert plugin
   - Verify no crash

2. **Parameter automation:**
   - Create automation track
   - Automate each parameter
   - Verify smooth changes

3. **Preset save/load:**
   - Save preset
   - Close/reopen project
   - Verify parameters restored

4. **Audio I/O:**
   - Generate test tone
   - Route through plugin
   - Verify output correct

5. **Performance:**
   - Monitor CPU usage
   - 100 instances running (stress test)
   - CPU should remain <10% per instance

**Test Report Template:**
```markdown
## Cross-DAW Testing Results

### Reaper (Windows)
- [ ] Plugin loads
- [ ] Parameters work
- [ ] Automation functional
- [ ] Presets save/load
- [ ] CPU: ___ %
- [ ] Notes: ___

### Studio One (macOS)
- [ ] Plugin loads
- [ ] Parameters work
- [ ] Automation functional
- [ ] Presets save/load
- [ ] CPU: ___ %
- [ ] Notes: ___

### Logic Pro (macOS)
- [ ] Plugin loads
- [ ] Parameters work
- [ ] Automation functional
- [ ] Presets save/load
- [ ] CPU: ___ %
- [ ] Notes: ___

### Ardour (Linux)
- [ ] Plugin loads
- [ ] Parameters work
- [ ] Automation functional
- [ ] CPU: ___ %
- [ ] Notes: ___
```

---

### Task 9.5: Performance Profiling

**Duration:** 3–4 days

Profile CPU, memory, and latency:

```bash
# Profile with VTune (Windows)
amplxe-cl -collect hpc -app Reaper.exe -- (run plugin test)

# Profile with Instruments (macOS)
xcrun xctrace record --template "System Trace" \
  --output trace.trace \
  --launch Logic\ Pro

# Verify latency
# Plugin must report 0 samples latency
# Measure with test tone (impulse response)
```

**Performance Targets:**
- CPU per instance: <10% on modern hardware
- Memory: <5 MB per instance
- Latency: 0 samples

---

### Task 9.6: Compatibility Edge Cases

**Duration:** 3–4 days

Test unusual scenarios:

- **Extreme parameters:**
  - All sliders at minimum/maximum
  - Parameter automation ramping 0→1 instantly

- **Sample rate changes:**
  - 44.1 → 48 kHz mid-stream (if supported)
  - Very high sample rates (192 kHz, 384 kHz)

- **Block size variations:**
  - Tiny blocks (8 samples)
  - Large blocks (16384 samples)

- **Multiple instances:**
  - 10, 50, 100 instances running simultaneously
  - Verify no crosstalk or state leakage

- **Noise conditions:**
  - DC offset
  - Very loud signals (clipping edge case)
  - Very quiet signals (noise floor)

---

### Task 9.7: Documentation & Final Report

**Duration:** 1 week

Create comprehensive test report:

```markdown
# Mangrove Plugin v2.0.0 - Testing Report

## Executive Summary

All test suites passed. Plugin ready for release.

## Test Coverage

- Unit Tests: 88 total, 88 pass (100%)
- Integration Tests: 24 total, 24 pass (100%)
- Regression Tests: Pass (RMS error < 0.01 dB)
- Cross-Platform: Windows, macOS, Linux ✓
- Cross-DAW: Reaper, Studio One, Logic, Ardour ✓

## Performance

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| CPU per instance | <10% | 3.2% | ✓ |
| Memory | <5 MB | 2.1 MB | ✓ |
| Latency | 0 samples | 0 samples | ✓ |

## Known Issues

None

## Recommendations

Ready for release.
```

---

## Deliverables

- [ ] Unit test results (88 tests, 100% pass)
- [ ] Regression test report (all signals pass)
- [ ] Cross-platform build verification
- [ ] Cross-DAW test results
- [ ] Performance profiling data
- [ ] Final testing report
- [ ] Known issues list (if any)

---

## Success Criteria

✅ All unit tests pass  
✅ Regression tests: RMS error < 0.01 dB  
✅ Builds on Windows, macOS, Linux  
✅ Works in Reaper, Studio One, Logic, Ardour  
✅ CPU <10% per instance  
✅ No latency  
✅ No crashes under stress testing  
✅ Comprehensive test report generated  

---

## Timeline

Weeks 1–2: Tasks 9.1–9.3  
Weeks 2–3: Task 9.4 (DAW testing)  
Week 3: Task 9.5 (performance)  
Week 4: Tasks 9.6–9.7  

**Total: 3–4 weeks**
