#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "c4::c4" for configuration "Release"
set_property(TARGET c4::c4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(c4::c4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libc4.a"
  )

list(APPEND _cmake_import_check_targets c4::c4 )
list(APPEND _cmake_import_check_files_for_c4::c4 "${_IMPORT_PREFIX}/lib64/libc4.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
