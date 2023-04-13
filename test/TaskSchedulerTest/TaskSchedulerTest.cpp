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

TEST(TaskSchedulerTest, newTasksCanBeAdded) {
    TaskScheduler taskScheduler = TaskScheduler();

    auto task1 = [] (){/* default */};
    auto task2 = [] (){/* default */};
    auto executionTime = timeProvider::now();

    auto initialSize = taskScheduler.taskQueueSize();

    taskScheduler.addTask(task1, executionTime);
    taskScheduler.addTask(std::move(task2), executionTime);

    auto sizeAfterAddingTasks = taskScheduler.taskQueueSize();

    EXPECT_EQ(initialSize, 0);
    EXPECT_EQ(sizeAfterAddingTasks, 2);
}

TEST(TaskSchedulerTest, taskLoopCanBeStoppedCleanelyAndRestarted) {
    TaskScheduler taskScheduler = TaskScheduler();

    auto initalState = taskScheduler.schedulerIsRunning();
    taskScheduler.startTaskLoop();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto stateAfter50ms = taskScheduler.schedulerIsRunning();

    taskScheduler.stopTaskLoop();
    auto stateAfterStopping = taskScheduler.schedulerIsRunning();

    EXPECT_FALSE(initalState);
    EXPECT_TRUE(stateAfter50ms);
    EXPECT_FALSE(stateAfterStopping);
}