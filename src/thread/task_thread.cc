// MIT License
//
// Copyright (c) 2022 Streamlet (streamlet@outlook.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <xl/task_thread>

namespace xl {

task_thread::task_thread() : thread_(std::bind(&task_thread::run, this)), event_(false, true) {
}

task_thread::~task_thread() {
  quit();
  join();
}

bool task_thread::post_task(std::function<void()> &&task) {
  lock_guard lock(locker_);
  if (quit_) {
    return false;
  }
  tasks_.push(std::move(task));
  event_.set();
  return true;
}

void task_thread::quit() {
  lock_guard lock(locker_);
  quit_ = true;
  event_.set();
}

void task_thread::join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

void task_thread::run() {
  bool run = true;
  while (run) {
    event_.wait();
    std::queue<std::function<void()>> tasks;
    {
      lock_guard lock(locker_);
      run = !quit_;
      std::swap(tasks, tasks_);
    }
    while (!tasks.empty()) {
      tasks.front()();
      tasks.pop();
    }
  }
}

} // namespace xl