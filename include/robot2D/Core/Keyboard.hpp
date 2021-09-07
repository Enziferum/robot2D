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

namespace robot2D {
    enum Key {
        Unknown = -1,
        SPACE = 32,
        APOSTROPHE =    39, /* ' */
        COMMA =         44,  /* , */
        MINUS =         45,  /* - */
        PERIOD =        46,  /* . */
        SLASH =         47, /* / */
        Num0 =             48,
        Num1 =             49,
        Num2 =             50,
        Num3 =             51,
        Num4 =             52,
        Num5 =             53,
        Num6 =             54,
        Num7 =             55,
        Num8 =             56,
        Num9 =             57,
        SEMICOLON =     59, /* ; */
        EQUAL =         61, /* = */
        A =             65,
        B =             66,
        C =             67,
        D =             68,
        E =             69,
        F =             70,
        G =             71,
        H =             72,
        I =             73,
        J =             74,
        K =             75,
        L =             76,
        M =             77,
        N =             78,
        O =             79,
        P =             80,
        Q =             81,
        R =             82,
        S =             83,
        T =             84,
        U =             85,
        V =             86,
        W =             87,
        X =             88,
        Y =             89,
        Z =             90,
        LEFT_BRACKET =  91,/* [ */
        BACKSLASH =     92,/* \ */
        RIGHT_BRACKET = 93,/* ] */
        GRAVE_ACCENT =  96,/* ` */

        WORLD_1 =       161,/* non-US #1 */
        WORLD_2 =       162,/* non-US #2 */

        /* Function = */
        ESCAPE =        256,
        ENTER =         257,
        TAB =           258,
        BACKSPACE =     259,
        INSERT =        260,
        DEL =        261,
        RIGHT =         262,
        LEFT =          263,
        DOWN =          264,
        UP =            265,
        PAGE_UP =       266,
        PAGE_DOWN =     267,
        HOME =          268,
        END =           269,
        CAPS_LOCK =     280,
        SCROLL_LOCK =   281,
        NUM_LOCK =      282,
        PRINT_SCREEN =  283,
        PAUSE =         284,
        F1 =            290,
        F2 =            291,
        F3 =            292,
        F4 =            293,
        F5 =            294,
        F6 =            295,
        F7 =            296,
        F8 =            297,
        F9 =            298,
        F10 =           299,
        F11 =           300,
        F12 =           301,
        F13 =           302,
        F14 =           303,
        F15 =           304,
        F16 =           305,
        F17 =           306,
        F18 =           307,
        F19 =           308,
        F20 =           309,
        F21 =           310,
        F22 =           311,
        F23 =           312,
        F24 =           313,
        F25 =           314,
        KP_0 =          320,
        KP_1 =          321,
        KP_2 =          322,
        KP_3 =          323,
        KP_4 =          324,
        KP_5 =          325,
        KP_6 =          326,
        KP_7 =          327,
        KP_8 =          328,
        KP_9 =          329,
        KP_DECIMAL =    330,
        KP_DIVIDE =     331,
        KP_MULTIPLY =   332,
        KP_SUBTRACT =   333,
        KP_ADD =        334,
        KP_ENTER =      335,
        KP_EQUAL =      336,
        LEFT_SHIFT =    340,
        LEFT_CONTROL =  341,
        LEFT_ALT =      342,
        LEFT_SUPER =    343,
        RIGHT_SHIFT =   344,
        RIGHT_CONTROL = 345,
        RIGHT_ALT =     346,
        RIGHT_SUPER =   347,
        MENU =          348
    };
}