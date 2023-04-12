#include "gtest/gtest.h"
#include "TaskScheduler/ScheduledTask.h"

TEST(ScheduledTask, defaultConstructorSetsExecutionTimeToNow) {
    auto beforeTaskCreation = timeProvider::now();
    auto scheduledTask = ScheduledTask();
    auto afterTaskCreation = timeProvider::now();

    auto executionTime = scheduledTask.getExecutionTime();

    EXPECT_GE(executionTime, beforeTaskCreation);
    EXPECT_LE(executionTime, afterTaskCreation);
}

TEST(ScheduledTask, canBeInitializedWithTimeAndTask) {
    timePoint timePoint1 = timeProvider::now();
    int value = 0;
    auto increaseValue = [&value](){value = 1;};
    auto scheduledTask = ScheduledTask(std::move(increaseValue), timePoint1);

    EXPECT_EQ(scheduledTask.getExecutionTime(), timePoint1);
    scheduledTask();
    EXPECT_EQ(value, 1);
}