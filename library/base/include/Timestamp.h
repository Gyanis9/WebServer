//
// Created by guo on 25-1-2.
//

#ifndef WEBSERVER_TIMESTAMP_H
#define WEBSERVER_TIMESTAMP_H

#include "copyable.h"
#include <boost/operators.hpp>

class Timestamp :
        public copyable,
        public boost::equality_comparable<Timestamp>,
        public boost::less_than_comparable<Timestamp> {
public:
    Timestamp() : m_microSecondsSinceEpoch(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch) : m_microSecondsSinceEpoch(microSecondsSinceEpoch) {}

    void swap(Timestamp &right) {
        std::swap(m_microSecondsSinceEpoch, right.m_microSecondsSinceEpoch);
    }

    std::string toString() const;

    std::string toFormattedString(bool showMicrosecond = false) const;

    bool valid() const { return m_microSecondsSinceEpoch > 0; }

    int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch; }

    time_t secondsSinceEpoch() const { return static_cast<time_t> (m_microSecondsSinceEpoch / kMicroSecondsPerSecond); }


public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    static Timestamp now();

    static Timestamp invalid() { return Timestamp(); }

    static Timestamp fromUnixTime(time_t time) { return fromUnixTime(time, 0); }

    static Timestamp fromUnixTime(time_t time, int microseconds) {
        return Timestamp(static_cast<int64_t>(time) * kMicroSecondsPerSecond + microseconds);
    }

private:
    int64_t m_microSecondsSinceEpoch;
};

inline bool operator<(const Timestamp &left, const Timestamp &right) {
    return left.microSecondsSinceEpoch() < right.microSecondsSinceEpoch();
}

inline bool operator==(const Timestamp &left, const Timestamp &right) {
    return left.microSecondsSinceEpoch() == right.microSecondsSinceEpoch();
}

inline double timeDifference(const Timestamp &high, const Timestamp &low) {
    int64_t difference = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double >(difference) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(const Timestamp &timestamp, double seconds) {
    int64_t delta = static_cast<int64_t >(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}


#endif //WEBSERVER_TIMESTAMP_H
