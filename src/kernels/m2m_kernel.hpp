#pragma once

#include "interaction_constants.hpp"
#include "interactions_iterators.hpp"
#include "multiindex.hpp"
#include "struct_of_array_data.hpp"
#include "taylor.hpp"

#include <vector>

namespace octotiger {
namespace fmm {

    class m2m_kernel
    {
    private:
        // std::vector<expansion>& local_expansions;
        struct_of_array_data<expansion, real, 20>& local_expansions_SoA;

        // com0 = *(com_ptr[0])
        // std::vector<space_vector>& center_of_masses;
        // down from 4 components
        struct_of_array_data<space_vector, real, 3>& center_of_masses_SoA;

        // multipole expansion on this cell (L)
        // std::vector<expansion>& potential_expansions;
        struct_of_array_data<expansion, real, 20>& potential_expansions_SoA;
        // angular momentum correction on this cell (L_c)
        // std::vector<space_vector>& angular_corrections;
        struct_of_array_data<space_vector, real, 3>& angular_corrections_SoA;

        gsolve_type type;

        const simd_vector theta_rec_sqared;

        // // for superimposed stencil
        // template <typename F>
        // void iterate_inner_cells_padded_stencil(multiindex<>& stencil_element, F& f) {
        //     for (size_t i0 = 0; i0 < INNER_CELLS_PER_DIRECTION; i0++) {
        //         for (size_t i1 = 0; i1 < INNER_CELLS_PER_DIRECTION; i1++) {
        //             // for (size_t i2 = 0; i2 < INNER_CELLS_PER_DIRECTION; i2++) {
        //             for (size_t i2 = 0; i2 < INNER_CELLS_PER_DIRECTION; i2 += simd_vector::Size)
        //             {
        //                 const multiindex<> cell_index(i0 + INNER_CELLS_PADDING_DEPTH,
        //                     i1 + INNER_CELLS_PADDING_DEPTH, i2 + INNER_CELLS_PADDING_DEPTH);
        //                 // BUG: indexing has to be done with uint32_t because of Vc limitation
        //                 const int64_t cell_flat_index = to_flat_index_padded(cell_index);
        //                 const multiindex<> cell_index_unpadded(i0, i1, i2);
        //                 const int64_t cell_flat_index_unpadded =
        //                     to_inner_flat_index_not_padded(cell_index_unpadded);
        //                 const multiindex<> interaction_partner_index(
        //                     cell_index.x + stencil_element.x, cell_index.y + stencil_element.y,
        //                     cell_index.z + stencil_element.z);

        //                 const int64_t interaction_flat_partner_index =
        //                     to_flat_index_padded(interaction_partner_index);
        //                 // std::cout << "cur: " << cell_index
        //                 //           << " partner: " << interaction_partner_index << std::endl;
        //                 f(cell_index, cell_flat_index, cell_index_unpadded,
        //                     cell_flat_index_unpadded, interaction_partner_index,
        //                     interaction_flat_partner_index);
        //             }
        //         }
        //     }
        // }

        // template <typename F>
        // void iterate_inner_cells_padded_stridded_stencil(
        //     multiindex<>& offset, multiindex<>& stencil_element, F& f) {
        //     for (size_t i0 = offset.x; i0 < INNER_CELLS_PER_DIRECTION; i0 += 2) {
        //         for (size_t i1 = offset.y; i1 < INNER_CELLS_PER_DIRECTION; i1 += 2) {
        //             for (size_t i2 = offset.z; i2 < INNER_CELLS_PER_DIRECTION; i2 += 2) {
        //                 // for (size_t i0 = 0; i0 < INNER_CELLS_PER_DIRECTION; i0 += 1) {
        //                 //     for (size_t i1 = 0; i1 < INNER_CELLS_PER_DIRECTION; i1 += 1) {
        //                 //         for (size_t i2 = 0; i2 < INNER_CELLS_PER_DIRECTION; i2 += 1) {

        //                 const multiindex<> cell_index(i0 + INNER_CELLS_PADDING_DEPTH,
        //                     i1 + INNER_CELLS_PADDING_DEPTH, i2 + INNER_CELLS_PADDING_DEPTH);
        //                 const size_t cell_flat_index = to_flat_index_padded(cell_index);
        //                 const multiindex<> cell_index_unpadded(i0, i1, i2);
        //                 const size_t cell_flat_index_unpadded =
        //                     to_inner_flat_index_not_padded(cell_index_unpadded);
        //                 const multiindex<> interaction_partner_index(
        //                     cell_index.x + stencil_element.x, cell_index.y + stencil_element.y,
        //                     cell_index.z + stencil_element.z);
        //                 const size_t interaction_partner_flat_index =
        //                     to_flat_index_padded(interaction_partner_index);

        //                 // std::cout << "cur: " << cell_index
        //                 //           << " partner: " << interaction_partner_index << std::endl;
        //                 f(cell_index, cell_flat_index, cell_index_unpadded,
        //                     cell_flat_index_unpadded, interaction_partner_index,
        //                     interaction_partner_flat_index);
        //             }
        //         }
        //     }
        // }

        void single_interaction(const multiindex<>& cell_index, const int64_t cell_flat_index,
            const multiindex<int_simd_vector>& cell_index_coarse,
            const multiindex<>& cell_index_unpadded, const int64_t cell_flat_index_unpadded,
            const multiindex<>& interaction_partner_index,
            const int64_t interaction_partner_flat_index);

    public:
        m2m_kernel(struct_of_array_data<expansion, real, 20>& local_expansions_SoA,
            struct_of_array_data<space_vector, real, 3>& center_of_masses_SoA,
            struct_of_array_data<expansion, real, 20>& potential_expansions_SoA,
            struct_of_array_data<space_vector, real, 3>& angular_corrections_SoA, gsolve_type type);

        m2m_kernel(m2m_kernel& other) = delete;

        m2m_kernel(const m2m_kernel& other) = delete;

        m2m_kernel operator=(const m2m_kernel& other) = delete;

        void apply_stencil(std::vector<multiindex<>>& stencil);
    };

}    // namespace fmm
}    // namespace octotiger
