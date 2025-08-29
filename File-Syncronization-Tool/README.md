# File Synchronization Utility

## Overview
This project is a C program that synchronizes files between a source and a destination directory. It copies new files, updates changed files, and skips identical files. The program handles directories, creates missing destination directories, and ensures that only newer files overwrite older ones.

---

## Features
- Synchronizes files from a source directory to a destination directory.
- Detects new files and copies them automatically.
- Compares file contents and timestamps to update only modified files.
- Creates missing destination directories with specified permissions.
- Works recursively with directories and supports multiple file types.
- Displays informative messages for actions performed during synchronization.

---