set(spglib_SRCS
  cell.c
  debug.c
  hall_symbol.c
  lattice.c
  mathfunc.c
  pointgroup.c
  primitive.c
  refinement.c
  spacegroup.c
  spg_database.c
  spglib.c
  symmetry.c
  kpoint.c
  site_symmetry.c
  sitesym_database.c
  spglib_f.c
  spin.c
  tetrahedron_method.c
)

set(spglib_HDRS
  cell.h
  debug.h
  hall_symbol.h
  kpoint.h
  lattice.h
  mathfunc.h
  pointgroup.h
  primitive.h
  refinement.h
  sitesym_database.h
  site_symmetry.h
  spacegroup.h
  spg_database.h
  spglib.h
  spin.h
  symmetry.h
  tetrahedron_method.h
)

add_library(spglib ${spglib_SRCS})
set_target_properties(spglib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
if(UNIX)
  target_link_libraries(spglib m)
endif()

install(TARGETS spglib
  EXPORT "spglibTargets"
  RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
  LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}"
  ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}")

install(FILES ${spglib_HDRS}
  DESTINATION "${INSTALL_INCLUDE_DIR}/spglib")
