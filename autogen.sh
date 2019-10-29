#!/bin/sh

autoheader
aclocal
automake -a -c
autoreconf
