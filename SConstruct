import os

def get_default_env():
    """Return a default environment."""
    return Environment(
        ENV = os.environ,
        CPPPATH = "#./include",
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
    #~ result.Prepend(LIBS=["autosave"])   # libautosave must be *prepended*.
    #~ result.Append(LIBPATH=["./build"])  # So that libautosave is found.
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


# Define settings.

AddOption("--build-debug",
          dest="build-debug",
          action='store_true',
          help='Build debug version of Autosave')
VariantDir("build", "src")
VariantDir("build-tests", "test")

# Declare main targets.

main_env = get_default_env()
configure_debug(main_env, GetOption("build-debug"))
if not GetOption('clean'):
    configure_libs(main_env, ["pthread", "xcb", "xcb-keysyms", "xcb-xtest"])

libautosave, autosave = SConscript("build/SConscript", exports="main_env")

# Declare test targets.

test_env = get_test_env(main_env)
specs, run = SConscript("build-tests/SConscript", exports="test_env libautosave")

# Deckare phony default target.

Default("all")
Command(
    target = "all",
    source = [autosave, run],
    action = "",
    )

