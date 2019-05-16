# Linux kernel module - Hello, file!

This project is an introduction to the linux kernel pseudo device files.
It is just a simple module that will create a device that can be read.
This file always loop over the same content: `Hello, file!\n`.

## How to use

```sh
$ make
$ insmod hello_file.ko
$ cat /dev/hello_file_dev
```

## How to uninstall

```sh
$ rmmod hello_file
```
