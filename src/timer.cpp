#include "timer.h"
#include <chrono>
#include <iostream>
#include <utility>

Timer::Timer(std::string label)
    : label_{std::move(label)}, startedAt_{Clock::now()} {}

Timer::~Timer() noexcept {
  const auto finishedAt = Clock::now();
  const auto elapsed =
      std::chrono::duration<double, std::milli>(finishedAt - startedAt_);

  try {
    std::cout << label_ << ": " << elapsed.count() << " ms\n";
  } catch (...) {
  }
}
