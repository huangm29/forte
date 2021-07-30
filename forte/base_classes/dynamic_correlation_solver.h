/*
 * @BEGIN LICENSE
 *
 * Forte: an open-source plugin to Psi4 (https://github.com/psi4/psi4)
 * that implements a variety of quantum chemistry methods for strongly
 * correlated electrons.
 *
 * Copyright (c) 2012-2020 by its authors (see COPYING, COPYING.LESSER, AUTHORS).
 *
 * The copyrights for code used from other parties are included in
 * the corresponding files.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 * @END LICENSE
 */

#ifndef _dynamic_correlation_solver_h_
#define _dynamic_correlation_solver_h_

#include <memory>

#include "integrals/integrals.h"
#include "integrals/active_space_integrals.h"
#include "base_classes/rdms.h"
#include "base_classes/forte_options.h"

namespace forte {

class SCFInfo;

class DynamicCorrelationSolver {
  public:
    /**
     * Constructor
     * @param ref_wfn The reference wavefunction object
     * @param options The main options object
     * @param ints A pointer to an allocated integral object
     * @param mo_space_info The MOSpaceInfo object
     */
    DynamicCorrelationSolver(RDMs rdms, std::shared_ptr<SCFInfo> scf_info,
                             std::shared_ptr<ForteOptions> options,
                             std::shared_ptr<ForteIntegrals> ints,
                             std::shared_ptr<MOSpaceInfo> mo_space_info);

    /// Compute energy
    virtual double compute_energy() = 0;

    /// Compute dressed Hamiltonian
    virtual std::shared_ptr<ActiveSpaceIntegrals> compute_Heff_actv() = 0;

    /// Destructor
    virtual ~DynamicCorrelationSolver() = default;

    /// Set whether to read amplitudes or not manually
    void set_read_amps_cwd(bool read) { read_amps_cwd_ = read; }

    /// Clean up amplitudes checkpoint files
    void clean_checkpoints();

  protected:
    /// The molecular integrals
    std::shared_ptr<ForteIntegrals> ints_;

    /// The MO space info
    std::shared_ptr<MOSpaceInfo> mo_space_info_;

    /// The RDMs and cumulants of the reference wave function
    RDMs rdms_;

    /// The SCFInfo
    std::shared_ptr<SCFInfo> scf_info_;

    /// The ForteOptions
    std::shared_ptr<ForteOptions> foptions_;

    /// Common settings
    void startup();

    /// Nuclear repulsion energy
    double Enuc_;

    /// Frozen core energy
    double Efrzc_;

    /// Printing level
    int print_;

    /// The integral type
    std::string ints_type_;
    /// If ERI density fitted or Cholesky decomposed
    bool eri_df_;

    // ==> DIIS control <==

    /// Cycle number to start DIIS
    int diis_start_;
    /// Minimum number of DIIS vectors
    int diis_min_vec_;
    /// Maximum number of DIIS vectors
    int diis_max_vec_;
    /// Frequency of extrapolating the current DIIS vectors
    int diis_freq_;

    // ==> internal amplitudes <==

    /// How to consider internal amplitudes
    std::string internal_amp_;
    /// Include which part of internal amplitudes?
    std::string internal_amp_select_;

    /// Relative indices of GASn within the active
    std::map<std::string, std::vector<size_t>> gas_actv_rel_mos_;

    /// T1 internal types (e.g., [(GAS1,GAS1,1), (GAS1,GAS2,0), ...])
    /// Boolean is true if it is pure internal
    std::vector<std::tuple<std::string, std::string, bool>> t1_internals_;
    /// T2 internal types (e.g., [(GAS1,GAS1,GAS1,GAS1,1), (GAS1,GAS1,GAS2,GAS2,0), ...])
    std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>> t2_internals_;

    /// Figure out allowed internal amplitudes types
    void build_internal_amps_types();
    /// Figure out allowed internal T1 amplitudes types
    void build_t1_internal_types();
    /// Figure out allowed internal T2 amplitudes types
    void build_t2_internal_types();

    // ==> amplitudes file names <==

    /// Checkpoint file for T1 amplitudes
    std::string t1_file_chk_;
    /// Checkpoint file for T2 amplitudes
    std::string t2_file_chk_;

    /// File name for T1 amplitudes to be saved in current directory
    std::string t1_file_cwd_;
    /// File name for T2 amplitudes to be saved in current directory
    std::string t2_file_cwd_;

    /// Dump amplitudes to current directory
    bool dump_amps_cwd_ = false;
    /// Read amplitudes from current directory
    bool read_amps_cwd_ = false;

    /// Dump the converged amplitudes to disk
    /// Iterative methods should override this function
    virtual void dump_amps_to_disk() {}
};

std::shared_ptr<DynamicCorrelationSolver>
make_dynamic_correlation_solver(const std::string& type, std::shared_ptr<ForteOptions> options,
                                std::shared_ptr<ForteIntegrals> ints,
                                std::shared_ptr<MOSpaceInfo> mo_space_info);

} // namespace forte

#endif // DYNAMIC_CORRELATION_SOLVER_H
