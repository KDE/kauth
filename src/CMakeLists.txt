# This file handles all the logic for compiling KAuth's backends
include(ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

ecm_create_qm_loader(KAuth_QM_LOADER kauth5_qt)

set(kauthdebug_SRCS)
ecm_qt_declare_logging_category(kauthdebug_SRCS HEADER kauthdebug.h IDENTIFIER KAUTH CATEGORY_NAME kf5.kauth)

if(TARGET Qt5::Widgets)
    # TODO KF6 Rename KAuth to KAuthWidgets
    # TODO KAuthCore should be installed also when no widgets available, in the header installation etc, so ignoring for now
    set(libkauth_core_SRCS
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

    add_library(KF5AuthCore ${libkauth_core_SRCS})
    generate_export_header(KF5AuthCore BASE_NAME KAuthCore)
    target_link_libraries(KF5AuthCore PUBLIC Qt5::Core KF5::CoreAddons)  # for KJob
    target_link_libraries(KF5AuthCore PRIVATE Qt5::DBus)
    target_include_directories(KF5AuthCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KAuth>")
    install(TARGETS KF5AuthCore EXPORT KF5AuthTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
    set_target_properties(KF5AuthCore PROPERTIES VERSION  ${KAUTH_VERSION_STRING}
                                            SOVERSION ${KAUTH_SOVERSION}
                                            EXPORT_NAME AuthCore)

    set(libkauth_SRCS
        ${kauthdebug_SRCS}
        kauthobjectdecorator.cpp
    )

    add_library(KF5Auth ${libkauth_SRCS})
    generate_export_header(KF5Auth BASE_NAME KAuth)
    add_library(KF5::Auth ALIAS KF5Auth)

    target_include_directories(KF5Auth INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KAuth>")

    target_link_libraries(KF5Auth PUBLIC KF5AuthCore)
    target_link_libraries(KF5Auth PRIVATE Qt5::Widgets)
    set_target_properties(KF5Auth PROPERTIES VERSION   ${KAUTH_VERSION_STRING}
                                            SOVERSION ${KAUTH_SOVERSION}
                                            EXPORT_NAME Auth
    )

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
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KAuth COMPONENT Devel
    )
endif()

find_package(PythonModuleGeneration)

if (PythonModuleGeneration_FOUND)
  ecm_generate_python_binding(
    TARGET KF5::Auth
    PYTHONNAMESPACE PyKF5
    MODULENAME KAuth
    RULES_FILE "${CMAKE_SOURCE_DIR}/cmake/rules_PyKF5.py"
    SIP_INCLUDES $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
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

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE" AND TARGET Qt5::Widgets)
    set(KAUTH_BACKEND_SRCS ${KAUTH_BACKEND_SRCS})
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

if (NOT "${KAUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE" AND TARGET Qt5::Widgets)
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

if(BUILD_QCH)
    ecm_add_qch(
        KF5Auth_QCH
        NAME KAuth
        BASE_NAME KF5Auth
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KAuth_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            KF5CoreAddons_QCH
        BLANK_MACROS
            KAUTHCORE_EXPORT
            KAUTH_EXPORT
            KAUTH_DEPRECATED
            KAUTH_DEPRECATED_EXPORT
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
ecm_generate_pri_file(BASE_NAME KAuth LIB_NAME KF5Auth DEPS "core KCoreAddons" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KAuth)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
