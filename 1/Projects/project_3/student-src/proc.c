#include "proc.h"
#include "mmu.h"
#include "pagesim.h"
#include "address_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 3 --------------------------------------
 * Checkout PDF section 4 for this problem
 * 
 * This function gets called every time a new process is created.
 * You will need to allocate a frame for the process's page table using the
 * free_frame function. Then, you will need update both the frame table and
 * the process's PCB. 
 * 
 * @param proc pointer to process that is being initialized 
 * 
 * HINTS:
 *      - pcb_t: struct defined in pagesim.h that is a process's PCB.
 *      - You are not guaranteed that the memory returned by the free frame allocator
 *      is empty - an existing frame could have been evicted for our new page table.
 * ----------------------------------------------------------------------------------
 */
void proc_init(pcb_t *proc) {
// TODO: initialize proc's page table.
    // Allocate a free frame for the process' page table
    pfn_t frame = free_frame();
    // Update the PCB with new page table base register
    proc->saved_ptbr = frame;
    // Mark the frame holding the page table as protected to prevent eviction
    frame_table[frame].protected = 1;
    // Clear memory from the page table
    pte_t *pt = get_page_table(frame, mem);
    memset(pt, 0, PAGE_SIZE);
}


/**
 * --------------------------------- PROBLEM 4 --------------------------------------
 * Checkout PDF section 5 for this problem
 * 
 * Switches the currently running process to the process referenced by the proc 
 * argument.
 * 
 * Every process has its own page table, as you allocated in proc_init. You will
 * need to tell the processor to use the new process's page table.
 * 
 * @param proc pointer to process to become the currently running process.
 * 
 * HINTS:
 *      - Look at the global variables defined in pagesim.h. You may be interested in
 *      the definition of pcb_t as well.
 * ----------------------------------------------------------------------------------
 */
void context_switch(pcb_t *proc) {
    // TODO: update any global vars and proc's PCB to match the context_switch.
    PTBR = proc->saved_ptbr;
}

/**
 * --------------------------------- PROBLEM 8 --------------------------------------
 * Checkout PDF section 8 for this problem
 * 
 * When a process exits, you need to free any pages previously occupied by the
 * process.
 * 
 * HINTS:
 *      - If the process has swapped any pages to disk, you must call
 *      swap_free() using the page table entry pointer as a parameter.
 *      - If you free any protected pages, you must also clear their"protected" bits.
 * ----------------------------------------------------------------------------------
 */
void proc_cleanup(pcb_t *proc) {
    // Get the process' page table using its base register
    pte_t *pt = get_page_table(proc->saved_ptbr, mem);
    // TODO: Iterate the proc's page table and clean up each valid page
    for (size_t i = 0; i < NUM_PAGES; i++) {
        pte_t *pte = &pt[i];
        // If page is swapped then free its swap entry
        if (swap_exists(pte)) {
            swap_free(pte);
        }
        // If mapped in memory, free it
        if (pte->valid) {
            pfn_t frame = pte->pfn;
            // If the frame is marked as protected then clear the protected bit
            if (frame_table[frame].protected) {
                frame_table[frame].protected = 0;
            }
            // Mark frame as free
            frame_table[frame].mapped = 0;
            frame_table[frame].process = NULL;
            // Invalidate the page table entry
            pte->valid = 0;
        }

        // Free the frame holding the page table itself
        if (frame_table[proc->saved_ptbr].protected) {
            frame_table[proc->saved_ptbr].protected = 0;
        }
        frame_table[proc->saved_ptbr].mapped = 0;
        frame_table[proc->saved_ptbr].process = NULL;
    }
}

#pragma GCC diagnostic pop
