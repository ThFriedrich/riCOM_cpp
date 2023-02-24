/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu,
 * University of Antwerp - All Rights Reserved.
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Authors:
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#ifndef BOUNDED_THREAD_POOL_H
#define BOUNDED_THREAD_POOL_H

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

class BoundedThreadPool
{
private:
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

    void wait_for_task()
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_empty.wait(lock, [this]
                              { return !tasks.empty() || !b_running; });
        if (!tasks.empty())
        {
            std::function<void()> task = std::move(tasks.front());
            tasks.pop();
            cnd_buffer_full.notify_one();
            task();
        }
    }

    void worker()
    {
        while (b_running)
        {
            try
            {
                wait_for_task();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }

public:
    int n_threads;
    int limit;

    template <typename T>
    void push_task(const T &task)
    {
        std::unique_lock<std::mutex> lock(mtx_queue);
        cnd_buffer_full.wait(lock, [this]
                             { return ((int)tasks.size() < limit); });
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

    void init(int n_threads, int limit)
    {
        int n_threads_max = std::thread::hardware_concurrency();
        if (n_threads > n_threads_max || n_threads < 1)
        {
            this->n_threads = n_threads_max;
        }
        else
        {
            this->n_threads = n_threads;
        }
        this->limit = limit;
        b_running = true;
        create_threads();
    }

    explicit BoundedThreadPool(int n_threads) : limit(8)
    {
        init(n_threads, limit);
    }

    explicit BoundedThreadPool(int n_threads, int limit)
    {
        init(n_threads, limit);
    }

    BoundedThreadPool() :  b_running(false), n_threads(0), limit(0) {}

    ~BoundedThreadPool()
    {
        wait_for_completion();
        b_running = false;
        cnd_buffer_empty.notify_all();
        join_threads();
    }
};

#endif