//
// Created by nathan on 1/29/23.
//

#ifndef CRISPY_CORE_H
#define CRISPY_CORE_H

#if defined PLATFORM_LINUX
# define IN_LINUX(x) x
#else
# define IN_LINUX(x)
#endif

#if defined PLATFORM_MACOS
# define IN_MACOS(x) x
#else
# define IN_MACOS(x)
#endif

#if defined DEBUG
# define IN_DEBUG(x) x
#else
# define IN_DEBUG(x)
#endif

#endif //CRISPY_CORE_H
