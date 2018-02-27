# Platform detection
config_setting(
    name = "linux",
    values = {"cpu": "k8"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "windows",
    values = {"cpu": "x64_windows"},
    visibility = ["//visibility:public"],
)

# Binaries
cc_binary(
    name = "emunisce_linux",
    visibility = ["//visibility:public"],

    deps = [
        "//wxApplication:application_lib",
    ],

    linkopts = [
        # from $ wx-config --libs --gl-libs
        #"-L/usr/lib/x86_64-linux-gnu",
        "-pthread",
        "-Wl,-Bsymbolic-functions",
        "-Wl,-z,relro",
    ],
)

cc_binary(
    name = "emunisce_windows_wx",
    visibility = ["//visibility:public"],

    deps = [
        "//wxApplication:application_lib",
    ],

    linkopts = [
        "-SUBSYSTEM:WINDOWS"
    ],
)

cc_binary(
    name = "emunisce_windows_win32",
    visibility = ["//visibility:public"],

    deps = [
        "//WindowsApplication:application_lib",
    ],

    linkopts = [
        "-SUBSYSTEM:WINDOWS"
    ],
)

# Platform-specific repository selection
cc_library(
    name = "gl",
    visibility = ["//visibility:public"],

    deps = select({
        "//:linux": [
            "@gl_linux//:gl"
        ],
        "//:windows": [
            "@win32//:gl"
        ],
    })
)

cc_library(
    name = "wx",
    visibility = ["//visibility:public"],

    deps = select({
        "//:linux": [
            "@wx_linux//:wx",
            "@wx_linux_gtk//:wx_gtk",
            "@wx_linux_gtk_libs//:wx_gtk_libs",
        ],
        "//:windows": [
            "@wx_windows//:wx"
        ],
    })
)
