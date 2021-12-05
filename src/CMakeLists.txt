# This file handles all the logic for compiling KAuth's backends
include(ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

ecm_create_qm_loader(KAuth_QM_LOADER kauth5_qt)

set(kauthdebug_SRCS)
ecm_qt_declare_logging_category(kauthdebug_SRCS
    HEADER kauthdebug.h
    IDENTIFIER KAUTH
    CATEGORY_NAME kf.auth
    OLD_CATEGORY_NAMES kf5.kauth
    DESCRIPTION "KAuth"
    EXPORT KAUTH
)

if(TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    # TODO KF6 Rename KAuth to KAuthWidgets
    # TODO KAuthCore should be installed also when no widgets available, in the header installation etc, so ignoring for now
    add_library(KF5AuthCore)
    add_library(KF5::AuthCore ALIAS KF5AuthCore)
    set_target_properties(KF5AuthCore PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
        EXPORT_NAME AuthCore
    )

    target_sources(KF5AuthCore PRIVATE
        kauthaction.cpp
        kauthactionreply.cpp
        kauthexecutejob.cpp
        AuthBackend.cpp
        BackendsManager.cpp
        HelperProxy.cpp
        kauthhelpersupport.cpp
        ${kauthdebug_SRCS}
        backends/fake/FakeBackend.cpp
        backends/fakehelper/FakeHelperProxy.cpp
        ${KAuth_QM_LOADER}
    )

    ecm_generate_export_header(KF5AuthCore
        BASE_NAME KAuthCore
        GROUP_BASE_NAME KF
        VERSION ${KF_VERSION}
        DEPRECATED_BASE_VERSION 0
        DEPRECATION_VERSIONS 5.71
        EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    )
    target_link_libraries(KF5AuthCore PUBLIC Qt${QT_MAJOR_VERSION}::Core KF5::CoreAddons)  # for KJob
    target_link_libraries(KF5AuthCore PRIVATE Qt${QT_MAJOR_VERSION}::DBus)

    target_include_directories(KF5AuthCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KAuth>")
    install(TARGETS KF5AuthCore EXPORT KF5AuthTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

    add_library(KF5Auth)
    add_library(KF5::Auth ALIAS KF5Auth)
    set_target_properties(KF5Auth PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
        EXPORT_NAME Auth
    )

    target_sources(KF5Auth PRIVATE
        ${kauthdebug_SRCS}
        kauthobjectdecorator.cpp
    )

    generate_export_header(KF5Auth BASE_NAME KAuth)

    target_include_directories(KF5Auth INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KAuth>")

    target_link_libraries(KF5Auth PUBLIC KF5AuthCore)
    target_link_libraries(KF5Auth PRIVATE Qt${QT_MAJOR_VERSION}::Widgets)

    ecm_generate_headers(KAuth_HEADERS
        HEADER_NAMES
        KAuth
        KAuthHelperSupport
        KAuthAction
        KAuthActionReply
        KAuthExecuteJob
        KAuthObjectDecorator

        REQUIRED_HEADERS KAuth_HEADERS
    )

    install(TARGETS KF5Auth EXPORT KF5AuthTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kauth_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/kauthcore_export.h
        ${KAuth_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KAuth COMPONENT Devel
    )
endif()

find_package(PythonModuleGeneration)

if (PythonModuleGeneration_FOUND)
  ecm_generate_python_binding(
    TARGET KF5::Auth
    PYTHONNAMESPACE PyKF5
    MODULENAME KAuth
    RULES_FILE "${CMAKE_SOURCE_DIR}/cmake/rules_PyKF5.py"
    SIP_INCLUDES $<TARGET_PROPERTY:Qt${QT_MAJOR_VERSION}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    SIP_DEPENDS
      QtWidgets/QtWidgetsmod.sip
      PyKF5/KCoreAddons/KCoreAddonsmod.sip
    HEADERS
      kauth.h
      kauthhelpersupport.h
      kauthaction.h
      kauthactionreply.h
      kauthexecutejob.h
      kauthobjectdecorator.h
  )
endif()

# KAuth policy generator executable

# Compile only if fake backend has not been selected

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    # KAUTH_POLICY_GEN_SRCS has been generated from ConfigureChecks.cmake
    add_executable(kauth-policy-gen ${KAUTH_POLICY_GEN_SRCS})
    add_executable(KF5::kauth-policy-gen ALIAS kauth-policy-gen)
    if (APPLE)
        include(ECMMarkNonGuiExecutable)
        ecm_mark_nongui_executable(kauth-policy-gen)
    endif ()

    # KAUTH_POLICY_GEN_LIBRARIES has been generated from ConfigureChecks.cmake
    target_link_libraries( kauth-policy-gen PRIVATE ${KAUTH_POLICY_GEN_LIBRARIES} )

    install( TARGETS kauth-policy-gen EXPORT KF5AuthToolsTargets DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
endif ()

# KAuth backend plugin

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE" AND TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    # KAUTH_BACKEND_SRCS has been set from ConfigureChecks.cmake
    # KAuth::AuthBackend is not exported
    add_library(kauth_backend_plugin MODULE ${KAUTH_BACKEND_SRCS} AuthBackend.cpp ${kauthdebug_SRCS})
    target_link_libraries(kauth_backend_plugin PRIVATE ${KAUTH_BACKEND_LIBS})
    set_target_properties(kauth_backend_plugin PROPERTIES PREFIX "")

    install(TARGETS kauth_backend_plugin
            LIBRARY DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_BACKEND_PLUGIN_DIR}"
            ARCHIVE DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_BACKEND_PLUGIN_DIR}"
            RUNTIME DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_BACKEND_PLUGIN_DIR}"
    )
endif ()

# KAuth helper plugin

if (NOT "${KAUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE" AND TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    # KAuth::HelperProxy is not exported
    add_library(kauth_helper_plugin MODULE ${KAUTH_HELPER_BACKEND_SRCS} HelperProxy.cpp ${kauthdebug_SRCS})
    target_link_libraries(kauth_helper_plugin PRIVATE ${KAUTH_HELPER_BACKEND_LIBS})
    set_target_properties(kauth_helper_plugin PROPERTIES PREFIX "")
    install(TARGETS kauth_helper_plugin
            LIBRARY DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_HELPER_PLUGIN_DIR}"
            ARCHIVE DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_HELPER_PLUGIN_DIR}"
            RUNTIME DESTINATION "${KDE_INSTALL_PLUGINDIR}/${KAUTH_HELPER_PLUGIN_DIR}"
    )
endif ()

ecm_qt_install_logging_categories(
    EXPORT KAUTH
    FILE kauth.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Auth_QCH
        NAME KAuth
        BASE_NAME KF5Auth
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KAuth_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            KF5CoreAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KAUTHCORE_EXPORT
            KAUTH_EXPORT
            KAUTH_DEPRECATED
            KAUTH_DEPRECATED_EXPORT
            "KAUTHCORE_DEPRECATED_VERSION(x, y, t)"
            "KAUTHCORE_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

# TODO: Remove once we depend on polkit-qt1 > 0.99.0
# core/polkitqt1-actiondescription.h from polkit currently fails when built with -pedantic
string(REPLACE "--pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "--pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KAuthCore LIB_NAME KF5AuthCore DEPS "core KCoreAddons" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KAuth)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
ecm_generate_pri_file(BASE_NAME KAuth LIB_NAME KF5Auth DEPS "KAuthCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KAuth)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
