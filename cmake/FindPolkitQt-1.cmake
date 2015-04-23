# - Try to find PolkitQt-1
# Once done this will define
#
#  POLKITQT-1_FOUND - system has Polkit-qt
#  POLKITQT-1_INCLUDE_DIR - the Polkit-qt include directory
#  POLKITQT-1_LIBRARIES - Link these to use all Polkit-qt libs
#  POLKITQT-1_CORE_LIBRARY - Link this to use the polkit-qt-core library only
#  POLKITQT-1_GUI_LIBRARY - Link this to use GUI elements in polkit-qt (polkit-qt-gui)
#  POLKITQT-1_AGENT_LIBRARY - Link this to use the agent wrapper in polkit-qt
#  POLKITQT-1_DEFINITIONS - Compiler switches required for using Polkit-qt
#
# The minimum required version of PolkitQt-1 can be specified using the
# standard syntax, e.g. find_package(PolkitQt-1 1.0)

# Copyright (c) 2009, Dario Freddi, <drf@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

# Support POLKITQT-1_MIN_VERSION for compatibility:
if ( NOT PolkitQt-1_FIND_VERSION AND POLKITQT-1_MIN_VERSION )
  set ( PolkitQt-1_FIND_VERSION ${POLKITQT-1_MIN_VERSION} )
endif ( NOT PolkitQt-1_FIND_VERSION AND POLKITQT-1_MIN_VERSION )

set( _PolkitQt-1_FIND_QUIETLY  ${PolkitQt-1_FIND_QUIETLY} )
find_package( PolkitQt-1 ${PolkitQt-1_FIND_VERSION} QUIET NO_MODULE PATHS ${LIB_INSTALL_DIR}/PolkitQt-1/cmake )
set( PolkitQt-1_FIND_QUIETLY ${_PolkitQt-1_FIND_QUIETLY} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( PolkitQt-1 DEFAULT_MSG PolkitQt-1_CONFIG )

if (POLKITQT-1_FOUND)
    if (NOT POLKITQT-1_INSTALL_DIR STREQUAL CMAKE_INSTALL_PREFIX)
        message("WARNING: Installation prefix does not match PolicyKit install prefixes. You probably will need to move files installed "
                "in POLICY_FILES_INSTALL_DIR and by dbus_add_activation_system_service to the ${PC_POLKITQT-1_PREFIX} prefix")
    endif (NOT POLKITQT-1_INSTALL_DIR STREQUAL CMAKE_INSTALL_PREFIX)
endif (POLKITQT-1_FOUND)
