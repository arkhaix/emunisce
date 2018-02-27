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
        # from $ wx-config --libs --gl-libs
        #"-L/usr/lib/x86_64-linux-gnu",
        "-SUBSYSTEM:WINDOWS"
    ],
)
