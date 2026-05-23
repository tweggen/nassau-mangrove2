#include "MangroveUI.h"
#include "MangrovePlugin.h"
#include "IControls.h"

using namespace iplug;
using namespace iplug::igraphics;

void MangroveUI::Layout(IGraphics& ui, MangrovePlugin& plugin)
{
    ui.LoadFont("Roboto-Regular", ROBOTO_FN);

    const IColor bg(255, 40, 40, 40);
    const IColor text(255, 255, 255, 255);
    const IColor toggleLblColor(255, 0, 0, 0);
    const IColor knobColor(255, 100, 150, 200);

    const IText header(18, text, "Roboto-Regular", EAlign::Center, EVAlign::Middle);
    const IText section(14, text, "Roboto-Regular", EAlign::Center, EVAlign::Middle);
    const IText knobLbl(11, text, "Roboto-Regular", EAlign::Center, EVAlign::Middle);

    const IVStyle toggleStyle = DEFAULT_STYLE
        .WithValueText(IText(11, text, "Roboto-Regular", EAlign::Center, EVAlign::Middle))
        .WithLabelText(IText(12, toggleLblColor, "Roboto-Regular", EAlign::Center, EVAlign::Middle));

    ui.AttachPanelBackground(bg);

    // Header
    ui.AttachControl(new ITextControl(IRECT(20,  10, 300, 40), "MANGROVE", header));
    ui.AttachControl(new ITextControl(IRECT(500, 10, 620, 40), "Nassau",   header));

    // Section headers
    ui.AttachControl(new ITextControl(IRECT( 20, 60, 180, 80), "INPUT",   section));
    ui.AttachControl(new ITextControl(IRECT(230, 60, 390, 80), "LEVEL",   section));
    ui.AttachControl(new ITextControl(IRECT(440, 60, 600, 80), "DENSITY", section));

    // INPUT section — x=20
    ui.AttachControl(new IVKnobControl(IRECT(20,  90, 90, 170), kInputGain));
    ui.AttachControl(new ITextControl (IRECT(20, 175, 90, 190), "Gain",     knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(20, 200, 90, 280), kInputLoCut));
    ui.AttachControl(new ITextControl (IRECT(20, 285, 90, 300), "LoCut",    knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(20, 310, 90, 390), kInputSaturate));
    ui.AttachControl(new ITextControl (IRECT(20, 395, 90, 410), "Saturate", knobLbl));

    // LEVEL section — x=230
    ui.AttachControl(new IVKnobControl(IRECT(230,  90, 300, 170), kLevelThreshold));
    ui.AttachControl(new ITextControl (IRECT(230, 175, 300, 190), "Thresh",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(310,  90, 380, 170), kLevelRatio));
    ui.AttachControl(new ITextControl (IRECT(310, 175, 380, 190), "Ratio",   knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(230, 200, 300, 280), kLevelAttack));
    ui.AttachControl(new ITextControl (IRECT(230, 285, 300, 300), "Attack",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(310, 200, 380, 280), kLevelRelease));
    ui.AttachControl(new ITextControl (IRECT(310, 285, 380, 300), "Release", knobLbl));

    // Level toggles — 4 across the LEVEL column (x=230..389), labels drawn by control itself
    ui.AttachControl(new IVToggleControl(IRECT(230, 315, 266, 380), kLevelLoCut,    "LoCut", toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(271, 315, 307, 380), kLevelTubeGain, "Tube",  toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(312, 315, 348, 380), kLevelFeedback, "FB",    toggleStyle));
    ui.AttachControl(new IVToggleControl(IRECT(353, 315, 389, 380), kLevelFast,     "Fast",  toggleStyle));

    // DENSITY section — x=440
    ui.AttachControl(new IVKnobControl(IRECT(440,  90, 510, 170), kDensityThreshold));
    ui.AttachControl(new ITextControl (IRECT(440, 175, 510, 190), "Thresh",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(520,  90, 590, 170), kDensityRatio));
    ui.AttachControl(new ITextControl (IRECT(520, 175, 590, 190), "Ratio",   knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(440, 200, 510, 280), kDensityAttack));
    ui.AttachControl(new ITextControl (IRECT(440, 285, 510, 300), "Attack",  knobLbl));

    ui.AttachControl(new IVKnobControl(IRECT(520, 200, 590, 280), kDensityRelease));
    ui.AttachControl(new ITextControl (IRECT(520, 285, 590, 300), "Release", knobLbl));

    ui.Resize(PLUG_WIDTH, PLUG_HEIGHT, 1.);
}
