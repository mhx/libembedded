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

import numpy as np
import os
import re
from textwrap import dedent, indent, fill
from scipy.signal import iirfilter, lfilter, sosfilt

STATIC = True
IMPULSE_LEN = 1024


def file_header(fh, fptype, ftype):
    hmap = {
        "bessel": "bessel.h",
        "butter": "butterworth.h",
        "cheby1": "chebyshev.h",
        "cheby2": "chebyshev.h",
    }
    header = hmap[ftype]
    eps = 1e-7 if fptype == "float" else 1e-13
    fh.write(
        dedent(
            f"""    /*
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

    #include <array>
    #include <vector>

    #include "embedded/signal/{header}"
    #include "embedded/signal/filter.h"

    #include <gmock/gmock.h>
    #include <gtest/gtest.h>

    using namespace embedded::signal;

    namespace {{

    template <typename F1, typename F2>
    constexpr bool almost_equal(F1 a, F2 b) noexcept {{
      return (a < b ? b - a : a - b) / (b < 0 ? -b : b) < {eps};
    }}

    template <typename T>
    std::vector<typename T::value_type> impulse_response(T& filter, size_t len) {{
      typename T::value_type x{{1}};
      std::vector<typename T::value_type> y;
      y.resize(len);
      for (size_t i = 0; i < len; ++i) {{
        y[i] = filter(x);
        x = 0;
      }}
      return y;
    }}

    }} // namespace

    // clang-format off
    """
        )
    )


def test_base_name(fptype, order, freq, btype, ftype, args, fs):
    fargs = "_".join([f"{k}_{v}" for k, v in args.items()])
    if fargs:
        fargs = "_" + fargs
    test_name = f"{ftype[0]}_{btype}_{order}_{freq}_{fs}{fargs}_{fptype}"
    return re.sub("\W", "_", test_name)


def base_design(fptype, order, freq, btype, ftype, args, fs):
    fmap = {
        "bessel": "bessel",
        "butter": "butterworth",
        "cheby1": "chebyshev1",
        "cheby2": "chebyshev2",
    }
    amap = {
        "bessel": lambda _: "",
        "butter": lambda _: "",
        "cheby1": lambda x: str(x["rp"]),
        "cheby2": lambda x: str(x["rs"]),
    }
    design = f"iirfilter<double>({fs}).{btype}({fmap[ftype[0]]}<{order}>({amap[ftype[0]](args)}), {freq})"
    return design


def start_test(fh, fptype, order, freq, btype, ftype, args, fs):
    test_name = test_base_name(fptype, order, freq, btype, ftype, args, fs)
    design = base_design(fptype, order, freq, btype, ftype, args, fs)

    fh.write(
        dedent(
            f"""
        TEST(signal_generated, {test_name}) {{
          constexpr auto base_design = {design};\n"""
        )
    )


def end_test(fh):
    fh.write("}\n")


def static_assert_almost_equal(fh, e1, e2):
    if STATIC:
        fh.write(f'    static_assert(almost_equal({e1}, {e2}), "{e1} == {e2}");\n')
    else:
        fh.write(f"    EXPECT_NEAR({e1}, {e2}, {abs(1e-13*e2)});\n")


def generate_test_poly(fh, fptype, order, freq, btype, ftype, fs, b, a):
    fh.write(
        indent(
            dedent(
                f"""
      {{ // polynomial (DF2T)
        constexpr auto design = base_design.poly<{fptype}>();
        double max_deviation{{}};

        static_assert(design.order() == {order}, "order");
        static_assert(design.b().size() == {order + 1}, "b.size");
        static_assert(design.a().size() == {order + 1}, "a.size");

    """
            ),
            "  ",
        )
    )

    for i, bi in enumerate(b):
        static_assert_almost_equal(fh, f"design.b()[{i}]", b[i])
        fh.write(
            f"    max_deviation = std::max(max_deviation, std::abs(design.b()[{i}] - {b[i]}));\n"
        )

    for i, bi in enumerate(b):
        static_assert_almost_equal(fh, f"design.a()[{i}]", a[i])
        fh.write(
            f"    max_deviation = std::max(max_deviation, std::abs(design.a()[{i}] - {a[i]}));\n"
        )

    fh.write(
        f'    std::cout << "max_deviation({ftype[0]},{btype},{order},{fptype}) = " << max_deviation << "\\n";\n'
    )

    inp = np.zeros(IMPULSE_LEN)
    inp[0] = 1
    ref = lfilter(b, a, inp)

    if fptype == "float":
        delta = 1e-4 if order < 4 else 1e-2
    else:
        delta = 1e-11 if order < 8 else 1e-7

    fh.write(
        indent(
            dedent(
                f"""
        auto filter = design.instance();
        auto impulse = impulse_response(filter, {IMPULSE_LEN});
        static constexpr std::array<{fptype}, {IMPULSE_LEN}> expected{{{{
        {fill(', '.join(str(y) for y in ref), width=96, initial_indent='  ', subsequent_indent='          ')}
        }}}};
        EXPECT_THAT(impulse, testing::Pointwise(testing::DoubleNear({delta}), expected));\n"""
            ),
            "    ",
        )
    )

    fh.write("  }\n")


def generate_test_sos(fh, fptype, order, freq, btype, ftype, fs, sos):
    fh.write(
        indent(
            dedent(
                f"""
      {{ // DF2T-based SOS
        constexpr auto design = base_design.sos<{fptype}>();

        static_assert(design.order() == {order}, "order");\n"""
            ),
            "  ",
        )
    )

    inp = np.zeros(IMPULSE_LEN)
    inp[0] = 1
    ref = sosfilt(sos, inp)

    if fptype == "float":
        delta = 1e-5
    else:
        delta = 1e-14 if order < 40 else 1e-5

    fh.write(
        indent(
            dedent(
                f"""
        auto filter = design.instance();
        auto impulse = impulse_response(filter, {IMPULSE_LEN});

        static constexpr std::array<{fptype}, {IMPULSE_LEN}> expected{{{{
        {fill(', '.join(str(y) for y in ref), width=96, initial_indent='  ', subsequent_indent='          ')}
        }}}};

        EXPECT_THAT(impulse, testing::Pointwise(testing::DoubleNear({delta}), expected));\n"""
            ),
            "    ",
        )
    )

    fh.write("  }\n")


ftypes = [
    ["bessel", [{}]],
    ["butter", [{}]],
    ["cheby1", [{"rp": 0.5}, {"rp": 3.0}]],
    ["cheby2", [{"rs": 20.0}, {"rs": 80.0}]],
]
freq = 40
fs = 1000
dirname = os.path.dirname(__file__)

for ftype in ftypes:
    for fptype in ["double", "float"]:
        filename = os.path.join(dirname, f"../tests/signal_{ftype[0]}_{fptype}.cpp")
        with open(filename, "w") as fh:
            print(f"writing {filename}...")
            file_header(fh, fptype, ftype[0])
            for args in ftype[1]:
                for btype in ["lowpass", "highpass"]:
                    for order in [1, 2, 3, 4, 5, 9, 48]:
                        if fptype == "float":
                            if order > 24:
                                continue
                            max_poly_order = 6
                        else:
                            max_poly_order = 12

                        start_test(fh, fptype, order, freq, btype, ftype, args, fs)
                        if order <= max_poly_order:
                            b, a = iirfilter(
                                order, freq, btype=btype, ftype=ftype[0], **args, fs=fs
                            )
                            generate_test_poly(
                                fh, fptype, order, freq, btype, ftype, fs, b, a
                            )

                        sos = iirfilter(
                            order,
                            freq,
                            btype=btype,
                            ftype=ftype[0],
                            **args,
                            fs=fs,
                            output="sos",
                        )
                        generate_test_sos(
                            fh, fptype, order, freq, btype, ftype, fs, sos
                        )
                        end_test(fh)
