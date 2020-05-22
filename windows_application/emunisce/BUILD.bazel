cc_library(
    name = "application_lib",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//platform:platform",
        "//machine:machine",
        "//base_application:base_application",

        "@win32//:win32",

        "//windows_application/win32_common:win32_common",
        "//windows_application/gdi_plus_renderer:gdi_plus_renderer",
        "//windows_application/wave_out_sound:wave_out_sound",
    ],
)

cc_library(
    name = "application_headers",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),
)
