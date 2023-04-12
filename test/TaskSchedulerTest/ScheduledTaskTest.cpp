#include "gtest/gtest.h"
#include "TaskScheduler/ScheduledTask.h"

TEST(ScheduledTask, defaultConstructorSetsExecutionTimeToNow) {

    auto beforeTaskCreation = timeProvider::now();
    auto task = ScheduledTask();
    auto afterTaskCreation = timeProvider::now();

    auto executionTime = task.getExecutionTime();

    EXPECT_GE(executionTime, beforeTaskCreation);
    EXPECT_LE(executionTime, afterTaskCreation);
}