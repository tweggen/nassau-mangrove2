# Optional Plugin Platforms & Formats

**Status:** Planning (not included in core implementation plan, but recommended additions)

**This document** describes additional plugin formats and platforms that may be valuable but are beyond the scope of the primary refactoring plan. Each can be added after Phase 10 (release).

---

## Overview

The core plan targets:
- ✅ **VST 3** (Windows/macOS/Linux)
- ✅ **AudioUnit v3** (macOS)
- ✅ **LADSPA** (Linux)

These optional formats extend reach and compatibility:

| Format | Platforms | Use Case | Effort | Priority |
|--------|-----------|----------|--------|----------|
| **VST 2** | Windows/macOS/Linux | Legacy DAW support | 2 weeks | MEDIUM |
| **Standalone App** | Windows/macOS/Linux | Direct testing, distribution | 3 weeks | MEDIUM |
| **CLAP** | Windows/macOS/Linux | Future-proof format | 3–4 weeks | LOW |
| **AU (AUv2)** | macOS | Legacy Logic/Final Cut | 2 weeks | LOW |
| **iOS AU** | iOS 14+ | iPad/iPhone apps | 3–4 weeks | LOW |

---

## 1. VST 2 Plugin (Recommended Addition)

### Why Add It?

- **Backwards compatibility:** Many studios still use VST 2-only hosts
- **Alternative to VST 3:** Simpler, more stable in older DAWs
- **Minimal extra effort:** VST 2 SDK is similar to VST 3

### Implementation (Post-Phase 10)

**Duration:** 1–2 weeks  
**Dependencies:** Completed VST 3 (Phase 4–6)  
**Files to Create:**
- `Source/VST2/MangroveVST2.cpp`
- `Source/VST2/vst2_wrapper.h`

**Key Differences from VST 3:**
- Older parameter format (no extended metadata)
- No MIDI 2.0 support
- Simpler callback structure
- Some hosts don't support parameter automation

**Using IPlug2:**
```cpp
// IPlug2 supports both VST 2 and VST 3 from same code
// Just configure CMake: -DIPLUG_PLUGINFORMATS="VST2;VST3"
```

**Pros:**
- Increases compatibility
- Minimal code changes (IPlug2 handles abstraction)

**Cons:**
- Maintenance burden (Steinberg deprecated VST 2 in 2016)
- Some DAWs have issues with VST 2

**Recommendation:** Add VST 2 if users request it; otherwise focus on VST 3.

---

## 2. Standalone Executable (Recommended Addition)

### Why Add It?

- **Testing without host:** Developers can test the plugin directly
- **User testing:** Non-technical users can evaluate without DAW
- **Distribution:** Standalone is easier to distribute than plugins
- **Demo:** Show off the plugin at conferences/online

### Implementation (Post-Phase 10)

**Duration:** 2–3 weeks  
**Dependencies:** Completed DSP (Phase 1–3), VST 3 GUI (Phase 5)  
**Files to Create:**
- `Source/Standalone/main.cpp`
- `Source/Standalone/audio_engine.h`

**Architecture:**
```
Standalone App
├── Audio I/O (PortAudio or JUCE AudioDeviceManager)
├── GUI (same as VST 3 via IGraphics)
├── File I/O (load/save WAV files)
└── CompressorChain DSP (shared)
```

**Using IPlug2:**
```cpp
// IPlug2 includes standalone wrapper
// Just configure: -DIPLUG_PLUGINFORMATS="APP"
```

**Example Flow:**
```
1. User opens Mangrove Standalone
2. Loads audio file (WAV, AIFF)
3. Configures parameters via GUI
4. Renders output
5. Saves result
```

**Pros:**
- Increases accessibility
- Easier onboarding for new users
- No plugin host needed

**Cons:**
- Requires audio I/O library (PortAudio)
- File format handling (WAV/AIFF/MP3)
- Installer complexity

**Recommendation:** Add after VST 3 is stable; low additional effort with IPlug2.

---

## 3. CLAP Plugin (Emerging Standard)

### Why Consider It?

- **Next-generation format:** Designed to fix VST 3 limitations
- **Industry adoption:** Reaper, Studio One, Ardour increasingly support CLAP
- **Modern design:** Better parameter handling, MIDI 2.0 native
- **Active development:** Ongoing community improvements

### Implementation (Post-Phase 11)

**Duration:** 3–4 weeks  
**Dependencies:** Completed VST 3 (Phase 4–6)  
**Files to Create:**
- `Source/CLAP/MangroveClap.cpp`
- `Source/CLAP/clap_wrapper.h`

**Key Differences from VST 3:**
- Parameter events (more flexible than VST 3 automation)
- Quick controls (like hardware knobs)
- Remote GUI support
- Transport sync

**Using IPlug2 (if supported):**
```cmake
# Check IPlug2 version for CLAP support
# As of early 2026, CLAP support is in development
```

**Pros:**
- Future-proof format
- Better parameter system
- Active community development

**Cons:**
- Newer, fewer DAWs support it
- IPlug2 support still evolving
- Longer learning curve

**Recommendation:** Monitor adoption; add in 2026–2027 when ecosystem matures.

---

## 4. AudioUnit v2 (AUv2) - Legacy

### Why Consider It?

- **Legacy support:** Some users still on macOS 10.7–10.13
- **Deprecation notice:** Apple removed AUv2 support in macOS 10.15+
- **App Store ban:** App Store rejects AUv2 plugins

### Implementation (If Required)

**Duration:** 2 weeks  
**Status:** Deprecated, not recommended  
**Impact:** Minimal user base

**Recommendation:** Skip unless specific client request. Focus on AUv3.

---

## 5. iOS AudioUnit (AUv3 on iOS)

### Why Consider It?

- **iOS market:** Growing number of iOS music apps
- **iPad Pro:** Increasingly used for professional music production
- **Same DSP:** Reuse CompressorChain (Phase 1)
- **Monetization:** Potential revenue from iOS users

### Implementation (Post-Phase 11)

**Duration:** 3–4 weeks  
**Dependencies:** CompressorChain (Phase 1), macOS AUv3 (Phase 7)  
**Platform Requirements:**
- Xcode 13+
- iOS 14.0+
- macOS 11.0+ (for building)

**Key Differences from macOS AUv3:**
- Touch-based UI (no mouse)
- Smaller screen (iPhone constraints)
- Audio API differences (AVAudioEngine vs. Core Audio)
- App extension sandbox restrictions

**Implementation Path:**
```swift
// iOS AUv3 wrapper
import AudioToolbox
import SwiftUI

class MangroveAUiOS: AUAudioUnit {
  override func allocateRenderResources() {
    // Allocate CompressorChain
  }
  
  var viewController: UIViewController {
    return MangroveUIViewController(...)
  }
}

// Touch-optimized SwiftUI UI
struct MangroveUITouch: View {
  @State var selectedParameter: Int = 0
  
  var body: some View {
    VStack(spacing: 20) {
      // Large touch targets
      ForEach(0..<14, id: \.self) { paramIdx in
        SliderWithLabel(...)
      }
    }.padding()
  }
}
```

**Challenges:**
- Touch UI vs. mouse UI (size, layout)
- Screen sizes (iPhone 5.5" to iPad Pro 12.9")
- Audio routing (AudioBus, IAA, AUv3 limitations)
- App approval (Apple review process)

**Pros:**
- Access to iOS music app ecosystem
- Revenue potential
- Differentiation

**Cons:**
- Separate development/testing
- iOS-specific bugs
- Apple review time
- Small user base compared to desktop

**Recommendation:** Consider after macOS AUv3 is stable and mature.

---

## 6. Browser-Based Plugin (Web Audio)

### Why Consider It?

- **Zero installation:** No plugin bundle needed
- **Cross-platform:** Works on any OS with browser
- **Web future:** DAWs increasingly browser-based (Soundtrap, BeatMaker, etc.)

### Implementation (Post-Phase 12)

**Duration:** 6–8 weeks  
**Technology:** Web Audio API + WebAssembly (WASM)  
**Files to Create:**
- `Source/WebAudio/compressor_wasm.cpp` (C++ compiled to WASM)
- `Source/WebAudio/mangrove_ui.jsx` (React UI)
- `Source/WebAudio/index.html`

**Approach:**

```javascript
// 1. Compile CompressorChain to WebAssembly
// Using Emscripten: emcripten.org
emcc Source/DSP/compressor_chain.cpp -o mangrove.js -s WASM=1

// 2. Load in browser
const MangroveModule = await loadMangroveWasm();
const dsp = new MangroveModule.CompressorChain();

// 3. Connect to Web Audio API
const processorNode = new AudioWorkletNode(audioContext, 'mangrove-processor');
processorNode.port.onmessage = (msg) => {
  // Parameter changes from UI
  dsp.setInputGain(msg.data.inputGain);
};

// 4. React UI
<MangroveParameterUI 
  onParameterChange={(param, value) => {
    processorNode.port.postMessage({ param, value });
  }}
/>
```

**Challenges:**
- WASM compilation from C++
- Real-time performance (WASM slower than native)
- Audio context security restrictions (HTTPS only)
- Browser compatibility (different Web Audio implementations)
- No persistent storage (plugin can't save state to disk)

**Pros:**
- Universal distribution (web link)
- No installation
- Potential web DAW integration

**Cons:**
- Performance overhead (WASM)
- Limited capabilities vs. native
- No file I/O
- Niche market in 2026

**Recommendation:** Experimental; not recommended for professional use.

---

## 7. SoundFont/Sample Playback (Non-Plugin)

### Not a Plugin, But Related

If the user wants to create SoundFont presets (instrument packs), that's a separate format:

- **SoundFont 2 (.sf2):** 30-year-old format, still widely used
- **SoundFont 3 (.sfz):** Modern text-based format
- **MachineHead:** Proprietary preset format

These are **not plugins**, but could bundle with the plugin as presets or educational materials.

---

## Recommendation Summary

### For MVP Release (Phase 10):
- ✅ VST 3 (Windows/macOS/Linux)
- ✅ AudioUnit v3 (macOS)
- ✅ LADSPA (Linux)

### Recommended Additions (Phase 11+):
- ⭐ **VST 2** (2 weeks, significant backwards compatibility benefit)
- ⭐ **Standalone App** (3 weeks, increases accessibility)

### Future Considerations (Phase 12+):
- 🔹 **CLAP** (3–4 weeks, when ecosystem matures in 2026–2027)
- 🔹 **iOS AUv3** (3–4 weeks, if iOS market justifies)
- 🔹 **Web Audio** (6–8 weeks, experimental, lower performance)

### Not Recommended:
- ❌ **AUv2** (deprecated, minimal user base)
- ❌ **AU2** (deprecated)
- ❌ **AAX** (Avid ProTools plugin, closed ecosystem, licensing expensive)

---

## Decision Matrix

Use this table to decide which formats to implement based on your priorities:

| Format | Effort | Compatibility | Performance | Future | Recommend |
|--------|--------|---------------|-------------|--------|-----------|
| VST 3 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ✅ CORE |
| AUv3 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ (macOS) | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ✅ CORE |
| LADSPA | ⭐⭐ | ⭐⭐⭐ (Linux) | ⭐⭐⭐⭐⭐ | ⭐⭐ | ✅ CORE |
| VST 2 | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐ ADD |
| Standalone | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐ ADD |
| CLAP | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 🔹 LATER |
| iOS AUv3 | ⭐⭐⭐⭐ | ⭐⭐⭐ (iOS) | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 🔹 LATER |
| Web Audio | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ (browser) | ⭐⭐⭐ | ⭐⭐⭐ | ❌ SKIP |

---

## Cost-Benefit Analysis

### High ROI:
- **VST 2:** 2 weeks effort, significant user benefit (backwards compatibility)
- **Standalone:** 3 weeks effort, increases accessibility and testing

### Medium ROI:
- **CLAP:** 3–4 weeks, future-proof but ecosystem still emerging
- **iOS AUv3:** 3–4 weeks, niche market but growing

### Low ROI:
- **Web Audio:** 6–8 weeks, lower performance, limited DAW support
- **AUv2:** 2 weeks, but deprecated; only add if specific request

---

## Platform-Specific Notes

### Windows
- **VST 3:** Primary target (nearly all DAWs)
- **VST 2:** Still used by some studios
- **CLAP:** Growing adoption (Reaper, Studio One)
- **Standalone:** Useful for demos

### macOS
- **AUv3:** Native integration with Logic, Final Cut (highly recommended)
- **VST 3:** Universal compatibility
- **VST 2:** Legacy support (still used by some professionals)
- **Standalone:** Useful for testing and demos
- **iOS AUv3:** If targeting iPad musicians

### Linux
- **VST 3:** Universal
- **LADSPA:** Native, simple plugins (Ardour, JACK)
- **CLAP:** Growing support (Reaper, Ardour)
- **Standalone:** Useful, can be distributed via Flatpak/AppImage

---

## Next Steps

1. **Complete core plan** (Phases 1–10)
2. **Gather user feedback:** Ask which formats users need most
3. **Prioritize additions:** Create PHASE_11, PHASE_12, etc. as needed
4. **Plan VST 2** next (easy win, good ROI)
5. **Plan Standalone** after VST 3 is stable

---

## Questions to Ask Users

When deciding on optional formats:

- **Q:** Do you need backwards compatibility with older DAWs?
  - **A:** Add VST 2 support

- **Q:** Do you want a plugin that works without a DAW?
  - **A:** Add Standalone executable

- **Q:** Are you targeting cutting-edge DAWs only?
  - **A:** VST 3 + AUv3 + CLAP sufficient

- **Q:** Do you have iOS musicians as users?
  - **A:** Consider iOS AUv3

- **Q:** Do you need browser-based solution?
  - **A:** Web Audio (6–8 weeks, lower performance)

---

## Resources

- **VST 3 SDK:** https://github.com/steinbergmedia/vst3sdk
- **CLAP Spec:** https://github.com/free-audio/clap
- **IPlug2:** https://github.com/iPlug2/iPlug2 (supports VST 2, VST 3, AU, CLAP, AAX)
- **PortAudio:** http://www.portaudio.com/ (audio I/O for standalone)

---

**Last Updated:** May 2026  
**Status:** Planning  
**Review Schedule:** Quarterly (reassess adoption rates, new formats)
