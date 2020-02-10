
find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

qt5_add_dbus_adaptor(kauth_dbus_adaptor_tests_SRCS
                     ../src/backends/dbus/org.kde.kf5auth.xml
                     ../src/backends/dbus/DBusHelperProxy.h
                     KAuth::DBusHelperProxy)

include(ECMAddTests)

set(kauthdebug_SRCS)
ecm_qt_declare_logging_category(kauthdebug_tests_SRCS HEADER kauthdebug.h IDENTIFIER KAUTH CATEGORY_NAME kf5.kauth)


set(libkauth_tests_static_SRCS
    ../src/kauthaction.cpp
    ../src/kauthactionreply.cpp
    ../src/kauthexecutejob.cpp
    ../src/AuthBackend.cpp
    # Use our "special" backends manager
    BackendsManager.cpp
    ../src/HelperProxy.cpp
    ../src/kauthhelpersupport.cpp
    TestBackend.cpp
    ../src/backends/dbus/DBusHelperProxy.cpp
    ${kauth_dbus_adaptor_tests_SRCS}
    ${kauthdebug_tests_SRCS}
)

add_library(kauth_tests_static STATIC ${libkauth_tests_static_SRCS})
# make sure all executables using this library have the define set to make sure it builds on MSVC
target_compile_definitions(kauth_tests_static
    PUBLIC KAUTHCORE_STATIC_DEFINE=1
    PUBLIC KAUTH_STATIC_DEFINE=1
)
ecm_mark_as_test(kauth_tests_static)

target_include_directories(kauth_tests_static
  PUBLIC
    $<TARGET_PROPERTY:KF5::Auth,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(kauth_tests_static PUBLIC Qt5::DBus KF5::CoreAddons)

########### next target ###############

set(SetupActionTest_SRCS SetupActionTest.cpp)
ecm_add_test(${SetupActionTest_SRCS}
    TEST_NAME KAuthSetupActionTest
    LINK_LIBRARIES Qt5::Test kauth_tests_static
)

########### next target ###############

set(HelperTest_SRCS HelperTest.cpp TestHelper.cpp)
ecm_add_test(${HelperTest_SRCS}
    TEST_NAME KAuthHelperTest
    LINK_LIBRARIES Qt5::Test kauth_tests_static
)

########### test kauth-policy-gen ###############
add_test(NAME KAuthPolicyGenTest
         COMMAND kauth-policy-gen ${CMAKE_SOURCE_DIR}/autotests/foo_actions.actions
                                  ${CMAKE_BINARY_DIR}/generated-foo.policy)
add_test(KAuthPolicyGenTestCompare ${CMAKE_COMMAND} -E compare_files
         ${CMAKE_BINARY_DIR}/generated-foo.policy ${CMAKE_SOURCE_DIR}/autotests/foo.policy)

