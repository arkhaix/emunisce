new_local_repository(
    name = "gl_linux",
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
    name = "gl_windows",
    path = "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64",

    build_file_content = """
cc_library(
    name = "gl",
    visibility = ["//visibility:public"],
    srcs = ["OpenGL32.lib"],
)
    """
)

new_local_repository(
    name = "wx_linux",
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
    name = "wx_linux_gtk",
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
    name = "wx_linux_gtk_libs",
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

new_http_archive(
    name = "wx_windows",
    url = "https://storage.googleapis.com/arkhaix-emunisce/wxWidgets-3.0.3-binariesonly.zip",
    build_file_content = """
cc_library(
    name = "wx",
    visibility = ["//visibility:public"],
    hdrs = glob([
        "include/wx/*.h",
        "include/wx/**/*.h",
        "include/msvc/wx/*.h",
        "lib/vc_x64_lib/**/*.h"
    ]),
    includes = [
        "include",
        "include/msvc",
    ],
    srcs = glob([
        "lib/vc_x64_lib/wxbase30u.lib",
        "lib/vc_x64_lib/wxbase30u_*.lib",
        "lib/vc_x64_lib/wxmsw30u_*.lib",
        "lib/vc_x64_lib/wxexpat.lib",
        "lib/vc_x64_lib/wxjpeg.lib",
        "lib/vc_x64_lib/wxpng.lib",
        "lib/vc_x64_lib/wxregexu.lib",
        "lib/vc_x64_lib/wxscintilla.lib",
        "lib/vc_x64_lib/wxtiff.lib",
        "lib/vc_x64_lib/wxzlib.lib",
    ]),
)
    """
)