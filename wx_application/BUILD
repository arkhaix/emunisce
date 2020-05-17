cc_library(
    name = "application_lib",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//Platform:platform",
        "//Machine:machine",
        "//BaseApplication:base_application",

        "//:wx",
    ],

    copts = select({
        "//:linux": [
            # from wx-config --cppflags
            "-D_FILE_OFFSET_BITS=64",
            "-D_LARGE_FILES",
            "-D__WXGTK__",
        ],

        "//:windows": [
            "-D__WXMSW__",
            "-DwxUSE_GUI=1",
            "-DUNICODE",
            "-D_UNICODE",
            "-DWIN32",
            "-DWINDOWS",
            "-DWINVER=0x0400",
            "-D_CRT_SECURE_NO_DEPRECATE",
            "-D_CRT_NONSTDC_NO_DEPRECATE",
        ],
    }),
)