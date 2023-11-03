#include <iostream>
#include <chrono>
#include "logger.hpp"
using namespace logger;

class TPSMeasuring {
public:
    // TPSMeasuring(std::shared_ptr<spdlog::logger> logger)
    TPSMeasuring()
        // : logger_(logger)
        : transactionCount_(0)
        , lastLogTime_(std::chrono::steady_clock::now())
        , transactionId_(0) {}

    void StartTransaction() {
        transactionStartTime_ = std::chrono::steady_clock::now();
        transactionId_++;
    }

    void FinishTransaction() {
        // Calculate transaction duration
        auto transactionEndTime = std::chrono::steady_clock::now();
        auto transactionDuration = std::chrono::duration_cast<std::chrono::microseconds>(transactionEndTime - transactionStartTime_);

        // Log transaction details
        INFO_LOG("Transaction " << transactionId_ << ": Duration: "<< transactionDuration.count() << " microseconds");

        // Increment transaction count
        transactionCount_++;

        // Log TPS at the specified interval
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastLogTime_);

        if (elapsedTime.count() >= LOG_INTERVAL_SECONDS) {
            double tps = static_cast<double>(transactionCount_) / elapsedTime.count();
            INFO_LOG("Transactions per second: " <<  tps);

            // Reset counters
            transactionCount_ = 0;
            lastLogTime_ = currentTime;
        }
    }

private:
    // std::shared_ptr<spdlog::logger> logger_;
    int transactionCount_;
    std::chrono::time_point<std::chrono::steady_clock> lastLogTime_;
    std::chrono::time_point<std::chrono::steady_clock> transactionStartTime_;
    int transactionId_; // Unique ID for each transaction
    const int LOG_INTERVAL_SECONDS = 1; // Adjust as needed
};
