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

// ToDo: redesign this test. Spurious behaviour.
TEST(TaskSchedulerTest, tasksAreExecutedAccordingToGivenExecutionTime) {
    TaskScheduler taskScheduler = TaskScheduler();

    std::atomic<int> value{0};
    auto task1 = [&value] (){value = 1;};
    auto task2 = [&value] (){value = 2;};
    auto task3 = [&value] (){value = 3;};
    auto task4 = [&value] (){value = 4;};
    auto executionTime1 = timeProvider::now();
    auto executionTime2 = timeProvider::now() + std::chrono::milliseconds(200);
    auto executionTime3 = timeProvider::now() + std::chrono::milliseconds(300);
    auto executionTime4 = timeProvider::now() + std::chrono::milliseconds(400);

    taskScheduler.addTask(task1, executionTime1);
    taskScheduler.addTask(std::move(task3), executionTime3);
    taskScheduler.addTask(std::move(task2), executionTime2);
    taskScheduler.addTask(std::move(task4), executionTime4);

    auto valueInitial = static_cast<int>(value);
    taskScheduler.startTaskLoop();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto valueAfterTask1 = static_cast<int>(value);

    std::this_thread::sleep_until(executionTime2 + std::chrono::milliseconds(20));
    auto valueAfterTask2 = static_cast<int>(value);

    std::this_thread::sleep_until(executionTime3 + std::chrono::milliseconds(20));
    auto valueAfterTask3 = static_cast<int>(value);

    taskScheduler.stopTaskLoop();
    auto valueAfterStoppedTaskScheduler = static_cast<int>(value);

    EXPECT_EQ(valueInitial, 0);
    EXPECT_EQ(valueAfterTask1, 1);
    EXPECT_EQ(valueAfterTask2, 2);
    EXPECT_EQ(valueAfterTask3, 3);
    EXPECT_EQ(valueAfterStoppedTaskScheduler, 3);
}

TEST(TaskSchedulerTest, taskAreExecutedCorrectlyWhenSubsequentTaskBecomesReadyBeforeItHasFinished) {
    TaskScheduler taskScheduler = TaskScheduler();

    int valueTargetSlowTask = 0;
    int valueTargetSubsequentTask = 0;
    auto slowTask = [&valueTargetSlowTask] (){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        valueTargetSlowTask = 1;
    };
    auto subsequentTask = [&valueTargetSubsequentTask] (){valueTargetSubsequentTask = 2;};
    auto executionTimeSlowTask = timeProvider::now();
    auto executionTimeSubsequentTask = timeProvider::now() + std::chrono::milliseconds(50);

    taskScheduler.addTask(std::move(slowTask), executionTimeSlowTask);
    taskScheduler.addTask(std::move(subsequentTask), executionTimeSubsequentTask);

    taskScheduler.startTaskLoop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    taskScheduler.stopTaskLoop();

    EXPECT_EQ(valueTargetSlowTask, 1);
    EXPECT_EQ(valueTargetSubsequentTask, 2);
}

TEST(TaskSchedulerTest, taskCanBeExecutedPeriodically) {
    TaskScheduler taskScheduler = TaskScheduler();

    std::atomic_int valueTargetTask{0};
    auto repeatedTask = [&valueTargetTask](){valueTargetTask++;};
    auto period = std::chrono::milliseconds(20);

    taskScheduler.addTask(std::move(repeatedTask), timeProvider::now() + period, period);

    taskScheduler.startTaskLoop();
    std::this_thread::sleep_for(std::chrono::milliseconds(102));
    taskScheduler.stopTaskLoop();

    EXPECT_EQ(valueTargetTask, 4);
}