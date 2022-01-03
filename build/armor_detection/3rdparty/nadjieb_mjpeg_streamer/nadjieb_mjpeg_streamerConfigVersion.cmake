# This is essentially cmake's BasicConfigVersion-SameMajorVersion.cmake.in but without the 32/64-bit check.
# Since nadjieb_mjpeg_streamer is a header-only library, it doesn't matter if it was built on a different platform than what it is used on

set(PACKAGE_VERSION "2.0.0")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()

  if(PACKAGE_FIND_VERSION_MAJOR STREQUAL "2")
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
  else()
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
  endif()

  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
      set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
