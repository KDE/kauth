cmake_minimum_required(VERSION 2.8.12)

project(KAuth)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(ECMPoQmTools)

set(KF5_VERSION "5.4.0") # handled by release scripts
set(KF5_DEP_VERSION "5.3.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KAUTH
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
                        SOVERSION 5)

find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)

set(KAUTH_HELPER_INSTALL_DIR "${LIBEXEC_INSTALL_DIR}/kauth")

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Auth")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5AuthConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KF5AuthMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF5AuthTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5AuthTargets.cmake NAMESPACE KF5:: COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel )


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
