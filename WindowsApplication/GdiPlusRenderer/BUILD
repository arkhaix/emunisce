cc_library(
    name = "gdi_plus_renderer",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//Platform:platform",
        "//Machine:machine",
        "//BaseApplication:base_application",

        "@win32//:gdiplus",

        "//WindowsApplication/Emunisce:application_headers",
        "//WindowsApplication/WaveOutSound:wave_out_sound_headers",
    ],
)

cc_library(
    name = "gdi_plus_renderer_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)
