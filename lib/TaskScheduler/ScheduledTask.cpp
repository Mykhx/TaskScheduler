#include "ScheduledTask.h"

ScheduledTask::ScheduledTask() : executableTask([]() {}), executionTime(timeProvider::now()) {}

ScheduledTask::ScheduledTask(task executableAction, timePoint executionTime) : executableTask(std::move(executableAction)),
                                                                               executionTime(executionTime) {}

void ScheduledTask::operator()() {
    executableTask();
}

bool ScheduledTask::operator<(const ScheduledTask &other) const {
    return other.executionTime < this->executionTime;
}

timePoint ScheduledTask::getExecutionTime() const {
    return executionTime;
}
