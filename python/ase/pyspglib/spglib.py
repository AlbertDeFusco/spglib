"""
Spglib interface for ASE
"""

import pyspglib._spglib as spg
import numpy as np

def get_symmetry(bulk, use_magmoms=False, symprec=1e-5, angle_tolerance=-1.0):
    """
    Return symmetry operations as hash.
    Hash key 'rotations' gives the numpy integer array
    of the rotation matrices for scaled positions
    Hash key 'translations' gives the numpy double array
    of the translation vectors in scaled positions
    """

    # Atomic positions have to be specified by scaled positions for spglib.
    positions = bulk.get_scaled_positions().copy()
    lattice = bulk.get_cell().T.copy()
    numbers = np.intc(bulk.get_atomic_numbers()).copy()

    # Get number of symmetry operations and allocate symmetry operations
    # multi = spg.multiplicity(cell, positions, numbers, symprec)
    multi = 48 * bulk.get_number_of_atoms()
    rotation = np.zeros((multi, 3, 3), dtype='intc')
    translation = np.zeros((multi, 3))

    # Get symmetry operations
    if use_magmoms:
        magmoms = bulk.get_magnetic_moments()
        num_sym = spg.symmetry_with_collinear_spin(rotation,
                                                   translation,
                                                   lattice,
                                                   positions,
                                                   numbers,
                                                   magmoms,
                                                   symprec,
                                                   angle_tolerance)
    else:
        num_sym = spg.symmetry(rotation,
                               translation,
                               lattice,
                               positions,
                               numbers,
                               symprec,
                               angle_tolerance)

    return {'rotations': rotation[:num_sym].copy(),
            'translations': translation[:num_sym].copy()}

def get_symmetry_dataset(bulk, symprec=1e-5, angle_tolerance=-1.0):
    """
    number: International space group number
    international: International symbol
    hall: Hall symbol
    transformation_matrix:
      Transformation matrix from lattice of input cell to Bravais lattice
      L^bravais = L^original * Tmat
    origin shift: Origin shift in the setting of 'Bravais lattice'
    rotations, translations:
      Rotation matrices and translation vectors
      Space group operations are obtained by
        [(r,t) for r, t in zip(rotations, translations)]
    wyckoffs:
      Wyckoff letters
    """
    positions = bulk.get_scaled_positions().copy()
    lattice = bulk.get_cell().T.copy()
    numbers = np.array(bulk.get_atomic_numbers(), dtype='intc').copy()
    keys = ('number',
            'international',
            'hall',
            'transformation_matrix',
            'origin_shift',
            'rotations',
            'translations',
            'wyckoffs',
            'equivalent_atoms')
    dataset = {}
    for key, data in zip(keys, spg.dataset(lattice,
                                           positions,
                                           numbers,
                                           symprec,
                                           angle_tolerance)):
        dataset[key] = data

    dataset['international'] = dataset['international'].strip()
    dataset['hall'] = dataset['hall'].strip()
    dataset['transformation_matrix'] = np.array(
        dataset['transformation_matrix'], dtype='double')
    dataset['origin_shift'] = np.array(dataset['origin_shift'],
                                       dtype='double')
    dataset['rotations'] = np.array(dataset['rotations'], dtype='intc')
    dataset['translations'] = np.array(dataset['translations'],
                                       dtype='double')
    letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    dataset['wyckoffs'] = [letters[x] for x in dataset['wyckoffs']]
    dataset['equivalent_atoms'] = np.array(dataset['equivalent_atoms'],
                                           dtype='intc')

    return dataset

def get_spacegroup(bulk, symprec=1e-5, angle_tolerance=-1.0):
    """
    Return space group in international table symbol and number
    as a string.
    """
    # Atomic positions have to be specified by scaled positions for spglib.
    return spg.spacegroup(bulk.get_cell().T.copy(),
                          bulk.get_scaled_positions().copy(),
                          np.array(bulk.get_atomic_numbers(),
                                   dtype='intc').copy(),
                          symprec,
                          angle_tolerance)

def get_pointgroup(rotations):
    """
    Return point group in international table symbol and number.
    The symbols are mapped to the numbers as follows:
    1   "1    "
    2   "-1   "
    3   "2    "
    4   "m    "
    5   "2/m  "
    6   "222  "
    7   "mm2  "
    8   "mmm  "
    9   "4    "
    10  "-4   "
    11  "4/m  "
    12  "422  "
    13  "4mm  "
    14  "-42m "
    15  "4/mmm"
    16  "3    "
    17  "-3   "
    18  "32   "
    19  "3m   "
    20  "-3m  "
    21  "6    "
    22  "-6   "
    23  "6/m  "
    24  "622  "
    25  "6mm  "
    26  "-62m "
    27  "6/mmm"
    28  "23   "
    29  "m-3  "
    30  "432  "
    31  "-43m "
    32  "m-3m "
    """

    # (symbol, pointgroup_number, transformation_matrix)
    return spg.pointgroup(np.array(rotations, dtype='intc').copy())

def refine_cell(bulk, symprec=1e-5, angle_tolerance=-1.0):
    """
    Return refined cell
    """
    # Atomic positions have to be specified by scaled positions for spglib.
    num_atom = bulk.get_number_of_atoms()
    lattice = bulk.get_cell().T.copy()
    pos = np.zeros((num_atom * 4, 3), dtype='double')
    pos[:num_atom] = bulk.get_scaled_positions()

    numbers = np.zeros(num_atom * 4, dtype='intc')
    numbers[:num_atom] = np.array(bulk.get_atomic_numbers(), dtype='intc')
    num_atom_bravais = spg.refine_cell(lattice,
                                       pos,
                                       numbers,
                                       num_atom,
                                       symprec,
                                       angle_tolerance)

    return (lattice.T.copy(),
            pos[:num_atom_bravais].copy(),
            numbers[:num_atom_bravais].copy())

def find_primitive(bulk, symprec=1e-5, angle_tolerance=-1.0):
    """
    A primitive cell in the input cell is searched and returned
    as an object of Atoms class.
    If no primitive cell is found, (None, None, None) is returned.
    """

    # Atomic positions have to be specified by scaled positions for spglib.
    positions = bulk.get_scaled_positions().copy()
    lattice = bulk.get_cell().T.copy()
    numbers = np.array(bulk.get_atomic_numbers(), dtype='intc').copy()

    # lattice is transposed with respect to the definition of Atoms class
    num_atom_prim = spg.primitive(lattice,
                                  positions,
                                  numbers,
                                  symprec,
                                  angle_tolerance)
    if num_atom_prim > 0:
        return (lattice.T.copy(),
                positions[:num_atom_prim].copy(),
                numbers[:num_atom_prim].copy())
    else:
        return None, None, None
  
def get_ir_reciprocal_mesh(mesh,
                           bulk,
                           is_shift=np.zeros(3, dtype='intc'),
                           is_time_reversal=True,
                           symprec=1e-5):
    """
    Return k-points mesh and k-point map to the irreducible k-points
    The symmetry is serched from the input cell.
    is_shift=[0, 0, 0] gives Gamma center mesh.
    """

    mapping = np.zeros(np.prod(mesh), dtype='intc')
    mesh_points = np.zeros((np.prod(mesh), 3), dtype='intc')
    spg.ir_reciprocal_mesh(mesh_points,
                           mapping,
                           np.array(mesh, dtype='intc').copy(),
                           np.array(is_shift, dtype='intc').copy(),
                           is_time_reversal * 1,
                           bulk.get_cell().T.copy(),
                           bulk.get_scaled_positions().copy(),
                           np.array(bulk.get_atomic_numbers(),
                                    dtype='intc').copy(),
                           symprec)
  
    return mapping, mesh_points

def relocate_BZ_grid_address(grid_address,
                             mesh,
                             reciprocal_lattice, # column vectors
                             is_shift=np.zeros(3, dtype='intc')):
    """
    Grid addresses are relocated inside Brillouin zone. 
    Number of ir-grid-points inside Brillouin zone is returned. 
    It is assumed that the following arrays have the shapes of 
      bz_grid_address[prod(mesh + 1)][3] 
      bz_map[prod(mesh * 2)] 
    where grid_address[prod(mesh)][3]. 
    Each element of grid_address is mapped to each element of 
    bz_grid_address with keeping element order. bz_grid_address has 
    larger memory space to represent BZ surface even if some points 
    on a surface are translationally equivalent to the other points 
    on the other surface. Those equivalent points are added successively 
    as grid point numbers to bz_grid_address. Those added grid points 
    are stored after the address of end point of grid_address, i.e. 
                                                                          
    |-----------------array size of bz_grid_address---------------------| 
    |--grid addresses similar to grid_address--|--newly added ones--|xxx| 
                                                                          
    where xxx means the memory space that may not be used. Number of grid 
    points stored in bz_grid_address is returned. 
    bz_map is used to recover grid point index expanded to include BZ 
    surface from grid address. The grid point indices are mapped to 
    (mesh[0] * 2) x (mesh[1] * 2) x (mesh[2] * 2) space (bz_map).
    """
    
    bz_grid_address = np.zeros(
        ((mesh[0] + 1) * (mesh[1] + 1) * (mesh[2] + 1), 3), dtype='intc')
    bz_map = np.zeros(
        (2 * mesh[0]) * (2 * mesh[1]) * (2 * mesh[2]), dtype='intc')
    num_bz_ir = spg.BZ_grid_address(
        bz_grid_address,
        bz_map,
        grid_address,
        np.array(mesh, dtype='intc').copy(),
        np.array(reciprocal_lattice, dtype='double').copy(),
        np.array(is_shift, dtype='intc').copy())

    return bz_grid_address[:num_bz_ir], bz_map
  
def get_stabilized_reciprocal_mesh(mesh,
                                   rotations,
                                   is_shift=np.zeros(3, dtype='intc'),
                                   is_time_reversal=True,
                                   qpoints=np.array([], dtype='double')):
    """
    Return k-point map to the irreducible k-points and k-point grid points .

    The symmetry is searched from the input rotation matrices in real space.
    
    is_shift=[0, 0, 0] gives Gamma center mesh and the values 1 give
    half mesh distance shifts.
    """
    
    mapping = np.zeros(np.prod(mesh), dtype='intc')
    mesh_points = np.zeros((np.prod(mesh), 3), dtype='intc')
    qpoints = np.array(qpoints, dtype='double').copy()
    if qpoints.shape == (3,):
        qpoints = np.array([qpoints], dtype='double')
    if qpoints.shape == (0,):
        qpoints = np.array([[0, 0, 0]], dtype='double')
    spg.stabilized_reciprocal_mesh(mesh_points,
                                   mapping,
                                   np.array(mesh, dtype='intc').copy(),
                                   np.array(is_shift, dtype='intc'),
                                   is_time_reversal * 1,
                                   np.array(rotations, dtype='intc').copy(),
                                   np.array(qpoints, dtype='double'))
    
    return mapping, mesh_points

def get_triplets_reciprocal_mesh_at_q(fixed_grid_number,
                                      mesh,
                                      rotations,
                                      is_time_reversal=True):

    weights = np.zeros(np.prod(mesh), dtype='intc')
    third_q = np.zeros(np.prod(mesh), dtype='intc')
    mesh_points = np.zeros((np.prod(mesh), 3), dtype='intc')

    spg.triplets_reciprocal_mesh_at_q(weights,
                                      mesh_points,
                                      third_q,
                                      fixed_grid_number,
                                      np.array(mesh, dtype='intc').copy(),
                                      is_time_reversal * 1,
                                      np.array(rotations, dtype='intc').copy())

    return weights, third_q, mesh_points
        
def get_BZ_triplets_at_q(grid_point,
                         bz_grid_address,
                         bz_map,
                         weights,
                         mesh):
    """grid_address is overwritten."""
    num_ir_tripltes = (weights > 0).sum()
    triplets = np.zeros((num_ir_tripltes, 3), dtype='intc')
    num_ir_ret = spg.BZ_triplets_at_q(triplets,
                                      grid_point,
                                      bz_grid_address,
                                      bz_map,
                                      weights,
                                      np.array(mesh, dtype='intc').copy())
    return triplets

def get_triplets_tetrahedra_vertices(relative_grid_address,
                                     mesh,
                                     triplets,
                                     bz_grid_address,
                                     bz_map):
    num_tripltes = len(triplets)
    vertices = np.zeros((num_tripltes, 2, 24, 4), dtype='intc')
    num_ir_ret = spg.triplets_tetrahedra_vertices(
        vertices,
        relative_grid_address,
        np.array(mesh, dtype='intc').copy(),
        triplets,
        bz_grid_address,
        bz_map)

    return vertices

def get_tetrahedra_relative_grid_address(reciprocal_lattice):
    """
    reciprocal_lattice:
      column vectors of reciprocal lattice which can be obtained by
      reciprocal_lattice = np.linalg.inv(bulk.get_cell())
    """
    
    relative_grid_address = np.zeros((24, 4, 3), dtype='intc')
    spg.tetrahedra_relative_grid_address(relative_grid_address,
                                         np.array(reciprocal_lattice,
                                                  dtype='double').copy())
    return relative_grid_address

    
def get_tetrahedra_integration_weight(omegas,
                                      tetrahedra_omegas,
                                      function='I'):
    if isinstance(omegas, float):
        return spg.tetrahedra_integration_weight(
            omegas,
            np.array(tetrahedra_omegas, dtype='double').copy(),
            function)
    else:
        integration_weights = np.zeros(len(omegas), dtype='double')
        spg.tetrahedra_integration_weight_at_omegas(
            integration_weights,
            np.array(omegas, dtype='double'),
            np.array(tetrahedra_omegas, dtype='double').copy(),
            function)
        return integration_weights

                                      
                                               
