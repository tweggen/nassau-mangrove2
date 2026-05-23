#include "MangroveUI.h"
#include "MangrovePlugin.h"
#include "IControls.h"

using namespace iplug;
using namespace iplug::igraphics;

void MangroveUI::Layout(IGraphics& ui, MangrovePlugin& plugin)
{
    const IColor bg(255, 40, 40, 40);
    const IColor text(255, 240, 240, 240);
    const IColor knobColor(255, 100, 150, 200);

    ui.AttachBackground(bg);

    // Header
    ui.AttachControl(new IVLabelControl(IRECT(20, 10, 300, 40), "MANGROVE",
                                         {14, EVAlign::Center}, &text));
    ui.AttachControl(new IVLabelControl(IRECT(500, 10, 620, 40), "Nassau",
                                         {12, EVAlign::Center}, &text));

    // Section headers
    ui.AttachControl(new IVLabelControl(IRECT(20, 60, 180, 80), "INPUT",
                                         {11, EVAlign::Center}, &text));
    ui.AttachControl(new IVLabelControl(IRECT(230, 60, 390, 80), "LEVEL",
                                         {11, EVAlign::Center}, &text));
    ui.AttachControl(new IVLabelControl(IRECT(440, 60, 600, 80), "DENSITY",
                                         {11, EVAlign::Center}, &text));

    // INPUT section — x=20, cols at y=90, 180, 270
    ui.AttachControl(new IVKnobControl(IRECT(20, 90, 90, 170), kInputGain));
    ui.AttachControl(new IVLabelControl(IRECT(20, 175, 90, 190), "Gain",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(20, 200, 90, 280), kInputLoCut));
    ui.AttachControl(new IVLabelControl(IRECT(20, 285, 90, 300), "LoCut",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(20, 310, 90, 390), kInputSaturate));
    ui.AttachControl(new IVLabelControl(IRECT(20, 395, 90, 410), "Saturate",
                                         {10, EVAlign::Center}, &text));

    // LEVEL section — x=230
    ui.AttachControl(new IVKnobControl(IRECT(230, 90, 300, 170), kLevelThreshold));
    ui.AttachControl(new IVLabelControl(IRECT(230, 175, 300, 190), "Thresh",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(310, 90, 380, 170), kLevelRatio));
    ui.AttachControl(new IVLabelControl(IRECT(310, 175, 380, 190), "Ratio",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(230, 200, 300, 280), kLevelAttack));
    ui.AttachControl(new IVLabelControl(IRECT(230, 285, 300, 300), "Attack",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(310, 200, 380, 280), kLevelRelease));
    ui.AttachControl(new IVLabelControl(IRECT(310, 285, 380, 300), "Release",
                                         {10, EVAlign::Center}, &text));

    // Level toggles
    ui.AttachControl(new IVToggleControl(IRECT(230, 310, 270, 330), kLevelLoCut));
    ui.AttachControl(new IVLabelControl(IRECT(275, 310, 360, 330), "LoCut",
                                         {9, EVAlign::Left}, &text));

    ui.AttachControl(new IVToggleControl(IRECT(310, 310, 350, 330), kLevelTubeGain));
    ui.AttachControl(new IVLabelControl(IRECT(355, 310, 430, 330), "Tube",
                                         {9, EVAlign::Left}, &text));

    ui.AttachControl(new IVToggleControl(IRECT(380, 310, 420, 330), kLevelFeedback));
    ui.AttachControl(new IVLabelControl(IRECT(425, 310, 480, 330), "FB",
                                         {9, EVAlign::Left}, &text));

    // DENSITY section — x=440
    ui.AttachControl(new IVKnobControl(IRECT(440, 90, 510, 170), kDensityThreshold));
    ui.AttachControl(new IVLabelControl(IRECT(440, 175, 510, 190), "Thresh",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(520, 90, 590, 170), kDensityRatio));
    ui.AttachControl(new IVLabelControl(IRECT(520, 175, 590, 190), "Ratio",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(440, 200, 510, 280), kDensityAttack));
    ui.AttachControl(new IVLabelControl(IRECT(440, 285, 510, 300), "Attack",
                                         {10, EVAlign::Center}, &text));

    ui.AttachControl(new IVKnobControl(IRECT(520, 200, 590, 280), kDensityRelease));
    ui.AttachControl(new IVLabelControl(IRECT(520, 285, 590, 300), "Release",
                                         {10, EVAlign::Center}, &text));

    ui.Resize(PLUG_WIDTH, PLUG_HEIGHT, 1.);
}
