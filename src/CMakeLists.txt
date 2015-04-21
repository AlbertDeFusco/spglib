set(spglib_SRCS
  cell.c
  debug.c
  hall_symbol.c
  kpoint.c
  lattice.c
  mathfunc.c
  niggli.c
  pointgroup.c
  primitive.c
  refinement.c
  site_symmetry.c
  sitesym_database.c
  spacegroup.c
  spg_database.c
  spglib.c
  spglib_f.c
  spin.c
  symmetry.c
  tetrahedron_method.c
  triplet_kpoint.c
)

set(spglib_HDRS
  cell.h
  debug.h
  hall_symbol.h
  kpoint.h
  lattice.h
  mathfunc.h
  niggli.h
  pointgroup.h
  primitive.h
  refinement.h
  site_symmetry.h
  sitesym_database.h
  spacegroup.h
  spg_database.h
  spglib.h
  spin.h
  symmetry.h
  tetrahedron_method.h
  triplet_kpoint.h
)

add_library(symspg ${spglib_SRCS})
set_target_properties(symspg PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
if(UNIX)
  target_link_libraries(symspg m)
endif()

install(TARGETS symspg
  EXPORT "spglibTargets"
  RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
  LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}"
  ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}")

install(FILES ${spglib_HDRS}
  DESTINATION "${INSTALL_INCLUDE_DIR}/spglib")
