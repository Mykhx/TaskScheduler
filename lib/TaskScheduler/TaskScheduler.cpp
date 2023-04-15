
#include "TaskScheduler.h"

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
        // should consider using a pointer. What happens if thread is not yet finished with task and next task is set?
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
    taskLoopThread = std::jthread([this](const std::stop_token& stopToken){
        while(!stopToken.stop_requested()) this->executeDelayedQueue(stopToken);
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

void TaskScheduler::addTask(task &&executableAction, timePoint executionTime, duration period) {
    auto repeatedTask = [this, &executableAction, executionTime, period] () {
        executableAction();
        this->addTask(std::move(executableAction), executionTime + period, period);
    };
    addTask(std::move(repeatedTask), executionTime);
}
