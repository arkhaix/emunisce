GL_BUILD = """
cc_library(
    name = "gl",
    visibility = ["//visibility:public"],
    srcs = ["libGL.so","libGLEW.so"],
)
    """

new_local_repository(
    name = "gl_debian",
    path = "/usr/lib/x86_64-linux-gnu/",

    build_file_content = GL_BUILD,
)

new_local_repository(
    name = "gl_arch",
    path = "/usr/lib/",

    build_file_content = GL_BUILD,
)

new_local_repository(
    name = "win32",
    path = "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64",

    build_file_content = """
cc_library(
    name = "win32",
    visibility = ["//visibility:public"],
    srcs = [
        "comdlg32.lib",
        "kernel32.lib",
        "gdi32.lib",
        "shell32.lib",
        "shlwapi.lib",
        "user32.lib",
    ],
)
cc_library(
    name = "gl",
    visibility = ["//visibility:public"],
    srcs = ["OpenGL32.lib"],
)
cc_library(
    name = "gdiplus",
    visibility = ["//visibility:public"],
    srcs = ["gdiplus.lib"],
)
cc_library(
    name = "winmm",
    visibility = ["//visibility:public"],
    srcs = ["winmm.lib"],
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

WX_SETUP_BUILD = """
cc_library(
    name = "wx_setup",
    visibility = ["//visibility:public"],
    hdrs = glob([
        "wx/*.h",
        "wx/**/*.h",
    ]),

    includes = ["."],
)
"""

new_local_repository(
    name = "wx_setup_debian",
    path = "/usr/lib/x86_64-linux-gnu/wx/include/gtk3-unicode-3.0",

    build_file_content = WX_SETUP_BUILD,
)

new_local_repository(
    name = "wx_setup_arch",
    path = "/usr/lib/wx/include/gtk2-unicode-3.0",

    build_file_content = WX_SETUP_BUILD,
)

GTK_BUILD = """
cc_library(
    name = "wx_gtk",
    visibility = ["//visibility:public"],

    hdrs = glob([
        "wx/*.h",
    ]),

    includes = ["."],
)
"""

new_local_repository(
    name = "wx_debian_gtk",
    path = "/usr/lib/x86_64-linux-gnu/",

    build_file_content = GTK_BUILD,
)

new_local_repository(
    name = "wx_arch_gtk",
    path = "/usr/lib/",

    build_file_content = GTK_BUILD,
)

GTK_LIBS_BUILD = """
cc_library(
    name = "wx_gtk_libs",
    visibility = ["//visibility:public"],
    srcs = [
        "libwx_gtk3u_gl-3.0.so",
        "libwx_gtk3u_xrc-3.0.so",
        "libwx_gtk3u_qa-3.0.so",
        "libwx_gtk3u_html-3.0.so",
        "libwx_gtk3u_adv-3.0.so",
        "libwx_gtk3u_core-3.0.so",
        "libwx_baseu_xml-3.0.so",
        "libwx_baseu_net-3.0.so",
        "libwx_baseu-3.0.so",
    ],
)
"""

new_local_repository(
    name = "wx_debian_gtk_libs",
    path = "/usr/lib/x86_64-linux-gnu/",
    build_file_content = GTK_LIBS_BUILD,
)

new_local_repository(
    name = "wx_arch_gtk_libs",
    path = "/usr/lib/",
    build_file_content = GTK_LIBS_BUILD,
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
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