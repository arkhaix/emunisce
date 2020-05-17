cc_library(
    name = "wave_out_sound",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//Platform:platform",
        "//Machine:machine",
        "//BaseApplication:base_application",

        "@win32//:winmm",
        
        "//WindowsApplication/Win32Common:win32_common",
        "//WindowsApplication/Emunisce:application_headers",
        "//WindowsApplication/GdiPlusRenderer:gdi_plus_renderer_headers",
    ],
)

cc_library(
    name = "wave_out_sound_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)