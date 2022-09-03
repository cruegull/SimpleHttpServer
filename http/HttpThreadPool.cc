#include <iostream>

#include "./include/HttpThreadPool.h"

using namespace simplehttpserver;

HttpThreadPool::HttpThreadPool(int pool_size):
    m_mutex{},
    m_threads{},
    m_tasks{},
    m_cond{},
    m_run(false)
{
    m_run = true;

    int thread_num = 0;
    if (pool_size <= 0)
    {
        thread_num = std::max(std::thread::hardware_concurrency(), (unsigned int)1);
    }
    else
    {
        thread_num = pool_size;
    }
    m_threads.resize(thread_num);
    printf("HttpThreadPool create size=%d\n", static_cast<int>(m_threads.size()));

    for (size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i] = std::thread([this](){
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    while (m_tasks.empty())
                    {
                        m_cond.wait(lock);
                        if (!m_run) return;
                    }
                    task = m_tasks.front();
                    m_tasks.pop();
                }
                task();
            }
        });
    }
}

HttpThreadPool::~HttpThreadPool()
{
    m_run = false;
    m_cond.notify_all();
    for (auto& i : m_threads)
    {
        if (i.joinable())
            i.join();
    }
}

int HttpThreadPool::getSize()
{
    return m_threads.size();
}