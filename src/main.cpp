#include "defs.hpp"

#include "node_server.hpp"
#include "node_client.hpp"
#include "future.hpp"
#include "problem.hpp"
#include "options.hpp"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include <fenv.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#else
#include <float.h>
#endif

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/lcos/broadcast.hpp>

#include "compute_factor.hpp"
#include "multipole_interactions/calculate_stencil.hpp"
#include "multipole_interactions/multipole_interaction_interface.hpp"
#include "monopole_interactions/calculate_stencil.hpp"
#include "monopole_interactions/monopole_interaction_interface.hpp"


options opts;

bool gravity_on = true;
bool hydro_on = true;
HPX_PLAIN_ACTION(grid::set_pivot, set_pivot_action);
HPX_REGISTER_BROADCAST_ACTION_DECLARATION (set_pivot_action)
HPX_REGISTER_BROADCAST_ACTION (set_pivot_action)

void compute_ilist();

void initialize(options _opts, std::vector<hpx::id_type> const& localities) {
	options::all_localities = localities;
	opts = _opts;
	grid::get_omega() = opts.omega;
#if !defined(_MSC_VER)
	feenableexcept(FE_DIVBYZERO);
	feenableexcept(FE_INVALID);
	feenableexcept(FE_OVERFLOW);
#else
	_controlfp(_EM_INEXACT | _EM_DENORMAL | _EM_INVALID, _MCW_EM);
#endif
	grid::set_scaling_factor(opts.xscale);
	grid::set_max_level(opts.max_level);
#ifdef RADIATION
	if (opts.problem == RADIATION_TEST) {
		gravity_on = false;
		set_problem(radiation_test_problem);
		set_refine_test(radiation_test_refine);
	} else
#endif
	if (opts.problem == DWD) {
		set_problem(scf_binary);
		set_refine_test(refine_test);
	} else if (opts.problem == SOD) {
		grid::set_fgamma(7.0 / 5.0);
		gravity_on = false;
		set_problem(sod_shock_tube_init);
		set_refine_test(refine_sod);
		grid::set_analytic_func(sod_shock_tube_analytic);
	} else if (opts.problem == BLAST) {
		grid::set_fgamma(7.0 / 5.0);
		gravity_on = false;
		set_problem(blast_wave);
		set_refine_test(refine_blast);
	} else if (opts.problem == STAR) {
		grid::set_fgamma(5.0 / 3.0);
		set_problem(star);
		set_refine_test(refine_test_moving_star);
	} else if (opts.problem == MOVING_STAR) {
		grid::set_fgamma(5.0 / 3.0);
		grid::set_analytic_func(moving_star_analytic);
		set_problem(moving_star);
		set_refine_test(refine_test_moving_star);
	} else if (opts.problem == SOLID_SPHERE) {
		hydro_on = false;
		set_problem(init_func_type([](real x, real y, real z, real dx) {
			return solid_sphere(x,y,z,dx,0.25);
		}));
	} else {
		printf("No problem specified\n");
		throw;
	}
	node_server::set_gravity(gravity_on);
	node_server::set_hydro(hydro_on);
	compute_ilist();
    compute_factor();
    octotiger::fmm::multipole_interactions::multipole_interaction_interface::
        stencil = octotiger::fmm::multipole_interactions::calculate_stencil();
    octotiger::fmm::monopole_interactions::monopole_interaction_interface::stencil =
        octotiger::fmm::monopole_interactions::calculate_stencil().first;
    octotiger::fmm::monopole_interactions::monopole_interaction_interface::four =
        octotiger::fmm::monopole_interactions::calculate_stencil().second;
}

HPX_PLAIN_ACTION(initialize, initialize_action);
HPX_REGISTER_BROADCAST_ACTION_DECLARATION (initialize_action)
HPX_REGISTER_BROADCAST_ACTION (initialize_action)

real OMEGA;
void node_server::set_pivot() {
	space_vector pivot = grid_ptr->center_of_mass();
	hpx::lcos::broadcast < set_pivot_action > (options::all_localities, pivot).get();
}
namespace scf_options {
void read_option_file();
}
int hpx_main(int argc, char* argv[]) {
	printf("###########################################################\n");
#if defined(__AVX512F__)
	printf("Compiled for AVX512 SIMD architectures.\n");
#elif defined(__AVX2__)
	printf("Compiled for AVX2 SIMD architectures.\n");
#elif defined(__AVX__)
	printf("Compiled for AVX SIMD architectures.\n");
#elif defined(__SSE2__ )
	printf("Compiled for SSE2 SIMD architectures.\n");
#else
	printf("Not compiled for a known SIMD architecture.\n");
#endif
	printf("###########################################################\n");

	printf("Running\n");

	try {
		if (opts.process_options(argc, argv)) {
			auto all_locs = hpx::find_all_localities();
			hpx::lcos::broadcast < initialize_action > (all_locs, opts, all_locs).get();

			node_client root_id = hpx::new_ < node_server > (hpx::find_here());
			node_client root_client(root_id);
			node_server* root = root_client.get_ptr().get();

			int ngrids = 0;
	//		printf("1\n");
			if (opts.found_restart_file) {
				set_problem(null_problem);
				const std::string fname = opts.restart_filename;
				printf("Loading from %s...\n", fname.c_str());
				root->load_from_file(fname, opts.data_dir);
				ngrids = root->regrid(root_client.get_gid(), ZERO, -1, true);
				printf("Done. \n");
			} else {
				for (integer l = 0; l < opts.max_level; ++l) {
					ngrids = root->regrid(root_client.get_gid(), grid::get_omega(), -1, false);
					printf("---------------Created Level %i---------------\n\n", int(l + 1));
				}
				ngrids = root->regrid(root_client.get_gid(), grid::get_omega(), -1, false);
				printf("---------------Regridded Level %i---------------\n\n", int(opts.max_level));
			}
		//	printf("2\n");
			if (gravity_on && !opts.output_only) {
				printf("solving gravity------------\n");
				root->solve_gravity(false,false);
				printf("...done\n");
			}
		//	printf("3\n");
			//    if( !opts.output_only ) {
			hpx::async(&node_server::start_run, root, opts.problem == DWD && !opts.found_restart_file, ngrids).get();
			//   }
			root->report_timing();
		}
	} catch (...) {
		throw;
	}
	printf("Exiting...\n");
    std::cout << "Overall multipole interactions calls:: RHO: " <<
            octotiger::fmm::multipole_interactions::m2m_kernel::multipole_callcounter_rho
              << " NON-RHO: " <<
            octotiger::fmm::multipole_interactions::m2m_kernel::multipole_callcounter_non_rho
              << std::endl;
    std::cout << "Overall monopoles monopoles interactions calls: " <<
            octotiger::fmm::monopole_interactions::p2p_kernel::p2p_counter << std::endl;
    std::cout << "Overall monopole multipole interactions calls:: RHO: " <<
            octotiger::fmm::monopole_interactions::p2m_kernel::p2m_counter_rho
              << " NON-RHO: " <<
            octotiger::fmm::monopole_interactions::p2m_kernel::p2m_counter_non_rho
              << std::endl;
    const size_t multipole_flops_non_rho =
            octotiger::fmm::multipole_interactions::m2m_kernel::multipole_callcounter_non_rho *
            octotiger::fmm::multipole_interactions::non_rho_flop;
    const size_t multipole_flops_rho =
            octotiger::fmm::multipole_interactions::m2m_kernel::multipole_callcounter_rho *
            octotiger::fmm::multipole_interactions::rho_flop;
    const size_t multipole_overall_flops = multipole_flops_non_rho + multipole_flops_rho;

    const size_t p2p_flops =
            octotiger::fmm::monopole_interactions::p2p_kernel::p2p_counter *
            octotiger::fmm::monopole_interactions::p2p_flop;

    const size_t p2m_flops_non_rho =
            octotiger::fmm::monopole_interactions::p2m_kernel::p2m_counter_non_rho *
            octotiger::fmm::monopole_interactions::non_rho_p2m_flop;
    const size_t p2m_flops_rho =
            octotiger::fmm::monopole_interactions::p2m_kernel::p2m_counter_rho *
            octotiger::fmm::monopole_interactions::rho_p2m_flop;
    const size_t monopole_overall_flops = p2m_flops_non_rho + p2m_flops_rho;

    const size_t g_divisor = 1000 * 1000 * 1000;

    std::cout << "Multipole GFLOPs non rho: " << multipole_flops_non_rho << " -> "
              << static_cast<double>(multipole_flops_non_rho) / g_divisor << " GFLOP " << std::endl;
    std::cout << "Multipole GFLOPs rho: " << multipole_flops_rho << " -> "
              << static_cast<double>(multipole_flops_rho) / g_divisor << " GFLOP " << std::endl;
    std::cout << "Monopole Monopole GFLOPs : " << p2p_flops << " -> "
              << static_cast<double>(p2p_flops) / g_divisor << " GFLOP " << std::endl;
    std::cout << "Monopole Multipole GFLOPs non rho : " << p2m_flops_non_rho << " -> "
              << static_cast<double>(p2m_flops_non_rho) / g_divisor << " GFLOP " << std::endl;
    std::cout << "Monopole Multipole GFLOPs rho : " << p2m_flops_rho << " -> "
              << static_cast<double>(p2m_flops_rho) / g_divisor << " GFLOP " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    const size_t overall_flops = multipole_flops_non_rho + multipole_flops_rho + p2p_flops +
        p2m_flops_rho + p2m_flops_non_rho;
    std::cout << "Overall: " << overall_flops << " -> "
              << static_cast<double>(overall_flops) / g_divisor << "GFLOPs" << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[]) {
	std::vector<std::string> cfg = { "hpx.commandline.allow_unknown=1", // HPX should not complain about unknown command line options
			"hpx.scheduler=local-priority-lifo",       // use LIFO scheduler by default
			"hpx.parcel.mpi.zero_copy_optimization!=0" // Disable the usage of zero copy optimization for MPI...
			};

	hpx::register_pre_shutdown_function([]() {options::all_localities.clear();});

	hpx::init(argc, argv, cfg);
}
