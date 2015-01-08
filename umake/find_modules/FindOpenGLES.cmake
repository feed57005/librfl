# - Try to find OpenGLES
# Once done this will define
#
# OPENGLES_FOUND - system has OpenGLES
# OPENGLES_INCLUDE_DIR - the GL include directory
# OPENGLES_LIBRARIES - Link these to use OpenGLES

#include(FindPkgMacros)
include(FindPackageHandleStandardArgs)

if (WIN32)

  if (0) # Disabled, until further testing
  if (CYGWIN)
    find_path (OPENGLES_INCLUDE_DIR GLES/gl.h )
    find_library (OPENGLES_gl_LIBRARY libgles_cm )
  else (CYGWIN)
    if(BORLAND)
      set (OPENGLES_gl_LIBRARY import32 CACHE STRING "OpenGL library for win32")
    else(BORLAND)
    # MS compiler - todo - fix the following line:
      set (OPENGLES_gl_LIBRARY ${OGRE_SOURCE_DIR}/Dependencies/lib/release/libgles_cm.lib CACHE STRING "OpenGL library for win32")
    endif(BORLAND)
  endif (CYGWIN)
  endif ()

else (WIN32)

  if (APPLE)
    #create_search_paths(/Developer/Platforms)
    #findpkg_framework(OpenGLES)
    set(OPENGLES_gl_LIBRARY "-framework OpenGLES")
  else (APPLE)
    if (1) # Disabled, untill further testing
    find_path (OPENGLES_INCLUDE_DIR GLES/gl.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /opt/vc/include
      /usr/include
    )

    find_library (OPENGLES_gl_LIBRARY
      NAMES GLES_CM
      PATHS /opt/graphics/OpenGL/lib
      /opt/vc/lib
      /usr/openwin/lib
      /usr/shlib /usr/X11R6/lib
      /usr/lib
    )

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't
    # think this is always true.
    # It's not true on OSX.

    if (OPENGLES_gl_LIBRARY)
      if (NOT X11_FOUND)
        include (FindX11)
      endif (NOT X11_FOUND)
      if (X11_FOUND)
        if (NOT APPLE)
          set (OPENGLES_LIBRARIES ${X11_LIBRARIES})
        endif (NOT APPLE)
      endif (X11_FOUND)
    endif (OPENGLES_gl_LIBRARY)

    endif()

  endif (APPLE)
endif (WIN32)

set (OPENGLES_FOUND "NO")
if (OPENGLES_gl_LIBRARY)
  set (OPENGLES_LIBRARIES ${OPENGLES_gl_LIBRARY} ${OPENGLES_LIBRARIES})
  set (OPENGLES_FOUND "YES")
endif (OPENGLES_gl_LIBRARY)

mark_as_advanced ( OPENGLES_INCLUDE_DIR OPENGLES_gl_LIBRARY)
