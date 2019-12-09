#!/usr/bin/env python
# -*- coding: utf-8 -*-

import psi4
import forte
from forte import forte_options

def test_mospaceinfo():
    """Test class constructors"""
    print("Testing mospaceinfo")

    geom = "H 0.0 0.0 0.0\nH 0.0 0.0 1.0"""

    mol = psi4.geometry(geom)

    # set basis/options
    psi4.set_options({'basis': 'cc-pVDZ', 'scf_type': 'pk'})

    # pipe output to the file output.dat
    psi4.core.set_output_file('output.dat', False)

    # run scf and return the energy and a wavefunction object (will work only if pass return_wfn=True)
    E_scf, wfn = psi4.energy('scf', return_wfn=True)



    options = psi4.core.get_options() # options = psi4 option object
    options.set_current_module('FORTE') # read options labeled 'FORTE'
    forte_options.update_psi_options(options)

    # Setup forte and prepare the active space integral class
    mos_spaces = {'FROZEN_DOCC' :     [1,0,0,0,0,0,0,0],
                  'RESTRICTED_DOCC' : [0,0,0,0,0,1,0,0],
                  'FROZEN_UOCC' :     [1,0,0,0,0,0,0,0]}
    mo_space_info = forte.make_mo_space_info_from_map(wfn,mos_spaces,[])

    assert mo_space_info.size('FROZEN_DOCC') == 1
    assert mo_space_info.size('RESTRICTED_DOCC') == 1
    assert mo_space_info.size('ACTIVE') == 7
    assert mo_space_info.size('RESTRICTED_UOCC') == 0
    assert mo_space_info.size('FROZEN_UOCC') == 1
#    assert d1 == d3
