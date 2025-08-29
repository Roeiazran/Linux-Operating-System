# CPU Scheduling Simulator

## Overview
This project is a CPU scheduling simulator written in C. It reads process data from a CSV file and simulates various scheduling algorithms, including:

- **First-Come-First-Serve (FCFS)**
- **Shortest Job First (SJF)** (non-preemptive)
- **Priority Scheduling** (non-preemptive)
- **Round Robin (RR)** (preemptive)

The simulator creates child processes to mimic CPU execution, supports preemption using signals, and provides detailed output including CPU bursts, idle times, and turnaround/waiting times.

---

## Features
- Reads process information (name, description, arrival time, burst time, priority) from a CSV file.
- Simulates both **preemptive** and **non-preemptive** scheduling.
- Supports configurable **time quantum** for Round Robin scheduling.
- Handles idle CPU time when no process has arrived.
- Generates detailed output with running processes and timing information.
- Supports process sorting based on arrival time, burst time, or priority.
- Uses signals (`SIGALRM`, `SIGUSR1`) for burst timing and preemption.

---