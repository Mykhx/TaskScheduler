#include "ScheduledTask.h"

#include <utility>

ScheduledTask::ScheduledTask()
: sharedExecutableTaskPtr(std::make_shared<task>([]{})),
executionTime(timeProvider::now()) {}

ScheduledTask::ScheduledTask(task executableAction, timePoint executionTime)
: sharedExecutableTaskPtr(std::make_shared<task>(std::move(executableAction))),
        executionTime(executionTime) {}

void ScheduledTask::operator()() {
    std::invoke(*sharedExecutableTaskPtr);
}

bool ScheduledTask::operator<(const ScheduledTask &other) const {
    return other.executionTime < this->executionTime;
}

timePoint ScheduledTask::getExecutionTime() const {
    return executionTime;
}

ScheduledTask::ScheduledTask(sharedTaskPtr sharedTask, timePoint executionTime)
: sharedExecutableTaskPtr(std::move(sharedTask)),
executionTime(executionTime) {}

