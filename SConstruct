
import os

def get_default_env():
    """Return a default environment."""
    return Environment(
        ENV = os.environ,
        CPPPATH = "./include",
        CXXFLAGS = ["-std=c++14", "-pthread", "-Wall", "-Wextra"],
        #~ CPPFLAGS = ["-DTEST_PROCESS_BY_EXE"],
        )

def get_test_env(env):
    """Return an environment for unit test building derived from `env`."""
    # Note that libautosave is prepended, not appended.
    result = env.Clone(LIBS=["autosave"])
    result.Append(
        LIBPATH = ["./build"],  # So that libautosave is found.
        LIBS = env["LIBS"],
        )
    return result

def configure_libs(env):
    """Check for necessary libs, return the configured environment."""
    conf = Configure(env)
    for libname in ["pthread", "xcb", "xcb-keysyms", "xcb-xtest"]:
        if not conf.CheckLib(libname):
            print "Did not find lib{}, exiting".format(libname)
            Exit(1)
    return conf.Finish()

def configure_debug(env, is_debug):
    """Set flags, etc. which depend on whether we build for debugging."""
    if is_debug:
        env.Append(CXXFLAGS=["-g"], CPPFLAGS=["-D_DEBUG"])
    else:
        env.Append(CXXFLAGS=["-O2", "-flto"], CPPFLAGS=["-DNDEBUG"])

def get_main_sources(env):
    """Return list of source files for the main program.

    This does not include the main file autosave.cpp.
    """
    platform_path = "build/*/{}/".format(env["PLATFORM"])
    result = []
    result.extend(Glob("build/core/*.cpp"))
    result.extend(Glob(platform_path + "*.cpp"))
    result.extend(Glob(platform_path + "*/*.cpp"))
    return result

def get_test_sources(env):
    """Return list of source files for unit tests.

    This does include the main file specs.cpp.
    """
    platform_path = "test/*/{}/".format(env["PLATFORM"])
    result = []
    result.extend(Glob("test/specs.cpp"))
    result.extend(Glob("test/core/*.cpp"))
    result.extend(Glob(platform_path + "*.cpp"))
    result.extend(Glob(platform_path + "*/*.cpp"))
    return result

def declare_main_targets(env):
    """Declare targets autosave and libautosave.a."""
    autosave_sources = get_main_sources(main_env)
    main_env.StaticLibrary(
        target = "build/libautosave.a",
        source = autosave_sources,
        )

    main_env.Program(
        target = "autosave",
        source = autosave_sources + ["build/autosave.cpp"],
        )

def declare_test_targets(env):
    """Declare targets specs and run_tests."""
    test_env.Command(
        target="run_tests",
        source="test/specs",
        action="test/specs",
        )

    test_env.Program(
        target = "test/specs",
        source = get_test_sources(test_env),
        )


# Main.

AddOption('--build-debug',
          dest='build-debug',
          type='string',
          nargs=0,
          action='store',
          help='Build debug version of Autosave')
VariantDir("build", "src", duplicate=0)
Default("autosave")

main_env = get_default_env()

if GetOption('clean'):
    pass
else:
    main_env = configure_libs(main_env)
    configure_debug(main_env, GetOption("build-debug"))
    test_env = get_test_env(main_env)
    declare_main_targets(main_env)
    declare_test_targets(test_env)
