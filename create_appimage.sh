#!/bin/bash

SCRIPTDIR=$(dirname ${0})

function create_app_image () {
    if type appimagetool-x86_64.AppImage > /dev/null 2>&1; then
        ARCH=x86_64 appimagetool-x86_64.AppImage ${SCRIPTDIR}/appimage_dir
    else
        echo "not found appimagetool-x86_64.AppImage"
    fi
}

if type linuxdeploy-x86_64.AppImage > /dev/null 2>&1; then
    if [ -e ${SCRIPTDIR}/src/simple_comics_viewer ]; then
        linuxdeploy-x86_64.AppImage -e ./src/simple_comics_viewer -d ./for_appimage_assets/simple_comics_viewer.desktop -i ./for_appimage_assets/icon.svg --appdir appimage_dir
        create_app_image
        rm -rf ./appimage_dir
    else
        echo "not done build execute file"
    fi
else
    echo "not found linuxdeploy-x86_64.AppImage"
fi

