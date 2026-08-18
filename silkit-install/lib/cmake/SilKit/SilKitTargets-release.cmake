#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SilKit::SilKit" for configuration "Release"
set_property(TARGET SilKit::SilKit APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SilKit::SilKit PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libSilKit.so"
  IMPORTED_SONAME_RELEASE "libSilKit.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS SilKit::SilKit )
list(APPEND _IMPORT_CHECK_FILES_FOR_SilKit::SilKit "${_IMPORT_PREFIX}/lib/libSilKit.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
