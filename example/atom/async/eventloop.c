#include "atom/async/eventloop.h"

#include <stdio.h>

// Example callback function for file descriptors
void onFdReady(int fd, void *userData) {
    printf("File descriptor %d is ready. User Data: %s\n", fd, (char *)userData);
}

// Example work procedure
void workProc(void *userData) {
    printf("Executing work procedure. User Data: %s\n", (char *)userData);
}

// Example timer callback function
void onTimer(void *userData) {
    printf("Timer fired. User Data: %s\n", (char *)userData);
}

int main() {
    // Starting the event loop
    printf("Starting Event Loop\n");

    // Adding file descriptor callback example
    int fd_example = /* Assume you have a valid file descriptor */;
    int callbackId = addCallback(fd_example, onFdReady, "File Descriptor User Data");

    // Adding a work procedure
    int workProcId = addWorkProc(workProc, "Work Procedure User Data");

    // Adding a one-shot timer
    int timerId = addTimer(1000 /* ms */, onTimer, "One-Shot Timer");

    // Adding a periodic timer
    int periodicTimerId = addPeriodicTimer(2000 /* ms */, onTimer, "Periodic Timer");

    // Run the event loop
    eventLoop();

    // Cleanup
    rmCallback(callbackId);
    rmWorkProc(workProcId);
    rmTimer(timerId);
    rmTimer(periodicTimerId);

    printf("Ending Event Loop\n");
    return 0;
}
