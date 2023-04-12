#include <gtest/gtest.h>
#include "TaskScheduler/TaskScheduler.h"

TEST(TaskSchedulerTest, throwsIfTryStopWhileNotRunning) {
    TaskScheduler taskScheduler = TaskScheduler();
    EXPECT_FALSE(taskScheduler.schedulerIsRunning());
    EXPECT_THROW(taskScheduler.stopTaskLoop(), TaskSchedulerError);
}

TEST(TaskSchedulerTest, throwsIftryStartWhileRunning) {
    TaskScheduler taskScheduler = TaskScheduler();
    EXPECT_FALSE(taskScheduler.schedulerIsRunning());
    EXPECT_NO_THROW(taskScheduler.startTaskLoop());
    EXPECT_TRUE(taskScheduler.schedulerIsRunning());
}