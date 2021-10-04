/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once

#define MAJOR_VERSION 0
#define MINOR_VERSION 2

#if defined(_WIN32)
    #define ROBOT2D_WINDOWS
#elif defined(__APPLE__)
    #define ROBOT2D_MACOS
#endif

#if(!defined(ROBOT2D_STATIC))
    // Windows dll export options
    #if ROBOT2D_WINDOWS
        #define ROBOT2D_EXPORT_API __declspec(dllexport)
        #define ROBOT2D_IMPORT_API __declspec(dllimport)

        // For Visual C++ compilers, we also need to turn off this annoying C4251 warning
        #ifdef _MSC_VER
            #pragma warning(disable: 4251)
        #endif
    #else
        #if __GNUC__ >= 4

            // GCC 4 has special keywords for showing/hidding symbols,
            // the same keyword is used for both importing and exporting
            #define ROBOT2D_EXPORT_API __attribute__ ((__visibility__ ("default")))
            #define ROBOT2D_EXPORT_API __attribute__ ((__visibility__ ("default")))

        #else
            // GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
            #define ROBOT2D_EXPORT_API
            #define ROBOT2D_EXPORT_API

        #endif
    #endif
#else
    // Static build doesn't need import/export macros
    #define ROBOT2D_EXPORT_API
    #define ROBOT2D_EXPORT_API
#endif