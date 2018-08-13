from conans import ConanFile, CMake


class KAuthConan(ConanFile):
    name = "kauth"
    version = "5.50.0"
    license = "GPLv2"
    url = "https://api.kde.org/frameworks/kauth/html/index.html"
    description = "Library that helps execute actions as privileged user."

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "extra-cmake-modules/5.50.0@kde/testing",

        "Qt/5.11.1@bincrafters/stable",
        # "qt-core/5.8.0@qt/testing",
        # "qt-widgets/5.8.0@qt/testing",
        # "qt-dbus/5.8.0@qt/testing",
        # "qt-test/5.8.0@qt/testing",

        "kcoreaddons/5.50.0@kde/testing",
    )

    generators = "cmake"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
     }

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package_info(self):
        self.cpp_info.resdirs = ["share"]
