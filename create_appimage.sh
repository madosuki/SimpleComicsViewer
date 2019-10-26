#!/bin/sh

if type appimagetool-x86_64.AppImage > /dev/null 2>&1; then
    if [ -e ./build/simple_comics_viewer ]; then
        ARCH=x86_64 appimagetool-x86_64.AppImage ./build
    fi
fi
