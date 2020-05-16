cc_library(
    name = "application_lib",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//Platform:platform",
        "//Machine:machine",
        "//BaseApplication:base_application",

        "@win32//:win32",

        "//WindowsApplication/Win32Common:win32_common",
        "//WindowsApplication/GdiPlusRenderer:gdi_plus_renderer",
        "//WindowsApplication/WaveOutSound:wave_out_sound",
    ],
)

cc_library(
    name = "application_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)
