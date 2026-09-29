#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> // for mmap() function
#include <string.h>   // for memcpy function
#include <fcntl.h>    // for file descriptors

#define BUFFER_SIZE 10                  // maximum characters read per line
#define PAGES 256                       // logical pages (2^16 / 2^8)
#define FRAMES 128                      // physical frames (2^15 / 2^8)
#define OFFSET_MASK 0xFF                // mask for lower 8 bits
#define OFFSET_BITS 8                   // 256 bytes per page
#define TLB_SIZE 16                     // maximum number of entries in TLB
#define PAGE_SIZE 256                   // size of page in bytes
#define MEMORY_SIZE (PAGES * PAGE_SIZE) // total amount of memory

// data structure for TLB entry (page number, frame number) pair
typedef struct
{
    int page_number;
    int frame_number;
} TLBEntry;

int page_table[PAGES];   // stores frame number corresponding to page number
int frame_queue[FRAMES]; // stores pages corresponding to each frame

int next_frame = 0;  // next avaliable frame
int queue_index = 0; // index at which we replace next frame (indicates oldest frame in memory)

TLBEntry tlb[TLB_SIZE]; // translation lookaside buffer
int tlb_index = 0;      // index at which we insert next TLBEntry

signed char *mmapfptr;                           // pointer to memory mapped region
signed char physical_memory[FRAMES * PAGE_SIZE]; // circulary array that simulates phyiscal memory

// statisitcs
int tlb_hits = 0;
int page_faults = 0;
int total_addresses = 0;

// searches for page_number in TLB, returns corresponding frame_number on TLB hit, -1 otherwise
int search_TLB(int page_number)
{
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (tlb[i].page_number == page_number)
        {
            tlb_hits++;
            return tlb[i].frame_number;
        }
    }
    return -1;
}

// adds new (page_number, frame_number) pair into TLB
void TLB_add(int page_number, int frame_number)
{
    tlb[tlb_index].page_number = page_number;
    tlb[tlb_index].frame_number = frame_number;

    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

// update (page_number, frame_number) pair at old_page_number
void TLB_update(int old_page_number, int new_page_number, int frame_number)
{
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (tlb[i].page_number == old_page_number)
        {
            tlb[i].page_number = new_page_number;
            tlb[i].frame_number = frame_number;
            return;
        }
    }
}

int main()
{
    // open binary file, read only
    int mmapfile_fd = open("BACKING_STORE.bin", O_RDONLY);

    // returns a pointer to the start of the memory mapped region
    mmapfptr = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);

    // open file
    FILE *fptr = fopen("addresses.txt", "r");
    char buff[BUFFER_SIZE];

    // initialize page table entries to -1
    for (int i = 0; i < PAGES; i++)
        page_table[i] = -1;

    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        // compute page number and offset using logic from Lab 4
        int logical_address = atoi(buff);
        int page_number = logical_address >> OFFSET_BITS;
        int offset = logical_address & OFFSET_MASK;

        // look up frame number in TLB
        int frame_number = search_TLB(page_number);

        // if search_TLB returns -1, we have a TLB miss
        if (frame_number == -1)
        {
            frame_number = page_table[page_number];

            // if frame_number is not in the page table, we have a page fault
            if (frame_number == -1)
            {
                page_faults++;

                // copy paged from mapped file into RAM
                signed char *src = mmapfptr + (page_number * PAGE_SIZE);

                // check if there is space in memory (if there is a free frame available)
                if (next_frame < FRAMES)
                {
                    // free frame available
                    frame_number = next_frame;

                    memcpy(&physical_memory[frame_number * PAGE_SIZE], src, PAGE_SIZE);

                    page_table[page_number] = frame_number;
                    frame_queue[next_frame] = page_number;
                    next_frame++;
                }
                else
                {
                    // memory is full, replace oldest page
                    int victim_page = frame_queue[queue_index];
                    int victim_frame = page_table[victim_page];

                    // load new page in victim frame
                    page_table[victim_page] = -1;

                    // load new page into victim frame
                    memcpy(&physical_memory[victim_frame * PAGE_SIZE], src, PAGE_SIZE);

                    // update page table and queue
                    page_table[page_number] = victim_frame;
                    frame_queue[queue_index] = page_number;
                    queue_index = (queue_index + 1) % FRAMES;

                    // update victim slot in TLB
                    TLB_update(victim_page, page_number, victim_frame);

                    frame_number = victim_frame;
                }
                // add new pair to TLB
                TLB_add(page_number, frame_number);
            }
            else
            {
                // present in page table, add to TLB
                TLB_add(page_number, frame_number);
            }
        }

        // compute physical address
        int physical_address = (frame_number << OFFSET_BITS) | offset;
        signed char value = physical_memory[physical_address];

        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, (int)value);

        total_addresses++;
    }
    // print stats
    printf("Total addresses: %d\n", total_addresses);
    printf("Page faults: %d\n", page_faults);
    printf("TLB Hits: %d\n", tlb_hits);

    // free memory mapped region, close ptr
    munmap(mmapfptr, MEMORY_SIZE);
    fclose(fptr);

    return 0;
}
