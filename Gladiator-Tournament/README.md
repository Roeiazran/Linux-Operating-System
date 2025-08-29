# Gladiator Tournament Simulation

This program simulates a gladiator tournament using multiple processes in C. Each gladiator is represented by a separate child process, and the main process manages the tournament, waits for the gladiators to finish, and declares the winner.

---

## Features

- Spawns a child process for each gladiator.
- Each gladiator reads its own file for stats and engages in a simulated fight.
- Tracks surviving gladiators to determine the winner.
- Uses `fork()`, `execvp()`, and `waitpid()` for process management.
- Logs each gladiator's progress and actions.

---

## Files

- `main.c` – Contains the tournament logic and process management.
- `gladiator.c` – Simulates individual gladiator actions and health management.
- `G1.txt`, `G2.txt`, `G3.txt`, `G4.txt` – Files containing each gladiator's stats (health, attack, opponents).
- Log files for each gladiator are generated during execution (`*_log.txt`).