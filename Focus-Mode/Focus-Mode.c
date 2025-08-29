#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/**
 * @brief Prints a string to the standard output.
 * @param str The string to print
 */
void print(const char *str) {
    // write the string to STDOUT
    write(STDOUT_FILENO, str, strlen(str));
}

/**
 * @brief Prints the distraction simulation menu
 */
void print_menu() {
    print("\nSimulate a distraction:\n\
  1 = Email notification\n\
  2 = Reminder to pick up delivery\n\
  3 = Doorbell Ringing\n\
  q = Quit\n\
>> ");
}

/**
 * @brief Prints the top section of the focus round UI
 * @param round The current focus round number
 */
void print_top(int round) {
    char buf[326];
    // format the top border and round info
    sprintf(buf, "══════════════════════════════════════════════\n\
                Focus Round %d                \n\
──────────────────────────────────────────────\n", round);

    print(buf);
}

/**
 * @brief Prints the bottom section of the focus round UI
 */
void print_bottom() {
    print("──────────────────────────────────────────────\n\
             Back to Focus Mode.              \n\
══════════════════════════════════════════════\n");
}

/**
 * @brief Prints the middle section of the UI
 */
void print_middle() {
    print("──────────────────────────────────────────────\n\
        Checking pending distractions...      \n\
──────────────────────────────────────────────\n");
}

/**
 * @brief Reads a single character from the user and converts it to an integer.
 * @returns Integer value of the input, or 0 if 'q' is pressed
 */
int read_user_input() {
    char c;
    // read one character from standard input
    read(STDIN_FILENO, &c, sizeof(char));

    // skip newline characters
    if (c == '\n') {
        read(STDIN_FILENO, &c, sizeof(char)); 
    }

    // handle quit input
    if (c == 'q') {
        return 0;
    }
    // convert numeric character to integer
    return c - '0';
}


/**
 * @brief Simulates user-triggered interrupts based on menu input
 * @param duration Number of interruptions to simulate
 */
void get_user_interrupts(int duration) {
    
    // loop for the specified number of interruptions
    while (duration--) {
        
        // print the distraction menu
        print_menu();
        
        // read user input and trigger the corresponding signal
        switch (read_user_input()) {
            case 1:
                // simulate Email notification
                raise(SIGUSR1);
                break;
            case 2:
                // simulate Reminder to pick up delivery
                raise(SIGUSR2);
                break;
            case 3:
                // simulate Doorbell ringing
                raise(SIGINT);
                break;
            case 0: /* user pressed 'q' */
                return; // exit the function
            default:
                break; // ignore invalid inputs
        }
    }
}


/**
 * @brief Clears the pending signals specified in the mask
 * @param mask Pointer to the sigset_t mask object containing signals to clear
 */
void clear_pending(sigset_t *mask) {
    // Unblock signals in the mask to clear them from pending state
    sigprocmask(SIG_UNBLOCK, mask, NULL);
}

/**
 * @brief Handles pending interrupts sent by the user.
 *        Checks for different types of distractions and prints outcomes.
 */
void handle_pending_interrupts() {

    // Print middle section indicating pending distractions are being checked
    print_middle();

    sigset_t pending;      // Holds the set of pending signals
    sigpending(&pending);  // Retrieve pending signals
    int distracted = 0;    // Flag to track if any distractions occurred

    // Check if Email notification signal is pending
    if (sigismember(&pending, SIGUSR1)) {
        distracted = 1;
        print(" - Email notification is waiting.\n");
        print("[Outcome:] The TA announced: Everyone get 100 on the exercise!\n");
    }

    // Check if Reminder to pick up delivery signal is pending
    if (sigismember(&pending, SIGUSR2)) {
        distracted = 1;
        print(" - You have a reminder to pick up your delivery.\n");
        print("[Outcome:] You picked it up just in time.\n");
    }

    // Check if Doorbell ringing signal is pending
    if (sigismember(&pending, SIGINT)) {
        distracted = 1;
        print(" - The doorbell is ringing.\n");
        print("[Outcome:] Food delivery is here.\n");
    }

    // If no signals were pending
    if (!distracted) {
        print("No distractions reached you this round.\n");
    }

    // Print bottom section to close the round
    print_bottom();
}


/**
 * @brief Runs the focus mode simulation
 *        Blocks distractions for a set number of rounds and handles user interrupts.
 * @param numOfRounds Number of focus mode rounds
 * @param duration Number of interrupts per round
 */
void runFocusMode(int numOfRounds, int duration) {

    int round = 1;  // Track current focus round

    struct sigaction sa;
    sigset_t mask, old;

    sigemptyset(&mask);          // Initialize empty signal set
    sigaddset(&mask, SIGUSR1);   // Add Email notification signal
    sigaddset(&mask, SIGUSR2);   // Add Reminder signal
    sigaddset(&mask, SIGINT);    // Add Doorbell signal
    sa.sa_mask = mask;            // Apply mask to signal action

    print("Entering Focus Mode. All distractions are blocked.\n");

    while (round <= numOfRounds) {
        sigprocmask(SIG_BLOCK, &mask, &old);  // Block signals for this round

        print_top(round++);                    // Print top section for current round
        get_user_interrupts(duration);         // Get user input (simulated distractions)
        handle_pending_interrupts();           // Process any pending distractions
        clear_pending(&mask);                  // Clear pending signals before next round
    }

    print("\nFocus Mode complete. All distractions are now unblocked.");
}
