# libembedded - Modern C++ for Embedded Systems

This library will hopefully, over time, become a collection of useful,
modern C++ abstractions, with a particular focus on being useful in
the context of embedded systems.

There isn't really a lot of code yet, except for a polymorphic function
wrapper and [a version of](https://github.com/mpark/variant) `std::variant`
by Michael Park back-ported to C++11.

The goal for this library is to provide abstractions that

- work nicely without exceptions and RTTI enabled,

- work nicely without dynamic memory allocation and

- require only C++11 compliant compilers.
