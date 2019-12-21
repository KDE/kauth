cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.66.0") # handled by release scripts
set(KF5_DEP_VERSION "5.65.0") # handled by release scripts
project(KAuth VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.65.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION 5.12.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
option(KAUTH_BUILD_CODEGENERATOR_ONLY "Only build the kauth-policy-gen code generator." OFF)
if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
endif()
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)

include(ECMPoQmTools)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

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
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054100)
add_definitions(-DQT_NO_FOREACH)
add_subdirectory(src)
if(TARGET Qt5::Widgets)
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
    PATH_VARS KDE_INSTALL_DATADIR_KF5
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5AuthConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KF5AuthMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


if(TARGET KF5Auth)
    install(EXPORT KF5AuthTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5AuthTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
endif()

if(NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    install(EXPORT KF5AuthToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
            FILE KF5AuthToolsTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )

install(FILES kauth.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
