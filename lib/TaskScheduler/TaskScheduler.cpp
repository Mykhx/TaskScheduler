
#include "TaskScheduler.h"

#include <utility>

void TaskScheduler::shutdown() {
    isRunning = false;
    taskLoopThread.request_stop();
    queueCondition.notify_all();
}

void TaskScheduler::emplaceTask(ScheduledTask &&task) {
    std::scoped_lock<std::mutex> scopedLock(queueMutex);
    taskQueue.emplace(std::move(task));
}

void TaskScheduler::executeDelayedQueue(const std::stop_token &stopToken) {
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
        taskExecutionThread = std::jthread(task);
        taskExecutionThread.detach();
    }
}

bool TaskScheduler::nextTaskNotReady() const {
    return (taskQueue.empty() or timeProvider::now() <= taskQueue.top().getExecutionTime());
}

void TaskScheduler::startTaskLoop() {
    if (isRunning)
        throw TaskSchedulerError("Cannot start TaskScheduler. It is already running.");
    taskLoopThread = std::jthread([this](const std::stop_token &stopToken) {
        while (!stopToken.stop_requested()) this->executeDelayedQueue(stopToken);
    });
    isRunning = true;
}

void TaskScheduler::stopTaskLoop() {
    if (!isRunning)
        throw TaskSchedulerError("TaskScheduler stopped while not running.");
    shutdown();
}

bool TaskScheduler::schedulerIsRunning() const {
    return isRunning;
}

unsigned long long TaskScheduler::taskQueueSize() const {
    return taskQueue.size();
}

void TaskScheduler::addTask(task &&executableAction, timePoint executionTime) {
    emplaceTask({std::forward<task>(executableAction), executionTime});
    queueCondition.notify_all();
}

void TaskScheduler::addTask(sharedTaskPtr sharedTask, timePoint executionTime) {
    emplaceTask({std::move(sharedTask), executionTime});
    queueCondition.notify_all();
}

void TaskScheduler::addTask(task &&executableAction, timePoint executionTime, duration period) {
    addTask(std::make_shared<task>(executableAction), executionTime, period);
}

// todo: check if closures might work here (increase execution time)
/*
 * [time, period] () mutable { time+=period; }
 * or something similar
 */
void TaskScheduler::addTask(const sharedTaskPtr &sharedTask, timePoint executionTime, duration period) {
    auto repeatedTask = [this, sharedTask, executionTime, period]() {
        std::invoke(*sharedTask);
        this->addTask(sharedTask, executionTime + period, period);
    };
    addTask(std::move(std::make_shared<task>(repeatedTask)), executionTime);
}

void TaskScheduler::clearTaskQueue() {
    std::scoped_lock<std::mutex> scopedLock(queueMutex);
    taskQueue = taskSchedulerQueue();
}
