new_local_repository(
    name = "gl",
    path = "/usr/lib/x86_64-linux-gnu/",

    build_file_content = """
cc_library(
    name = "gl",
    visibility = ["//visibility:public"],
    srcs = ["libGL.so"],
)
    """
)

new_local_repository(
    name = "wx",
    path = "/usr/include/wx-3.0",

    build_file_content = """
cc_library(
    name = "wx",
    visibility = ["//visibility:public"],

    hdrs = glob([
        "wx/*.h",
        "wx/**/*.h",
    ]),

    includes = ["."],
)
    """
)

new_local_repository(
    name = "wx_gtk",
    path = "/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0",

    build_file_content = """
cc_library(
    name = "wx_gtk",
    visibility = ["//visibility:public"],

    hdrs = glob([
        "wx/*.h",
    ]),

    includes = ["."],
)
    """
)

new_local_repository(
    name = "wx_gtk_libs",
    path = "/usr/lib/x86_64-linux-gnu",
    build_file_content = """
cc_library(
    name = "wx_gtk_libs",
    visibility = ["//visibility:public"],
    srcs = [
        "libwx_gtk2u_gl-3.0.so",
        "libwx_gtk2u_xrc-3.0.so",
        "libwx_gtk2u_qa-3.0.so",
        "libwx_gtk2u_html-3.0.so",
        "libwx_gtk2u_adv-3.0.so",
        "libwx_gtk2u_core-3.0.so",
        "libwx_baseu_xml-3.0.so",
        "libwx_baseu_net-3.0.so",
        "libwx_baseu-3.0.so",
    ],
)
    """
)