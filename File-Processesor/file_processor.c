#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

enum instruction_type {
    READ, WRITE, QUIT, INVALID
};

typedef struct {
    char type;
    int start_offset;
    int end_offset;
    char * text;
} instruction;

typedef struct {
    FILE * file;
    int fd;
    int file_size;
} file;

file data_file, result_file, request_file;

/**
 * @brief Opens a file safely with error handling.
 *
 * This is a wrapper around fopen. If fopen fails, it prints the given error
 * message using perror and exits the program.
 *
 * @param name Path to the file
 * @param perm File opening mode (e.g., "r", "w", "a")
 * @param err Error message to display if opening fails
 * @return FILE* pointer to the opened file (never NULL, program exits on error)
 */
FILE * Fopen(char * name, char * o_mode, char * err) {
    FILE * file = fopen(name, o_mode);  
    if (file == NULL) {
        perror(err);
        exit(1);
    }
    return file;
}

/**
 * @brief Computes the size of a file in bytes from its file descriptor.
 *
 * This function uses lseek to move the file offset to the end to determine
 * the size in bytes, then resets the offset back to the beginning.
 *
 * @param fd File descriptor of the opened file.
 * @return File size in bytes, or -1 on error (check errno).
 */
int get_file_size(int fd) {
    // move pointer to the end
    int size = lseek(fd, 0, SEEK_END);

    if (size == -1){
        perror("lseek failed");
        return -1;
    }

    // move it back to the beginning
    int p = lseek(fd, 0, SEEK_SET);
    return size;
}

/**
 * @brief Reads a line from request_file.file and assigns it to ins->text.
 *
 * Dynamically reallocates ins->text if needed.
 *
 * @param ins Pointer to the instruction instance whose text will be assigned.
 */
void assign_text_string(instruction * ins) {
    char c;
    int len = strlen(ins->text); // Get the length of the previous instruction string
    int size = 0; // Counter for the instruction characters
    
    fread(&c, sizeof(char), 1, request_file.file); /* read the first character */
    while (c != '\n') {

        size += 1;
        if (size < len) { /* Allocate space only when needed */
            ins->text[size - 1] = c;
        } else {

            ins->text = (char *)realloc(ins->text, sizeof(char) * (size + 1));
            ins->text[size - 1] = c;
        }
        fread(&c, sizeof(char), 1, request_file.file); // read the next character
    }
    ins->text[size] = '\0'; // null terminate the string
}

/**
 * @brief Skip the rest of the current line in request_file.file.
 *
 * This is used to discard invalid instructions safely.
 */
void skip_invalid_string() {
    char c;
    while (fread(&c, sizeof(char), 1, request_file.file) == 1) {
        if (c == '\n') {
            break; // stop at end of line
        }
    }
}

/**
 * @brief Validate an instruction against file size and type.
 * @param ins The instruction instance.
 * @return 1 if valid, 0 otherwise.
 */
int is_instruction_valid(instruction ins) {
    // Negative offsets are never valid
    if (ins.start_offset < 0 || ins.end_offset < 0) {
        return 0;
    }

    // start must not come after end
    if (ins.start_offset > ins.end_offset) {
        return 0;
    }

    if (ins.type == READ) {
        // End must be strictly within file
        if (ins.end_offset >= data_file.file_size) {
            return 0;
        }
    } else {
        // For WRITE start must be within file
        if (ins.start_offset >= data_file.file_size) {
            return 0;
        }
    }

    return 1;
}


/**
 * @brief Get the next instruction from the request file
 * @param ins Instruction instance to fill with data 
 */
void fetch_next_instruction(instruction * ins) {
    
    // Get the type of the instruction
    char type;
    fscanf(request_file.file, "%c ", &type);

    switch (type) {
        case 'R':
            // Decode
            ins->type = READ;
            fscanf(request_file.file, "%d %d ", &ins->start_offset, &ins->end_offset);

            // Validate
            if(!is_instruction_valid(*ins)) {
                ins->type = INVALID;
            }
            break;
        case 'W':
            ins->type = WRITE;
            fscanf(request_file.file, "%d ", &ins->start_offset);

            if(!is_instruction_valid(*ins)) {
                skip_invalid_string();
                ins->type = INVALID;
                break;
            }
            // Preper the instuction for execution
            assign_text_string(ins);
            break;
        case 'Q':
            ins->type = QUIT;
            break;
        default:
            break;
    }
}

/**
 * @brief Executes the instruction
 * On read operation it uses lseek, on write it shifts the end of the file
 * @param ins Instruction instance to execute
 */
void execute_instruction(instruction ins) {
    int size;
    char * buf;

    if (ins.type == READ) {

        // compute size and allocate buffer
        size = ins.end_offset - ins.start_offset + 1;
        buf = (char *)malloc(sizeof(char) * size);
        
        if (!buf) return;

        if(lseek(data_file.fd, ins.start_offset, SEEK_SET) == -1){
            free(buf);
            return;
        }

        // Read from data file
        if (read(data_file.fd, buf, size) != size)
        {
            free(buf);
            return;
        }
       
        // Write to result file
        write(result_file.fd, buf, size);
        write(result_file.fd, "\n", 1);

        free(buf);
    } else {
        
        // Stores the number of bytes needs to be shifted
        size = data_file.file_size - ins.start_offset;

        buf = size > 0? (char *)malloc(sizeof(char) * size) : NULL;

        int len = strlen(ins.text);
        data_file.file_size += len; // update file size

        // Move to the writing position
        lseek(data_file.fd, ins.start_offset, SEEK_SET);

        // Copy the end of the file into buf
        read(data_file.fd, buf, size);

        // Return back to the writing position
        lseek(data_file.fd, ins.start_offset, SEEK_SET);

        // Write to the file
        write(data_file.fd, ins.text, len);

        // Append the remaining characters
        write(data_file.fd, buf, size);

        free(buf);
    }
}

/**
 * @brief Main function that runs the procedure
 */
void run() {

    instruction ins;
    ins.text = (char *)malloc(sizeof(char));
    ins.start_offset = 0;
    ins.end_offset = 0;

    // Fetch first instruction
    fetch_next_instruction(&ins);
    
    while (ins.type != QUIT) {
        
        // Skip invalid instructions
        if (ins.type == INVALID) {
            fetch_next_instruction(&ins);
            continue;
        }

        // Execute and fetch the next
        execute_instruction(ins);
        fetch_next_instruction(&ins);
    }
    free(ins.text);
}

/**
 * @brief Main
 * @param argv contains paths to data and requests txt files
 */
int main(int argc, char *argv[]) {

    char * datap = argv[1];
    char * reqp = argv[2];

    data_file.file = Fopen(datap, "r+", "data.txt");
    data_file.fd = fileno(data_file.file); /* get file descriptor */
    data_file.file_size = get_file_size(data_file.fd);

    request_file.file =  Fopen(reqp, "r+", "requests.txt");
    request_file.fd = fileno(request_file.file);
    request_file.file_size = get_file_size(request_file.fd);
  
    result_file.file = Fopen("read_results.txt", "w+", "failed to open file");
    result_file.fd = fileno(result_file.file);
    result_file.file_size = get_file_size(result_file.fd);

    run();

    /* close files */
    close(result_file.fd);
    close(request_file.fd);
    close(data_file.fd);

    return 0;
}
