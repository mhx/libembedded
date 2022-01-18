#include <chrono>
#include <cstdio>
#include <iostream>
#include <vector>

#include "embedded/signal/chebyshev.h"
#include "embedded/signal/filter.h"

int main() {
  using namespace embedded::signal;

  // Repeat N times for more accurate benchmark
  constexpr size_t Repeat = 20;

  // Design the IIR filter:
  // - 10th-order Chebyshev Type-I lowpass filter
  // - Using double-precision for filter design
  // - Using single-precision for filter implementation
  // - Using a Second Order System (SOS) implementation
  //
  // The design is fully determined at compile time.
  constexpr double fs{1000.0}; // sample rate
  constexpr double fc{40.0};   // cutoff frequency
  constexpr double rp{3.0};    // passband ripple
  constexpr auto design =
      iirfilter<double>(fs).lowpass(chebyshev1<10>(rp), fc).sos<float>();

  // Set up input and output signal vectors
  std::vector<float> vec;
  std::vector<float> out;

  // Read signal from stdin
  {
    float x;
    while (std::fread(&x, sizeof(x), 1, stdin)) {
      vec.push_back(x);
    }
  }

  out.resize(vec.size());

  auto t1 = std::chrono::steady_clock::now();

  for (size_t i = 0; i < Repeat; ++i) {
    // Create a new instance of the filter design
    auto filter = design.instance();

    // Filter the input signal
    for (size_t k = 0; k < vec.size(); ++k) {
      out[k] = filter(vec[k]);
    }
  }

  auto t2 = std::chrono::steady_clock::now();

  std::cerr << 1e9 * std::chrono::duration<double>(t2 - t1).count() /
                   (Repeat * vec.size())
            << " ns/sample\n";

  // Write output signal to stdout
  std::fwrite(out.data(), sizeof(float), out.size(), stdout);

  return 0;
}
