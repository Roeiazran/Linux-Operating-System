#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_NAME_LEN 256
#define MAX_PATH_LEN 1024
/**
 * @brief Wrapper function for fork() with error handling
 * @return pid_t Process ID of the child (0 for child, >0 for parent)
 */
pid_t Fork(void)
{
    pid_t pid;

    // Call fork and check for failure
    if ((pid = fork()) < 0)
    {
        // Print error message if fork fails
        perror("fork failed");
    }

    // Return the PID
    return pid;
}

/**
 * @brief Wrapper function for execvp() with error handling
 * @param args Array of arguments, where args[0] is the command
 */
void Execvp(char * args[]) {

    // Execute the command with given arguments
    int st = execvp(args[0], args);

    // Check for failure
    if (st == -1) {
        // Print error and exit if execvp fails
        perror("execvp failed");
        exit(1);
    }
}

/**
 * @brief Wrapper function for getcwd() with error handling
 * @param buff Buffer to store the current working directory
 * @param len Length of the buffer
 * @return char* Pointer to the current working directory
 */
char * Getcwd(char * buff, int len) {
    
    // Get the current working directory
    char * path = getcwd(buff, len);

    // Check if getcwd failed
    if (!path) {
        perror("getcwd failed");
        exit(1);
    }

    // Return the path
    return path;
}

/**
 * @brief Wrapper for chdir() to change the current working directory
 * @param path Path to change the directory to
 * @return int Returns 0 on success, -1 on failure
 */
int Chdir(char path[]) {
    int st = chdir(path);
    return st;
}

/**
 * @brief Extracts the last directory or file name from a given path
 * @param path The full path string
 * @return char* Pointer to the start of the last component in the path
 */
char * extractDirName(char path[]) {
    
    int len = strlen(path);
    int lastIndex = 0;

    // Loop through the path to find the last '/'
    for (int i = 0; i < len; i++) {
        if (path[i] == '/') {
            lastIndex = i;
        }
    }
    
    // Return pointer to character after the last '/'
    return (path + lastIndex + 1);
}

/**
 * @brief Finds the index of the last slash '/' in a path
 * @param path The path string
 * @return int Index of the last '/' character
 */
int getLastSlashIndex(char path[]) {

    int len = strlen(path);
    int lastIndex = 0;

    // Loop to find the last '/'
    for (int i = 0; i < len; i++) {
        if (path[i] == '/') {
            lastIndex = i;
        }
    }

    return lastIndex;
}

/**
 * @brief Copies a file from source to destination using fork() and execvp()
 * @param srcPath Path to the source file
 * @param dstPath Path to the destination file
 */
void copyFile(char * srcPath, char * dstPath) {

    // Create a child process
    pid_t pid = Fork();

    if (!pid) {
        // In child process, execute 'cp' command
        char * args[] = {"cp", srcPath, dstPath, NULL};
        Execvp(args);
    } else {
        // In parent process, wait for child to finish
        waitpid(-1, NULL, 0);
        printf("Copied: %s -> %s\n", srcPath, dstPath);
    }
}

/**
 * @brief Creates a new directory with specified permissions
 * @param dirName Name of the directory to create
 * @param permissions Permissions string (e.g., "755")
 */
void Makedir(char dirName[], char * permissions) {

    // Fork a child process to execute mkdir
    pid_t pid = Fork();

    if (!pid) {
        // In child process, run mkdir with -m and -p options
        char * args[] = {"mkdir", "-m", permissions, "-p", dirName, NULL};
        Execvp(args);
    } else {
        // In parent process, wait for child to finish
        waitpid(-1, NULL, 0);
    }
}

/**
 * @brief Compares two strings lexicographically
 * @param str1 First string
 * @param str2 Second string
 * @return int Returns 0 if equal, 1 if str1 > str2, 2 if str1 < str2
 */
int cmp(char *str1, char *str2) {

    // Compare character by character
    while ((*str1 != 0) && (*str2 != 0)) {

        if (*str1 > *str2) {
            return 1;
        } else if (*str1 < *str2) {
            return 2;
        }
        str1++;
        str2++;
    }

    // If both strings end simultaneously, they are equal
    if (*str1 == *str2) {
        return 0;
    }

    // Otherwise, return based on which string is longer
    return ((*str1 - *str2) > 0) ? 1 : 2;
}

/**
 * @brief Sorts an array of strings lexicographically
 * @param arr Array of string pointers
 * @param len Length of the array
 */
void sort(char * arr[], int len) {

    int c;
    // Bubble sort algorithm
    for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
            int c = cmp(arr[i], arr[j]);
            if (c == 1) {
                // Swap if arr[i] > arr[j]
                char * t = arr[i];
                arr[i] = arr[j];
                arr[j] = t;
            }
        }
    }
}
/**
 * @brief Counts the number of regular files in a directory
 * @param path Path to the directory
 * @return int Number of regular files
 */
int getFilesCount(char * path) {

    DIR * dirp = opendir(path); // Open the directory
    struct dirent *dptr;        // Pointer to directory entry
    int len = 0;                     

    // Iterate through all entries in the directory
    while ((dptr = readdir(dirp))) {
        if (dptr->d_type == DT_REG) { // DT_REG == regular file
            len++;                    // Increment file count
        }
    }
    return len;
}

/**
 * @brief Retrieves and sorts names of regular files in a directory
 * @param path Path to the directory
 * @param filesCount Number of files (pre-counted)
 * @return char** Array of pointers to sorted file names
 */
char ** getSortedFilesNames(char * path, int filesCount) {

    DIR * dirp = opendir(path); // Open the directory
    struct dirent *dptr; // Directory entry pointer
    int index = 0;                    
    char ** names = (char **)malloc(sizeof(char*) * filesCount); // Allocate array for file names

    // Allocate space for each file name
    for (int i = 0; i < filesCount; i++) {
        names[i] = (char *)malloc(sizeof(char) * MAX_NAME_LEN);
    }

    // Read directory entries
    while ((dptr = readdir(dirp))) {
        if (dptr->d_type == 8) {           // If regular file
            names[index++] = dptr->d_name; // Store the name
        }
    }

    sort(names, filesCount); // Sort file names alphabetically
    return names;
}

/**
 * @brief Concatenates a file name to a directory path
 * @param path Directory path
 * @param name File name
 * @return char* Pointer to the newly allocated concatenated string
 */
char * concatFileNameToPath(char * path, char * name) {

    // Allocate memory for the resulting path + name
    char * res = (char *)calloc(sizeof(char) * (MAX_NAME_LEN + MAX_PATH_LEN), sizeof(char));

    strcat(res, path);   // Append directory path
    strcat(res, "/");    // Append slash
    strcat(res, name);   // Append file name
    return res;          // Return full path string
}

/**
 * @brief Checks if two files are different using the `diff` command
 * @param srcPath Path to source file
 * @param dstPath Path to destination file
 * @return int 0 if files are the same, 1 if different
 */
int isDiff(char * srcPath, char * dstPath) {

    int pid = Fork();     // Fork a child process
    int status;

    if (!pid) {           // Child process
        int null_fd = open("/dev/null", O_WRONLY);  // Redirect output to /dev/null

        if (null_fd == -1) {
            perror("failed to open");
            exit(1);
        }

        dup2(null_fd, STDOUT_FILENO);  // Redirect stdout
        if (close(null_fd) == -1) {    // Close unused file descriptor
            perror("failed to close");
            exit(1);
        }

        // Execute diff -q srcPath dstPath
        execl("/usr/bin/diff", "diff", "-q", srcPath, dstPath, NULL);

    } else {             // Parent process
        waitpid(-1, &status, 0);   // Wait for child to finish

        if (WEXITSTATUS(status) == 2) { // diff command error
            perror("failed to diff");
            exit(1);
        }
    }
    
    return WEXITSTATUS(status); // Return diff exit code (0 or 1)
}

/**
 * @brief Checks if source file is newer than destination file
 * @param srcPath Path to source file
 * @param dstPath Path to destination file
 * @return int 1 if srcPath is newer, 0 otherwise
 */
int isEarlier(char * srcPath, char * dstPath) {

    struct stat srcSt;
    struct stat dstSt;

    char time[50];  // Not used here but could store formatted time

    stat(srcPath, &srcSt);  // Get source file stats
    stat(dstPath, &dstSt);  // Get destination file stats

    return (srcSt.st_ctime > dstSt.st_ctime); // Compare creation times
}

/**
 * @brief Synchronizes files from source directory to destination directory
 * 
 * This function compares the contents of the source and destination directories,
 * copies new files, and updates files in the destination if the source has newer versions.
 * 
 * @param src Path to the source directory
 * @param dst Path to the destination directory
 */
void synchronize(char * src, char * dst) {

    // Get the number of files and their sorted names from the source
    int srcFilesCount = getFilesCount(src);
    char ** srcNames = getSortedFilesNames(src, srcFilesCount);

    // Get the number of files and their sorted names from the destination
    int destFilesCount = getFilesCount(dst);
    char ** destNames = getSortedFilesNames(dst, destFilesCount);
    
    char * srcFullPath;
    char * destFullPath;

    /* Handle empty destination directory: copy all source files */
    if (destFilesCount == 0) {
        for (int i = 0; i < srcFilesCount; i++) {
            srcFullPath = concatFileNameToPath(src, srcNames[i]);
            destFullPath = concatFileNameToPath(dst, srcNames[i]);

            printf("New file found: %s\n", srcNames[i]);
            copyFile(srcFullPath, destFullPath);  // Copy file to destination

            free(srcFullPath);
            free(destFullPath);
        }
        return;
    } else if (srcFilesCount == 0) {  // If source is empty, nothing to do
        return;
    }

    int res;
    for (int i = 0, destIndex = 0; i < srcFilesCount; i++) {

        // If we reached the end of destination files, no match found
        if (destIndex == destFilesCount) {
            res = 2;  // File not found in destination
        } else {
            res = cmp(destNames[destIndex], srcNames[i]);  // Compare file names
        }
        
        // Move through destination files until we find a match
        while ((res == 2) && (destIndex < destFilesCount)) { 
            res = cmp(destNames[destIndex++], srcNames[i]);
        }

        // Construct full paths for source and destination files
        srcFullPath = concatFileNameToPath(src, srcNames[i]);
        destFullPath = concatFileNameToPath(dst, srcNames[i]);

        if (res != 0) { // File not found in destination
            printf("New file found: %s\n", srcNames[i]);
            copyFile(srcFullPath, destFullPath);
        } else {
            // File exists, check for differences
            if (isDiff(srcFullPath, destFullPath)) {
                // Source file is newer than destination
                if(isEarlier(srcFullPath, destFullPath)) {
                    printf("File %s is newer in source. Updating...\n", srcNames[i]);
                    copyFile(srcFullPath, destFullPath);
                } else {  // Destination file is newer
                    printf("File %s is newer in destination. Skipping..\n", srcNames[i]);
                }
            } else { // Files are identical
                printf("File %s is identical. Skipping...\n", srcNames[i]);
            }
        }
        free(srcFullPath);
        free(destFullPath);
    }
}

/**
 @brief Gets and validates the source directory path
 @param argvSrc path from argv
 @param srcPath buffer to store the absolute source path
 */
void getSourcePath(char *argvSrc, char *srcPath) {

    // attempt to change to the source directory to check if it exists
    if (Chdir(argvSrc)) {
        // extract directory name for friendly error message
        char *dirName = extractDirName(argvSrc);
        printf("Error: Source directory '%s' does not exist.\n", dirName);
        exit(1); // exit if source directory is invalid
    }

    // get and store the absolute path of the source directory
    Getcwd(srcPath, MAX_PATH_LEN);
}

/**
 @brief Ensures destination directory exists; creates it if missing
 @param argvDst path from argv
 @param currPath current working directory before processing
 @param dstPath buffer to store the absolute destination path
 */
void getOrCreateDestinationPath(char *argvDst, char *currPath, char *dstPath) {

    // return to original directory before processing destination
    Chdir(currPath);

    // if destination does not exist, create it
    if (Chdir(argvDst)) {

        // extract directory name for informative message
        int lastIndex = getLastSlashIndex(argvDst);
        char dirName[MAX_PATH_LEN];
        if (lastIndex != 0) 
            strcpy(dirName, argvDst + lastIndex + 1);
        else 
            strcpy(dirName, argvDst);

        // create destination directory with 0700 permissions
        Makedir(argvDst, "0700");
        printf("Created destination directory '%s'.\n", dirName);

        // change to newly created destination
        Chdir(argvDst);
    }

    // get and store the absolute path of the destination
    Getcwd(dstPath, MAX_PATH_LEN);
}

/**
 * @brief Main function for file synchronization
 * @param argc Argument count
 * @param argv Argument vector: expects source and destination directories
 * @return Exit status
 */
int main(int argc, char *argv[]) {
    char srcPath[MAX_PATH_LEN];
    char dstPath[MAX_PATH_LEN];
    char curr[MAX_PATH_LEN];

    // Get current working directory
    Getcwd(curr, MAX_PATH_LEN);

    if (argc < 3) {
        printf("Usage: file_sync <source_directory> <destination_directory>\n");
        exit(1);
    }

    // Get and validate paths
    getSourcePath(argv[1], srcPath);
    getOrCreateDestinationPath(argv[2], curr, dstPath);

    // Synchronize files
    printf("Synchronizing from %s to %s\n", srcPath, dstPath);
    synchronize(srcPath, dstPath);
    printf("Synchronization complete.\n");

    return 0;
}
