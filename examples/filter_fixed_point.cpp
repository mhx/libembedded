#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

#include <fpm/fixed.hpp>

#include "embedded/ostream_ops.h"
#include "embedded/signal/butterworth.h"
#include "embedded/signal/filter.h"

int main() {
  using namespace embedded::signal;

  // Repeat N times for more accurate benchmark
  constexpr size_t Repeat = 1;

  constexpr unsigned FractionBits = 28;
  constexpr int ScaleFactor = 1 << (FractionBits - 8 * sizeof(int16_t));

  // Define our fixed-point arithmetic type
  using value_type = fpm::fixed<int32_t, int64_t, FractionBits>;

  // Design the IIR filter:
  // - 20th-order Butterworth highpass filter
  // - Using double-precision for filter design
  // - Using fixed-point for filter implementation
  // - Using a Second Order System (SOS) implementation
  //
  // The design is fully determined at compile time.
  constexpr double fs{1000.0}; // sample rate
  constexpr double fc{40.0};   // cutoff frequency
  constexpr auto design = iirfilter<double>(fs)
                              .highpass(butterworth<20>(), fc)
                              .sos<value_type>(sos_gain::distribute);

  // Set up input and output signal vectors
  std::vector<int16_t> vec;
  std::vector<int16_t> out;

  // Read signal from stdin
  {
    int16_t x;
    while (std::fread(&x, sizeof(x), 1, stdin)) {
      vec.push_back(x);
    }
  }

  // Reserve space for output signal and individual SOS sections
  out.resize(vec.size() * (1 + design.sos().size()));

  auto t1 = std::chrono::steady_clock::now();

  for (size_t i = 0; i < Repeat; ++i) {
    // Create a new instance of the filter design
    auto filter = design.instance();

    // Filter the input signal
    for (size_t k = 0; k < vec.size(); ++k) {
      out[k] =
          filter(value_type::from_raw_value(ScaleFactor * vec[k])).raw_value() /
          ScaleFactor;
    }
  }

  auto t2 = std::chrono::steady_clock::now();

  std::cerr << 1e9 * std::chrono::duration<double>(t2 - t1).count() /
                   (Repeat * vec.size())
            << " ns/sample\n";

  // Filter the signal through each SOS section individually
  for (size_t i = 0; i < design.sos().size(); ++i) {
    auto const& section = design.sos()[i];
    std::decay<decltype(section)>::type::state_type state;

    for (size_t k = 0; k < vec.size(); ++k) {
      out[(i + 1) * vec.size() + k] =
          section
              .filter(state, value_type::from_raw_value(ScaleFactor * vec[k]))
              .raw_value() /
          ScaleFactor;
    }
  }

  // Write output and SOS decomposition signals to stdout
  std::fwrite(out.data(), sizeof(out[0]), out.size(), stdout);

  return 0;
}
