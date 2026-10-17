#include "MangroveUI.h"
#include "MangrovePlugin.h"
#include "IControls.h"
#include <cassert>

using namespace iplug;
using namespace iplug::igraphics;

void MangroveUI::Layout(IGraphics& ui, MangrovePlugin& /*plugin*/)
{
    // Register the UI font under the id "default", by FAMILY NAME, on EVERY
    // editor open. Same two bugs nassau-zermatt fixed in 7383dd9:
    //
    // 1. "Helvetica" does not exist on Windows. LoadFont returned false, every
    //    IText referenced an unregistered font id, and the NanoVG editor drew
    //    only the toggle bodies — no text, no knobs.
    // 2. A `static bool` guard ran the load once per process, but IGraphics (and
    //    its font storage) is recreated on every editor open, so the second
    //    editor came up with no font even on macOS.
    //
    // Loading by family name (not file path) also avoids the Skia crash when
    // measuring text that the old comment here guarded against.
    static const char* const kFontFamilies[] = {
#ifdef OS_WIN
        "Segoe UI", "Arial", "Tahoma", "Verdana"
#else
        "Helvetica", "Arial"
#endif
    };
    bool fontLoaded = false;
    for (const char* family : kFontFamilies) {
        if (ui.LoadFont("default", family, ETextStyle::Normal)) {
            fontLoaded = true;
            break;
        }
    }
    assert(fontLoaded && "No UI font could be loaded — all text will be invisible");
    (void) fontLoaded;

    const IColor bg(255, 40, 40, 40);
    const IColor text(255, 220, 220, 220);

    const IText header(24, text, "default");
    const IText section(18, text, "default");
    const IText knobLbl(14, text, "default");

    const IVStyle toggleStyle = DEFAULT_STYLE
        .WithValueText(IText(16, text, "default"))
        .WithLabelText(IText(16, text, "default"));

    // DEFAULT_STYLE's text names iPlug2's DEFAULT_FONT ("Roboto-Regular"),
    // which IGraphics does not auto-load, so knobs must be pointed at "default".
    const IVStyle knobStyle = DEFAULT_STYLE
        .WithLabelText(IText(14, text, "default"))
        .WithValueText(IText(12, text, "default"));

    ui.AttachPanelBackground(bg);

    // Header
    ui.AttachControl(new ITextControl(IRECT(20,  10, 300, 40), "MANGROVE", header));
    ui.AttachControl(new ITextControl(IRECT(500, 10, 620, 40), "Nassau",   header));

    // Section headers
    ui.AttachControl(new ITextControl(IRECT( 20, 60, 180, 80), "INPUT",   section));
    ui.AttachControl(new ITextControl(IRECT(230, 60, 390, 80), "LEVEL",   section));
    ui.AttachControl(new ITextControl(IRECT(440, 60, 600, 80), "DENSITY", section));

    // INPUT section — x=20, same two-column grid as LEVEL and DENSITY. A third
    // row at y=310 put the Saturate label at y=395..410, past PLUG_HEIGHT.
    ui.AttachControl(new IVKnobControl(IRECT(20,  90, 90, 170), kInputGain, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(20, 175, 90, 190), "Gain",     knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(100,  90, 170, 170), kInputSaturate, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(100, 175, 170, 190), "Saturate", knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(20, 200, 90, 280), kInputLoCut, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(20, 285, 90, 300), "LoCut",    knobLbl));

    // LEVEL section — x=230
    ui.AttachControl(new IVKnobControl(IRECT(230,  90, 300, 170), kLevelThreshold, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(230, 175, 300, 190), "Thresh",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(310,  90, 380, 170), kLevelRatio, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(310, 175, 380, 190), "Ratio",   knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(230, 200, 300, 280), kLevelAttack, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(230, 285, 300, 300), "Attack",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(310, 200, 380, 280), kLevelRelease, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(310, 285, 380, 300), "Release", knobLbl));

    // Level toggles — 4 across the LEVEL column (x=230..389), labels drawn by control itself
    ui.AttachControl(new IVToggleControl(IRECT(230, 315, 266, 380), kLevelLoCut,    "LoCut", toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(271, 315, 307, 380), kLevelTubeGain, "Tube",  toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(312, 315, 348, 380), kLevelFeedback, "FB",    toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(353, 315, 389, 380), kLevelFast,     "Fast",  toggleStyle));

    // DENSITY section — x=440
    ui.AttachControl(new IVKnobControl(IRECT(440,  90, 510, 170), kDensityThreshold, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(440, 175, 510, 190), "Thresh",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(520,  90, 590, 170), kDensityRatio, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(520, 175, 590, 190), "Ratio",   knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(440, 200, 510, 280), kDensityAttack, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(440, 285, 510, 300), "Attack",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(520, 200, 590, 280), kDensityRelease, "", knobStyle));
    ui.AttachControl(new ITextControl (IRECT(520, 285, 590, 300), "Release", knobLbl));
}
