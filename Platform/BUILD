cc_library(
    name = "platform",
    visibility = ["//visibility:public"],

    hdrs = glob(["*.h"]),

    deps = select({
        "//:linux": [
            "//LinuxPlatform:linux_platform"
        ],
        "//:windows": [
            "//WindowsPlatform:windows_platform"
        ],
    }),

    includes = ["."],
)