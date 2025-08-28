#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum {
    ARRIVAL, SHORTEST_JOB, PRIORITY
} sort_by;

typedef struct process {
    char Name[51];
    char Description[101];
    int Arrival_Time;
    int Burst_Time;
    int Remaining_Time;
    int Priority;
    pid_t pid;
} P;

char buf[396];

void output_to_screen() {
    /* writes the contents of buf to the standard output */
    write(STDOUT_FILENO, buf, strlen(buf));
}

/**
 @brief Forks a new process
 @return PID of the child process, or -1 on failure
 */
pid_t Fork(void) {
    pid_t pid;

    if ((pid = fork()) < 0) {
        perror("fork failed");
    }

    return pid;
}

/**
 @brief Opens a CSV file in read/write mode
 @param path Path to the CSV file
 @return FILE pointer to the opened file
 */
FILE * get_csv_file(char * path) {
    FILE * file = fopen(path, "r+");  
    if (file == NULL) {
        perror("csv file not found!");
        exit(1);
    }
    return file;
}

/**
 @brief Returns the smaller of two integers
 @param a First integer
 @param b Second integer
 @return Smaller of a and b
 */
int min(int a, int b) {
    return (a < b) ? a : b;
}

/**
 @brief Returns the larger of two integers
 @param a First integer
 @param b Second integer
 @return Larger of a and b
 */
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 @brief Fills an array of processes with data from a CSV file
 @param path Path to the CSV file
 @param p_array Array of processes to fill
 @return Number of processes read from the file
 */
int fill_processes_array(char * path, P p_array[]) {
    FILE * csv = get_csv_file(path);
    int i = 0;
    char c;
    while (fscanf(csv, "%[^,]%*c%[^,]%*c%d,%d,%d", p_array[i].Name, p_array[i].Description,
        &p_array[i].Arrival_Time, &p_array[i].Burst_Time, &p_array[i].Priority) != EOF) {
            p_array[i].Remaining_Time = p_array[i].Burst_Time;
            i++;
            fscanf(csv, "%c", &c);
        }
    return i;
}

/**
 @brief Blocks all signals except SIGALRM
 */
void block_all_signal() {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

/* Handler for signals */
void handler(int sig, siginfo_t *si, void *ucontext) {}

/**
 @brief Sets the alarm handler for SIGALRM and SIGUSR1
 */
void set_alarm_handler() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
}

int time = 0; // global time

/**
 @brief Simulates idle time of the CPU
 @param burst_time Duration of idle time
 */
void idle_burst(int burst_time) {
    alarm(burst_time); /* Set alarm */
    pause(); /* Wait for the alarm */

    sprintf(buf, "%d → %d: Idle.\n", time, burst_time + time);
    output_to_screen();
}

/**
 @brief Simulates a CPU burst for a child process
 @param pid Process ID of the child
 @param burst_time Duration of the CPU burst
 */
void burst(pid_t pid, int burst_time) {
    kill(pid, SIGCONT); /* Continue the child process */
    alarm(burst_time); /* Set alarm */
    pause(); /* Wait for the alarm */
    kill(pid, SIGSTOP); /* Stop the child process */
}

/**
 * @brief Simulates a CPU burst for a single process
 * @param p Pointer to the process instance
 */
void simulate_cpu_burst(P * p) {

    int Burst_Time = p->Burst_Time;
    p->Remaining_Time -= Burst_Time;

    burst(p->pid, Burst_Time);
    sprintf(buf, "%d → %d: %s Running %s.\n", time, Burst_Time + time, p->Name, p->Description);
    output_to_screen();

    if (p->Remaining_Time == 0) { /* Process finished running */
        kill(p->pid, SIGKILL);
        waitpid(p->pid, NULL, 0);
    }
}

/**
 * @brief Simulates preemptive scheduling algorithms (e.g., Round-Robin)
 * @param p_array Array of processes
 * @param len Length of the process array
 */
void run_preemptive(P p_array[], int len) {

    int finished = 0, i = 0, j, arrived = 1;

    while (finished != len) { /* Continue until all processes have finished */
        
        if ((p_array[i].Arrival_Time - time) > 0) { /* Check for idle time */
            idle_burst(p_array[i].Arrival_Time - time);
            time = p_array[i].Arrival_Time;
        }

        if (p_array[i].Remaining_Time) {

            p_array[i].Burst_Time = min(p_array[i].Burst_Time, p_array[i].Remaining_Time);
            simulate_cpu_burst(&p_array[i]); /* Simulate a burst */
            time += p_array[i].Burst_Time;

            if (p_array[i].Remaining_Time == 0) {
                finished++;
                i++;
                continue;
            }
        }
  
        j = i + 1;
        while (j < len && p_array[j++].Arrival_Time <= time) 
            arrived++; /* Update number of processes that have arrived */
       
        i = (i + 1) % arrived;
    }
}
/**
 * @brief Simulates non-preemptive scheduling algorithms (FCFS, SJF, Priority)
 * @param p_array Array of processes
 * @param len Length of the process array
 * @return Average waiting time of all processes
 */
float run_non_preemptive(P p_array[], int len) {

    float waiting_time = 0;

    for (int i = 0; i < len; i++) {

        if ((p_array[i].Arrival_Time - time) > 0) { /* Check for idle CPU time */
            idle_burst(p_array[i].Arrival_Time - time);
            time = p_array[i].Arrival_Time;
        }

        waiting_time += time - p_array[i].Arrival_Time;
  
        simulate_cpu_burst(&p_array[i]); /* Run the process */
        time += p_array[i].Burst_Time;
    }

    return waiting_time / len;
}

/**
 * @brief Compares processes by arrival time
 * @param a1 First process
 * @param a2 Second process
 * @return 1 if a1 arrives before or at the same time as a2, 0 otherwise
 */
int cmp_arrival(P a1, P a2) {
    return (a1.Arrival_Time <= a2.Arrival_Time);
}

/**
 * @brief Compares processes by burst time
 * @param a1 First process
 * @param a2 Second process
 * @return 1 if a1 has shorter or equal burst time than a2, 0 otherwise
 */
int cmp_shortest_job(P a1, P a2) {
    return (a1.Burst_Time <= a2.Burst_Time);
}

/**
 * @brief Compares processes by priority
 * @param a1 First process
 * @param a2 Second process
 * @return 1 if a1 has higher or equal priority than a2, 0 otherwise
 */
int cmp_priority(P a1, P a2) {
    return (a1.Priority <= a2.Priority);
}

/**
 * @brief Merges two sorted subarrays of processes according to a comparison function.
 * @param arr Array of processes
 * @param left Left index of the subarray
 * @param mid Middle index to divide the subarrays
 * @param right Right index of the subarray
 * @param cmp Comparison function to decide the ordering
 */
void merge(P arr[], int left, int mid, int right, int (*cmp)(P, P)) {

    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    P L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0; j = 0; k = left;

    while (i < n1 && j < n2) {
        if (cmp(L[i], R[j])) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];
}

/**
 * @brief Performs merge sort on an array of processes using a custom comparison function.
 * @param arr Array of processes
 * @param left Left index of the array/subarray
 * @param right Right index of the array/subarray
 * @param cmp Comparison function used to order elements
 */
void mergeSort(P arr[], int left, int right, int (*cmp)(P, P)) {

    if (left < right) {
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        mergeSort(arr, left, mid, cmp);
        mergeSort(arr, mid + 1, right, cmp);

        // Merge the sorted halves
        merge(arr, left, mid, right, cmp);
    }
}

/**
 * @brief Sorts an array of processes according to a specified criterion.
 * @param p_array Array of processes
 * @param len Length of the array
 * @param sortby Criterion to sort by (ARRIVAL, SHORTEST_JOB, PRIORITY)
 */
void sort(P p_array[], int len, sort_by sortby) {

    switch (sortby)
    {
        case ARRIVAL:
            mergeSort(p_array, 0, len - 1, cmp_arrival);
            break;
        case SHORTEST_JOB:
            mergeSort(p_array, 0, len - 1, cmp_shortest_job);
            break;
        case PRIORITY:
            mergeSort(p_array, 0, len - 1, cmp_priority);
            break;
        default:
            break;
    }
}

/**
 * @brief Creates child processes for all processes in the array.
 * Each child process will execute an infinite loop and will be stopped initially.
 * @param p_array Array of processes
 * @param len Length of the array
 */
void create_child_proccess(P p_array[], int len) {

    for (int i = 0; i < len; i++) {
        p_array[i].Remaining_Time = p_array[i].Burst_Time; /* initialize remaining time */

        pid_t pid = fork();

        if (pid != 0) {  /* Parent process */
            p_array[i].pid = pid;
            kill(pid, SIGSTOP); /* stop the child process until scheduled */
        } else {  /* Child process */
            while(1) {};  /* infinite loop to simulate running process */
        }
    }
}

/**
 * @brief Prints the opening banner for the scheduler with the selected algorithm.
 * @param alg Name of the scheduling algorithm
 */
void print_opening(char * alg) {
    sprintf(buf, "══════════════════════════════════════════════\n\
>> Scheduler Mode : %s\n\
>> Engine Status  : Initialized\n\
──────────────────────────────────────────────\n\n", alg);
    output_to_screen();
}

/**
 * @brief Prints the closing summary for non-preemptive scheduling algorithms.
 * @param avg_WT Average waiting time of all processes
 */
void print_np_closing(double avg_WT) {

    sprintf(buf, "\n──────────────────────────────────────────────\n\
>> Engine Status  : Completed\n\
>> Summary        :\n\
   └─ Average Waiting Time : %.2f time units\n\
>> End of Report\n\
══════════════════════════════════════════════\n", avg_WT);
    output_to_screen();
}

/**
 * @brief Prints the closing summary for preemptive scheduling algorithms.
 * @param TAT Total Turnaround Time of all processes
 */
void print_p_closing(int TAT) {

    sprintf(buf, "\n──────────────────────────────────────────────\n\
>> Engine Status  : Completed\n\
>> Summary        :\n\
   └─ Total Turnaround Time : %d time units\n\
\n\
>> End of Report\n\
══════════════════════════════════════════════\n", TAT);

    output_to_screen();
}
/**
 * @brief Moves an array element from one position to another.
 *        Allows a process that arrived earlier to run first.
 * @param p_array Processes array
 * @param from Index to move from
 * @param to Index to move to
 */
void bring_to_pos(P p_array[], int from, int to) {

    P temp;
    for (int i = from; i > to; i--) {
        temp = p_array[i];
        p_array[i] = p_array[i - 1];
        p_array[i - 1] = temp;
    }
}

/**
 * @brief Sorts the process array by arrival time.
 *        Ensures that processes that arrive first get to run first.
 * @param p_array Processes array
 * @param len Length of the process array
 */
void sort_by_prop(P p_array[], int len) {

    int time = 0;

    for (int i = 0; i < len; i++) {
        for (int j = i; j < len; j++) {
            if (p_array[j].Arrival_Time <= time) {
                time += p_array[j].Burst_Time;
                bring_to_pos(p_array, j, i);
                break;
            }
        }
    }
}
/**
 * @brief Runs the First-Come-First-Serve (FCFS) scheduling algorithm
 * @param p_array Array of processes
 * @param len Length of the process array
 */
void FCFS(P p_array[], int len) {

    print_opening("FCFS");
    sort(p_array, len, ARRIVAL);

    create_child_proccess(p_array, len);

    float avg_WT = run_non_preemptive(p_array, len);

    print_np_closing(avg_WT);
}

/**
 * @brief Runs the Shortest Job First (SJF) non-preemptive scheduling algorithm
 * @param p_array Array of processes
 * @param len Length of the process array
 */
void SJF(P p_array[], int len) {
    
    time = 0;
    print_opening("SJF");

    sort(p_array, len, SHORTEST_JOB);
    sort_by_prop(p_array, len);

    create_child_proccess(p_array, len);
    float avg_WT = run_non_preemptive(p_array, len);

    print_np_closing(avg_WT);
}

/**
 * @brief Runs the Priority non-preemptive scheduling algorithm
 * @param p_array Array of processes
 * @param len Length of the process array
 */
void PS(P p_array[], int len) {

    time = 0;
    print_opening("Priority");

    sort(p_array, len, PRIORITY);
    sort_by_prop(p_array, len);

    create_child_proccess(p_array, len);
    float avg_WT = run_non_preemptive(p_array, len);

    print_np_closing(avg_WT);
}

/**
 * @brief Runs the Round Robin (RR) preemptive scheduling algorithm
 * @param p_array Array of processes
 * @param len Length of the process array
 * @param time_quantum Time quantum for each CPU burst
 */
void RR(P p_array[], int len, int time_quantum) {

    time = 0;
    sort(p_array, len, ARRIVAL);
    create_child_proccess(p_array, len);

    for (int i = 0; i < len; i++) {
        p_array[i].Burst_Time = time_quantum;
    }

    print_opening("Round Robin");
    run_preemptive(p_array, len);
    print_p_closing(time);
}

/**
 * @brief Copies one process array to another
 * @param p_array Source process array
 * @param copy Destination process array
 * @param len Length of the arrays
 */
void copy_array(P p_array[], P copy[], int len) {
    for (int i = 0; i < len; i++) {
        copy[i] = p_array[i];
    }
}

/**
 * @brief Runs all CPU scheduling algorithms on the processes in a CSV file.
 *        Executes FCFS, SJF, Priority, and Round Robin algorithms.
 * @param processesCsvFilePath Path to the CSV file containing process data
 * @param time_quantum Time quantum to be used for the Round Robin algorithm
 */
void runCPUScheduler(char* processesCsvFilePath, int time_quantum) {

    P p_array[1001], copy[1001];
    int len;

    /* Fill the processes array from the CSV file */
    len = fill_processes_array(processesCsvFilePath, p_array);
    
    /* Make a copy for preemptive scheduling */
    copy_array(p_array, copy, len);
    
    /* Block all signals except SIGALRM */
    block_all_signal();

    /* Set the SIGALRM and SIGUSR1 handlers */
    set_alarm_handler();
    
    /* Run non-preemptive algorithms */
    FCFS(p_array, len);
    sprintf(buf, "\n");
    output_to_screen();
    
    SJF(p_array, len);
    sprintf(buf, "\n");
    output_to_screen();

    PS(p_array, len);
    sprintf(buf, "\n");
    output_to_screen();

    /* Run preemptive Round Robin algorithm */
    RR(copy, len, time_quantum);
}
