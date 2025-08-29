This program processes read and write requests from a file (requests.txt) and modifies a data file (data.txt) accordingly. The implementation ensures that writes insert data without overwriting existing content, and reads retrieve the correct portion of the file.
Features:
1. Reads a specified range from data.txt and saves the results to read_results.txt.
2. Writes new data at a given offset while preserving the rest of the file.
3. Processes requests from an input file instead of interactive user input.
4. Handles edge cases, such as invalid offsets, while maintaining file integrity.
5. Ensures data persistence using proper file operations (lseek, read, write).
