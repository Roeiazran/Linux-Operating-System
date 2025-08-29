# Directory Backup Utility

## Overview
This C program provides a **safe and efficient way to backup directories**. It replicates the directory structure of a source directory into a destination directory, using:

- **Hard links** for regular files (to avoid data duplication)
- **Symbolic links** for link files
- Automatic creation of directories with preserved permissions

It recursively traverses all subdirectories, ensuring the destination matches the source structure.

---

## Features
- Checks for the existence of source and destination directories.
- Creates destination directories with the same permissions as the source.
- Uses **hard links** to replicate files without copying actual content.
- Handles **symbolic links**, ensuring the target paths are correctly recreated.
- Recursively copies entire directory trees, including nested directories.
- Provides robust error handling with descriptive messages.