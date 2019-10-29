#!/bin/bash

SCRIPTDIR=$(dirname ${0})

if type appimagetool-x86_64.AppImage > /dev/null 2>&1; then
    if [ -e ${SCRIPTDIR}/src/simple_comics_viewer ]; then

        echo ${SCRIPTDIR}

        cp ${SCRIPTDIR}/src/simple_comics_viewer ${SCRIPTDIR}/appimage_dir/ &&
            ARCH=x86_64 appimagetool-x86_64.AppImage ${SCRIPTDIR}/appimage_dir &&
            rm ${SCRIPTDIR}/appimage_dir/simple_comics_viewer
    fi
else
    echo "not found appimagetool-x86_64.AppImage"
fi
