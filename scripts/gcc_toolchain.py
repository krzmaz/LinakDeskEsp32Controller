Import("env")

env.Replace(
    AR="xtensa-esp32-elf-gcc-ar",
    RANLIB="xtensa-esp32-elf-gcc-ranlib"
)