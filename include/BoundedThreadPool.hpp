#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class BoundedThreadPool
{
private:
    unsigned int limit = 8;

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::atomic<bool> b_running;
    std::mutex mtx_queue;
    std::condition_variable cnd_buffer_full;
    std::condition_variable cnd_buffer_empty;

    void create_threads()
    {
        for (int i = 0; i < n_threads; i++)
        {
            threads.push_back(std::thread(&BoundedThreadPool::worker, this));
        }
    }

    void wait_for_space()
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_full.wait(lock, [this]
                             { return (tasks.size() < limit); });
    }

    std::function<void()> wait_for_task()
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_empty.wait(lock, [this]
                              { return !tasks.empty(); });
        std::function<void()> task = std::move(tasks.front());
        tasks.pop();
        cnd_buffer_full.notify_one();
        return task;
    }

    void worker()
    {
        while (b_running)
        {
            try
            {
                std::function<void()> task = wait_for_task();
                task();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

public:
    int n_threads;

    template <typename T>
    void push_task(const T &task)
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_full.wait(lock, [this]
                             { return (tasks.size() < limit); });
        tasks.push(std::function<void()>(task));
        cnd_buffer_empty.notify_one();
    }

    template <typename T, typename... A>
    void push_task(const T &task, const A &...args)
    {
        push_task([task, args...]
                  { task(args...); });
    }

    void wait_for_completion()
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_full.wait(lock, [this]
                              { return tasks.empty(); });
    }

    void join_threads()
    {
        for (int i = 0; i < n_threads; i++)
        {
            threads[i].join();
        }
    }

    BoundedThreadPool() : b_running(true)
    {
        n_threads = std::thread::hardware_concurrency();
        create_threads();
    }
    BoundedThreadPool(int n_threads) : b_running(true)
    {
        this->n_threads = n_threads;
        create_threads();
    }

    ~BoundedThreadPool()
    {
        b_running = false;
        join_threads();
    }
};
