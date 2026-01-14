#ifndef QUEUE_H
#define QUEUE_H

// ----------------

#include <queue>
#include <iostream>

class QueueAverage
{
private:
    std::queue<double> dataPoints;
    int queueSize;
    double sum;
    long long int totalCount;

public:
    QueueAverage() : queueSize(10), sum(0.0f), totalCount(0) {};

    void setMaxQueueSize(int num) { queueSize = num; };
    // The default size of the queue is 10 entries, but can be adjusted using the above method.

    void addQueue(double value)
    {
        dataPoints.push(value);
        totalCount++;

        if (dataPoints.size() > queueSize)
        {
            dataPoints.pop();
        };

        sum += ((value - sum) / totalCount); // incremental mean formula
    };

    double displayRunningAverage() { return sum; };

    long long int displayTotalCount() { return totalCount; };
};

// ---------------

#endif
