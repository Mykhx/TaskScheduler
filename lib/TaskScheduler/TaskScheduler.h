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
#include <utility>

using taskSchedulerQueue = std::priority_queue<ScheduledTask>;

class TaskScheduler {
private:
    taskSchedulerQueue taskQueue;
    std::atomic_bool isRunning;

    std::jthread taskLoopThread;
    std::jthread taskExecutionThread;

    std::mutex queueMutex;
    std::condition_variable_any queueCondition;

    void shutdown() {
        isRunning = false;
        taskLoopThread.request_stop();
        queueCondition.notify_all();
    };

    void emplaceTask(ScheduledTask &&task) {
        std::scoped_lock<std::mutex> scopedLock(queueMutex);
        taskQueue.emplace(std::move(task));
    }

    void executeDelayedQueue(const std::stop_token& stopToken) {
        std::unique_lock<std::mutex> uniqueLock(queueMutex);

        while (!stopToken.stop_requested() and nextTaskNotReady()) {
            if (taskQueue.empty()) {
                queueCondition.wait(uniqueLock);
            } else {
                queueCondition.wait_until(uniqueLock, taskQueue.top().getExecutionTime());
            }
        }

        if (!(stopToken.stop_requested() or taskQueue.empty())) {
            auto task = taskQueue.top();
            taskQueue.pop();
            //task();
            // should consider using a pointer. What happens if thread is not yet finished with task and next task is set?
            taskExecutionThread = std::jthread(task);
            taskExecutionThread.detach();
        }
    }

    [[nodiscard]] bool nextTaskNotReady() const {
        return (taskQueue.empty() or timeProvider::now() <= taskQueue.top().getExecutionTime());
    };

public:
    TaskScheduler() : isRunning(false) {}

    ~TaskScheduler() {
        shutdown();
    }

    void startTaskLoop() {
        if (isRunning)
            throw TaskSchedulerError("Cannot start TaskScheduler. It is already running.");
        taskLoopThread = std::jthread([this](const std::stop_token& stopToken){
            while(!stopToken.stop_requested()) this->executeDelayedQueue(stopToken);
        });
        isRunning = true;
    }

    void stopTaskLoop() {
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
