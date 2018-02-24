genrule(
    name = "emunisce_linux",
    srcs = [":emunisce_linux_app"],
    outs = ["emunisce"],
    cmd = "cp $(location :emunisce_linux_app) $@"
)

cc_binary(
    name = "emunisce_linux_app",
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