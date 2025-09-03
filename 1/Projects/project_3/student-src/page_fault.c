#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 6 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Page fault handler.
 * 
 * When the CPU encounters an invalid address mapping in a page table, it invokes the 
 * OS via this handler. Your job is to put a mapping in place so that the translation 
 * can succeed.
 * 
 * @param addr virtual address in the page that needs to be mapped into main memory.
 * 
 * HINTS:
 *      - You will need to use the global variable current_process when
 *      altering the frame table entry.
 *      - Use swap_exists() and swap_read() to update the data in the 
 *      frame as it is mapped in.
 * ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t address) {
    stats.page_faults++;
    // TODO: Get a new frame, then correctly update the page table and frame table
    // Get the vpn from address
    vaddr_t vpn = get_vaddr_vpn(address);
    // Get the page table entry for faulting page in current process' page table
    pte_t *pte = get_page_table_entry(vpn, current_process->saved_ptbr, mem);
    // Allocate a new frame for this page
    pfn_t frame = free_frame();

    // Check if swap entry exists for this page
    if (swap_exists(pte)) {
        // Read the page from swap into the frame
        swap_read(pte, &mem[frame * PAGE_SIZE]);
    } else {
        // If the page is not in swap, allocate a new page in swap
        memset(mem + frame * PAGE_SIZE, 0, PAGE_SIZE);
    }

    // Update the page table entry for the faulting page
    pte->valid = 1;
    pte->pfn = frame;
    pte->dirty = 0;

    // Update the corresponding frame table entry
    frame_table[frame].mapped = 1;
    frame_table[frame].process = current_process;
    frame_table[frame].vpn = vpn;
    frame_table[frame].ref_count = 0;
}

#pragma GCC diagnostic pop
