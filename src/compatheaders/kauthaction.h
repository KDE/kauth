/*
    SPDX-FileCopyrightText: 2022 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kauth/kauthcore_export.h>

#if KAUTHCORE_ENABLE_DEPRECATED_SINCE(5, 92)
#include <kauth/action.h>
#if KAUTHCORE_DEPRECATED_WARNINGS_SINCE >= 0x055c00
#pragma message("Deprecated header. Since 5.92, use #include <kauth/action.h> instead")
#endif
#else
#error "Include of deprecated header is disabled"
#endif
