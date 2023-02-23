

from os.path import join, isfile

Import("env")

PLATFORM_DIR = env.PioPlatform().get_dir()
patchflag_path = join(PLATFORM_DIR, ".patching-done")

# patch file only if we didn't do it before
if not isfile(join(PLATFORM_DIR, ".patching-done")):
    original_file = join(PLATFORM_DIR, "builder", "main.py")
    patched_file = join("patches", "gcc-ar-ranlib.patch")

    assert isfile(original_file) and isfile(patched_file)

    # If the patching fails, you can change the platform package manually and create 
    # a file called `.patching-done` to indicate that. For more details see:
    # https://github.com/krzmaz/LinakDeskEsp32Controller/issues/13
    if env.Execute("patch --ignore-whitespace --fuzz 3 --binary %s %s" % (original_file, patched_file)) != 0:
        raise Exception("Problem while applying platform patches!\n\n"
            "See scripts/apply_patches.py for more details!\n")

    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))