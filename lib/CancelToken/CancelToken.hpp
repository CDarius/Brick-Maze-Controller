#pragma once

#include <Arduino.h>
#include <mutex>

// Usage:
// EXECUTE_IF_CANCELLED(token, {
//     // code to execute if token.IsCancelled() is true
// });
#define IF_CANCELLED(token, block) \
    do { \
        if ((token).isCancelled()) { \
            block \
        } \
    } while(0);

    
class CancelToken {
private:
    bool cancelled = false;
    std::mutex instanceMutex;

public:
    bool isCancelled();
    void cancel();
};

void delayCancellable(unsigned long delayMs, CancelToken& token, unsigned long pollIntervalMs = 10);
