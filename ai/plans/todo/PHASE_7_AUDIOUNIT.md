# Phase 7: AudioUnit v3 (macOS)

**Objective:** Create native macOS AudioUnit v3 plugin using Apple's AudioUnit framework.

**Duration:** 4–6 weeks  
**Dependencies:** Phase 1–3 (CompressorChain DSP), Phase 5 (GUI design)  
**Produces:** `.auv3` plugin for Logic Pro, Final Cut Pro  
**Platforms:** macOS 11.0+ (Intel + Apple Silicon)  

---

## Overview

AudioUnit v3 (AUv3) is Apple's modern plugin format:

- ✅ Native macOS integration (Logic, Final Cut, GarageBand)
- ✅ App Extension (sandboxed, App Store compatible)
- ✅ Swift UI for custom GUI (optional)
- ✅ AudioUnit framework (lower-level than JUCE)

### Comparison: AUv3 vs. VST3

| Feature | AUv3 | VST3 |
|---------|------|------|
| macOS only | ✓ | ✗ |
| App Store | ✓ | ✗ |
| GUI required | ✗ (optional) | ✗ |
| MIDI support | Limited | Full |
| Presets | Host-managed | Plugin-managed |
| Learning curve | Medium | High |

---

## Detailed Tasks

### Task 7.1: Set Up AUv3 Xcode Project

**Duration:** 2–3 hours

**Steps:**

1. **Create new AUv3 target in Xcode:**
   ```
   File → New → Target
   → AudioUnit Extension
   → Product Name: MangroveAUv3
   ```

2. **Configure bundle ID and team:**
   - Bundle ID: `com.nassau.mangrove.auv3`
   - Team: Your Apple Developer account

3. **Link CompressorChain library:**
   - Add DSP library to build phases
   - Link in Build Phases → Link Binary With Libraries

4. **Create project structure:**
   ```
   Source/AU/
   ├── MangroveAUv3.swift
   ├── MangroveAUv3Bridge.h
   ├── MangroveAUv3Bridge.cpp
   └── UI/
       └── MangroveAUv3UI.swift
   ```

**Acceptance:** Xcode project builds (empty plugin)

---

### Task 7.2: Implement Core AUv3 Class

**File:** `Source/AU/MangroveAUv3Bridge.h`

```cpp
#pragma once

#include "compressor_chain.h"
#include <AudioUnit/AudioUnit.h>

// C++ bridge to access DSP
class MangroveAUv3Bridge {
public:
  MangroveAUv3Bridge();
  ~MangroveAUv3Bridge();
  
  CompressorChain& GetDSP() { return mDSP; }
  void ProcessAudio(const AudioBuffer* inL, const AudioBuffer* inR,
                    AudioBuffer* outL, AudioBuffer* outR,
                    UInt32 frameCount);
  
private:
  CompressorChain mDSP;
};
```

**File:** `Source/AU/MangroveAUv3.swift`

```swift
import AudioUnit
import AVFoundation

// Main AUv3 plugin class
class MangroveAUv3: AUAudioUnit {
  var mDSP: MangroveAUv3Bridge
  
  override init(componentDescription: AudioComponentDescription,
                options: AudioComponentInstantiationOptions = []) throws {
    mDSP = MangroveAUv3Bridge()
    try super.init(componentDescription: componentDescription, options: options)
    
    // Configure audio I/O
    configureAudioI()
    
    // Create parameter tree
    createParameters()
  }
  
  func configureAudioI() {
    // Configure input/output buses (stereo)
    let stereoFormat = AVAudioFormat(standardFormatWithSampleRate: 44100,
                                      channels: 2)!
    do {
      try outputBusses[0].setFormat(stereoFormat)
    } catch {
      fatalError("Failed to set output format")
    }
  }
  
  func createParameters() {
    // Create 14 parameters matching VST3
    let paramTree = AUParameterTree(children: [
      AUParameter(identifier: "inputGain",
                  name: "Input Gain",
                  address: 0,
                  range: AUParameterTree.Range(minimum: -24.0, maximum: 24.0),
                  unit: .decibels,
                  // ... etc for all 14 params
                  )
    ])
    
    self.parameterTree = paramTree
  }
  
  override var internalRenderBlock: AUInternalRenderBlock {
    return { [weak self] actionFlags, timestamp, frameCount, 
             inputBusNumber, inputData, outputData, renderContext in
      
      guard let self = self else { return kAudioUnitErr_NoConnection }
      
      // Get input/output pointers
      let inBufferList = UnsafePointer(inputData)?.pointee
      let outBufferList = UnsafeMutablePointer(outputData)?.pointee
      
      // Process audio
      self.mDSP.ProcessAudio(
        inBufferList?.mBuffers[0],
        inBufferList?.mBuffers[1],
        &outBufferList?.mBuffers[0],
        &outBufferList?.mBuffers[1],
        frameCount)
      
      return noErr
    }
  }
}
```

---

### Task 7.3: Create SwiftUI Custom UI (Optional)

**File:** `Source/AU/UI/MangroveAUv3UI.swift`

```swift
import SwiftUI

struct MangroveAUv3UI: View {
  @State var inputGain: Float = 0.0
  @State var levelThreshold: Float = -10.0
  // ... other parameters
  
  var body: some View {
    VStack(spacing: 20) {
      Text("Mangrove Compressor").font(.headline)
      
      GroupBox(label: Text("Input")) {
        VStack {
          HStack {
            Text("Gain"); Spacer()
            Slider(value: $inputGain, in: -24...24)
            Text("\(Int(inputGain)) dB")
          }
          HStack {
            Text("LoC ut"); Spacer()
            Slider(value: .constant(80), in: 20...300)
            Text("80 Hz")
          }
        }
      }
      
      GroupBox(label: Text("Level Compressor")) {
        VStack {
          HStack {
            Text("Threshold"); Spacer()
            Slider(value: $levelThreshold, in: -60...0)
            Text("\(Int(levelThreshold)) dB")
          }
          // ... more controls
        }
      }
      
      Spacer()
    }
    .padding()
  }
}
```

---

### Task 7.4: Parameter Bridging

Connect SwiftUI parameters to AUParameterTree:

```swift
// In MangroveAUv3.swift
@objc override dynamic var inputGain: AUValue {
  get { parameterTree?.parameter(withAddress: 0)?.value ?? 0.0 }
  set { parameterTree?.parameter(withAddress: 0)?.value = newValue }
}

// In SwiftUI
@ObservedObject var audioUnit: MangroveAUv3

// Then bind
Slider(value: Binding(
  get: { audioUnit.inputGain },
  set: { audioUnit.inputGain = $0 }
))
```

---

### Task 7.5: Testing in Logic Pro

**Duration:** 2–3 hours

1. **Build AUv3:**
   ```
   Product → Build (⌘B)
   ```

2. **Run host app (Logic Pro):**
   ```
   Product → Scheme → Edit Scheme
   → Run → Executable: Logic Pro
   → Build → Mangrove AUv3 target
   ```

3. **Test in Logic:**
   - Create audio track
   - Insert AU plugin (Logic → Plugins → Mangrove)
   - Test parameter automation
   - Verify audio I/O

---

### Task 7.6: Code Signing & Notarization

**For App Store distribution:**

```
Xcode → Signing & Capabilities
→ Team ID: (your Apple account)
→ Signing Certificate: (auto-managed)
```

---

## Deliverables

- [ ] Xcode project with AUv3 target
- [ ] MangroveAUv3Bridge C++ wrapper
- [ ] MangroveAUv3.swift (core AudioUnit)
- [ ] MangroveAUv3UI.swift (SwiftUI GUI)
- [ ] Parameter tree (14 parameters)
- [ ] Audio I/O working in Logic Pro
- [ ] Code signing certificate configured

---

## Success Criteria

✅ Plugin loads in Logic Pro  
✅ All 14 parameters appear  
✅ Parameter automation works  
✅ Audio processes correctly  
✅ Compiles without warnings on Apple Silicon + Intel  
✅ App Store requirements met (if distributing)  

---

## Timeline

Days 1–2: Tasks 7.1–7.2  
Days 3–4: Task 7.3–7.4  
Day 5: Task 7.5  
Days 6–7: Task 7.6 + polish  

**Total: 4–6 weeks**

---

## Resources

- Apple AudioUnit Documentation: https://developer.apple.com/documentation/audiounit
- AUv3 Sample: https://developer.apple.com/library/archive/samplecode/auriounit/Introduction/Intro.html
- Signing Guide: https://developer.apple.com/support/code-signing/
