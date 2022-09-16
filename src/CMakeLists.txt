# This file handles all the logic for compiling KAuth's backends
include(ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

set(KAUTH_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuth")
set(KAUTHCORE_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuthCore")
set(KAUTHWIDGETS_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KAuthWidgets")

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

if(NOT KAUTH_BUILD_CODEGENERATOR_ONLY)
    add_library(KF5AuthCore)
    add_library(KF5::AuthCore ALIAS KF5AuthCore)
    set_target_properties(KF5AuthCore PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
        EXPORT_NAME AuthCore
    )

    target_sources(KF5AuthCore PRIVATE
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

    target_include_directories(KF5AuthCore INTERFACE
        "$<INSTALL_INTERFACE:${KAUTHCORE_INSTALL_INCLUDEDIR};${KAUTH_INSTALL_INCLUDEDIR}>"
    )
    install(TARGETS KF5AuthCore EXPORT KF5AuthTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

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

    add_library(KF5AuthWidgets)
    add_library(KF5::AuthWidgets ALIAS KF5AuthWidgets)
    set_target_properties(KF5AuthWidgets PROPERTIES
        VERSION     ${KAUTH_VERSION}
        SOVERSION   ${KAUTH_SOVERSION}
    )
    if (QT_MAJOR_VERSION STREQUAL "5")
        # backward-compat name
        set_target_properties(KF5AuthWidgets PROPERTIES
            OUTPUT_NAME KF5Auth
            EXPORT_NAME Auth
        )
    else()
        set_target_properties(KF5AuthWidgets PROPERTIES
            EXPORT_NAME AuthWidgets
        )
    endif()

    target_sources(KF5AuthWidgets PRIVATE
        ${kauthdebug_SRCS}
        objectdecorator.cpp
    )

    ecm_generate_export_header(KF5AuthWidgets
        BASE_NAME KAuth
        GROUP_BASE_NAME KF
        VERSION ${KF_VERSION}
        DEPRECATED_BASE_VERSION 0
        DEPRECATION_VERSIONS 5.92
        EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    )

    target_include_directories(KF5AuthWidgets INTERFACE "$<INSTALL_INTERFACE:${KAUTHWIDGETS_INSTALL_INCLUDEDIR}>")

    target_link_libraries(KF5AuthWidgets PUBLIC KF5AuthCore)
    target_link_libraries(KF5AuthWidgets PRIVATE Qt${QT_MAJOR_VERSION}::Widgets)

    install(TARGETS KF5AuthWidgets EXPORT KF5AuthTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

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

    # provide KF6-forward compat import target
    if (QT_MAJOR_VERSION STREQUAL "5")
        add_library(KF5AuthFutureWidgets INTERFACE)
        target_link_libraries(KF5AuthFutureWidgets INTERFACE KF5AuthWidgets)
        set_target_properties(KF5AuthFutureWidgets PROPERTIES
            EXPORT_NAME AuthWidgets
        )
        install(TARGETS KF5AuthFutureWidgets EXPORT KF5AuthTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
    endif()

    # provide compat headers for old C++ namespace-part-of-filename includes
    if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
    EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.92.0)

        install(FILES
                compatheaders/KAuth
                compatheaders/kauth.h
                compatheaders/KAuthAction
                compatheaders/kauthaction.h
                compatheaders/KAuthActionReply
                compatheaders/kauthactionreply.h
                compatheaders/KAuthExecuteJob
                compatheaders/kauthexecutejob.h
                compatheaders/KAuthHelperSupport
                compatheaders/kauthhelpersupport.h
                compatheaders/KAuthObjectDecorator
                compatheaders/kauthobjectdecorator.h
            DESTINATION ${KAUTH_INSTALL_INCLUDEDIR}
            COMPONENT Devel
        )
    endif()
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
        KF5Auth_QCH
        NAME KAuth
        BASE_NAME KF5Auth
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KAuthCore_HEADERS}
            ${KAuthWidgets_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            KF5CoreAddons_QCH
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
    LIB_NAME KF5AuthCore
    DEPS "core KCoreAddons"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIRS
        ${KAUTHCORE_INSTALL_INCLUDEDIR}
        ${KAUTH_INSTALL_INCLUDEDIR}
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
ecm_generate_pri_file(
    BASE_NAME KAuth
    LIB_NAME KF5Auth
    DEPS "KAuthCore"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIRS ${KAUTHWIDGETS_INSTALL_INCLUDEDIR}
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
