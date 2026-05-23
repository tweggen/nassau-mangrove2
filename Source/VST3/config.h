#pragma once

// ===== Plugin Identity =====
#define PLUG_NAME             "Mangrove"
#define PLUG_MFR              "Nassau"
#define PLUG_VERSION_HEX      0x00040000
#define PLUG_VERSION_STR      "4.0.0"
#define PLUG_URL_STR          ""
#define PLUG_EMAIL_STR        ""
#define PLUG_COPYRIGHT_STR    "© Nassau 2026"

// ===== Unique Identifiers =====
#define PLUG_UNIQUE_ID        'Mngv'
#define PLUG_MFR_ID           'Nssa'
#define BUNDLE_MFR            "Nassau"
#define BUNDLE_NAME           "Mangrove"
#define BUNDLE_DOMAIN         "com.nassau"

// ===== Plugin Type =====
#define PLUG_TYPE             0  // 0 = Effect, 1 = Instrument, 2 = MIDI Effect
#define PLUG_CLASS_NAME       MangrovePlugin

// ===== Audio I/O =====
#define PLUG_CHANNEL_IO       "2-2"
#define PLUG_LATENCY          0
#define PLUG_DOES_MIDI_IN     0
#define PLUG_DOES_MIDI_OUT    0
#define PLUG_DOES_MPE         0
#define PLUG_DOES_STATE_CHUNKS 0

// ===== GUI Configuration (Phase 4: disabled) =====
#define PLUG_HAS_UI           0
#define PLUG_WIDTH            600
#define PLUG_HEIGHT           400
#define PLUG_FPS              60
#define PLUG_HOST_RESIZE      0
#define PLUG_MIN_WIDTH        600
#define PLUG_MAX_WIDTH        600
#define PLUG_MIN_HEIGHT       400
#define PLUG_MAX_HEIGHT       400

// ===== Parameters & Presets =====
#define PLUG_N_PARAMS         14
#define PLUG_N_PRESETS        1

// ===== Formats =====
#define VST3_API              1
#define VST3_SUBCATEGORY      "Dynamics"

// ===== Resources =====
#define PLUG_SHARED_RESOURCES 0
