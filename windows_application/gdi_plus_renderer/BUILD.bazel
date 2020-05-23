cc_library(
    name = "gdi_plus_renderer",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//platform:platform",
        "//machine:machine",
        "//base_application:base_application",

        "@win32//:gdiplus",

        "//windows_application/win32_common:win32_common",
        "//windows_application/emunisce:application_headers",
        "//windows_application/wave_out_sound:wave_out_sound_headers",
    ],
)

cc_library(
    name = "gdi_plus_renderer_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)
