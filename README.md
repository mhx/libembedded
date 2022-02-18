[![Codacy Badge](https://app.codacy.com/project/badge/Grade/5e1f58ea5d434b09bfba571413a02811)](https://www.codacy.com/gh/mhx/libembedded/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mhx/libembedded&amp;utm_campaign=Badge_Grade)

# libembedded - Modern C++ for Embedded Systems

This library will hopefully, over time, become a collection of useful,
modern C++ abstractions, with a particular focus on being useful in
the context of embedded systems.

The goal for this library is to provide abstractions that

- work nicely without exceptions and RTTI enabled,

- work nicely without dynamic memory allocation and

- require only C++11 compliant compilers.

# What's inside

## A polymorphic function wrapper with in-place storage

`embedded::function` is loosely modeled after `std::function` and inspired
by both SG14's `inplace_function` and `folly::Function`. It requires no
dynamic memory allocation, but, contrary to `etl::delegate`, is able to
wrap lambdas and other function objects as long as they fit into the
in-place storage.

## A circular buffer adapter

The `circular_buffer_adapter` template allows the construction of circular
buffers on top of a arbitrary, and possibly persistent, memory. This means
that you can easily build circular buffers on top of EEPROM sections.

## Variable length integers

`embedded::varint` implements encoding and decoding of interger values to
and from byte streams, using as few bytes as possible, and independent of
byte order.

## Type traits

A few type traits have been back-ported from later C++ standards. More
will be added on-demand.

## An experimental compile-time IIR filter design library

This library (in `embedded/signal`) can be used to directly design IIR
filters in C++ code without any run-time overhead. This is currently in
experimental state as the interface isn't yet settled and there's quite
a few things that are still missing. However, you can do things like:

```
constexpr auto design =
    iirfilter<>(1000.0).lowpass(chebyshev1<8>(3.0), 40.0).sos<float>();

auto filter = design.instance();

for (;;) {
    output(filter(input()));
}
```

This will design an 8th-order Chebyshev Type I low-pass filter with
3 dB of passband ripple, a sample rate of 1 kHz and a cutoff frequency
of 40 Hz. All filter coefficients will be determined at compile-time
and the run-time filter code is extremely efficient. For example, this
is the full ARM assembly for the above code, but using a second-order
filter:

```
process():
        push            {r3, lr}
        vpush.64        {d8, d9, d10}
        vldr.32         s17, .L5
        vldr.32         s20, .L5+4
        vldr.32         s19, .L5+8
        vmov.f32        s16, s17
        vldr.32         s18, .L5+12
.L2:
        bl              input()
        vmul.f32        s13, s0, s20
        vmov.f32        s15, s0
        vldr.32         s12, .L5+16
        vadd.f32        s14, s16, s13
        vmul.f32        s16, s14, s18
        vmls.f32        s13, s14, s12
        vmov.f32        s0, s14
        vnmls.f32       s16, s15, s19
        vadd.f32        s16, s16, s17
        vmov.f32        s17, s13
        bl              output(float)
        b               .L2
.L5:
        .word   0
        .word   1005574271
        .word   1013962879
        .word   -1075339548
        .word   1062851613
```

The library is also able to build fixed-point arithmetic filters
if a suitable fixed-point implementation is provided
(e.g. [fpm](https://github.com/MikeLankamp/fpm)).

You can find examples in the `examples` directory of the repo.

## Experimental compile-time math library

This is primarily used by the compile-time filter design library and
currently only contains the minimum functionality needed to implement
the filter design. It does contain things like `constexpr` functions
(provided either via `libstdc++`'s non-standard builtins or via the
[gcem library](https://github.com/kthohr/gcem)), `constexpr` complex
numbers and `constexpr` vectors.
