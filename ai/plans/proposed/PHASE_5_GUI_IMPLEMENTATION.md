# Phase 5: GUI Implementation (IGraphics)

**Objective:** Build parameter UI using IPlug2's IGraphics framework (sliders, toggles, meters).

**Duration:** 2–3 weeks  
**Dependencies:** Phase 4 (VST 3 wrapper)  
**Produces:** Interactive parameter controls for VST3 + AUv3 via IPlug2 Plugin path  
**Tests Required:** UI responsiveness, parameter binding, meter accuracy  
**Build Path:** `Source/Plugin/` (IPlug2-based, cross-platform)

---

## Overview

This phase adds visual controls to the plugin using **IGraphics** (IPlug2's GUI framework). We replicate the original JUCE interface:

### Build Path Recommendation

**Use `Source/Plugin/CMakeLists.txt` instead of `Source/VST3/`.**

The IPlug2 Plugin build (`Source/Plugin/`) provides:
- ✅ Cleaner VST3 integration (fewer linking issues)
- ✅ Native AUv3 support (macOS) automatically
- ✅ Better-maintained IPlug2 framework usage
- ✅ GUI and plugin code co-located
- ✅ Simpler build configuration

The custom `Source/VST3/` build was Phase 4 prototype work. Phase 5 GUI leverages IPlug2 directly for production quality.

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

**Files:** 
- `Source/Plugin/MangroveUI.h` (already started, see Phase 4)
- `Source/Plugin/MangroveUI.cpp` (implementation)

**Build:** Use `Source/Plugin/CMakeLists.txt` which handles IGraphics linking

**Approach:**

Using IPlug2 Plugin pattern (see `Source/Plugin/MangrovePlugin.cpp` for parameter init):

```cpp
// In Source/Plugin/MangroveUI.cpp

void MangroveUI::Layout(IGraphics& g, MangrovePlugin& plugin)
{
  const IRECT bounds = g.GetBounds();
  const int WIDTH = 640;
  const int HEIGHT = 400;
  
  // ===== INPUT STAGE (Column 1) =====
  const int COL1_X = 50, COL1_W = 150;
  
  g.AttachControl(new IVSliderControl(
    IRECT(COL1_X, 50, COL1_X + 40, 200),
    kInputGain,
    IVStyle().WithColor(kFG, COLOR_WHITE)));
  
  g.AttachControl(new IVSliderControl(
    IRECT(COL1_X + 60, 50, COL1_X + 100, 200),
    kInputLoCut,
    IVStyle().WithColor(kFG, COLOR_BLUE)));
    
  // ===== LEVEL COMPRESSOR (Column 2) =====
  const int COL2_X = 250, COL2_W = 150;
  
  g.AttachControl(new IVSliderControl(
    IRECT(COL2_X, 50, COL2_X + 40, 200),
    kLevelThreshold));
    
  // Toggle buttons
  g.AttachControl(new IVToggleControl(
    IRECT(COL2_X, 220, COL2_X + 30, 250),
    kLevelFeedback));
  
  // ===== DENSITY COMPRESSOR (Column 3) =====
  // ... similar layout
  
  // ===== METERS (Bottom) =====
  const int METER_Y = 280;
  
  // Use ISender to push meter data from audio thread
  // See MangroveUI::PushMeterData() below
  
  // ===== LOGO/TEXT =====
  g.DrawText(IText(24, IColor(200, 200, 200)),
    "Mangrove", IRECT(10, 10, WIDTH - 10, 40));
}
```

**Key Pattern:** Use `MangrovePlugin::mLayoutFunc` lambda in constructor to define UI layout.

### Task 5.2: Meter Data Callback (IPlug2 Pattern)

The `CompressorChain` already provides meter data via `getMeterData()`. Use IPlug2's **ISender** pattern for thread-safe meter updates:

```cpp
// In Source/Plugin/MangrovePlugin.h
#include "IPlug_include_in_plug_hdr.h"
#include "compressor_chain.h"

class MangrovePlugin : public iplug::Plugin {
  // ...
private:
  CompressorChain mChain;
  
  void OnIdle() override {
    // Push meter data to UI thread (called from main thread)
    auto [inLevel, levelRed, densityRed] = mChain.getMeterData();
    
    // Update UI controls (if they exist)
    if (GetUI() != nullptr) {
      // Meter controls will read values via GetParam()
    }
  }
};
```

**Key:** IPlug2 Plugin architecture handles threading automatically. Just call `getMeterData()` in `OnIdle()` and update controls.

### Task 5.3: Custom Control Styling (Optional)

IPlug2 provides `IVStyle` for most needs. For special text overlays:

```cpp
// In Source/Plugin/MangroveUI.cpp

void MangroveUI::AttachLevelRatioSlider(IGraphics& g, const IRECT& bounds, int paramID)
{
  // Use IVStyle for standard styling
  IVStyle style;
  style.WithColor(kFG, IColor(255, 100, 150, 200));
  style.WithColor(kBG, IColor(255, 30, 30, 40));
  
  auto slider = new IVSliderControl(bounds, paramID, "Level Ratio", style);
  g.AttachControl(slider);
  
  // Optional: Add custom label updater
  slider->SetValueToStringFunction([](double value) {
    if (value >= 9.99) return "Vari-Mu";
    return std::to_string((int)value) + ":1";
  });
}
```

For most controls, use IPlug2's default `IVSliderControl` which handles display automatically.

### Task 5.4: Layout & Styling

Define layout in `MangroveUI::Layout()` method (see Task 5.1):

```cpp
// In Source/Plugin/MangroveUI.cpp

void MangroveUI::Layout(IGraphics& g, MangrovePlugin& plugin)
{
  // Layout constants
  const int SECTION_WIDTH = 140;
  const int INPUT_LEFT = 40;
  const int LEVEL_LEFT = 200;
  const int DENSITY_LEFT = 360;
  const int SLIDER_H = 150;
  const int TOGGLE_H = 30;
  
  // Define color scheme
  const IVStyle sliderStyle = IVStyle()
    .WithColor(kFG, COLOR_CYAN)
    .WithColor(kBG, COLOR_DARK_GRAY);
    
  const IVStyle toggleStyle = IVStyle()
    .WithColor(kFG, COLOR_GREEN)
    .WithColor(kBG, COLOR_DARK_GRAY);
  
  // Attach controls (see MangroveUI.h for full method)
  AttachInputControls(g, INPUT_LEFT, sliderStyle);
  AttachLevelControls(g, LEVEL_LEFT, sliderStyle, toggleStyle);
  AttachDensityControls(g, DENSITY_LEFT, sliderStyle, toggleStyle);
  AttachMeters(g, sliderStyle);
}
```

**Note:** IPlug2 handles DPI scaling automatically when using `IRECT` coordinates.

### Task 5.5: Meter Display & Updates

Use IPlug2's built-in meter controls with smooth decay:

```cpp
// In Source/Plugin/MangroveUI.cpp

void MangroveUI::AttachMeters(IGraphics& g, const IVStyle& style)
{
  // Input meter
  g.AttachControl(new IMeterControl<float>(
    IRECT(40, 320, 180, 360),
    style,
    "Input Level"));
    
  // Level reduction meter
  g.AttachControl(new IMeterControl<float>(
    IRECT(200, 320, 340, 360),
    style,
    "Level Reduction"));
    
  // Density reduction meter
  g.AttachControl(new IMeterControl<float>(
    IRECT(360, 320, 500, 360),
    style,
    "Density Reduction"));
}

// In MangrovePlugin::OnIdle() - update meter display
void MangrovePlugin::OnIdle()
{
  auto [inLevel, levelRed, densityRed] = mChain.getMeterData();
  
  // Meters automatically decay/update on the UI thread
  // Just push new values periodically (20 Hz ~ every 50ms)
  static int counter = 0;
  if (++counter % 3 == 0) {  // Assuming 60 FPS idle timer
    // Update control values (implementation detail)
  }
}
```

**Note:** IPlug2's `IMeterControl` handles decay and smoothing automatically.

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

- [ ] `Source/Plugin/MangroveUI.h` (GUI class definition — started)
- [ ] `Source/Plugin/MangroveUI.cpp` (layout & control attachment)
- [ ] Parameter binding verified (14 params working)
- [ ] Meters display correctly (input, level, density)
- [ ] All controls responsive on Windows/macOS/Linux
- [ ] Test suite: UI responsiveness, meter accuracy
- [ ] GUI Documentation updated

**Build:** Test with `Source/Plugin/CMakeLists.txt` (handles both VST3 + AUv3)

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

---

## Build Path Justification

**Why `Source/Plugin/` instead of `Source/VST3/`?**

| Factor | `Source/Plugin/` (IPlug2) | `Source/VST3/` (Custom) |
|--------|--------------------------|-------------------------|
| Linking | ✅ Clean (uses IPlug2 targets) | ⚠️ Fragile (custom SDK build) |
| Maintenance | ✅ IPlug2-standard patterns | ⚠️ Custom CMakeLists needed |
| AUv3 Support | ✅ Automatic (IPlug2 built-in) | ❌ Requires Phase 7 separate work |
| Cross-Platform | ✅ Windows/macOS/Linux tested | ⏳ macOS needs more work |
| Build Time | ✅ Faster (reuses targets) | ❌ Rebuilds VST3 SDK |

**Decision:** Phase 4 proved VST3 wrapper works (all tests pass). Phase 5 leverages IPlug2 framework for production GUI quality, avoiding custom CMakeLists complexity. Phase 7 (AudioUnit) gets automatic IPlug2 support as bonus.

---

## Integration Checklist

Before starting Phase 5:

- [ ] Verify `Source/Plugin/CMakeLists.txt` builds successfully
- [ ] Confirm IPlug2 submodule initialized (`external/iplug2/`)
- [ ] Test base IGraphics example (NanoVG should work on macOS)
- [ ] Confirm `MangroveUI.h` skeleton exists (from Phase 4)
- [ ] Review `Source/Plugin/config.h` for UI dimensions
