# Phase 6: Serialization

**Objective:** Implement preset save/load system (binary + JSON formats).

**Duration:** 1–2 weeks  
**Dependencies:** Phase 4 (VST 3 wrapper)  
**Produces:** Preset file format (.mangrove, .json), round-trip save/load  

---

## Overview

Implement two preset formats:

1. **Binary (.mangrove):** Fast, compact (56 bytes = 14 × 4-byte floats)
2. **JSON (.json):** Human-readable, editable

### Binary Format

```
Byte Layout:
  0-3:   float inputGain
  4-7:   float inputLoCut
  8-11:  float inputSaturate
  12-15: float levelThreshold
  16-19: float levelRatio
  20-23: float levelAttack
  24-27: float levelRelease
  28:    bool  levelLoCut
  29:    bool  levelTubeGain
  30:    bool  levelFeedback
  31-34: float densityThreshold
  35-38: float densityRatio
  39-42: float densityAttack
  43-46: float densityRelease
```

### JSON Format

```json
{
  "version": 1,
  "name": "Preset Name",
  "parameters": {
    "inputGain": 0.0,
    "inputLoCut": 80.0,
    "inputSaturate": 0.0,
    "levelThreshold": -10.0,
    "levelRatio": 2.5,
    "levelAttack": 10.0,
    "levelRelease": 300.0,
    "levelLoCut": false,
    "levelTubeGain": false,
    "levelFeedback": true,
    "densityThreshold": -10.0,
    "densityRatio": 1.0,
    "densityAttack": 10.0,
    "densityRelease": 300.0
  }
}
```

---

## Detailed Tasks

### Task 6.1: Create Preset Serializer Class

**File:** `Source/DSP/preset_serializer.h`

```cpp
#pragma once

#include <string>
#include <map>
#include <vector>

struct PresetData {
  std::string name;
  std::map<std::string, float> parameters;
};

class PresetSerializer {
public:
  // Binary I/O
  static bool SaveBinary(const std::string& filePath,
                         const PresetData& preset);
  static bool LoadBinary(const std::string& filePath,
                         PresetData& outPreset);
  
  // JSON I/O
  static bool SaveJSON(const std::string& filePath,
                       const PresetData& preset);
  static bool LoadJSON(const std::string& filePath,
                       PresetData& outPreset);
  
  // Conversion
  static PresetData CreateFromPlugin(class MangroveVST3* plugin);
  static void ApplyToPlugin(class MangroveVST3* plugin,
                            const PresetData& preset);
};
```

### Task 6.2: Implement Binary Serialization

**File:** `Source/DSP/preset_serializer.cpp`

```cpp
#include "preset_serializer.h"
#include <fstream>
#include <cstring>

bool PresetSerializer::SaveBinary(const std::string& filePath,
                                   const PresetData& preset) {
  std::ofstream file(filePath, std::ios::binary);
  if (!file) return false;
  
  // Version marker
  uint32_t version = 1;
  file.write((char*)&version, sizeof(version));
  
  // 14 parameter values (floats)
  std::vector<float> values = {
    preset.parameters.at("inputGain"),
    preset.parameters.at("inputLoCut"),
    preset.parameters.at("inputSaturate"),
    preset.parameters.at("levelThreshold"),
    preset.parameters.at("levelRatio"),
    preset.parameters.at("levelAttack"),
    preset.parameters.at("levelRelease"),
    preset.parameters.at("levelLoCut"),
    preset.parameters.at("levelTubeGain"),
    preset.parameters.at("levelFeedback"),
    preset.parameters.at("densityThreshold"),
    preset.parameters.at("densityRatio"),
    preset.parameters.at("densityAttack"),
    preset.parameters.at("densityRelease"),
  };
  
  for (float v : values) {
    file.write((char*)&v, sizeof(float));
  }
  
  file.close();
  return true;
}

bool PresetSerializer::LoadBinary(const std::string& filePath,
                                   PresetData& outPreset) {
  std::ifstream file(filePath, std::ios::binary);
  if (!file) return false;
  
  // Check version
  uint32_t version;
  file.read((char*)&version, sizeof(version));
  if (version != 1) return false;
  
  // Read 14 floats
  std::vector<std::string> paramNames = {
    "inputGain", "inputLoCut", "inputSaturate",
    "levelThreshold", "levelRatio", "levelAttack", "levelRelease",
    "levelLoCut", "levelTubeGain", "levelFeedback",
    "densityThreshold", "densityRatio", "densityAttack", "densityRelease"
  };
  
  for (const auto& name : paramNames) {
    float value;
    file.read((char*)&value, sizeof(float));
    outPreset.parameters[name] = value;
  }
  
  file.close();
  return true;
}
```

### Task 6.3: Implement JSON Serialization

Use a lightweight JSON library (e.g., nlohmann/json or minijson):

```cpp
#include "preset_serializer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool PresetSerializer::SaveJSON(const std::string& filePath,
                                 const PresetData& preset) {
  json j;
  j["version"] = 1;
  j["name"] = preset.name;
  j["parameters"] = preset.parameters;
  
  std::ofstream file(filePath);
  if (!file) return false;
  
  file << j.dump(2);  // Pretty-print with 2-space indent
  file.close();
  return true;
}

bool PresetSerializer::LoadJSON(const std::string& filePath,
                                 PresetData& outPreset) {
  std::ifstream file(filePath);
  if (!file) return false;
  
  json j;
  file >> j;
  
  if (j["version"] != 1) return false;
  
  outPreset.name = j["name"];
  outPreset.parameters = j["parameters"].get<std::map<std::string, float>>();
  
  file.close();
  return true;
}
```

### Task 6.4: Integration with VST3 Plugin

In `MangroveVST3`:

```cpp
class MangroveVST3 : public Plugin {
public:
  // ... existing methods ...
  
  // Export preset
  void ExportPreset(const std::string& filePath) {
    PresetData preset = PresetSerializer::CreateFromPlugin(this);
    PresetSerializer::SaveJSON(filePath, preset);
  }
  
  // Import preset
  void ImportPreset(const std::string& filePath) {
    PresetData preset;
    if (PresetSerializer::LoadJSON(filePath, preset)) {
      PresetSerializer::ApplyToPlugin(this, preset);
    }
  }
};
```

### Task 6.5: Write Round-Trip Tests

```cpp
TEST_CASE("Preset serialization: binary round-trip") {
  PresetData original;
  original.name = "Test Preset";
  original.parameters = {
    {"inputGain", 6.0f},
    {"levelThreshold", -15.0f},
    // ... all 14 parameters
  };
  
  // Save
  PresetSerializer::SaveBinary("/tmp/test.mangrove", original);
  
  // Load
  PresetData loaded;
  PresetSerializer::LoadBinary("/tmp/test.mangrove", loaded);
  
  // Verify
  REQUIRE(loaded.parameters["inputGain"] == Catch::Approx(6.0f));
  REQUIRE(loaded.parameters["levelThreshold"] == Catch::Approx(-15.0f));
}

TEST_CASE("Preset serialization: JSON round-trip") {
  // Similar test with JSON format
}

TEST_CASE("Preset: VST3 plugin import/export") {
  MangroveVST3 plugin;
  
  // Set parameters
  plugin.GetParam(kInputGain)->Set(3.0);
  plugin.GetParam(kLevelThreshold)->Set(-12.0);
  
  // Export
  plugin.ExportPreset("/tmp/my_preset.json");
  
  // Reset
  plugin.GetParam(kInputGain)->Set(0.0);
  plugin.GetParam(kLevelThreshold)->Set(-10.0);
  
  // Import
  plugin.ImportPreset("/tmp/my_preset.json");
  
  // Verify restored
  REQUIRE(plugin.GetParam(kInputGain)->Value() == Catch::Approx(3.0));
  REQUIRE(plugin.GetParam(kLevelThreshold)->Value() == Catch::Approx(-12.0));
}
```

### Task 6.6: Preset Browser (Optional)

If time permits, add simple preset directory UI:

```cpp
class PresetBrowser {
private:
  std::vector<std::string> mPresetFiles;
  int mSelectedPreset = 0;
  
public:
  void ScanPresets(const std::string& presetsDir) {
    // List all .json files in directory
    // TODO: OS-specific directory scanning
  }
  
  std::string GetSelectedPresetPath() const {
    return mPresetFiles[mSelectedPreset];
  }
};
```

### Task 6.7: Documentation

Create preset format documentation:

```markdown
# Preset Format

## Binary Format (.mangrove)

56 bytes total:
- Version: 4 bytes (uint32_t = 1)
- 14 parameters: 56 bytes (14 × 4 bytes)

Quick, compact, not human-editable.

## JSON Format (.json)

Human-readable, editable in text editor.

Example:
```json
{
  "version": 1,
  "name": "Dark Compression",
  "parameters": {
    "inputGain": 6.0,
    "inputLoCut": 80.0,
    ...
  }
}
```

## Compatibility

- Version 1: Mangrove 2.0.0+
- Future versions will include migration code
```

---

## Deliverables

- [ ] `Source/DSP/preset_serializer.h` (class definition)
- [ ] `Source/DSP/preset_serializer.cpp` (implementation)
- [ ] `Tests/serialization_tests.cpp` (round-trip tests)
- [ ] Integration with MangroveVST3
- [ ] Preset format documentation
- [ ] Example presets (3–5 factory presets)

---

## Success Criteria

✅ Binary serialization works (56-byte format)  
✅ JSON serialization works (human-readable)  
✅ Round-trip: save → load → values match (all 14 params)  
✅ Works with VST3 plugin state save/load  
✅ No data loss in conversion  
✅ Tests pass on all platforms  

---

## Timeline

Days 1–2: Tasks 6.1–6.2  
Days 3: Task 6.3  
Days 4: Task 6.4  
Days 5: Task 6.5–6.7  

**Total: 1–2 weeks**
