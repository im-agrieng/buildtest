find_program(LINUXDEPLOY_EXECUTABLE linuxdeploy linuxdeploy-x86_64.AppImage)
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

set(CPACK_GENERATOR)

function(macdeployqt bundle targetdir _PACKAGER)
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/CPackMacDeployQt-${_PACKAGER}.cmake
                  CONTENT "execute_process(COMMAND \"${MACDEPLOYQT_EXECUTABLE}\" \"${CPACK_PACKAGE_DIRECTORY}/_CPack_Packages/Darwin/${_PACKAGER}/${targetdir}/${bundle}\" -always-overwrite)")
    install(SCRIPT ${CMAKE_BINARY_DIR}/CPackMacDeployQt-${_PACKAGER}.cmake COMPONENT Runtime)
    include(InstallRequiredSystemLibraries)
endfunction()

if(WIN32)
    message(STATUS "   + WIX                             YES ")
    set(CPACK_GENERATOR "WIX")
endif()

get_target_property(qmake_executable Qt5::qmake IMPORTED_LOCATION)

set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS qfield)

# Qt IFW packaging framework
if(BINARYCREATOR_EXECUTABLE)
    list(APPEND CPACK_GENERATOR IFW)
    message(STATUS "   + Qt Installer Framework               YES ")
else()
    message(STATUS "   + Qt Installer Framework                NO ")
endif()


if(LINUXDEPLOY_EXECUTABLE)
    message(STATUS "   + AppImage                             YES ")
    set(CPACK_GENERATOR "External;${CPACK_GENERATOR}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CPackLinuxDeployQt.cmake.in "${CMAKE_BINARY_DIR}/CPackExternal.cmake")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/CPackExternal.cmake")
else()
    message(STATUS "   + AppImage                              NO ")
endif()

include(CPack)
