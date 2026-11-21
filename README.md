# Task 3: System Programming / File I/O & Processes (C++)

## Overview
A C++ system utility combining a **size-based Log Rotator** (demonstrating POSIX/C++ File I/O) and a **Parent-Child Process Communicator** (demonstrating `fork()`, `waitpid()`, and anonymous `pipe()` IPC).

## Build & Run Commands
Ensure you have `g++` installed on a POSIX/Linux environment (or WSL on Windows), then run:
```bash
g++ -std=c++17 -Wall task3_sysprog.cpp -o task3
./task3
```

## Features & Edge Cases Handled
* **Automatic Log Rotation:** Monitors `app.log` via `stat()`; once the file exceeds the byte threshold, it safely removes any stale backup (`app.log.1`), renames the active log, and initializes a fresh file.
* **Invalid Path / Permission Errors:** Gracefully catches `ofstream` open failures (e.g., writing to `/invalid_dir/test.log`) and reports errors via `stderr` without crashing.
* **Process & Pipe Failure Handling:** Validates return codes for `pipe()`, `fork()`, and `read()` using `perror()`, and prevents zombie processes using `waitpid()`.
