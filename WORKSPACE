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
    path = "/usr/include/wx-2.8",

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
    path = "/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-release-2.8",

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
        "libwx_gtk2u_gl-2.8.so",
        "libwx_gtk2u_richtext-2.8.so",
        "libwx_gtk2u_aui-2.8.so",
        "libwx_gtk2u_xrc-2.8.so",
        "libwx_gtk2u_qa-2.8.so",
        "libwx_gtk2u_html-2.8.so",
        "libwx_gtk2u_adv-2.8.so",
        "libwx_gtk2u_core-2.8.so",
        "libwx_baseu_xml-2.8.so",
        "libwx_baseu_net-2.8.so",
        "libwx_baseu-2.8.so",
    ],
)
    """
)