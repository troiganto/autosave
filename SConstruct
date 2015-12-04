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

def configure_release(env, is_release):
    """Set flags, etc. which depend on whether we build for debugging."""
    if is_release:
        env.Append(CXXFLAGS=["-O2", "-flto"], CPPFLAGS=["-DNDEBUG"])
    else:
        env.Append(CXXFLAGS=["-g"], CPPFLAGS=["-D_DEBUG"])

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

AddOption("--release",
          dest="release",
          action="store_true",
          help="Build release version of Autosave")
VariantDir("build", "src")
VariantDir("build-tests", "tests")

# Configure environment.

main_env = get_default_env()
configure_release(main_env, GetOption("release"))
if not GetOption('clean'):
    configure_libs(main_env, ["pthread", "xcb", "xcb-keysyms", "xcb-xtest"])

# Declare targets.

libautosave, autosave = SConscript("build/SConscript",
                                   exports=["main_env"])
specs, run_tests = SConscript("build-tests/SConscript",
                              exports=["main_env", "libautosave"])
Command(target = "all",
        source = [autosave, run_tests],
        action = "")
Default("all")

