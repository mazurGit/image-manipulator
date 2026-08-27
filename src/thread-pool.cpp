#include "thread-pool.h"

ThreadPool::ThreadPool(std::size_t threadCount) {
  if (threadCount == 0) {
    throw std::invalid_argument("thread count must be positive");
  }

  workers_.reserve(threadCount);
  try {
    for (std::size_t index = 0; index < threadCount; ++index) {
      workers_.emplace_back(&ThreadPool::worker, this);
    }
  } catch (...) {
    stop();
    joinWorkers();
    throw;
  }
}

ThreadPool::~ThreadPool() {
  stop();
  joinWorkers();
}

void ThreadPool::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
  }
  cv_.notify_all();
}

void ThreadPool::joinWorkers() {
  for (std::thread &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::worker() {
  while (true) {
    Task task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return stopped_ || !tasks_.empty(); });
      if (stopped_ && tasks_.empty()) {
        return;
      }
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}
