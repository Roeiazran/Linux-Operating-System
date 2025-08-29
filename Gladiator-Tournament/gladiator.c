#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NUM_GLADIATORS 4

typedef struct {
    int health;
    int id;
    int attack;
    FILE * file;
} opponents;

typedef struct {
    int health;
    int attack;
    char * name;
    opponents opponents[NUM_GLADIATORS - 1];
    FILE * file;
    FILE * logfile;
} player;

player pl;

/**
 * @brief Opens a file safely and exits on failure
 * @param name  File name
 * @param perm  File permission mode (e.g., "r+", "w")
 * @return FILE* Pointer to opened file
 */
FILE * Fopen(char * name, char * perm) {
    FILE * file = fopen(name, perm);  // try to open the file
    if (file == NULL) {               // check for failure
        perror("failed to open file"); // print error message
        exit(1);                        // terminate program
    }
    return file;                       // return the opened file pointer
}

/**
 * @brief Extracts the numeric ID from a filename (assumes Gx.txt)
 * @param fname  File name string (e.g., "G0.txt")
 * @return int   Gladiator ID
 */
int extract_id(char * fname) {
    return (int)(fname[1] - '0');     // convert character at index 1 to integer
}

/**
 * @brief Sets up each opponent's file, health, and attack values
 * Reads from files named G0.txt, G1.txt, ...
 */
void set_opponents_info() {
    char fname[] = "G0.txt";           // base filename

    for (int i = 0; i < NUM_GLADIATORS - 1; i++) {
        fname[1] += pl.opponents[i].id;            // adjust filename based on opponent ID
        pl.opponents[i].file = Fopen(fname, "r+"); // open the opponent's file safely
        fscanf(pl.opponents[i].file, "%d, %d", &pl.opponents[i].health, &pl.opponents[i].attack); // read health & attack
        fname[1] = '0';                            // reset filename for next iteration
    }
}

/**
 * @brief Logs the start of a gladiator process
 * @param id Gladiator ID
 */
void log_first_line(int id) {
    fprintf(pl.logfile, "Gladiator process started. %d:\n", id); // write to logfile
}


/**
 * @brief Sets up the player with name, file, logfile, and opponent info
 * @param name  Player's name
 * @param fname Base filename for player data (e.g., "G0")
 */
void set_player_info(char * name, char * fname) {
    
    pl.name = name;                    // store player name
    char log_fname[11];                 // buffer for log filename

    strcpy(log_fname, fname);           // copy base filename
    strcat(log_fname, "_log.txt");      // create logfile name
    strcat(fname, ".txt");              // append ".txt" for player data file

    pl.file = Fopen(fname, "r+");       // open player data file
    pl.logfile = Fopen(log_fname, "w+"); // open logfile file for writing

    // read player's stats and opponent IDs from file
    fscanf(pl.file, "%d, %d, %d, %d, %d", &pl.health, &pl.attack,
        &pl.opponents[0].id, &pl.opponents[1].id, &pl.opponents[2].id);

    set_opponents_info();               // initialize opponent info

    log_first_line(extract_id(fname));  // log player start in logfile
}

/**
 * @brief Safely closes all files associated with the player and opponents
 */
void close_files() {
    for (int i = 0; i < NUM_GLADIATORS - 1; i++) {
        fclose(pl.opponents[i].file);   // close each opponent's file
    }
    fclose(pl.file);                     // close player's own file
    fclose(pl.logfile);                  // close player's log file
}

/**
 * @brief Main function for the Gladiator Battle simulation.
 *
 * Initializes the player, then loops through battles against three opponents.
 * Updates player health, logs each attack, and ends the program if the player falls.
 *
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments: player name and base file name
 * @return int Returns 0 on successful completion
 */
int main(int argc, char *argv[])
{
    // Initialize player info with name and file
    set_player_info(argv[1], argv[2]); 

    // Keep fighting while the player is alive
    while (pl.health > 0) {
        
        // Loop through each of the 3 opponents
        for (int i = 0; i < 3; i++) {
            
            int opponent_attack = pl.opponents[i].attack; // opponent's attack value
            int opponent_id = pl.opponents[i].id;         // opponent's ID

            // Log the attack
            fprintf(pl.logfile, "Facing opponent %d... Taking %d damage\n", opponent_id, opponent_attack);

            // Update player's health
            pl.health -= opponent_attack;

            // Check if the player survived
            if (pl.health > 0) {
                fprintf(pl.logfile, "Are you not entertained? Remaining health: %d\n", pl.health);
            } else {
                // Player has fallen: log and exit
                fprintf(pl.logfile, "The gladiator has fallen... Final health: %d\n", pl.health);
                close_files();  // close all files
                exit(0);        // terminate program
            }
        }
    }
    return 0;
}


