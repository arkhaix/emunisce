/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PLATFORMDEFINES_H
#define PLATFORMDEFINES_H

// Compiler

#if defined(_MSC_VER)
#define EMUNISCE_COMPILER_MSVC 1

#elif defined(__GNUC__)
#define EMUNISCE_COMPILER_GCC 1

#else
#define EMUNISCE_COMPILER_UNKNOWN 1
#pragma message("Unrecognized compiler")

#endif

// Platform

#if defined(_WIN32)
#define EMUNISCE_PLATFORM_WINDOWS 1

#elif defined(__linux)
#define EMUNISCE_PLATFORM_LINUX 1

#else
#define EMUNISCE_PLATFORM_UNKNOWN 1
#pragma message("Unrecognized platform")

#endif

// Architecture

#if defined(_M_IX86) || defined(__i386__) || defined(_X86_)
#define EMUNISCE_ARCHITECTURE_32BIT 1

#elif defined(_M_IA64) || defined(__ia64__)
#define EMUNISCE_ARCHITECTURE_64BIT 1

#elif defined(_M_X64) || defined(__amd64__)
#define EMUNISCE_ARCHITECTURE_64BIT 1

#else
#define EMUNISCE_ARCHITECTURE_UNKNOWN 1
#pragma message("Unrecognized architecture")

#endif

#endif  // PLATFORMDEFINES_H
