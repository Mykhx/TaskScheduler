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

    void shutdown();;

    void emplaceTask(ScheduledTask &&task);

    void executeDelayedQueue(const std::stop_token &stopToken);

    [[nodiscard]] bool nextTaskNotReady() const;;

public:
    TaskScheduler() : isRunning(false) {}

    ~TaskScheduler() {
        shutdown();
    }

    void startTaskLoop();

    void stopTaskLoop();

    [[nodiscard]] bool schedulerIsRunning() const;

    [[nodiscard]] unsigned long long taskQueueSize() const;

    void clearTaskQueue();

    void addTask(task &&executableAction, timePoint executionTime);

    void addTask(task &&executableAction, timePoint executionTime, duration period);
};

#endif //TASKSCHEDULER_TASKSCHEDULER_H
