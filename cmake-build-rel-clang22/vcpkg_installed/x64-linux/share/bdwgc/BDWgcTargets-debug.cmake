#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BDWgc::cord" for configuration "Debug"
set_property(TARGET BDWgc::cord APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BDWgc::cord PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libcord.a"
  )

list(APPEND _cmake_import_check_targets BDWgc::cord )
list(APPEND _cmake_import_check_files_for_BDWgc::cord "${_IMPORT_PREFIX}/debug/lib/libcord.a" )

# Import target "BDWgc::gc" for configuration "Debug"
set_property(TARGET BDWgc::gc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BDWgc::gc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libgc.a"
  )

list(APPEND _cmake_import_check_targets BDWgc::gc )
list(APPEND _cmake_import_check_files_for_BDWgc::gc "${_IMPORT_PREFIX}/debug/lib/libgc.a" )

# Import target "BDWgc::gccpp" for configuration "Debug"
set_property(TARGET BDWgc::gccpp APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BDWgc::gccpp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libgccpp.a"
  )

list(APPEND _cmake_import_check_targets BDWgc::gccpp )
list(APPEND _cmake_import_check_files_for_BDWgc::gccpp "${_IMPORT_PREFIX}/debug/lib/libgccpp.a" )

# Import target "BDWgc::gctba" for configuration "Debug"
set_property(TARGET BDWgc::gctba APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BDWgc::gctba PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libgctba.a"
  )

list(APPEND _cmake_import_check_targets BDWgc::gctba )
list(APPEND _cmake_import_check_files_for_BDWgc::gctba "${_IMPORT_PREFIX}/debug/lib/libgctba.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
