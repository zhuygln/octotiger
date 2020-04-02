#pragma once
//  Copyright (c) 2019 AUTHORS
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <cstdint>

#if !defined(__CUDA_ARCH__) && defined(OCTOTIGER_HAVE_VC)

#include <Vc/Vc>

#ifdef __AVX__
//using m2m_vector = typename Vc::datapar<double, Vc::datapar_abi::avx512>;
// for 8-wide (and not 16-wide) integers
//using m2m_int_vector = typename Vc::datapar<int32_t, Vc::datapar_abi::avx>;
// using m2m_int_vector = Vc::datapar<int64_t, Vc::datapar_abi::avx512>;
//#elif defined(Vc_HAVE_AVX)
#endif
#ifdef __AVX2__  // assumes AVX 2
using m2m_vector = Vc::Vector<double, Vc::VectorAbi::Avx>;
using m2m_int_vector = Vc::Vector<std::int32_t, Vc::VectorAbi::Avx>;
// using m2m_int_vector = typename Vc::datapar<int64_t, Vc::datapar_abi::avx>;
#else                         // falling back to fixed_size types
using m2m_vector = Vc::Vector<double, Vc::VectorAbi::Scalar>;
using m2m_int_vector = Vc::Vector<std::int32_t, Vc::VectorAbi::Scalar>;
#endif

// using multipole_v = taylor<4, m2m_vector>;
// using expansion_v = taylor<4, m2m_vector>;


#else /* !defined(__CUDA_ARCH__) && defined(OCTOTIGER_HAVE_VC) */

// no simd
using m2m_vector = double;
using m2m_int_vector = std::int32_t;

#endif /* !defined(__CUDA_ARCH__) && defined(OCTOTIGER_HAVE_VC) */