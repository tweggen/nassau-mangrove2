# Phase 5: GUI Implementation (IGraphics)

**Objective:** Build parameter UI using IPlug2's IGraphics framework (sliders, toggles, meters).

**Duration:** 2–3 weeks  
**Dependencies:** Phase 4 (VST 3 wrapper)  
**Produces:** Interactive parameter controls matching original JUCE design  
**Tests Required:** UI responsiveness, parameter binding, meter accuracy  

---

## Overview

This phase adds visual controls to the VST 3 plugin using **IGraphics** (IPlug2's GUI framework). We replicate the original JUCE interface:

- 10 horizontal sliders (gain, threshold, attack, release, etc.)
- 4 toggle buttons (LoC ut, TubeGain, Feedback, etc.)
- 3 real-time meters (input level, level reduction, density reduction)
- Static branding (logo/text)

### GUI Layout

```
┌─────────────────────────────────────────────────┐
│  Mangrove Compressor                            │
├─────────────────────────────────────────────────┤
│ INPUT          LEVEL           DENSITY          │
│ ┌──────┐      ┌──────┐        ┌──────┐        │
│ │ Gain │      │ Thr  │        │ Thr  │        │
│ │  -24 │      │ -10  │        │ -10  │        │
│ └──────┘      └──────┘        └──────┘        │
│                                                  │
│ ┌──────┐      ┌──────┐        ┌──────┐        │
│ │ Cut  │      │ Ratio│        │ Ratio│        │
│ │  80  │      │ 2.5  │        │ 1.0  │        │
│ └──────┘      └──────┘        └──────┘        │
│                                                  │
│ [⊗] LoC ut   [⊗] Feedback                    │
│                                                  │
│ METERS: Input: -∞     Level: -∞     Density: -∞│
└─────────────────────────────────────────────────┘
```

---

## Acceptance Criteria

- [ ] All 14 parameters have visual controls
- [ ] Sliders respond to mouse/touch input
- [ ] Toggle buttons are clickable
- [ ] Parameter values display as text
- [ ] Real-time meters update (20 Hz refresh)
- [ ] Layout matches original JUCE design
- [ ] Color scheme professional
- [ ] Responds on Windows, macOS, Linux
- [ ] No performance impact (<1% CPU for GUI)
- [ ] Touch-friendly on high-DPI displays

---

## Key Tasks

### Task 5.1: Create IGraphics GUI Class

**File:** `Source/VST3/gui_main.cpp`

```cpp
#include "IGraphics.h"
#include "MangroveVST3.h"

void CreateMangroveUI(IGraphics& graphics, MangroveVST3* plugin) {
  // Window size
  const int WIDTH = 630;
  const int HEIGHT = 330;
  graphics.Resize(WIDTH, HEIGHT);
  
  // Background
  graphics.FillRect(IColor(255, 40, 40, 50), IRECT(0, 0, WIDTH, HEIGHT));
  
  // ===== INPUT STAGE =====
  const int INPUT_X = 80;
  const int INPUT_Y = 50;
  
  graphics.AttachControl(new IVSliderControl(
    IRECT(INPUT_X, INPUT_Y, INPUT_X + 50, INPUT_Y + 100),
    kInputGain, "Input Gain"));
  
  graphics.AttachControl(new IVSliderControl(
    IRECT(INPUT_X, INPUT_Y + 120, INPUT_X + 50, INPUT_Y + 220),
    kInputLoCut, "Input LoC ut"));
  
  // ... similar for other sliders
  
  // ===== LEVEL COMPRESSOR =====
  const int LEVEL_X = 260;
  
  // Toggles
  graphics.AttachControl(new IVToggleControl(
    IRECT(LEVEL_X, INPUT_Y + 220, LEVEL_X + 30, INPUT_Y + 250),
    kLevelFeedback, "Feedback"));
  
  // ===== METERS =====
  const int METER_Y = 280;
  
  graphics.AttachControl(new IMeterControl<float>(
    IRECT(100, METER_Y, 150, METER_Y + 20),
    nullptr,  // Callback to fetch meter data
    "Input"));
  
  // ===== LOGO/TEXT =====
  graphics.DrawText(IText(16, IColor(200, 200, 200)),
    "Mangrove Compressor", IRECT(10, 10, WIDTH - 10, 30));
}
```

### Task 5.2: Meter Data Callback

Update `CompressorChain` to provide meter data to GUI:

```cpp
// In compressor_chain.h
struct MeterData {
  float inputGain;      // RMS input level
  float levelReduction; // Compression amount (0-1)
  float densityReduction;
};

MeterData getMeterData() const;
```

In VST3 wrapper, add method to fetch meters:

```cpp
// In MangroveVST3.h
CompressorChain::MeterData GetMeterData() const {
  return mDSP.getMeterData();
}
```

Connect in GUI callback:

```cpp
// In IGraphics callback
if (isMetersNeedUpdate()) {
  auto meters = plugin->GetMeterData();
  
  // Update meter controls
  GetControl(kMeterInput)->SetValue(meters.inputGain);
  GetControl(kMeterLevel)->SetValue(meters.levelReduction);
  GetControl(kMeterDensity)->SetValue(meters.densityReduction);
}
```

### Task 5.3: Custom Slider Rendering

Some sliders need special text display (like original JUCE code):

```cpp
// Custom slider for LoCut (shows "Disabled" below threshold)
class InputLoCutSlider : public IVSliderControl {
public:
  void Draw(IGraphics& graphics) override {
    IVSliderControl::Draw(graphics);
    
    double value = GetValue();
    if (value < 20.5) {
      graphics.DrawText(IText(12), "Disabled", mRECT);
    }
  }
};

// Custom slider for Level Ratio (shows "Vari-Mu" at max)
class LevelRatioSlider : public IVSliderControl {
  void Draw(IGraphics& graphics) override {
    IVSliderControl::Draw(graphics);
    
    double value = GetValue();
    if (value >= 9.99) {
      graphics.DrawText(IText(12), "Vari-Mu", mRECT);
    }
  }
};
```

### Task 5.4: Layout & Styling

Define consistent layout:

```cpp
// Constants
const int SECTION_WIDTH = 150;
const int INPUT_LEFT = 80;
const int LEVEL_LEFT = 260;
const int DENSITY_LEFT = 440;

const int SLIDER_HEIGHT = 100;
const int TOGGLE_SIZE = 30;

// Spacing
const int HGAP = 20;  // Horizontal gap between sections
const int VGAP = 15;  // Vertical gap between controls

// Colors
IColor BG_DARK(255, 40, 40, 50);
IColor TEXT_LIGHT(255, 200, 200, 200);
IColor ACCENT_BLUE(255, 100, 150, 200);
IColor METER_GREEN(255, 0, 255, 0);
```

### Task 5.5: Meter Animation

Smooth meter decay (visual polish):

```cpp
class AnimatedMeter : public IGraphics {
private:
  float mDecayRate = 0.95f;  // Per-frame decay
  
public:
  void OnIdleTimer() override {
    // Apply decay to meter values
    mMeterInput *= mDecayRate;
    mMeterLevel *= mDecayRate;
    mMeterDensity *= mDecayRate;
    
    // Fetch fresh values from DSP
    auto meters = mPlugin->GetMeterData();
    mMeterInput = std::max(mMeterInput, meters.inputGain);
    mMeterLevel = std::max(mMeterLevel, meters.levelReduction);
    mMeterDensity = std::max(mMeterDensity, meters.densityReduction);
  }
};
```

### Task 5.6: Responsiveness Testing

Test on different screen sizes:

```cpp
// Test viewport sizes
std::vector<std::pair<int, int>> testSizes = {
  {630, 330},      // Default
  {1260, 660},     // 2x zoom
  {315, 165},      // 0.5x zoom
  {2560, 1440},    // 4K
  {1920, 1080},    // FHD
};

for (auto [w, h] : testSizes) {
  graphics.Resize(w, h);
  // Verify all controls visible and clickable
}
```

### Task 5.7: Documentation & Examples

Create usage guide:

```markdown
# GUI Implementation

## Controls

### Input Stage
- **Input Gain:** -24 to +24 dB (linear slider)
- **Input LoC ut:** 20 to 300 Hz (log slider)
- **Input Saturate:** 0 to 5 (saturation amount)

### Level Compressor
- **Level Threshold:** -60 to 0 dB (log slider)
- **Level Ratio:** 1 to 10 (log slider, ≥9.99 → "Vari-Mu")
- **Level Attack:** 0 to 100 ms (exp slider)
- **Level Release:** 10 to 500 ms (exp slider)
- **Toggles:** LoC ut, TubeGain, Feedback

### Density Compressor
- Similar layout to Level

## Meters

Real-time display (updated 20 Hz):
- **Input:** RMS input level (dB scale)
- **Level:** Compression reduction (0-1 linear)
- **Density:** Limiter reduction (0-1 linear)

## Customization

Edit `gui_main.cpp` to modify:
- Layout/spacing (constants at top)
- Colors (defined in CreateMangroveUI)
- Control sizes
- Meter ranges
```

---

## Deliverables

- [ ] `Source/VST3/gui_main.cpp` (GUI creation)
- [ ] `Source/VST3/gui_controls.cpp` (custom controls)
- [ ] IGraphics integration tested
- [ ] Meters display correctly
- [ ] All controls responsive
- [ ] GUI Documentation

---

## Success Criteria

✅ All 14 parameters have visual controls  
✅ Sliders and toggles work  
✅ Meters update in real-time  
✅ Layout matches original design  
✅ Professional appearance  
✅ Works on high-DPI displays  
✅ <1% CPU overhead  

---

## Timeline

Days 1–2: Tasks 5.1–5.2  
Days 3–4: Tasks 5.3–5.4  
Days 5: Task 5.5 (animation)  
Days 6: Task 5.6–5.7  

**Total: 2–3 weeks**
