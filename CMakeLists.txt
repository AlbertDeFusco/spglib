cmake_minimum_required (VERSION 2.6)
project (spglib C)

###########
#
# Options
#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Compile GAMESS for Debug or Release"
    FORCE)
endif()
message(STATUS "The build type is ${CMAKE_BUILD_TYPE}")

option(BUILD_SHARED_LIBS "Build shared library" OFF)

set(CMAKE_C_FLAGS_DEBUG "-DDEBUG")

find_library(M_LIB m)

set(sources
 src/cell.c
 src/debug.c
 src/hall_symbol.c
 src/kpoint.c
 src/lattice.c
 src/mathfunc.c
 src/niggli.c
 src/pointgroup.c
 src/primitive.c
 src/refinement.c
 src/site_symmetry.c
 src/sitesym_database.c
 src/spacegroup.c
 src/spg_database.c
 src/spglib.c
 src/spglib_f.c
 src/spin.c
 src/symmetry.c
 src/tetrahedron_method.c
)

set(headers
 src/cell.h
 src/debug.h
 src/hall_symbol.h
 src/kpoint.h
 src/lattice.h
 src/mathfunc.h
 src/niggli.h
 src/pointgroup.h
 src/primitive.h
 src/refinement.h
 src/site_symmetry.h
 src/sitesym_database.h
 src/spacegroup.h
 src/spg_database.h
 src/spglib.h
 src/spin.h
 src/symmetry.h
 src/tetrahedron_method.h
)

add_library(symspg ${sources})
target_link_libraries(symspg ${M_LIB})

if(BUILD_SHARED_LIBS)
  install(TARGETS symspg
    LIBRARY DESTINATION lib)
else(BUILD_SHARED_LIBS)
  install(TARGETS symspg
    ARCHIVE DESTINATION lib)
endif(BUILD_SHARED_LIBS)

#headers
install(FILES ${headers}
  DESTINATION include)
