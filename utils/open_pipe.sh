#!/bin/sh

clear

exec 3>../misc/serial_pipe.out # open file descriptor 3 writing to the pipe

cat ../misc/serial_pipe.out

exec 3>&- # close file descriptor 3
