#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

#define NUM_GLADIATORS 4
/**
 * @brief Wrapper for fork system call that handles errors.
 * @return pid_t Process ID of the child for parent, 0 for child
 */
pid_t Fork(void) {
    pid_t pid;

    // Attempt to create a child process
    if ((pid = fork()) < 0) {
        perror("Fork failed"); // Print error if fork fails
        exit(1);
    }
    return pid;
}

/**
 * @brief Wrapper for execvp system call that handles errors.
 * @param args Null-terminated array of command arguments
 */
void Execvp(char * args[]) {
    execvp(args[0], args);   // Execute the command
    perror("failed to exec"); // Print error if execution fails
    exit(1);
}

/**
 * @brief Get the index of a given PID in the gladiators array.
 * @param pid Process ID to search for
 * @param gladiators Array of gladiator PIDs
 * @return int Index of the PID or -1 if not found
 */
int get_index_from_pid(pid_t pid, pid_t gladiators[]) {
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (gladiators[i] == pid)
            return i;
    }
    return -1; // PID not found
}

/**
 * @brief Get the index of the winning gladiator from an array.
 * @param winners Array marking winners (1 if won, 0 otherwise)
 * @return int Index of the winner or -1 if no winner
 */
int get_winner_index(int winners[]) {
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (winners[i]) {
            return i;
        }
    }
    return -1; // No winner found
}

/**
 * @brief Main function to simulate a gladiator tournament.
 *        Spawns multiple gladiator processes, waits for them to finish,
 *        and declares the winner based on who survives last.
 */
int main(int argc, char const *argv[])
{
    // Gladiator names and corresponding file identifiers
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};

    pid_t gladiators[NUM_GLADIATORS]; // Store PIDs of gladiator processes

    int finish_count = 0, retpid, status;
    int winners[] = {1, 1, 1 ,1}; // Tracks surviving gladiators (1 = alive, 0 = defeated)

    // Spawn a child process for each gladiator
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if ((gladiators[i] = Fork()) == 0) { /* child process */
            char *args[] = {"./gladiator" , gladiator_names[i], gladiator_files[i], NULL};
            execvp(args[0], args); // Execute gladiator program
        }
    }

    // Wait for gladiator processes to finish
    while ((retpid = waitpid(-1, &status, 0)) > 0) {
        finish_count++; // Increment finished count

        if (finish_count == NUM_GLADIATORS) {
            // All processes finished: declare the winner
            int winnerIndex = get_winner_index(winners);
            printf("The gods have spoken, the winner of the tournament is %s!", gladiator_names[winnerIndex]);
        } else {
            // Mark the losing gladiator as defeated
            int index = get_index_from_pid(retpid, gladiators);
            winners[index] = 0;
        }
    }
    return 0;
}