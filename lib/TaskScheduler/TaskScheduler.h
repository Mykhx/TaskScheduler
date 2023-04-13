//
// Created by mkehr on 12.04.2023.
//

#ifndef TASKSCHEDULER_TASKSCHEDULER_H
#define TASKSCHEDULER_TASKSCHEDULER_H

#include <queue>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ScheduledTask.h"
#include "TaskSchedulerError.h"
#include <iostream>

using taskSchedulerQueue = std::priority_queue<ScheduledTask>;

class TaskScheduler {
private:
    taskSchedulerQueue taskQueue;
    std::atomic<bool> isRunning;

    std::unique_ptr<std::thread> taskLoopThread; // ToDo: try jthread

    std::mutex queueMutex;
    std::condition_variable_any queueCondition;

    void shutdown() {
        isRunning = false;
        //taskLoopThread->request_stop();
        if (taskLoopThread->joinable()) {
            queueCondition.notify_all();
            taskLoopThread->join();
        }
    };

    void emplaceTask(ScheduledTask &&task) {
        std::scoped_lock<std::mutex> scopedLock(queueMutex);
        taskQueue.emplace(std::move(task));
    }

    void delayedQueue() {
        std::unique_lock<std::mutex> uniqueLock(queueMutex);
        while (isRunning and nextTaskNotReady()) {
            if (taskQueue.empty()) {
                std::cout << " no task. waiting " << std::endl;
                queueCondition.wait(uniqueLock);
                std::cout << " waking up " << std::endl;
            } else {
                queueCondition.wait_until(uniqueLock, taskQueue.top().getExecutionTime());
            }
        }

        if (isRunning and !taskQueue.empty()) {
            auto task = taskQueue.top();
            taskQueue.pop();
            task();
        }
    }

    [[nodiscard]] bool nextTaskNotReady() const {
        return (taskQueue.empty() or taskQueue.top().getExecutionTime() < timeProvider::now());
    };

public:
    TaskScheduler() : isRunning(false),
    taskLoopThread(std::make_unique<std::thread>([this]() { this->delayedQueue(); })) {};

    ~TaskScheduler() {
        std::cout << "call shutdown (dest)" << std::endl;
        shutdown();
        std::cout << "called shutdown (dest)" << std::endl;
    }

    void startTaskLoop() {
        if (isRunning)
            throw TaskSchedulerError("Cannot start TaskScheduler. It is already running.");
        isRunning = true;
        taskLoopThread->detach();
    }

    void stopTaskLoop() {
        std::cout << "is Running " << isRunning << std::endl;
        if (!isRunning)
            throw TaskSchedulerError("TaskScheduler stopped while not running.");
        shutdown();
    }

    [[nodiscard]] bool schedulerIsRunning() const {
        return isRunning;
    }

    [[nodiscard]] unsigned long long taskQueueSize() const {
        return taskQueue.size();
    }

    void addTask(task&& executableAction, timePoint executionTime) {
        emplaceTask({std::forward<task>(executableAction), executionTime});
        queueCondition.notify_all();
    }
};

#endif //TASKSCHEDULER_TASKSCHEDULER_H
