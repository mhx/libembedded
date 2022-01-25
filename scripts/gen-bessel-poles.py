#
# Copyright (c) Marcus Holland-Moritz
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

import os
from textwrap import dedent
from scipy.signal.filter_design import besselap

filename = os.path.join(
    os.path.dirname(__file__), f"../include/embedded/signal/detail/bessel_poles.h"
)

with open(filename, "w") as fh:

    fh.write(
        dedent(
            """            /*
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

            #include <cstddef>

            #include "../../constexpr_math/complex.h"
            #include "../../constexpr_math/vector.h"

            namespace embedded {
            namespace signal {
            namespace detail {

            template <typename F, std::size_t Order>
            struct bessel_poles;
            """
        )
    )

    for order in range(1, 101):
        fh.write(
            dedent(
                f"""
                template <typename F>
                struct bessel_poles<F, {order}> {{
                  static constexpr cmath::vector<cmath::complex<F>, {order}> value{{
                """
            )
        )

        _, ps, k = besselap(order, "phase")
        assert k == 1.0
        for p in ps:
            fh.write(f"      cmath::complex<F>{{{p.real}l, {p.imag}l}},\n")

        fh.write("  };\n};\n")

    fh.write(
        dedent(
            """
            } // namespace detail
            } // namespace signal
            } // namespace embedded
            """
        )
    )
