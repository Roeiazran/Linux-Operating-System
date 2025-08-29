# File Instruction Processor

## Overview
This C program processes a series of instructions from a request file to **read from or write to a data file**. It supports:

- **READ** operations: extract portions of the data file.
- **WRITE** operations: insert text at specific offsets, shifting file content as needed.
- **QUIT** command: terminates the program.
- Robust handling of **invalid instructions**.

Results of read operations are saved to a `read_results.txt` file.

---

## Features
- Validates instructions against file bounds.
- Handles overlapping writes by shifting existing content.
- Supports dynamic instruction text using memory allocation.
- Generates a result file for all READ operations.
- Graceful error handling with descriptive messages.