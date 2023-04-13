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

    void scheduledTaskInvoke() {
        std::cout << "Started delayed queue" << std::endl;
        std::unique_lock<std::mutex> uniqueLock(queueMutex);
        std::cout << "Acquired mutex" << std::endl;

        while (isRunning and nextTaskNotReady()) {
            if (taskQueue.empty()) {
                std::cout << " no task. waiting " << std::endl;
                queueCondition.wait(uniqueLock);
                std::cout << " waking up " << std::endl;
            } else {
                std::cout << "waiting until task ready" << std::endl;
                queueCondition.wait_until(uniqueLock, taskQueue.top().getExecutionTime());
                std::cout << "waking up" << std::endl;
            }
        }
        std::cout << "exited while loop" << std::endl;

        if (isRunning and !taskQueue.empty()) {
            auto task = taskQueue.top();
            taskQueue.pop();
            task();
            std::cout << "executed task" << std::endl;
        }
    }

    void executeDelayedQueue() {
        while(isRunning) {
            scheduledTaskInvoke();
        }
    }

    [[nodiscard]] bool nextTaskNotReady() const {
        std::cout << " Waiting until " << std::chrono::duration_cast<std::chrono::milliseconds>(taskQueue.top().getExecutionTime().time_since_epoch()).count() << "\n";
        std::cout << " Now          " << std::chrono::duration_cast<std::chrono::milliseconds>(timeProvider::now().time_since_epoch()).count() << "\n";
        std::cout << " Remaining    " << std::chrono::duration_cast<std::chrono::milliseconds>(taskQueue.top().getExecutionTime().time_since_epoch() - timeProvider ::now().time_since_epoch()).count() << "\n";
        std::cout << " Condition    " << std::boolalpha << (timeProvider::now() <= taskQueue.top().getExecutionTime())<< std::endl;
        std::cout << " Queue size " << taskQueue.size() << std::endl;
        return (taskQueue.empty() or timeProvider::now() <= taskQueue.top().getExecutionTime());
    };

public:
    TaskScheduler() : isRunning(false),
    taskLoopThread(std::make_unique<std::thread>([this]() {std::cout << "thread started" << std::endl;
        this->executeDelayedQueue(); })) {};

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
