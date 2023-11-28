#/bin/bash

gcc myfuse.c -o myfuse -DFUSE_USE_VERSION=25 -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -lfuse -L/usr/lib/aarch64-linux-gnu -pthread -g
