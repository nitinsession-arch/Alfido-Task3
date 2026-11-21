#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

// Helper: Get file size in bytes (returns -1 if file doesn't exist)
long getFileSize(const string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// 1. File I/O & Log Rotator Utility
void writeLogWithRotation(const string& logFile, const string& backupFile, const string& message, long maxBytes) {
    long currentSize = getFileSize(logFile);

    // Check if log file exceeds maxBytes -> rotate it
    if (currentSize >= maxBytes) {
        cout << "[ROTATE] '" << logFile << "' reached " << currentSize 
             << " bytes (Limit: " << maxBytes << " bytes). Rotating log...\n";
        
        // Remove old backup if it exists, then rename active log to backup
        remove(backupFile.c_str());
        if (rename(logFile.c_str(), backupFile.c_str()) != 0) {
            perror("[ERROR] Failed to rotate log file");
            return;
        }
        cout << "[ROTATE] Archived old log to '" << backupFile << "'.\n";
    }

    // Open active log file in append mode with error handling
    ofstream outFile(logFile, ios::app);
    if (!outFile.is_open()) {
        cerr << "[ERROR] Unable to open '" << logFile << "' for writing (check permissions/path).\n";
        return;
    }

    outFile << message << "\n";
    outFile.close();
    cout << "[WRITE] Appended log entry (" << message.size() + 1 << " bytes).\n";
}

// 2. Process Creation & IPC Demo (fork + pipe)
void demonstrateProcessAndPipe() {
    cout << "\n--- PROCESS CREATION & PIPE (IPC) DEMO ---\n";
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("[ERROR] Pipe creation failed");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("[ERROR] Fork failed");
        return;
    }

    if (pid == 0) {
        // Child Process: Writes system status message into pipe
        close(pipefd[0]); // Close unused read end
        string childMsg = "Child Process (PID: " + to_string(getpid()) + ") completed health check successfully.";
        write(pipefd[1], childMsg.c_str(), childMsg.size() + 1);
        close(pipefd[1]);
        exit(0);
    } else {
        // Parent Process: Reads message from child and waits for exit
        close(pipefd[1]); // Close unused write end
        char buffer[256];
        ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            cout << "[PARENT PID: " << getpid() << "] Received via Pipe -> " << buffer << "\n";
        } else {
            cerr << "[ERROR] Parent failed to read data from pipe.\n";
        }
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            cout << "[PARENT] Child process exited cleanly with status code: " << WEXITSTATUS(status) << "\n";
        }
    }
}

int main() {
    const string activeLog = "app.log";
    const string backupLog = "app.log.1";
    const long MAX_LOG_SIZE = 100; // Small 100-byte threshold to demonstrate rotation

    // Clean start for demonstration
    remove(activeLog.c_str());
    remove(backupLog.c_str());

    cout << "--- FILE I/O & LOG ROTATOR DEMO ---\n";
    writeLogWithRotation(activeLog, backupLog, "[INFO] System service initialized.", MAX_LOG_SIZE);
    writeLogWithRotation(activeLog, backupLog, "[INFO] User authentication module loaded.", MAX_LOG_SIZE);
    writeLogWithRotation(activeLog, backupLog, "[WARN] High memory threshold detected on worker node.", MAX_LOG_SIZE);
    // This 4th write triggers automatic rotation because size exceeds 100 bytes
    writeLogWithRotation(activeLog, backupLog, "[INFO] Post-rotation fresh log entry recorded.", MAX_LOG_SIZE);

    // Verify file contents after rotation
    ifstream checkFile(activeLog);
    string line;
    cout << "\n[VERIFY] Current contents of active '" << activeLog << "':\n";
    while (getline(checkFile, line)) {
        cout << "  > " << line << "\n";
    }
    checkFile.close();

    // Edge case test: Trying to open a non-existent/invalid directory path
    cout << "\n--- EDGE CASE & ERROR HANDLING TEST ---\n";
    writeLogWithRotation("/invalid_dir/test.log", "backup.log", "Test message", MAX_LOG_SIZE);

    // Run Process & Pipe IPC Demo
    demonstrateProcessAndPipe();

    return 0;
}
