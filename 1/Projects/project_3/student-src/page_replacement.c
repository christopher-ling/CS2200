#include "types.h"
#include "pagesim.h"
#include "mmu.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);

/**
 * --------------------------------- PROBLEM 7 --------------------------------------
 * Checkout PDF section 7 for this problem
 *
 * Make a free frame for the system to use. You call the select_victim_frame() method
 * to identify an "available" frame in the system (already given). You will need to
 * check to see if this frame is already mapped in, and if it is, you need to evict it.
 *
 * @return victim_pfn: a phycial frame number to a free frame be used by other functions.
 *
 * HINTS:
 *      - When evicting pages, remember what you checked for to trigger page faults
 *      in mem_access
 *      - If the page table entry has been written to before, you will need to use
 *      swap_write() to save the contents to the swap queue.
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void) {
    pfn_t victim_pfn;
    victim_pfn = select_victim_frame();

    // TODO: evict any mapped pages.
    if (frame_table[victim_pfn].mapped) {
        pte_t *victim_pte = get_page_table_entry(frame_table[victim_pfn].vpn, frame_table[victim_pfn].process->saved_ptbr, mem);

        // If the page is dirty, write its contents to swap space and clear dirty bit
        if (victim_pte->dirty) {
            swap_write(victim_pte, mem + victim_pfn * PAGE_SIZE);
            stats.writebacks++;
            victim_pte->dirty = 0;
        }
        // Invalidate the page table entry so no futre accesses trigger a page fault
        victim_pte->valid = 0;

        // Mark the frame as no longer mapped
        frame_table[victim_pfn].mapped = 0;
        frame_table[victim_pfn].process = NULL;
        frame_table[victim_pfn].ref_count = 0;
    }

    return victim_pfn;
}

/**
 * --------------------------------- PROBLEM 9 --------------------------------------
 * Checkout PDF section 7, 9, and 11 for this problem
 *
 * Finds a free physical frame. If none are available, uses either a
 * randomized, Approximate LRU, or FIFO algorithm to find a used frame for
 * eviction.
 *
 * @return The physical frame number of a victim frame.
 *
 * HINTS:
 *      - Use the global variables MEM_SIZE and PAGE_SIZE to calculate
 *      the number of entries in the frame table.
 *      - Use the global last_evicted to keep track of the pointer into the frame table
 * ----------------------------------------------------------------------------------
 */
pfn_t select_victim_frame() {
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped)
        {
            return i;
        }
    }

    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t unprotected_found = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                unprotected_found = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (unprotected_found < NUM_FRAMES) {
            return unprotected_found;
        }
    }
    else if (replacement == APPROX_LRU) {
        /* Implement a LRU algorithm here */
        // Select the unprotected mapped frame with the smallest reference count
        pfn_t victim = 0; // Stores the index of the frame to be evicted
        uint8_t min_ref = 0xFF; // Initialize min_ref

        // Look through all frames and select the one whose referenced bit is not set
        for (pfn_t i = 0; i < num_entries; i++) {
            // Only consider unprotected frames for eviciton
            if (!frame_table[i].protected && frame_table[i].ref_count < min_ref) {
                min_ref = frame_table[i].ref_count; // Update min_ref with the smallest value found
                victim = i; // Store the index of the frame with the lowest reference count
            }
        }
        return victim;
        
    }
    else if (replacement == FIFO) {
        /* Implement a FIFO algorithm here */
        // Use global variable to keep track of the oldest frame
        for (pfn_t i = last_evicted; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                last_evicted = i + 1; // Move the pointer to the next frame for the next FIFO eviction
                return i;
            }
        }
        // If no frame was found find the first part of the frame table
        for (pfn_t i = 0; i < last_evicted; i++) {
            if (!frame_table[i].protected) {
                last_evicted = i + 1; // Move the pointer to the next frame for the next FIFO eviction
                return i;
            }
        }
    }

    // If every frame is protected, give up. This should never happen on the traces we provide you.
    panic("System ran out of memory\n");
    exit(1);
}
/**
 * --------------------------------- PROBLEM 10.2 --------------------------------------
 * Checkout PDF for this problem
 *
 * Updates the associated variables for the Approximate LRU,
 * called every time the simulator daemon wakes up.
 *
 * ----------------------------------------------------------------------------------
 */
void daemon_update(void)
{
    /** FIX ME */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;

    // Iterate through all frames in the frame table
    for (size_t i = 0; i < num_entries; i++) {
        // Process only frames that are mapped (in use) and not protected
        if (frame_table[i].mapped && !frame_table[i].protected) {
            pte_t *pte = get_page_table_entry(frame_table[i].vpn, frame_table[i].process->saved_ptbr, mem);

            // Shift reference count to age the frames
            frame_table[i].ref_count >>= 1;

            // If referenced, set the MSB (Most Significant Bit) to indicate recent use
            if (pte->referenced) {
                frame_table[i].ref_count |= 0x80; // 0x80 == 10000000 in binary
                pte->referenced = 0; // Clear the referenced bit for the next cycle
            }
        }
    }

}
