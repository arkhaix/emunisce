cc_library(
    name = "machine",
    visibility = ["//visibility:public"],

    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h", "*.hpp"]),

    deps = [
        "//Platform:platform",
        "//Utility:utility",
        "//Gameboy:gameboy",
    ],
    includes = ["."],
)

cc_library(
    name = "machine_interfaces",
    visibility = ["//visibility:public"],

    hdrs = [
        "MachineIncludes.h",
        "MachineTypes.h",

        "MachineFactory.h",
        "MachineSerialization.h",

        "IMachineToApplication.h",

        "IEmulatedMachine.h",
        "IEmulatedDisplay.h",
        "IEmulatedInput.h",
        "IEmulatedMemory.h",
        "IEmulatedProcessor.h",
        "IEmulatedSound.h",

        "ScreenBuffer.h",
    ],

    includes = ["."],
)