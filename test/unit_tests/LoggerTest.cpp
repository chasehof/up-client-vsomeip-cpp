#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Logger.hpp"
#include "spdlog/sinks/ostream_sink.h"

/**
 *  @brief Test fixture for Logger.
 */
class LoggerTests : public ::testing::Test {
protected:
    /**
     *  @brief ostream object for capturing spdlog outputs.
     */
    //std::ostringstream log_output;

    /**
     *  @brief Setup for Logger. Initializes required variables.
     */
    void SetUp() override {
        // auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(log_output);
        // auto logger = std::make_shared<spdlog::logger>("test logger", ostream_sink);

        // spdlog::set_default_logger(logger);
    }

    /**
     *  @brief Teardown for Logger.
     */
    void TearDown() override {
    }
};

/**
 *  @brief Test for the logger setup to ensure it starts.
 */
// TEST_F(LoggerTests, SetupLoggerTest) {
//     auto logger = spdlog::default_logger();
//     EXPECT_FALSE(log_output.str().find("===================== START =====================") != std::string::npos);
//     setupLogger(logger);
//     EXPECT_TRUE(log_output.str().find("===================== START =====================") != std::string::npos);

//     EXPECT_EQ("test logger", logger->name());
//     EXPECT_EQ(spdlog::level::trace, logger->level());
// }

/**
 *  @brief Test for the logger setup when no logger ptr is passed.
 */
TEST_F(LoggerTests, SetupLoggerEmptyTest) {
    EXPECT_NO_THROW(setupLogger());
    auto logger = spdlog::default_logger();

    EXPECT_EQ("streamer-log", logger->name());
    EXPECT_EQ(spdlog::level::trace, logger->level());
}

/**
 *  @brief Test to ensure the level gets set properly by handleLogLevelSignal.
 */
TEST_F(LoggerTests, HandleLogLevelSignalTest) {
    union sigval sval;
    spdlog::set_level(spdlog::level::info);
    ASSERT_EQ(spdlog::level::info, spdlog::default_logger()->level());

    sval.sival_int = spdlog::level::warn;
    sigqueue(getpid(), SIGUSR1, sval);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(spdlog::level::warn, spdlog::default_logger()->level());

    sval.sival_int = spdlog::level::err;
    sigqueue(getpid(), SIGUSR1, sval);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(spdlog::level::err, spdlog::default_logger()->level());

    sval.sival_int = spdlog::level::debug;
    sigqueue(getpid(), SIGUSR1, sval);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(spdlog::level::debug, spdlog::default_logger()->level());
}