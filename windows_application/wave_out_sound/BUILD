cc_library(
    name = "wave_out_sound",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//platform:platform",
        "//machine:machine",
        "//base_application:base_application",

        "@win32//:winmm",
        
        "//windows_application/win32_common:win32_common",
        "//windows_application/emunisce:application_headers",
        "//windows_application/gdi_plus_renderer:gdi_plus_renderer_headers",
    ],
)

cc_library(
    name = "wave_out_sound_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)