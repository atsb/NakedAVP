# Locate SDL3 library
# This module defines
# SDL3_LIBRARY, the name of the library to link against
# SDL3_FOUND, if false, do not try to link to SDL3
# SDL3_INCLUDE, where to find SDL.h
#
# This module responds to the the flag:
# SDL3_BUILDING_LIBRARY
# If this is defined, then no SDL3main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL3_LIBRARY variable.
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the
# OS X framework based version. (Other versions link to -lSDL3main which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
#
# Additional Note: If you see an empty SDL3_LIBRARY in your configuration
# and no SDL3_LIBRARY, it means CMake did not find your SDL3 library
# (SDL3.dll, libSDL3.so, SDL3.framework, etc).
# Set SDL3_LIBRARY to point to your SDL3 library, and configure again.
# Similarly, if you see an empty SDL3MAIN_LIBRARY, you should set this value
# as appropriate. These values are used to generate the final SDL3_LIBRARY
# variable, but when these values are unset, SDL3_LIBRARY does not get created.
#
#
# $SDL3DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL3DIR
# used in building SDL3.
# l.e.galup  9-20-02
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDL3main which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# SDL3_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL3/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL3/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL3/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

SET(SDL3_SEARCH_PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/homebrew # macOS Homebrew
  /opt/csw # Blastwave
  /opt
)

FIND_PATH(SDL3_INCLUDE
  NAMES
    SDL3
  HINTS
    $ENV{SDL3DIR}
  PATH_SUFFIXES
    include
  PATHS
    ${SDL3_SEARCH_PATHS}
)

FIND_LIBRARY(SDL3_LIBRARY
  NAMES SDL3
  HINTS
  $ENV{SDL3DIR}
  PATH_SUFFIXES lib64 lib
  PATHS ${SDL3_SEARCH_PATHS}
)

# SDL3 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
  FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDL3main -lSDL3 -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
IF(MINGW)
  SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)

IF(SDL3_LIBRARY)

  # For OS X, SDL3 uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the
  # "real" variable in one-shot.
  IF(APPLE)
    SET(SDL3_LIBRARY ${SDL3_LIBRARY} "-framework Cocoa")
  ENDIF(APPLE)

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  IF(NOT APPLE)
    SET(SDL3_LIBRARY ${SDL3_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(NOT APPLE)

  # For MinGW library
  IF(MINGW)
    SET(SDL3_LIBRARY ${MINGW32_LIBRARY} ${SDL3_LIBRARY})
  ENDIF(MINGW)

  # Set the final string here so the GUI reflects the final state.
  SET(SDL3_LIBRARY ${SDL3_LIBRARY} CACHE STRING "Where the SDL3 Library can be found")
  # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
  SET(SDL3_LIBRARY "${SDL3_LIBRARY}" CACHE INTERNAL "")
ENDIF(SDL3_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL3 REQUIRED_VARS SDL3_LIBRARY SDL3_INCLUDE)
