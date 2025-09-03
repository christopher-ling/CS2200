/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 * Fall 2024
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

static unsigned int cpu_count;
static int timeslice = 0;

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 *
 * rq is a pointer to a struct you should use for your ready queue
 * implementation. The head of the queue corresponds to the process
 * that is about to be scheduled onto the CPU, and the tail is for
 * convenience in the enqueue function. See student.h for the
 * relevant function and struct declarations.
 *
 * Similar to current[], rq is accessed by multiple threads,
 * so you will need to use a mutex to protect it. ready_mutex has been
 * provided for that purpose.
 *
 * The condition variable queue_not_empty has been provided for you
 * to use in conditional waits and signals.
 *
 * Please look up documentation on how to properly use pthread_mutex_t
 * and pthread_cond_t.
 *
 * A scheduler_algorithm variable and sched_algorithm_t enum have also been
 * supplied to you to keep track of your scheduler's current scheduling
 * algorithm. You should update this variable according to the program's
 * command-line arguments. Read student.h for the definitions of this type.
 */
static pcb_t **current;
static queue_t *rq;

static pthread_mutex_t current_mutex;
static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;

static sched_algorithm_t scheduler_algorithm;
static unsigned int cpu_count;
static unsigned int age_weight;

/** ------------------------Problem 3-----------------------------------
 * Checkout PDF Section 5 for this problem
 * 
 * priority_with_age() is a helper function to calculate the priority of a process
 * taking into consideration the age of the process.
 * 
 * It is determined by the formula:
 * Priority With Age = Priority - (Current Time - Enqueue Time) * Age Weight
 * 
 * @param current_time current time of the simulation
 * @param process process that we need to calculate the priority with age
 * 
 */
extern double priority_with_age(unsigned int current_time, pcb_t *process) {
    return process->priority - (current_time - process->enqueue_time) * age_weight;
}

/** ------------------------Problem 0 & 3-----------------------------------
 * Checkout PDF Section 2 and 5 for this problem
 * 
 * enqueue() is a helper function to add a process to the ready queue.
 * 
 * NOTE: For Priority, FCFS, and SRTF scheduling, you will need to have additional logic
 * in this function and/or the dequeue function to account for enqueue time
 * and age to pick the process with the smallest age priority.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 *
 * @param queue pointer to the ready queue
 * @param process process that we need to put in the ready queue
 */
void enqueue(queue_t *queue, pcb_t *process)
{
    pthread_mutex_lock(&queue_mutex);
    process->enqueue_time = get_current_time();

    if (is_empty(queue)) {
        queue->head = process;
        queue->tail = process;
        process->next = NULL;
    } else if (scheduler_algorithm == PA) {
        unsigned int currTime = get_current_time();
        if (priority_with_age(currTime, process) > priority_with_age(currTime, queue->head)) {
            // new process has higher priority than head
            process->next = queue->head;
            queue->head = process;
        } else {
            // new process has lower priority than head so find spot in queue
            pcb_t *curr = queue->head;
            while (curr->next != NULL && priority_with_age(currTime, curr->next) > priority_with_age(currTime, process)) {
                curr = curr->next;
            }
            // put process in queue
            process->next = curr->next;
            curr->next = process;

            // fix tail pointer
            if (process->next == NULL) {
                queue->tail = process;
            }
        }
    } else if (scheduler_algorithm == SRTF) {
        unsigned int new_time = process->total_time_remaining;
        if (new_time < queue->head->total_time_remaining) {
            process->next = queue->head;
            queue->head = process;
        } else {
            // new process has longer CPU burst than head so find spot in queue
            pcb_t *curr = queue->head;
            while (curr->next != NULL && curr->next->total_time_remaining < new_time) {
                curr = curr->next;
            }
            // put process in queue
            process->next = curr->next;
            curr->next = process;

            // fix tail pointer
            if (process->next == NULL) {
                queue->tail = process;
            }
        }

    } else if (scheduler_algorithm == FCFS) {
        if (process->arrival_time < queue->head->arrival_time) {
            process->next = queue->head;
            queue->head = process;
        } else {
            // find spot in queue
            pcb_t *curr = queue->head;
            while (curr->next != NULL && curr->next->arrival_time < process->arrival_time) {
                curr = curr->next;
            }
            // put process in queue
            process->next = curr->next;
            curr->next = process;

            // fix tail pointer
            if (process->next == NULL) {
                queue->tail = process;
            }
        }

    } else {
        // just add to tail
        queue->tail->next = process;
        queue->tail = process;
        process->next = NULL;
    }
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

/**
 * dequeue() is a helper function to remove a process to the ready queue.
 *
 * NOTE: For Priority, FCFS, and SRTF scheduling, you will need to have additional logic
 * in this function and/or the enqueue function to account for enqueue time
 * and age to pick the process with the smallest age priority.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 * 
 * @param queue pointer to the ready queue
 */
pcb_t *dequeue(queue_t *queue)
{
    pthread_mutex_lock(&queue_mutex);
    
    if (is_empty(queue)) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }
    
    pcb_t *process = queue->head;
    queue->head = process->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    process->next = NULL;

    pthread_mutex_unlock(&queue_mutex);
    return process;
}

/** ------------------------Problem 0-----------------------------------
 * Checkout PDF Section 2 for this problem
 * 
 * is_empty() is a helper function that returns whether the ready queue
 * has any processes in it.
 * 
 * @param queue pointer to the ready queue
 * 
 * @return a boolean value that indicates whether the queue is empty or not
 */
bool is_empty(queue_t *queue)
{
    return queue->head == NULL;
}

/** ------------------------Problem 1B-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * schedule() is your CPU scheduler.
 * 
 * Remember to specify the timeslice if the scheduling algorithm is Round-Robin
 * 
 * @param cpu_id the target cpu we decide to put our process in
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t *newProcess = dequeue(rq);
    if (newProcess != NULL) {
        newProcess->state = PROCESS_RUNNING;
    } else {
        pthread_mutex_unlock(&queue_mutex);
        pthread_mutex_lock(&current_mutex);
        current[cpu_id] = NULL;
        context_switch(cpu_id, NULL, -1);
        return;
    }

    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = newProcess;
    pthread_mutex_unlock(&current_mutex);

    if (scheduler_algorithm == RR) {
        context_switch(cpu_id, newProcess, timeslice);
    } else {
        context_switch(cpu_id, newProcess, -1);
    }
}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled. This function should block until a process is added
 * to your ready queue.
 *
 * @param cpu_id the cpu that is waiting for process to come in
 */
extern void idle(unsigned int cpu_id)
{
   pthread_mutex_lock(&queue_mutex);
   while (is_empty(rq)) {
    pthread_cond_wait(&queue_not_empty, &queue_mutex);
   }
   pthread_mutex_unlock(&queue_mutex);
   schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    // mt_safe_usleep(1000000);
}

/** ------------------------Problem 2 & 3-----------------------------------
 * Checkout Section 4 and 5 for this problem
 * 
 * preempt() is the handler used in Round-robin, Preemptive Priority, and SRTF scheduling.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 * 
 * @param cpu_id the cpu in which we want to preempt process
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    pcb_t *proc = current[cpu_id];
    proc->state = PROCESS_READY;
    enqueue(rq, proc);
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * @param cpu_id the cpu that is yielded by the process
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    pcb_t *proc = current[cpu_id];
    proc->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3
 * 
 * terminate() is the handler called by the simulator when a process completes.
 * 
 * @param cpu_id the cpu we want to terminate
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    pcb_t *proc = current[cpu_id];
    proc->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/**  ------------------------Problem 1A & 3---------------------------------
 * Checkout PDF Section 3 and 5 for this problem
 * 
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes. 
 * This method will also need to handle priority and SRTF preemption.
 * Look in section 5 of the PDF for more info on priority.
 * Look in section 6 of the PDF for more info on SRTF preemption.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 * 
 * @param process the process that finishes I/O and is ready to run on CPU
 */
extern void wake_up(pcb_t *process)
{
    
    enqueue(rq, process);
    pthread_mutex_lock(&current_mutex);
    process->state = PROCESS_READY;

    if (scheduler_algorithm == SRTF) {
        

         // Check for any idle CPU
        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i] == NULL) {
                pthread_mutex_unlock(&current_mutex);
                return;
            }
        }

        unsigned int targetCPU = -1;
        int flagPreempt = 0;

        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i]->total_time_remaining > process->total_time_remaining) {
                force_preempt(i);
            }
        }

        pthread_mutex_unlock(&current_mutex);

    } else if (scheduler_algorithm == PA) {

        // Check for any idle CPU
        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i] == NULL) {
                pthread_mutex_unlock(&current_mutex);
                return;
            }
        }

        unsigned int lowestProcess = -1;
        int flagPreempt = 0;

        for (int i = 0; i <cpu_count; i++) {
            if (priority_with_age(get_current_time(), current[i]) > priority_with_age(get_current_time(), process)) {
                force_preempt(i);
                break;
            }
        }
        pthread_mutex_unlock(&current_mutex);
    }
}

/**
 * main() simply parses command line arguments, then calls start_simulator().
 * Add support for -r, -p, and -s parameters. If no argument has been supplied, 
 * you should default to FCFS.
 * 
 * HINT:
 * Use the scheduler_algorithm variable (see student.h) in your scheduler to 
 * keep track of the scheduling algorithm you're using.
 */
int main(int argc, char *argv[])
{
    scheduler_algorithm = FCFS;
    age_weight = 0;
    timeslice = -1;

    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
                        "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p <age weight> | -s ]\n"
                        "    Default : FCFS Scheduler\n"
                        "         -r : Round-Robin Scheduler\n1\n"
                        "         -p : Priority Aging Scheduler\n"
                        "         -s : Shortest Remaining Time First\n");
        return -1;
    } else if (argc == 4) {
        if (strcmp(argv[2], "-r") == 0) {
            scheduler_algorithm = RR;
            timeslice = atoi(argv[3]);
        } else if (strcmp(argv[2], "p") == 0) {
            scheduler_algorithm = PA;
            age_weight = atoi(argv[3]);
        }
    } else if (argc == 3)  {
        if (strcmp(argv[2], "-s") == 0) {
            scheduler_algorithm = PA;
            age_weight = atoi(argv[3]);
        }
    }


    /* Parse the command line arguments */
    cpu_count = strtoul(argv[1], NULL, 0);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t *) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    rq = (queue_t *)malloc(sizeof(queue_t));
    assert(rq != NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}

#pragma GCC diagnostic pop
