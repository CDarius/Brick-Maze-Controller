#include "CancelToken.hpp"

bool CancelToken::isCancelled() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    return cancelled;
}

void CancelToken::cancel() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    cancelled = true;
}

void delayCancellable(unsigned long delayMs, CancelToken& token, unsigned long pollIntervalMs) {
    unsigned long startTime = millis();
    
    while (!token.isCancelled()) {
        unsigned long elapsedTime = millis() - startTime;
        if (elapsedTime >= delayMs) {
            break; // Delay duration completed
        }
        
        // Calculate remaining time and delay interval
        unsigned long remainingTime = delayMs - elapsedTime;
        unsigned long sleepDuration = (remainingTime < pollIntervalMs) ? remainingTime : pollIntervalMs;
        delay(sleepDuration);
    }
}