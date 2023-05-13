cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.107.0") # handled by release scripts
set(KF_DEP_VERSION "5.106.0") # handled by release scripts
project(KAuth VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 5.106.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(ECMDeprecationSettings)
include(ECMPoQmTools)
include(ECMAddQch)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION}DBus ${REQUIRED_QT_VERSION} CONFIG REQUIRED)

option(KAUTH_BUILD_CODEGENERATOR_ONLY "Only build the kauth-policy-gen code generator." OFF)
if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
endif()


set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX KAUTH
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
                        SOVERSION 5)

find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)

set(KAUTH_HELPER_INSTALL_DIR "${KDE_INSTALL_LIBEXECDIR}/kauth")

if(IS_ABSOLUTE ${KAUTH_HELPER_INSTALL_DIR})
    set(KAUTH_HELPER_INSTALL_ABSOLUTE_DIR ${KAUTH_HELPER_INSTALL_DIR})
else()
    set(KAUTH_HELPER_INSTALL_ABSOLUTE_DIR "${CMAKE_INSTALL_PREFIX}/${KAUTH_HELPER_INSTALL_DIR}")
endif()

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.95
)

ecm_install_po_files_as_qm(poqm)

add_subdirectory(src)

if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    if (BUILD_TESTING)
        add_subdirectory(autotests)
    endif()
    add_subdirectory(examples)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Auth")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Auth_QCH
        FILE KF5AuthQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5AuthQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5AuthConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
    PATH_VARS KDE_INSTALL_DATADIR_KF
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KF5AuthMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    install(EXPORT KF5AuthTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5AuthTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
endif()

if(NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    install(EXPORT KF5AuthToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
            FILE KF5AuthToolsTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KAuth COMPONENT Devel)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
