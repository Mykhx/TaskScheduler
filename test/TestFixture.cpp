#include <gtest/gtest.h>

class myTestFixture: public ::testing::Test {
protected:
    myTestFixture( ) {
        // initialization;
        // can also be done in SetUp()
    }

    void SetUp( ) {
        // initialization or some code to run before each test
    }

    void TearDown( ) {
        // code to run after each test;
        // can be used instead of a destructor,
        // but exceptions can be handled in this function only
    }

    ~myTestFixture( )  {
        //resources cleanup, no exceptions allowed
    }

    // shared user data

};

TEST_F(myTestFixture, TestName) {
    EXPECT_TRUE(false);
}