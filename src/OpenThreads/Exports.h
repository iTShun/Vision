/* -*-c++-*- OpenThreads library, Copyright (C) 2002 - 2007  The Open Thread Group
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef _OPENTHREAD_EXPORTS_H_
#define _OPENTHREAD_EXPORTS_H_

#include "OpenThreads/Config.h"

#ifndef WIN32
    #define OPENTHREAD_EXPORT_DIRECTIVE
#else
    #if defined( OT_LIBRARY_STATIC )
        #define OPENTHREAD_EXPORT_DIRECTIVE
    #elif defined( OPENTHREADS_EXPORTS )
        #define OPENTHREAD_EXPORT_DIRECTIVE __declspec(dllexport)
    #else
        #define OPENTHREAD_EXPORT_DIRECTIVE __declspec(dllimport)
    #endif
#endif

#endif


