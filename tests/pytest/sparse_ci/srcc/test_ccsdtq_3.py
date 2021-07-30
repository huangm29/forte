#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pytest


@pytest.mark.skip(reason="This is a long test")
def test_ccsdtq_3():
    """Test CCSDTQ on Ne using RHF/cc-pVDZ orbitals"""

    import forte.proc.scc as scc
    import forte
    import psi4

    ref_energy = -128.679014931  # from Evangelista, J. Chem. Phys. 134, 224102 (2011).

    geom = "Ne"

    scf_energy, psi4_wfn = forte.utils.psi4_scf(geom, basis='cc-pVDZ', reference='RHF')
    forte_objs = forte.utils.prepare_forte_objects(psi4_wfn, mo_spaces={'FROZEN_DOCC': [1, 0, 0, 0, 0, 0, 0, 0]})
    calc_data = scc.run_cc(forte_objs[1], forte_objs[2], forte_objs[3], cc_type='cc', max_exc=4, e_convergence=1.0e-12)

    psi4.core.clean()

    energy = calc_data[-1][1]

    print(energy - ref_energy)
    assert energy == pytest.approx(ref_energy, 1.0e-9)


if __name__ == "__main__":
    test_ccsdtq_3()
