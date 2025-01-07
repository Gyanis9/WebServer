//
// Created by guo on 25-1-2.
//

#include "../include/Timestamp.h"
#include <sstream>
#include <iomanip>
#include <chrono>

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp should be same size as int64_t");


std::string Timestamp::toString() const {

    int64_t seconds = m_microSecondsSinceEpoch / kMicroSecondsPerSecond;
    int64_t microseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;
    std::ostringstream oss;
    oss << seconds << '.' << std::setw(6) << std::setfill('0') << microseconds;
    return oss.str();
}

std::string Timestamp::toFormattedString(bool showMicrosecond) const {
    std::ostringstream oss;
    time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
    tm tm_time;

    if (!localtime_r(&seconds, &tm_time)) {
        return "";
    }

    oss << std::setw(4) << std::setfill('0') << (tm_time.tm_year + 1900)
        << std::setw(2) << std::setfill('0') << (tm_time.tm_mon + 1)
        << std::setw(2) << std::setfill('0') << tm_time.tm_mday << ' '
        << std::setw(2) << std::setfill('0') << tm_time.tm_hour << ':'
        << std::setw(2) << std::setfill('0') << tm_time.tm_min << ':'
        << std::setw(2) << std::setfill('0') << tm_time.tm_sec;

    if (showMicrosecond) {
        int microseconds = static_cast<int>(m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
        oss << '.' << std::setw(6) << std::setfill('0') << microseconds;
    }

    return oss.str();
}

Timestamp Timestamp::now() {
    auto now = std::chrono::system_clock::now();
    auto us = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    return Timestamp(us);
}


