# Declare default build environment.

VariantDir("build", "src", duplicate=0)
default_env = Environment(
    CPPPATH = "./include",
    CXXFLAGS = ["-std=c++11", "-pthread", "-Wall", "-Wextra"],
    CPPFLAGS = ["-DTEST_PROCESS_BY_EXE"],
    )

if not default_env.GetOption('clean'):
    conf = Configure(default_env)
    # Check first if libxdo is available at all.
    if not conf.CheckLib("xdo", autoadd=False):
        print "Did not find libxdo.a or xdo.lib, exiting"
        Exit(1)
    # After that, do a more granular check for a specific function.
    # This is necessary because libxdo changed its API at some point.
    # And we want to get the right version.
    if not conf.CheckLibWithHeader(libs=["xdo"], header="xdo.h",
                                   call="xdo_get_active_window(0, 0);",
                                   language="C"):
        print "Did not find function xdo_get_active_window, exiting"
        print "(Make sure you have the latest version of libxdo)"
        Exit(1)
    default_env = conf.Finish()

# Declare diverging release and debug environments.

release_env = default_env.Clone()
release_env.Append(CXXFLAGS=["-O2", "-flto"], CPPFLAGS=["-DNDEBUG"])

debug_env = default_env.Clone()
debug_env.Append(CXXFLAGS=["-g"], CPPFLAGS=["-D_DEBUG"])

# Choose whether to debug or not.

AddOption('--build-debug',
          dest='build-debug',
          type='string',
          nargs=0,
          action='store',
          help='Build debug version of Autosave')
main_env = release_env if GetOption("build-debug") is None else debug_env

# All targets.

Default("autosave")

autosave_sources = (
    Glob("build/core/*.cpp") +
    Glob("build/*/{}/*.cpp".format(main_env["PLATFORM"]))
    )
main_env.Program(
    target = "autosave",
    source = autosave_sources + ["build/autosave.cpp"],
    )

main_env.StaticLibrary(
    target = "build/libautosave.a",
    source = autosave_sources,
    )

test_env = main_env.Clone()
test_env.Append(
    LIBPATH = ["./build"],
    LIBS = ["autosave"],
    )

test_env.Command(
    target="run_tests",
    source="test/specs",
    action="test/specs",
    )

test_sources = (
    Glob("test/specs.cpp") +
    Glob("test/core/*.cpp") +
    Glob("test/*/{}/*.cpp".format(main_env["PLATFORM"]))
    )
test_env.Program(
    target = "test/specs",
    source = test_sources,
    )

