# ***Work in progress, use at own risk!***

# LinakDeskEsp32Controller

The goal of this project is creating an easy to use bluetooth bridge for my Ikea Idasen desk, that uses a Linak controller.

I often switch computers, that's why the desktop controllers do not suit me, but they were a great source of information - you can see the links in the references section.

# Getting started
1. Create `src/credentials.h` file from the example one and fill the data.
2. Compile the `esp32dev` environment and upload the binary to an esp32.
3. For the first connection you will need to press the pairing button on the desk.

Current implementation offers a simple HTTP GET API for getting the height and moving to height:
```
linak-desk.local/getHeight
```

```
linak-desk.local/moveToHeight?destination=700
```

You can use the IP address of the device if you're having problems with mDNS


# References
* https://github.com/zewelor/linak_bt_desk/
* https://github.com/anetczuk/linak_bt_desk/
* https://github.com/anson-vandoren/linak-desk-spec