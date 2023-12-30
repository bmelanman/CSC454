#!/bin/bash

echo '-------'

checkmake Makefile

echo '-------'

checkmake --config=../.trunk/configs/checkmake.ini Makefile
echo ''

echo '-------'
