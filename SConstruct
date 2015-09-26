
import os

# Declare default build environment.

VariantDir("build", "src", duplicate=0)
default_env = Environment(
    CPPPATH = "./include",
    CXXFLAGS = ["-std=c++14", "-pthread", "-Wall", "-Wextra"],
    #~ CPPFLAGS = ["-DTEST_PROCESS_BY_EXE"],
    )

if not default_env.GetOption('clean'):
    conf = Configure(default_env)
    for libname in ["pthread", "xcb", "xcb-keysyms", "xcb-xtest"]:
        if not conf.CheckLib(libname):
            print "Did not find lib{}, exiting".format(libname)
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
    ENV = os.environ,
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

