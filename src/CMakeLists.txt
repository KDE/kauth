# This file handles all the logic for compiling KAuth's backends
include(ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

set(KAUTH_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuth")
set(KAUTHCORE_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuthCore")
set(KAUTHWIDGETS_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuthWidgets")

ecm_create_qm_loader(KAuth_QM_LOADER kauth6_qt)

set(kauthdebug_SRCS)
ecm_qt_declare_logging_category(kauthdebug_SRCS
    HEADER kauthdebug.h
    IDENTIFIER KAUTH
    CATEGORY_NAME kf.auth
    OLD_CATEGORY_NAMES kf5.kauth
    DESCRIPTION "KAuth"
    EXPORT KAUTH
)

if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    add_library(KF6AuthCore)
    add_library(KF6::AuthCore ALIAS KF6AuthCore)
    set_target_properties(KF6AuthCore PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
        EXPORT_NAME AuthCore
    )

    target_sources(KF6AuthCore PRIVATE
        action.cpp
        actionreply.cpp
        executejob.cpp
        AuthBackend.cpp
        BackendsManager.cpp
        HelperProxy.cpp
        helpersupport.cpp
        ${kauthdebug_SRCS}
        backends/fake/FakeBackend.cpp
        backends/fakehelper/FakeHelperProxy.cpp
        ${KAuth_QM_LOADER}
    )

    ecm_generate_export_header(KF6AuthCore
        BASE_NAME KAuthCore
        GROUP_BASE_NAME KF
        VERSION ${KF_VERSION}
        USE_VERSION_HEADER
        VERSION_BASE_NAME KAuth
        DEPRECATED_BASE_VERSION 0
        DEPRECATION_VERSIONS
        EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    )
    target_link_libraries(KF6AuthCore PUBLIC Qt6::Core KF6::CoreAddons)  # for KJob
    target_link_libraries(KF6AuthCore PRIVATE Qt6::DBus)

    target_include_directories(KF6AuthCore INTERFACE
        "$<INSTALL_INTERFACE:${KAUTHCORE_INSTALL_INCLUDEDIR};${KAUTH_INSTALL_INCLUDEDIR}>"
    )
    install(TARGETS KF6AuthCore EXPORT KF6AuthTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

    ecm_generate_headers(KAuthCore_CamelCase_HEADERS
        HEADER_NAMES
            Action
            ActionReply
            ExecuteJob
            HelperSupport

        PREFIX KAuth
        REQUIRED_HEADERS KAuthCore_HEADERS
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kauthcore_export.h
        ${KAuthCore_HEADERS}
        DESTINATION ${KAUTHCORE_INSTALL_INCLUDEDIR}/kauth # C++ namespace
        COMPONENT Devel
    )
    install(FILES
        ${KAuthCore_CamelCase_HEADERS}
        DESTINATION ${KAUTHCORE_INSTALL_INCLUDEDIR}/KAuth # C++ namespace
        COMPONENT Devel
    )

    add_library(KF6AuthWidgets)
    add_library(KF6::AuthWidgets ALIAS KF6AuthWidgets)
    set_target_properties(KF6AuthWidgets PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
        EXPORT_NAME AuthWidgets
    )

    target_sources(KF6AuthWidgets PRIVATE
        ${kauthdebug_SRCS}
        objectdecorator.cpp
    )

    ecm_generate_export_header(KF6AuthWidgets
        BASE_NAME KAuth
        GROUP_BASE_NAME KF
        VERSION ${KF_VERSION}
        USE_VERSION_HEADER
        DEPRECATED_BASE_VERSION 0
        DEPRECATION_VERSIONS
        EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    )

    target_include_directories(KF6AuthWidgets INTERFACE "$<INSTALL_INTERFACE:${KAUTHWIDGETS_INSTALL_INCLUDEDIR}>")

    target_link_libraries(KF6AuthWidgets PUBLIC KF6AuthCore)
    target_link_libraries(KF6AuthWidgets PRIVATE Qt6::Widgets)

    install(TARGETS KF6AuthWidgets EXPORT KF6AuthTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

    ecm_generate_headers(KAuthWidgets_CamelCase_HEADERS
        HEADER_NAMES
            ObjectDecorator

        PREFIX KAuth
        REQUIRED_HEADERS KAuthWidgets_HEADERS
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kauth_export.h
        ${KAuthWidgets_HEADERS}
        DESTINATION ${KAUTHWIDGETS_INSTALL_INCLUDEDIR}/kauth # C++ namespace
        COMPONENT Devel
    )
    install(FILES
        ${KAuthWidgets_CamelCase_HEADERS}
        DESTINATION ${KAUTHWIDGETS_INSTALL_INCLUDEDIR}/KAuth # C++ namespace
        COMPONENT Devel
    )
endif()

# KAuth policy generator executable

# Compile only if fake backend has not been selected

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    # KAUTH_POLICY_GEN_SRCS has been generated from ConfigureChecks.cmake
    add_executable(kauth-policy-gen ${KAUTH_POLICY_GEN_SRCS})
    add_executable(KF6::kauth-policy-gen ALIAS kauth-policy-gen)
    if (APPLE)
        include(ECMMarkNonGuiExecutable)
        ecm_mark_nongui_executable(kauth-policy-gen)
    endif ()

    # KAUTH_POLICY_GEN_LIBRARIES has been generated from ConfigureChecks.cmake
    target_link_libraries( kauth-policy-gen PRIVATE ${KAUTH_POLICY_GEN_LIBRARIES} )

    install( TARGETS kauth-policy-gen EXPORT KF6AuthToolsTargets DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
endif ()

# KAuth backend plugin

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE" AND NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
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

if (NOT "${KAUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE" AND NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
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
        KF6Auth_QCH
        NAME KAuth
        BASE_NAME KF6Auth
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KAuthCore_HEADERS}
            ${KAuthWidgets_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            KF6CoreAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        EXAMPLE_DIRS
            ${CMAKE_SOURCE_DIR}/examples
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

include(ECMGeneratePriFile)
ecm_generate_pri_file(
    BASE_NAME KAuthCore
    LIB_NAME KF6AuthCore
    DEPS "core KCoreAddons"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIRS
        ${KAUTHCORE_INSTALL_INCLUDEDIR}
        ${KAUTH_INSTALL_INCLUDEDIR}
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
ecm_generate_pri_file(
    BASE_NAME KAuth
    LIB_NAME KF6Auth
    DEPS "KAuthCore"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIRS ${KAUTHWIDGETS_INSTALL_INCLUDEDIR}
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
