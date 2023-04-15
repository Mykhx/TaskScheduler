#ifndef TASKSCHEDULER_SCHEDULEDTASK_H
#define TASKSCHEDULER_SCHEDULEDTASK_H



#include <chrono>
#include <functional>
#include <utility>

using timeProvider = std::chrono::high_resolution_clock;
using timePoint = std::chrono::time_point<timeProvider>;
using duration = timeProvider::duration;
using task = std::function<void()>;

class ScheduledTask {
private:
    task executableTask;
    timePoint executionTime;

public:
    ScheduledTask();

    ScheduledTask(task executableAction, timePoint executionTime);

    void operator()();

    bool operator<(const ScheduledTask &other) const;

    [[nodiscard]] timePoint getExecutionTime() const;
};



#endif //TASKSCHEDULER_SCHEDULEDTASK_H
