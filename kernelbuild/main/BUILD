cc_library(
    name = "mimalloc",
    srcs = [
        "mimalloc/src/mimalloc/src/alloc.c",
        "mimalloc/src/mimalloc/src/alloc-aligned.c",
        "mimalloc/src/mimalloc/src/alloc-posix.c",
        "mimalloc/src/mimalloc/src/arena.c",
        "mimalloc/src/mimalloc/src/bitmap.c",
        "mimalloc/src/mimalloc/src/heap.c",
        "mimalloc/src/mimalloc/src/init.c",
        "mimalloc/src/mimalloc/src/libc.c",
        "mimalloc/src/mimalloc/src/options.c",
        "mimalloc/src/mimalloc/src/os.c",
        "mimalloc/src/mimalloc/src/page.c",
        "mimalloc/src/mimalloc/src/random.c",
        "mimalloc/src/mimalloc/src/segment.c",
        "mimalloc/src/mimalloc/src/segment-map.c",
        "mimalloc/src/mimalloc/src/stats.c",
        "mimalloc/src/mimalloc/src/prim/prim.c"],
    hdrs = glob(["mimalloc/src/mimalloc/include/**/*.h", "mimalloc/src/mimalloc/src/**/*.h", "mimalloc/src/mimalloc/src/**/*.c"]),
    includes = ["mimalloc/src/mimalloc/include", "mimalloc/src/mimalloc/src"],
    visibility = ["//:__pkg__"],
)

cc_library(
    name = "stringzilla",
    srcs = ["stringzilla/c/lib.c"],
    hdrs = ["stringzilla/include/stringzilla/stringzilla.h", "stringzilla/include/stringzilla/stringzilla.hpp"],
    includes = ["stringzilla/include"],
    visibility = ["//:__pkg__"],
)

cc_library(
    name = "lean_basics",
    hdrs = glob(["src/include/**/*.h"]),
    includes = ["src/include"],
    deps = [":mimalloc"],
    visibility = ["//:__pkg__"],
)

cc_library(
    name = "kernel",
    srcs = glob(["src/kernel/**/*.cpp", "src/runtime/**/*.cpp", "src/util/**/*.cpp", "src/library/**/*.cpp", "initialize/init.cpp", "lean_export/**/*.c"], exclude=["src/runtime/uv/**/*.c", "src/library/compiler/**/*.cpp"]),
    hdrs = glob(["src/kernel/**/*.h", "src/runtime/**/*.h", "src/util/**/*.h", "src/library/**/*.h", "initialize/init.h", "stdlib_flags.h"]),
    includes = [".", "src"],
    deps = [":mimalloc", ":lean_basics"],
    visibility = ["//:__pkg__"],
)

cc_binary(
    name = "parser",
    srcs = ["parser/parser.cpp", "parser/parser.h", "parser/driver.cpp"],
    includes = ["."],
    visibility = ["//:__pkg__"],
    deps = [":stringzilla", ":kernel"],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [":kernel"],
)