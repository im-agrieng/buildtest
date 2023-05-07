#!/bin/bash
QT_PREFIX=${HOME}/.qt
QT_VERSION=6.5.0

# pip install aqt
# aqt install linux desktop ${QT_VERSION} ${QT_PREFIX} -m [...] <- check android.yaml
# make sure flex and bison are installed (use `apt` or similar)

export Qt6_DIR=${QT_PREFIX}/${QT_VERSION}
cmake -S . -B $(git rev-parse --show-toplevel)/build-x64-linux -DSYSTEM_QT=ON -DBUILD_WITH_QT6=ON -GNinja -DWITH_VCPKG=ON -DQt6_ROOT=${Qt6_DIR}
cmake --build $(git rev-parse --show-toplevel)/build-x64-linux
