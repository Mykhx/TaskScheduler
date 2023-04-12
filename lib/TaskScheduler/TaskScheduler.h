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

using taskSchedulerQueue = std::priority_queue<ScheduledTask>;

class TaskScheduler {
private:
    taskSchedulerQueue taskQueue;
    std::atomic<bool> isRunning = false;

    std::unique_ptr<std::thread> taskLoopThread;

    std::mutex queueMutex;
    std::condition_variable_any queueCondition;

    void shutdown() {
        isRunning = false;
        if (taskLoopThread->joinable()) taskLoopThread->join();
    };

    void emplaceTask(ScheduledTask &&task) {
        std::scoped_lock<std::mutex> scopedLock(queueMutex);
        taskQueue.emplace(std::move(task));
    }

    void delayedQueue() {
        std::unique_lock<std::mutex> uniqueLock(queueMutex);
        while (isRunning and nextTaskNotReady()) {
            if (taskQueue.empty()) {
                queueCondition.wait(uniqueLock);
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

public:
    TaskScheduler() : isRunning(false),
                      taskLoopThread(std::make_unique<std::thread>([this]() { this->startTaskLoop(); })) {};

    void startTaskLoop() {
        if (isRunning) throw TaskSchedulerError("Cannot start TaskScheduler. It is already running.");
        delayedQueue();
    }


    [[nodiscard]] bool nextTaskNotReady() const {
        return (taskQueue.empty() or taskQueue.top().getExecutionTime() < timeProvider::now());
    };

    void startTaskLoopDetached() {
        taskLoopThread->detach();
    }

    [[nodiscard]] bool schedulerIsRunning() const {
        return isRunning;
    }

    void addTask(task executableAction, timePoint executionTime) {
        emplaceTask({std::move(executableAction), executionTime});
        queueCondition.notify_all();
    }
};

#endif //TASKSCHEDULER_TASKSCHEDULER_H
