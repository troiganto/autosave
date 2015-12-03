
import os

def get_default_env():
    """Return a default environment."""
    return Environment(
        ENV = os.environ,
        CPPPATH = "./include",
        CXXFLAGS = ["-std=c++14", "-pthread", "-Wall", "-Wextra"],
        #~ CPPFLAGS = ["-DTEST_PROCESS_BY_EXE"],
        LIBS=[],
        )

def configure_debug(env, is_debug):
    """Set flags, etc. which depend on whether we build for debugging."""
    if is_debug:
        env.Append(CXXFLAGS=["-g"], CPPFLAGS=["-D_DEBUG"])
    else:
        env.Append(CXXFLAGS=["-O2", "-flto"], CPPFLAGS=["-DNDEBUG"])

def get_test_env(env):
    """Return an environment for unit test building derived from `env`."""
    result = env.Clone()
    result.Prepend(LIBS=["autosave"])   # libautosave must be *prepended*.
    result.Append(LIBPATH=["./build"])  # So that libautosave is found.
    return result

def configure_libs(env, libs):
    """Check for necessary libs, return the configured environment."""
    conf = Configure(env)
    missing = [lib for lib in libs if not conf.CheckLib(lib)]
    if missing:
        print "The following libs/headers could not be found:"
        for lib in missing:
            print "\t" + lib
        Exit(1)
    return conf.Finish()

def get_main_sources(env):
    """Return list of source files for the main program.

    This does not include the main file autosave.cpp.
    """
    platform_path = "build/*/{}/".format(env["PLATFORM"])
    result = ["build/autosave.cpp"]
    result.extend(Glob("build/core/*.cpp"))
    result.extend(Glob(platform_path + "*.cpp"))
    result.extend(Glob(platform_path + "*/*.cpp"))
    return result

def get_test_sources(env):
    """Return list of source files for unit tests.

    This does include the main file specs.cpp.
    """
    platform_path = "test/*/{}/".format(env["PLATFORM"])
    result = ["test/specs.cpp"]
    result.extend(Glob("test/core/*.cpp"))
    result.extend(Glob(platform_path + "*.cpp"))
    result.extend(Glob(platform_path + "*/*.cpp"))
    return result


# Define settings.

AddOption("--build-debug",
          dest="build-debug",
          action='store_true',
          help='Build debug version of Autosave')
VariantDir("build", "src", duplicate=0)

# Deckare phony default target.

Default("all")
Depends(
    target = "all",
    dependency = ["autosave", "run_tests"],
    )

# Declare main targets.

main_env = get_default_env()
configure_debug(main_env, GetOption("build-debug"))
if not GetOption('clean'):
    configure_libs(main_env, ["pthread", "xcb", "xcb-keysyms", "xcb-xtest"])

main_sources = get_main_sources(main_env)
main_env.Program(
    target = "autosave",
    source = main_sources,
    )
main_env.StaticLibrary(
    target = "build/libautosave.a",
    source = main_sources[1:],  # Ignore autosave.cpp.
    )

# Declare test targets.

test_env = get_test_env(main_env)

runner = "test/specs"
test_env.Command(
    target="run_tests",
    source = runner,
    action = runner,
    )

test_env.Program(
    target = runner,
    source = get_test_sources(test_env),
    )
