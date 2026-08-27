#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
public:
  explicit ThreadPool(std::size_t threadCount);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  std::size_t threadCount() const;

  template <typename Func> auto enqueue(Func &&func) {
    using Function = std::decay_t<Func>;
    using ReturnType = std::invoke_result_t<Function &>;

    auto packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(
        std::forward<Func>(func));
    std::future<ReturnType> future = packagedTask->get_future();

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopped_) {
        throw std::runtime_error("ThreadPool is stopped");
      }
      tasks_.emplace([packagedTask] { (*packagedTask)(); });
    }

    cv_.notify_one();
    return future;
  }

private:
  using Task = std::function<void()>;

  std::vector<std::thread> workers_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::queue<Task> tasks_;
  bool stopped_ = false;

  void stop();
  void joinWorkers();
  void worker();
};
