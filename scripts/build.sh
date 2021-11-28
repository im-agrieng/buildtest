#!/usr/bin/env bash

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"/..

export APK_VERSION_CODE=${APK_VERSION_CODE:-1}
export APP_VERSION_STR=${APP_VERSION_STR:-dev}

triplet=${triplet:-arm64-android}

docker build ${SRC_DIR}/.docker/android_dev -t qfield_and_dev

docker run --rm \
  -v "$SRC_DIR":/usr/src/qfield:Z \
  $(if [ -n "$CACHE_DIR" ]; then echo "-v $CACHE_DIR:/io/data"; fi) \
  -e "BUILD_FOLDER=build-${triplet}" \
  -e triplet=${triplet} \
  -e STOREPASS \
  -e KEYNAME \
  -e KEYPASS \
  -e APP_PACKAGE_NAME \
  -e APP_NAME \
  -e APP_ICON \
  -e APP_VERSION \
  -e APP_VERSION_STR \
  -e APK_VERSION_CODE \
  qfield_and_dev \
  /usr/src/qfield/scripts/build-vcpkg.sh
