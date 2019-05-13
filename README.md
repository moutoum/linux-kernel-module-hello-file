# Linux kernel module - Hello, file!

This project is an introduction to the linux kernel pseudo device files.
It is just a simple module that will create a device that can be read.
This file always loop over the same content: `Hello, file!\n`.

## How to use

```sh
$ make
$ insmod hello_file.ko
$ export HELLO_FILE_MAJOR_NUMBER=`cat /proc/devices | grep "hello-file" | cut -f 1 -d " "`
$ mknod /dev/hello-file c ${HELLO_FILE_MAJOR_NUMBER} 0
$ cat /dev/hello-file # Enjoy!
```

## How to uninstall
```sh
$ rm /dev/hello-file
$ rmmod hello_file
```
