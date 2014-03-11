# KAuth

Execute actions as privileged user

## Introduction

KAuth provides a convenient, system-integrated way to offload actions that need
to be performed as a privileged user (root, for example) to small (hopefully
secure) helper utilities.

## Usage

If you are using CMake, you need to have

    find_package(KF5Auth NO_MODULE)

(or find KF5 with the Auth component) in your CMakeLists.txt file, and you need
to link to KF5::Auth.

Executing privileged actions typically involves having a minimal helper utility
that does the actual work, and calling that utility with escalated privileges if
the user has permission to do so (often requiring the user to enter appropriate
credentials, like entering a password).

Therefore, use of the KAuth library is in two parts.  In the main part of your
code, you use KAuth::Action (and specifically KAuth::Action::execute()) when you
need to do something privileged, like write to a file normally only writable by
root.

The code that actually performs that action, such as writing to a file, needs to
be placed in the slot of a helper QObject class, which should use the methods of
KAuth::HelperSupport and be compiled into an executable.  You will also want to
use the `kauth_install_helper_files` and `kauth_install_actions` macros in your
CMakeLists.txt.

See <http://techbase.kde.org/Development/Tutorials/KAuth/KAuth_Basics> for a
detailed tutorial on how to use KAuth.

## Links

- Home page: <https://projects.kde.org/projects/frameworks/kauth>
- Mailing list: <https://mail.kde.org/mailman/listinfo/kde-frameworks-devel>
- IRC channel: \#kde-devel on Freenode
- Git repository: <https://projects.kde.org/projects/frameworks/kauth/repository>
