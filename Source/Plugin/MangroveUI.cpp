// Mangrove editor. Three panels in signal order on one row, INPUT / LEVEL /
// DENSITY, with LEVEL's four switches under its knobs.
//
// Built from nassau-plugin-sdk's shared editor style (include/nassau/
// ui_style.h, docs/UI_STYLE.md): the metrics, text, control styles and
// helpers are the SDK's; the palette and the panels are this plugin's.
//
// The ENTIRE body is inside #if IPLUG_EDITOR, so a headless build
// (NASSAU_FORCE_HEADLESS) compiles this file to nothing.
#if IPLUG_EDITOR

#include "MangroveUI.h"
#include "MangrovePlugin.h"
#include "nassau/ui_style.h"

using namespace iplug::igraphics;
namespace nui = nassau::ui;

void MangroveUI::Layout(IGraphics& ui, MangrovePlugin& /*plugin*/)
{
    nui::LoadFont(ui);

    // A dark mangrove green with a muted leaf-green accent: calm, and apart
    // from the amber siblings (zermatt, analogue), eq's blue-grey and bloom's
    // night blue.
    const nui::Palette pal{
        IColor(255, 16, 26, 22),     // bg
        IColor(255, 26, 40, 34),     // panel
        IColor(255, 214, 228, 218),  // text
        IColor(255, 128, 150, 138),  // textDim
        IColor(255, 74, 158, 120),   // accent
        IColor(255, 42, 62, 54),     // knobFace
    };

    // ---- Size: derived, and checked against config.h ---------------------------
    // One row of panels, INPUT 3 + LEVEL 4 + DENSITY 4 knob columns; its height
    // is LEVEL's, the knob row plus the toggle row under it.
    constexpr float rowH = nui::kPanelTitleH + nui::kKnobRowH + nui::kGap + nui::kToggleH;
    constexpr float kEditorW = 11.f * nui::kKnobColW + 2.f * nui::kColGap + 2.f * nui::kMargin;
    constexpr float kEditorH = 2.f * nui::kMargin + nui::kHeaderH + nui::kRowGap + rowH;
    static_assert(kEditorW == static_cast<float>(PLUG_WIDTH), "PLUG_WIDTH does not match the layout");
    static_assert(kEditorH == static_cast<float>(PLUG_HEIGHT), "PLUG_HEIGHT does not match the layout");

    const IVStyle knob = nui::KnobStyle(pal);
    const IVStyle tog = nui::ToggleStyle(pal);

    ui.AttachPanelBackground(pal.bg);

    // Header. Mangrove has no factory presets, so not nui::Header, which always
    // adds the preset selector: an empty one would be a control that does
    // nothing. Same rects and text styles, without the selector.
    ui.AttachControl(new ITextControl(
        IRECT(nui::kMargin + 2.f, nui::kMargin, nui::kMargin + 150.f, nui::kMargin + nui::kHeaderH), PLUG_NAME,
        nui::HeaderText(pal)));
    ui.AttachControl(new ITextControl(
        IRECT(kEditorW - 66.f, nui::kMargin, kEditorW - nui::kMargin, nui::kMargin + nui::kHeaderH), "Nassau",
        nui::BrandText(pal)));

    const float x0 = nui::kMargin, x1 = kEditorW - nui::kMargin;
    const float y1 = nui::kMargin + nui::kHeaderH + nui::kRowGap;
    const auto row = nui::PanelRow(IRECT(x0, y1, x1, y1 + rowH), {3.f, 4.f, 4.f});
    auto knobRow = [](const IRECT& area) { return IRECT(area.L, area.T, area.R, area.T + nui::kKnobRowH); };

    // INPUT: gain, the low cut and the saturator ahead of the two compressors.
    {
        const IRECT a = nui::Panel(ui, pal, row[0], "INPUT");
        nui::KnobRow(ui, knobRow(a), knob,
                     { { kInputGain, "Gain" }, { kInputLoCut, "Lo Cut" }, { kInputSaturate, "Saturate" } });
    }
    // LEVEL: the levelling compressor, its four switches under the knobs.
    {
        const IRECT a = nui::Panel(ui, pal, row[1], "LEVEL");
        const IRECT k = knobRow(a);
        nui::KnobRow(ui, k, knob,
                     { { kLevelThreshold, "Threshold" }, { kLevelRatio, "Ratio" }, { kLevelAttack, "Attack" },
                       { kLevelRelease, "Release" } });
        const float t = k.B + nui::kGap;
        nui::ToggleRow(ui, IRECT(a.L, t, a.R, t + nui::kToggleH), tog,
                       { { kLevelLoCut, "Lo Cut" }, { kLevelTubeGain, "Tube" }, { kLevelFeedback, "Feedback" },
                         { kLevelFast, "Fast" } });
    }
    // DENSITY: the second compressor.
    {
        const IRECT a = nui::Panel(ui, pal, row[2], "DENSITY");
        nui::KnobRow(ui, knobRow(a), knob,
                     { { kDensityThreshold, "Threshold" }, { kDensityRatio, "Ratio" },
                       { kDensityAttack, "Attack" }, { kDensityRelease, "Release" } });
    }

#ifndef NDEBUG
    nui::CheckBindings(ui, kNumParams, PLUG_NAME);
#endif
}

#endif // IPLUG_EDITOR
