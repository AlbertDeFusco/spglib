#include <Python.h>
#include <stdio.h>
#include <numpy/arrayobject.h>
#include <spglib.h>

static PyObject * get_dataset(PyObject *self, PyObject *args);
static PyObject * get_spacegroup(PyObject *self, PyObject *args);
static PyObject * get_pointgroup(PyObject *self, PyObject *args);
static PyObject * refine_cell(PyObject *self, PyObject *args);
static PyObject * get_symmetry(PyObject *self, PyObject *args);
static PyObject * get_symmetry_with_collinear_spin(PyObject *self, PyObject *args);
static PyObject * find_primitive(PyObject *self, PyObject *args);
static PyObject * get_ir_kpoints(PyObject *self, PyObject *args);
static PyObject * get_ir_reciprocal_mesh(PyObject *self, PyObject *args);
static PyObject * get_stabilized_reciprocal_mesh(PyObject *self, PyObject *args);
static PyObject * get_triplets_reciprocal_mesh_at_q(PyObject *self, PyObject *args);
static PyObject * get_grid_triplets_at_q(PyObject *self, PyObject *args);

static PyMethodDef functions[] = {
  {"dataset", get_dataset, METH_VARARGS,
   "Dataset for crystal symmetry"},
  {"spacegroup", get_spacegroup, METH_VARARGS,
   "International symbol"},
  {"pointgroup", get_pointgroup, METH_VARARGS,
   "International symbol of pointgroup"},
  {"refine_cell", refine_cell, METH_VARARGS,
   "Refine cell"},
  {"symmetry", get_symmetry, METH_VARARGS,
   "Symmetry operations"},
  {"symmetry_with_collinear_spin", get_symmetry_with_collinear_spin,
   METH_VARARGS, "Symmetry operations with collinear spin magnetic moments"},
  {"primitive", find_primitive, METH_VARARGS,
   "Find primitive cell in the input cell"},
  {"ir_kpoints", get_ir_kpoints, METH_VARARGS,
   "Irreducible k-points"},
  {"ir_reciprocal_mesh", get_ir_reciprocal_mesh, METH_VARARGS,
   "Reciprocal mesh points with map"},
  {"stabilized_reciprocal_mesh", get_stabilized_reciprocal_mesh, METH_VARARGS,
   "Reciprocal mesh points with map"},
  {"triplets_reciprocal_mesh_at_q", get_triplets_reciprocal_mesh_at_q,
   METH_VARARGS, "Triplets on reciprocal mesh points at a specific q-point"},
  {"grid_triplets_at_q", get_grid_triplets_at_q,
   METH_VARARGS, "Grid point triplets on reciprocal mesh points at a specific q-point are set from output variables of triplets_reciprocal_mesh_at_q"},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_spglib(void)
{
  Py_InitModule3("_spglib", functions, "C-extension for spglib\n\n...\n");
  return;
}

static PyObject * get_dataset(PyObject *self, PyObject *args)
{
  int i, j, k;
  double symprec, angle_tolerance;
  SpglibDataset *dataset;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  PyObject* array, *vec, *mat, *rot, *trans, *wyckoffs, *equiv_atoms;
  
  if (!PyArg_ParseTuple(args, "OOOdd",
			&lattice,
			&position,
			&atom_type,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  const int num_atom = position->dimensions[0];
  const int* typat = (int*)atom_type->data;

  dataset = spgat_get_dataset(lat,
			      pos,
			      typat,
			      num_atom,
			      symprec,
			      angle_tolerance);

  array = PyList_New(9);

  /* Space group number, international symbol, hall symbol */
  PyList_SetItem(array, 0, PyInt_FromLong((long) dataset->spacegroup_number));
  PyList_SetItem(array, 1, PyString_FromString(dataset->international_symbol));
  PyList_SetItem(array, 2, PyString_FromString(dataset->hall_symbol));

  /* Transformation matrix */
  mat = PyList_New(3);
  for (i = 0; i < 3; i++) {
    vec = PyList_New(3);
    for (j = 0; j < 3; j++) {
      PyList_SetItem(vec, j, PyFloat_FromDouble(dataset->transformation_matrix[i][j]));
    }
    PyList_SetItem(mat, i, vec);
  }
  PyList_SetItem(array, 3, mat);

  /* Origin shift */
  vec = PyList_New(3);
  for (i = 0; i < 3; i++) {
    PyList_SetItem(vec, i, PyFloat_FromDouble(dataset->origin_shift[i]));
  }
  PyList_SetItem(array, 4, vec);

  /* Rotation matrices */
  rot = PyList_New(dataset->n_operations);
  for (i = 0; i < dataset->n_operations; i++) {
    mat = PyList_New(3);
    for (j = 0; j < 3; j++) {
      vec = PyList_New(3);
      for (k = 0; k < 3; k++) {
	PyList_SetItem(vec, k, PyInt_FromLong((long) dataset->rotations[i][j][k]));
      }
      PyList_SetItem(mat, j, vec);
    }
    PyList_SetItem(rot, i, mat);
  }
  PyList_SetItem(array, 5, rot);

  /* Translation vectors */
  trans = PyList_New(dataset->n_operations);
  for (i = 0; i < dataset->n_operations; i++) {
    vec = PyList_New(3);
    for (j = 0; j < 3; j++) {
      PyList_SetItem(vec, j, PyFloat_FromDouble(dataset->translations[i][j]));
    }
    PyList_SetItem(trans, i, vec);
  }
  PyList_SetItem(array, 6, trans);

  /* Wyckoff letters, Equivalent atoms */
  wyckoffs = PyList_New(dataset->n_atoms);
  equiv_atoms = PyList_New(dataset->n_atoms);
  for (i = 0; i < dataset->n_atoms; i++) {
    PyList_SetItem(wyckoffs, i, PyInt_FromLong((long) dataset->wyckoffs[i]));
    PyList_SetItem(equiv_atoms, i, PyInt_FromLong((long) dataset->equivalent_atoms[i]));
  }
  PyList_SetItem(array, 7, wyckoffs);
  PyList_SetItem(array, 8, equiv_atoms);
  spg_free_dataset(dataset);

  return array;
}

static PyObject * get_spacegroup(PyObject *self, PyObject *args)
{
  int i;
  double symprec, angle_tolerance;
  char symbol_with_number[17], spg_symbol[11];
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOdd",
			&lattice,
			&position,
			&atom_type,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  const int num_atom = position->dimensions[0];
  const int* typat = (int*)atom_type->data;

  const int num_spg = spgat_get_international(spg_symbol,
					      lat,
					      pos,
					      typat,
					      num_atom,
					      symprec,
					      angle_tolerance);

  for (i = 9; i > 0; i--) {
    if (! isspace(spg_symbol[i])) { break; }
  }
  spg_symbol[i + 1] = 0;
  sprintf(symbol_with_number, "%s (%d)", spg_symbol, num_spg);

  return PyString_FromString(symbol_with_number);
}

static PyObject * get_pointgroup(PyObject *self, PyObject *args)
{
  PyArrayObject* rotations;
  if (! PyArg_ParseTuple(args, "O", &rotations)) {
    return NULL;
  }

  int *rot_int = (int*)rotations->data;

  int i, j, k;
  int trans_mat[3][3];
  char symbol[6];
  PyObject* array, * mat, * vec;
    
  const int num_rot = rotations->dimensions[0];
  int rot[num_rot][3][3];
  for (i = 0; i < num_rot; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
	rot[i][j][k] = (int) rot_int[i*9 + j*3 + k];
      }
    }
  }

  const int ptg_num = spg_get_pointgroup(symbol, trans_mat, rot, num_rot);

  /* Transformation matrix */
  mat = PyList_New(3);
  for (i = 0; i < 3; i++) {
    vec = PyList_New(3);
    for (j = 0; j < 3; j++) {
      PyList_SetItem(vec, j, PyInt_FromLong((long)trans_mat[i][j]));
    }
    PyList_SetItem(mat, i, vec);
  }

  array = PyList_New(3);
  PyList_SetItem(array, 0, PyString_FromString(symbol));
  PyList_SetItem(array, 1, PyInt_FromLong((long) ptg_num));
  PyList_SetItem(array, 2, mat);

  return array;
}

static PyObject * refine_cell(PyObject *self, PyObject *args)
{
  int num_atom;
  double symprec, angle_tolerance;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOidd",
			&lattice,
			&position,
			&atom_type,
			&num_atom,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  int* typat = (int*)atom_type->data;

  int num_atom_brv = spgat_refine_cell(lat,
				       pos,
				       typat,
				       num_atom,
				       symprec,
				       angle_tolerance);

  return PyInt_FromLong((long) num_atom_brv);
}


static PyObject * find_primitive(PyObject *self, PyObject *args)
{
  double symprec, angle_tolerance;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOdd",
			&lattice,
			&position,
			&atom_type,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  double (*lat)[3] = (double(*)[3])lattice->data;
  double (*pos)[3] = (double(*)[3])position->data;
  int num_atom = position->dimensions[0];
  int* types = (int*)atom_type->data;

  int num_atom_prim = spgat_find_primitive(lat,
					   pos,
					   types,
					   num_atom,
					   symprec,
					   angle_tolerance);

  return PyInt_FromLong((long) num_atom_prim);
}

static PyObject * get_symmetry(PyObject *self, PyObject *args)
{
  int i, j, k;
  double symprec, angle_tolerance;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* rotation;
  PyArrayObject* translation;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOOOdd",
			&rotation,
			&translation,
			&lattice,
			&position,
			&atom_type,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  const int* types = (int*)atom_type->data;
  const int num_atom = position->dimensions[0];
  int *rot_int = (int*)rotation->data;
  double (*trans)[3] = (double(*)[3])translation->data;
  const int num_sym_from_array_size = rotation->dimensions[0];

  int rot[num_sym_from_array_size][3][3];
  
  /* num_sym has to be larger than num_sym_from_array_size. */
  const int num_sym = spgat_get_symmetry(rot,
					 trans,
					 num_sym_from_array_size,
					 lat,
					 pos,
					 types,
					 num_atom,
					 symprec,
					 angle_tolerance);
  for (i = 0; i < num_sym; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
	rot_int[i*9 + j*3 + k] = (int)rot[i][j][k];
      }
    }
  }

  return PyInt_FromLong((long) num_sym);
}

static PyObject * get_symmetry_with_collinear_spin(PyObject *self,
						   PyObject *args)
{
  int i, j, k;
  double symprec, angle_tolerance;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* rotation;
  PyArrayObject* translation;
  PyArrayObject* atom_type;
  PyArrayObject* magmom;
  if (!PyArg_ParseTuple(args, "OOOOOOdd",
			&rotation,
			&translation,
			&lattice,
			&position,
			&atom_type,
			&magmom,
			&symprec,
			&angle_tolerance)) {
    return NULL;
  }

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  const double *spins = (double*) magmom->data;
  const int* types = (int*)atom_type->data;
  const int num_atom = position->dimensions[0];
  int *rot_int = (int*)rotation->data;
  double (*trans)[3] = (double(*)[3])translation->data;
  const int num_sym_from_array_size = rotation->dimensions[0];

  int rot[num_sym_from_array_size][3][3];
  
  /* num_sym has to be larger than num_sym_from_array_size. */
  const int num_sym = 
    spgat_get_symmetry_with_collinear_spin(rot,
					   trans,
					   num_sym_from_array_size,
					   lat,
					   pos,
					   types,
					   spins,
					   num_atom,
					   symprec,
					   angle_tolerance);
  for (i = 0; i < num_sym; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
	rot_int[i*9 + j*3 + k] = (int)rot[i][j][k];
      }
    }
  }

  return PyInt_FromLong((long) num_sym);
}

static PyObject * get_ir_kpoints(PyObject *self, PyObject *args)
{
  double symprec;
  int is_time_reversal;
  PyArrayObject* kpoint;
  PyArrayObject* kpoint_map;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOOOid", &kpoint_map, &kpoint, &lattice, &position,
			&atom_type, &is_time_reversal, &symprec))
    return NULL;

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  SPGCONST double (*kpts)[3] = (double(*)[3])kpoint->data;
  const int num_kpoint = kpoint->dimensions[0];
  const int* types = (int*)atom_type->data;
  const int num_atom = position->dimensions[0];
  int *map = (int*)kpoint_map->data;

  /* num_sym has to be larger than num_sym_from_array_size. */
  const int num_ir_kpt = spg_get_ir_kpoints(map,
					    kpts,
					    num_kpoint,
					    lat,
					    pos,
					    types,
					    num_atom,
					    is_time_reversal,
					    symprec);

  return PyInt_FromLong((long) num_ir_kpt);
}

static PyObject * get_ir_reciprocal_mesh(PyObject *self, PyObject *args)
{
  int i, j;
  double symprec;
  PyArrayObject* grid_point;
  PyArrayObject* map;
  PyArrayObject* mesh;
  PyArrayObject* is_shift;
  int is_time_reversal;
  PyArrayObject* lattice;
  PyArrayObject* position;
  PyArrayObject* atom_type;
  if (!PyArg_ParseTuple(args, "OOOOiOOOd",
			&grid_point,
			&map,
			&mesh,
			&is_shift,
			&is_time_reversal,
			&lattice,
			&position,
			&atom_type,
			&symprec))
    return NULL;

  SPGCONST double (*lat)[3] = (double(*)[3])lattice->data;
  SPGCONST double (*pos)[3] = (double(*)[3])position->data;
  const int num_grid = grid_point->dimensions[0];
  const int* types = (int*)atom_type->data;
  const int* mesh_int = (int*)mesh->data;
  const int* is_shift_int = (int*)is_shift->data;
  const int num_atom = position->dimensions[0];
  int *grid_pint = (int*)grid_point->data;
  int grid_int[num_grid][3];
  int*map_int = (int*)map->data;

  /* Check memory space */
  if (mesh_int[0] * mesh_int[1] * mesh_int[2] > num_grid) {
    return NULL;
  }

  /* num_sym has to be larger than num_sym_from_array_size. */
  const int num_ir = spg_get_ir_reciprocal_mesh(grid_int,
						map_int,
						mesh_int,
						is_shift_int,
						is_time_reversal,
						lat,
						pos,
						types,
						num_atom,
						symprec);
  
  for (i = 0; i < mesh_int[0] * mesh_int[1] * mesh_int[2]; i++) {
    for (j = 0; j < 3; j++) {
      grid_pint[i*3 + j] = (int) grid_int[i][j];
    }
  }
  
  return PyInt_FromLong((long) num_ir);
}

static PyObject * get_stabilized_reciprocal_mesh(PyObject *self, PyObject *args)
{
  int i, j, k;
  PyArrayObject* grid_point;
  PyArrayObject* map;
  PyArrayObject* mesh;
  PyArrayObject* is_shift;
  int is_time_reversal;
  PyArrayObject* rotations;
  PyArrayObject* qpoints;
  if (!PyArg_ParseTuple(args, "OOOOiOO",
			&grid_point,
			&map,
			&mesh,
			&is_shift,
			&is_time_reversal,
			&rotations,
			&qpoints)) {
    return NULL;
  }

  int *grid_pint = (int*)grid_point->data;
  const int num_grid = grid_point->dimensions[0];
  int grid_int[num_grid][3];

  int *map_int = (int*)map->data;
  const int* mesh_int = (int*)mesh->data;
  const int* is_shift_int = (int*)is_shift->data;
  const int* rot_int = (int*)rotations->data;
  const int num_rot = rotations->dimensions[0];
  int rot[num_rot][3][3];
  for (i = 0; i < num_rot; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
	rot[i][j][k] = rot_int[i*9 + j*3 + k];
      }
    }
  }

  SPGCONST double (*q)[3] = (double(*)[3])qpoints->data;
  const int num_q = qpoints->dimensions[0];

  /* Check memory space */
  if (mesh_int[0] * mesh_int[1] * mesh_int[2] > num_grid) {
    return NULL;
  }

  const int num_ir = spg_get_stabilized_reciprocal_mesh(grid_int,
							map_int,
							mesh_int,
							is_shift_int,
							is_time_reversal,
							num_rot,
							rot,
							num_q,
							q);

  for (i = 0; i < mesh_int[0] * mesh_int[1] * mesh_int[2]; i++) {
    for (j = 0; j < 3; j++) {
      grid_pint[i*3 + j] = grid_int[i][j];
    }
  }
  
  return PyInt_FromLong((long) num_ir);
}

static PyObject * get_triplets_reciprocal_mesh_at_q(PyObject *self, PyObject *args)
{
  PyArrayObject* weights;
  PyArrayObject* grid_points;
  PyArrayObject* third_q;
  int fixed_grid_number;
  PyArrayObject* mesh;
  int is_time_reversal;
  PyArrayObject* rotations;
  if (!PyArg_ParseTuple(args, "OOOiOiO",
			&weights,
			&grid_points,
			&third_q,
			&fixed_grid_number,
			&mesh,
			&is_time_reversal,
			&rotations)) {
    return NULL;
  }

  int i, j, k;

  const int num_grid = grid_points->dimensions[0];
  int *grid_points_pint = (int*)grid_points->data;
  int grid_points_int[num_grid][3];
  int *weights_int = (int*)weights->data;
  int *third_q_int = (int*)third_q->data;

  const int* mesh_int = (int*)mesh->data;
  const int* rot_int = (int*)rotations->data;
  const int num_rot = rotations->dimensions[0];
  int rot[num_rot][3][3];
  for (i = 0; i < num_rot; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
	rot[i][j][k] = rot_int[i*9 + j*3 + k];
      }
    }
  }

  const int num_ir = 
    spg_get_triplets_reciprocal_mesh_at_q(weights_int,
					  grid_points_int,
					  third_q_int,
					  fixed_grid_number,
					  mesh_int,
					  is_time_reversal,
					  num_rot,
					  rot);

  for (i = 0; i < num_grid; i++) {
    for (j = 0; j < 3; j++) {
      grid_points_pint[i*3 + j] = grid_points_int[i][j];
    }
  }
  
  return PyInt_FromLong((long) num_ir);
}


static PyObject * get_grid_triplets_at_q(PyObject *self, PyObject *args)
{
  PyArrayObject* triplets_py;
  PyArrayObject* grid_points_py;
  PyArrayObject* third_q_py;
  PyArrayObject* weights_py;
  PyArrayObject* mesh_py;
  int q_grid_point;
  if (!PyArg_ParseTuple(args, "OiOOOO",
			&triplets_py,
			&q_grid_point,
			&grid_points_py,
			&third_q_py,
			&weights_py,
			&mesh_py)) {
    return NULL;
  }

  int i, j;
  
  int *p_triplets = (int*)triplets_py->data;
  const int num_ir_triplets = (int)triplets_py->dimensions[0];
  const int *p_grid_points = (int*)grid_points_py->data;
  const int num_grid_points = (int)grid_points_py->dimensions[0];
  const int *third_q = (int*)third_q_py->data;
  const int *weights = (int*)weights_py->data;
  const int *mesh = (int*)mesh_py->data;
  int triplets[num_ir_triplets][3];
  int grid_points[num_grid_points][3];
  for (i = 0; i < num_grid_points; i++) {
    for (j = 0; j < 3; j++) {
      grid_points[i][j] = p_grid_points[i * 3 + j];
    }
  }
  
  spg_set_grid_triplets_at_q(triplets,
			     q_grid_point,
			     grid_points,
			     third_q,
			     weights,
			     mesh);
  
  for (i = 0; i < num_ir_triplets; i++) {
    for (j = 0; j < 3; j++) {
      p_triplets[i * 3 + j] = triplets[i][j];
    }
  }

  Py_RETURN_NONE;
}


