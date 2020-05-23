# Platform detection
config_setting(
    name = "linux",
    constraint_values = [
        "@platforms//os:linux",
    ],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "windows",
    constraint_values = [
        "@platforms//os:windows",
    ],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "distro_arch",
    values = {"define": "distro=arch"},
)

config_setting(
    name = "distro_debian",
    values = {"define": "distro=debian"},
)

# Binaries
cc_binary(
    name = "emunisce_linux",
    visibility = ["//visibility:public"],

    deps = [
        "//wx_application:application_lib",
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
        "//wx_application:application_lib",
    ],

    linkopts = [
        "-SUBSYSTEM:WINDOWS"
    ],
)

cc_binary(
    name = "emunisce_windows_win32",
    visibility = ["//visibility:public"],

    deps = [
        "//windows_application:application_lib",
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
        "//:distro_arch": [
            "@gl_arch//:gl"
        ],
        "//:linux": [
            "@gl_debian//:gl"
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
        "//:distro_arch": [
            "@wx_setup_arch//:wx_setup",
            "@wx_linux//:wx",
            "@wx_arch_gtk//:wx_gtk",
            "@wx_arch_gtk_libs//:wx_gtk_libs",
        ],
        "//:linux": [
            "@wx_setup_debian//:wx_setup",
            "@wx_linux//:wx",
            "@wx_debian_gtk//:wx_gtk",
            "@wx_debian_gtk_libs//:wx_gtk_libs",
        ],
        "//:windows": [
            "@wx_windows//:wx"
        ],
    })
)
