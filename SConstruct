
# Declare build environments.

VariantDir("build", "src", duplicate=0)
default_env = Environment(
    CPPPATH = "./include",
    CXXFLAGS = ["-std=c++11", "-pthread", "-Wall", "-Wextra"],
    )

release_env = default_env.Clone()
release_env.Append(CXXFLAGS=["-O2", "-flto"])

debug_env = default_env.Clone()
debug_env.Append(CXXFLAGS=["-g"])

# Choose whether to debug or not.

AddOption('--build-debug',
          dest='build-debug',
          type='string',
          nargs=0,
          action='store',
          help='Build debug version of Autosave')
main_env = release_env if GetOption("build-debug") is None else debug_env

# Build description.

main_env.Program(
    target = "autosave",
    source =
        Glob("build/autosave.cpp") +
        Glob("build/core/*.cpp"),
    )
