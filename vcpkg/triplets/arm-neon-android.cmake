set(VCPKG_TARGET_ARCHITECTURE arm)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_MAKE_BUILD_TRIPLET "--host=armv7a-linux-androideabi")
set(ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})

if(BUILD_WITH_QT6)
  set(VCPKG_CXX_FLAGS "-fstack-protector-strong")
  set(VCPKG_C_FLAGS "-fstack-protector-strong")
else()
  set(VCPKG_CXX_FLAGS "-fstack-protector-strong -lunwind -Wl,--exclude-libs=libunwind.a")
  set(VCPKG_C_FLAGS "-fstack-protector-strong -lunwind -Wl,--exclude-libs=libunwind.a")
endif()
