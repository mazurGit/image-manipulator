#pragma once

#include <chrono>
#include <string>

class Timer {
public:
  explicit Timer(std::string label = "Elapsed time");
  ~Timer() noexcept;

  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;
  Timer(Timer &&) = delete;
  Timer &operator=(Timer &&) = delete;

private:
  using Clock = std::chrono::steady_clock;

  std::string label_;
  Clock::time_point startedAt_;
};
