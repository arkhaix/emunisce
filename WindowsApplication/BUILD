cc_library(
    name = "application_lib",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),

    deps = [
        "//Platform:platform",
        "//Machine:machine",
        "//BaseApplication:base_application",

        "@wx//:wx",
        "@wx_gtk//:wx_gtk",

        "@wx_gtk_libs//:wx_gtk_libs",
    ],

    copts = [
        # from wx-config --cppflags
        #"-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-release-2.8",
        #"-I/usr/include/wx-2.8",
        "-D_FILE_OFFSET_BITS=64",
        "-D_LARGE_FILES",
        "-D__WXGTK__",
    ],
)