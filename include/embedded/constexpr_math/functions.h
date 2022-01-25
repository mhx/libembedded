/*
 * Copyright (c) Marcus Holland-Moritz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

// TODO: this is probably a poor check
#if !defined(LIBEMB_DISABLE_GLIBCXX_CONSTEXPR) &&                              \
    !defined(__INTEL_COMPILER) && !defined(__clang__) && defined(__GNUG__) &&  \
    defined(_GLIBCXX_CONSTEXPR)

#include <algorithm>
#include <cmath>

namespace embedded {
namespace cmath {

using ::std::abs;
using ::std::acos;
using ::std::acosh;
using ::std::asin;
using ::std::asinh;
using ::std::atan;
using ::std::atan2;
using ::std::atanh;
using ::std::ceil;
using ::std::copysign;
using ::std::cos;
using ::std::cosh;
using ::std::erf;
using ::std::exp;
using ::std::expm1;
using ::std::floor;
using ::std::fmod;
using ::std::lgamma;
using ::std::log;
using ::std::log10;
using ::std::log1p;
using ::std::log2;
using ::std::max;
using ::std::min;
using ::std::pow;
using ::std::round;
using ::std::signbit;
using ::std::sin;
using ::std::sinh;
using ::std::sqrt;
using ::std::tan;
using ::std::tanh;
using ::std::tgamma;
using ::std::trunc;

} // namespace cmath
} // namespace embedded

#else

// #define GCEM_ERF_MAX_ITER (2*60)
// #define GCEM_ERF_INV_MAX_ITER (2*55)
// #define GCEM_EXP_MAX_ITER_SMALL (2*25)
// #define GCEM_LOG_MAX_ITER_SMALL (2*25)
// #define GCEM_LOG_MAX_ITER_BIG (2*255)
// #define GCEM_INCML_BETA_TOL 1E-16
// #define GCEM_INCML_BETA_MAX_ITER (2*205)
// #define GCEM_INCML_BETA_INV_MAX_ITER (2*35)
// #define GCEM_INCML_GAMMA_MAX_ITER (2*55)
// #define GCEM_INCML_GAMMA_INV_MAX_ITER (2*35)
// #define GCEM_SQRT_MAX_ITER (2*100)
// #define GCEM_TAN_MAX_ITER (2*35)
// #define GCEM_TANH_MAX_ITER (2*35)

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

#include <gcem.hpp>

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on

namespace embedded {
namespace cmath {

using ::gcem::abs;
using ::gcem::acos;
using ::gcem::acosh;
using ::gcem::asin;
using ::gcem::asinh;
using ::gcem::atan;
using ::gcem::atan2;
using ::gcem::atanh;
using ::gcem::ceil;
using ::gcem::copysign;
using ::gcem::cos;
using ::gcem::cosh;
using ::gcem::erf;
using ::gcem::exp;
using ::gcem::expm1;
using ::gcem::floor;
using ::gcem::fmod;
using ::gcem::lgamma;
using ::gcem::lmgamma;
using ::gcem::log;
using ::gcem::log10;
using ::gcem::log1p;
using ::gcem::log2;
using ::gcem::max;
using ::gcem::min;
using ::gcem::pow;
using ::gcem::round;
using ::gcem::signbit;
using ::gcem::sin;
using ::gcem::sinh;
using ::gcem::sqrt;
using ::gcem::tan;
using ::gcem::tanh;
using ::gcem::tgamma;
using ::gcem::trunc;

} // namespace cmath
} // namespace embedded

#endif
