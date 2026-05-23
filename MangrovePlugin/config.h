#pragma once

#define PLUG_NAME        "Mangrove"
#define PLUG_MFR         "Nassau"
#define PLUG_VERSION_HEX  0x00050000
#define PLUG_VERSION_STR  "5.0.0"

#define PLUG_CLASS_NAME   MangrovePlugin

#define BUNDLE_NAME       "Mangrove"
#define BUNDLE_MFR        "Nassau"
#define BUNDLE_DOMAIN     "com"

// 4-char codes — unique, required by AU and some VST3 hosts
#define PLUG_UNIQUE_ID    'Mng5'
#define PLUG_MFR_ID       'Nss2'

// New UID avoids collision with native VST3 plugin (50D5F78F…9CD66A0C)
#define PLUG_UID_STR  "50D5F78FCD0A5CE54E1CA6CC9CD66A0E"

#define PLUG_CHANNEL_IO        "2-2"
#define PLUG_LATENCY           0
#define PLUG_IS_INSTRUMENT     0
#define PLUG_DOES_MIDI_IN      0
#define PLUG_DOES_MIDI_OUT     0
#define PLUG_DOES_MPE          0
#define PLUG_DOES_STATE_CHUNKS 0

#define PLUG_N_PARAMS   14
#define PLUG_N_PRESETS  1

#define PLUG_HAS_UI     1
#define PLUG_WIDTH      640
#define PLUG_HEIGHT     400
#define PLUG_FPS        60

#define PLUG_SHARED_RESOURCES_SUBPATH "Mangrove"
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 0

#define VST3_SUBCATEGORY "Fx"

#define ROBOTO_FN "Roboto-Regular.ttf"

// AUv2 AudioComponents registration
#define PLUG_TYPE       0           // 0 = aufx (effect)
#define PLUG_SUBTYPE_STR "Mng5"    // 4-char OSType matching PLUG_UNIQUE_ID
#define PLUG_MFR_STR     "Nss2"    // 4-char manufacturer
#define PLUG_BUNDLE_ID   "com.nassau.mangrove.iplug"
