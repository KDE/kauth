cmake_minimum_required(VERSION 3.0)

set(KF5_VERSION "5.30.0") # handled by release scripts
set(KF5_DEP_VERSION "5.29.0") # handled by release scripts
project(KAuth VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.29.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.5.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(ECMPoQmTools)

ecm_setup_version(PROJECT VARIABLE_PREFIX KAUTH
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
                        SOVERSION 5)

find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)

set(KAUTH_HELPER_INSTALL_DIR "${KDE_INSTALL_LIBEXECDIR}/kauth")

if(IS_ABSOLUTE ${KAUTH_HELPER_INSTALL_DIR})
    set(KAUTH_HELPER_INSTALL_ABSOLUTE_DIR ${KAUTH_HELPER_INSTALL_DIR})
else()
    set(KAUTH_HELPER_INSTALL_ABSOLUTE_DIR "${CMAKE_INSTALL_PREFIX}/${KAUTH_HELPER_INSTALL_DIR}")
endif()

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Auth")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5AuthConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
    PATH_VARS KDE_INSTALL_DATADIR_KF5
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KF5AuthMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF5AuthTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5AuthTargets.cmake NAMESPACE KF5:: COMPONENT Devel)

if(NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    install(EXPORT KF5AuthToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
            FILE KF5AuthToolsTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
