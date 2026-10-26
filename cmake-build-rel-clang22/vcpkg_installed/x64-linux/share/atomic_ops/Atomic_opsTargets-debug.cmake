#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Atomic_ops::atomic_ops_gpl" for configuration "Debug"
set_property(TARGET Atomic_ops::atomic_ops_gpl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Atomic_ops::atomic_ops_gpl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libatomic_ops_gpl.a"
  )

list(APPEND _cmake_import_check_targets Atomic_ops::atomic_ops_gpl )
list(APPEND _cmake_import_check_files_for_Atomic_ops::atomic_ops_gpl "${_IMPORT_PREFIX}/debug/lib/libatomic_ops_gpl.a" )

# Import target "Atomic_ops::atomic_ops" for configuration "Debug"
set_property(TARGET Atomic_ops::atomic_ops APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Atomic_ops::atomic_ops PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libatomic_ops.a"
  )

list(APPEND _cmake_import_check_targets Atomic_ops::atomic_ops )
list(APPEND _cmake_import_check_files_for_Atomic_ops::atomic_ops "${_IMPORT_PREFIX}/debug/lib/libatomic_ops.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
