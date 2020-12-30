# ***Work in progress, use at own risk!***

Currently there is only a proof of concept which moves the desk to an hardcoded height after booting up.
# LinakDeskEsp32Controller

The goal of this project is creating an easy to use bluetooth bridge for my Ikea Idasen desk, that uses a Linak controller.

I often switch computers, that's why the desktop controllers do not suit me, but they were a great source of information - you can see the links in the references section.

# Getting started
1. Create `src/credentials.h` file with `bt_mac_address` defined.
2. Compile the `esp32dev` environment and upload the binary to an esp32.
3. For the first connection you will need to press the pairing button on the desk.


# References
* github.com/zewelor/linak_bt_desk/
* github.com/anetczuk/linak_bt_desk/
* github.com/anson-vandoren/linak-desk-spec