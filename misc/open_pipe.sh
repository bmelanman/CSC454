#!/bin/sh

exec 3>serial_pipe.out # open file descriptor 3 writing to the pipe

cat serial_pipe.out

exec 3>&- # close file descriptor 3
