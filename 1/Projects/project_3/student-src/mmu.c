#include "mmu.h"
#include "pagesim.h"
#include "address_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* The frame table pointer. You will set this up in system_init. */
fte_t *frame_table;

/**
 * --------------------------------- PROBLEM 2 --------------------------------------
 * Checkout PDF sections 4 for this problem
 * 
 * In this problem, you will initialize the frame_table pointer. The frame table will
 * be located at physical address 0 in our simulated memory. You should zero out the 
 * entries in the frame table, in case for any reason physical memory is not clean.
 * 
 * HINTS:
 *      - mem: Simulated physical memory already allocated for you.
 *      - PAGE_SIZE: The size of one page
 * ----------------------------------------------------------------------------------
 */
void system_init(void) {
    // TODO: initialize the frame_table pointer.
    frame_table = (fte_t *) mem;
    // Zero out the entries in the frame table
    memset(mem, 0, PAGE_SIZE);
    // Mark first frame as protected
    frame_table[0].protected = 1;
}


/**
 * --------------------------------- PROBLEM 5 --------------------------------------
 * Checkout PDF section 6 for this problem
 * 
 * Takes an input virtual address and performs a memory operation.
 * 
 * @param addr virtual address to be translated
 * @param access 'r' if the access is a read, 'w' if a write
 * @param data If the access is a write, one byte of data to write to our memory.
 *             Otherwise NULL for read accesses.
 * 
 * HINTS:
 *      - Remember that not all the entry in the process's page table are mapped in. 
 *      Check what in the pte_t struct signals that the entry is mapped in memory.
 * ----------------------------------------------------------------------------------
 */
uint8_t mem_access(vaddr_t address, char access, uint8_t data) {
    stats.accesses++;
    // TODO: translate virtual address to physical, then perform the specified operation
    // Get the vpn of the address
    vaddr_t vpn = get_vaddr_vpn(address);
    // Get the offset of the address
    uint16_t offset = get_vaddr_offset(address);
    // Get the page table entry of the vpn
    pte_t *pte = get_page_table_entry(vpn, PTBR, mem);

    // Check to see if mapped, if not page fault handler
    if (!pte->valid) {
        page_fault(address);
        pte = get_page_table_entry(vpn, PTBR, mem);
    }

    pfn_t pfn = pte->pfn;

    // After correct page mapped, mark frame as referenced
    pte->referenced = 1;
    
    paddr_t pa = get_physical_address(pfn, offset);

    // Either read or write the data to the physical address depending on 'rw'
    if (access == 'r') {
        // Read from physical address from physical memory
        return mem[pa];
    } else {
        // write data and mark page as dirty
        mem[pa] = data;
        pte->dirty = 1;
        return data;
    }

    return 0;
}
