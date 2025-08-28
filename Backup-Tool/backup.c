#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#define MAX_PATH_LENGTH 1024

/**
 @brief Forks the current process.
 Creates a new child process using fork(). If fork fails, prints an error message.
 @return pid_t Process ID of the child in the parent process, 0 in the child process, or -1 on failure.
 */
pid_t Fork(void)
{
    pid_t pid;

    // Attempt to fork the process
    if ((pid = fork()) < 0)
    {
        // Print error if fork fails
        perror("fork failed");
    }

    return pid;
}
/**
 @brief Executes a command using execvp.

 Replaces the current process image with a new process defined by args.
 If execvp fails, prints an error and terminates the process.

 @param args Null-terminated array of argument strings forming the command.
 */
void Execvp(char * args[]) {

    int st = execvp(args[0], args);

    if (st == -1) {
        perror("execvp failed");
        exit(1);
    }
}

/**
 @brief Creates a directory with specified permissions.

 Forks a new process to execute the "mkdir" command with the given path and permissions.
 Waits for the child process to complete before returning.

 @param path Full path of the directory to create.
 @param perm Permissions string (e.g., "755").
 */
void Makedir(char * path, char * perm) {

    pid_t pid = Fork();

    if (!pid) {
        char * args[] = {"mkdir", "-m", perm, "-p", path, NULL};
        Execvp(args);
    } else {
        // Parent waits for child to complete
        waitpid(-1, NULL, 0);
    }
}

/**
 @brief Wrapper for lstat system call.

 Performs lstat on the given path and stores the result in the provided struct stat pointer.
 If lstat fails, prints an error message and terminates the program.

 @param path Path of the file or directory to stat.
 @param s Pointer to a struct stat to store the results.
 */
void Stat(char * path, struct stat * s) {

    if (lstat(path, s) == -1) {
        perror("stat");
        exit(1);
    }
}

/**
 @brief Checks if a file exists in the filesystem.
 @param filepath The path to the file.
 @return 1 if the file exists, 0 otherwise.
 */
int file_exists(char * filepath) {
    return !access(filepath, F_OK);
}

/**
 @brief Checks if a given path points to a directory.
 @param path The path to check.
 @return 1 if the path is a directory, 0 otherwise.
 */
int is_directory(char * path) {
    struct stat s;
    if (stat(path, &s) != 0) {
        return 0;
    }
    return S_ISDIR(s.st_mode);
}

/**
 @brief Concatenates two strings with a '/' separator.
 @param s1 The first string.
 @param s2 The second string.
 @return A new string in the format "s1/s2". Caller must free the returned string.
 */
char * concat_str(char * s1, char * s2) {
    int len = strlen(s1) + strlen(s2) + 2;
    char * res = (char *) malloc(sizeof(char) * len);
    strcpy(res, s1);
    strcat(res, "/");
    strcat(res, s2);
    return res;
}
/**
 @brief Converts a directory's mode from a stat object into a string representing its permissions.
 @param perm Pointer to a char array that will store the permission string (e.g., "755").
 @param mode The mode field from a stat object containing the directory's permissions.
 */
void get_dir_permissions(char * perm, mode_t mode) {

    int pm = 0;
    pm |= ((mode & S_IRUSR) != 0) << 2;
    pm |= ((mode & S_IWUSR) != 0) << 1;
    pm |= ((mode & S_IXUSR) != 0);

    perm[0] = pm + '0';
    pm = 0;

    pm |= ((mode & S_IRGRP) != 0) << 2;
    pm |= ((mode & S_IWGRP) != 0) << 1;
    pm |= ((mode & S_IXGRP) != 0);
    
    perm[1] = pm + '0';
    pm = 0;

    pm |= ((mode & S_IROTH) != 0) << 2;
    pm |= ((mode & S_IWOTH) != 0) << 1;
    pm |= ((mode & S_IXOTH) != 0);
    
    perm[2] = pm + '0';
    perm[3] = '\0';
}

/**
 @brief Creates the directory at the destination path with the same permissions as the source directory.
 @param srcp The source directory path.
 @param dstp The destination directory path to create.
 */
void create_dir(char * srcp, char * dstp) {

    struct stat s;
    Stat(srcp, &s);

    /* get the directory permissions and create it */
    char perm[4];
    get_dir_permissions(perm, s.st_mode);
    Makedir(dstp, perm);
}

/**
 @brief Creates a hard link from the source file to the destination path.
 @param srcp Path to the source file.
 @param dstp Path to the destination file.
 */
void create_hard_link(char * srcp, char * dstp) {
    link(srcp, dstp);
}
/**
 @brief Creates hard links and directories from a source path into a destination path.
 Recursively traverses directories and subdirectories, creating directories and hard links for files.
 @param src Source path to copy the directory structure from.
 @param dst Destination path to create the directories and links in.
 */
void create_hard_links_and_directories(char * src, char * dst) {

    DIR * dirp = opendir(src);
    struct dirent *dptr;
    struct stat s;
    char * srcChar, * dstChar;

    /* skip . and .. directories */
    dptr = readdir(dirp);
    dptr = readdir(dirp);

    while ((dptr = readdir(dirp))) { /* loop through all entries in the current directory */

        /* concatenate the current entry name to the paths to check its type */
        srcChar = concat_str(src, dptr->d_name);
        dstChar = concat_str(dst, dptr->d_name);
        Stat(srcChar, &s);

        if (S_ISREG(s.st_mode)) {  /* regular file */
            create_hard_link(srcChar, dstChar);  
        } else if (S_ISDIR(s.st_mode)) { /* directory */
            /* create the directory and recurse into it */
            create_dir(srcChar, dstChar);
            create_hard_links_and_directories(srcChar, dstChar);
        }
        free(dstChar);
        free(srcChar);
    }
    closedir(dirp);
}

/**
 @brief Finds the index of the last slash '/' in a given path.
 @param str Path string.
 @param len Length of the string str.
 @return Index of the last occurrence of '/' in the path.
 */
int get_last_slash_index(char * str, int len) {

    int i;
    
    for(i = len - 1; i >= 0; i--) {
        if (str[i] == '/') {
            return i;
        }
    }
    return i + 1;
}

/**
 @brief Retrieves the absolute path to the file pointed by a symbolic link.
 Works similarly to realpath.
 @param symPath Path to the symbolic link.
 @param relPath Relative path that the symbolic link points to.
 @return String containing the absolute path of the target file.
 */
char * get_pointed_file_path(char * symPath, char * relPath) {

    char * buff = (char *)malloc(sizeof(char) * MAX_PATH_LENGTH);
    char curr_path[MAX_PATH_LENGTH];

    /* store the current working directory */
    getcwd(curr_path, MAX_PATH_LENGTH);

    /* navigate to the directory of the symbolic link and follow it */
    chdir(symPath);
    chdir(relPath);

    /* store the absolute path in buff and return to the previous directory */
    getcwd(buff, MAX_PATH_LENGTH);
    chdir(curr_path);
    return buff;
}

/**
 @brief Creates a symbolic (soft) link at the destination path pointing to a target relative path.
 @param relPath Relative path from the destination directory to the target file.
 @param destDir Destination directory where the symbolic link will be created.
 @param symName Name of the symbolic link to be created.
 */
void create_soft_link(char * relPath, char * destDir, char * symName) {

    char cur[MAX_PATH_LENGTH];
    getcwd(cur, MAX_PATH_LENGTH);

    /* navigate to the destination directory and create the symbolic link */
    chdir(destDir);
    symlink(relPath, symName);

    /* return to the original working directory */
    chdir(cur);
}


/**
 * @brief 
 */
/**
 @brief Recursively creates symbolic links for a file and its redirection.
 The function resolves the target of a symbolic link and ensures that 
 the corresponding path exists in the destination. Then it creates a soft link.
 @param src_file_path Source symbolic file path.
 @param dst_file_path Destination path where the symbolic link should be created.
 */
void create_links(char * src_file_path, char * dst_file_path) {
    char buf[MAX_PATH_LENGTH];
    int len = readlink(src_file_path, buf, MAX_PATH_LENGTH);
    
    /* remove the file name from the redirection path */
    int bls = get_last_slash_index(buf, len);
    char prevbuf = buf[bls];
    buf[len] = '\0'; /* readlink does not null-terminate the string */
    buf[bls] = '\0';

    /* remove the file name from the source and destination paths */
    int slc = get_last_slash_index(src_file_path, strlen(src_file_path));
    src_file_path[slc] = '\0';
    int dls = get_last_slash_index(dst_file_path, strlen(dst_file_path));
    dst_file_path[dls] = '\0';

    /* get the absolute paths to the pointed file for both src and dst */
    char * src_file_next = get_pointed_file_path(src_file_path, buf);
    char * dst_file_next = get_pointed_file_path(dst_file_path, buf);

    /* restore the file name in buf and append it to the resolved paths */
    buf[bls] = prevbuf;
    if (bls == 0) { /* no slash in redirection path */
        strcat(src_file_next, "/");
        strcat(src_file_next, buf);
        strcat(dst_file_next, "/");
        strcat(dst_file_next, buf);
    } else {
        strcat(src_file_next, buf + bls);
        strcat(dst_file_next, buf + bls);
    }

    /* if the file does not exist at the destination, recursively create links */
    if (!file_exists(dst_file_next)) {
        create_links(src_file_next, dst_file_next);
    } 

    /* create the symbolic link in the destination directory */
    create_soft_link(buf, dst_file_path, dst_file_path + dls + 1);

    free(src_file_next);
    free(dst_file_next);
}

/**
 @brief Creates the symbolic links present in a source directory at the destination directory.
 The function recursively traverses directories and subdirectories, creating symbolic links 
 for each link file in the source directory at the corresponding location in the destination.
 @param src Source directory path
 @param dst Destination directory path
 */
void create_soft_links(char *src, char * dst) {

    DIR * dirp = opendir(src);
    struct dirent *dptr;
    struct stat s;
    char * srcChar, * dstChar;

    /* skip . and .. directories */
    dptr = readdir(dirp);
    dptr = readdir(dirp);

    while ((dptr = readdir(dirp))) {

        /* build full paths for current source and destination entries */
        srcChar = concat_str(src, dptr->d_name);
        dstChar = concat_str(dst, dptr->d_name);
        Stat(srcChar, &s);

        if (S_ISLNK(s.st_mode)) { /* current file is a symbolic link */

            if (file_exists(dstChar)) { /* skip if link already exists at destination */
                continue;
            }
            create_links(srcChar, dstChar);
        } else if (S_ISDIR(s.st_mode)) { /* recurse into subdirectories */
            create_soft_links(srcChar, dstChar);
        }
        free(dstChar);
        free(srcChar);
    }
    closedir(dirp);
}


int debug() {
 
    if (!is_directory("../text")) { /* is srcdir not exists */

        perror("src dir");
        exit(1);
    }
    
    if (is_directory("./t")) { /* is dstdir exists */

        perror("backup dir");
        exit(1);
    } else { /* create the dst directory */

        create_dir("../text", "./t");
    }

    create_hard_links_and_directories("../text", "./t");
    create_soft_links("../text", "./t");
    return 0;
}

/**
 @brief Main function for creating a backup of a source directory.
 The program creates the destination directory if it does not exist,
 copies the directory structure and files using hard links, and
 recreates symbolic links.
 @param argc Number of command-line arguments
 @param argv Array of command-line argument strings
 @return 0 on successful completion
 */
int main(int argc, char *argv[])
{
    /* check if the source directory exists */
    if (!is_directory(argv[1])) {
        perror("src dir");
        exit(1);
    }
    
    /* check if the destination directory already exists */
    if (is_directory(argv[2])) {
        perror("backup dir");
        exit(1);
    } else { 
        /* create the destination directory */
        create_dir(argv[1], argv[2]);
    }

    /* create hard links for files and replicate the directory structure */
    create_hard_links_and_directories(argv[1], argv[2]);

    /* create symbolic links in the destination corresponding to the source */
    create_soft_links(argv[1], argv[2]);

    return 0;
}
