# Virtual Memory Simulator

A C program that simulates virtual-to-physical address translation using paging, a page table, a Translation Lookaside Buffer (TLB), and FIFO page replacement. The program reads virtual addresses and retrieves their corresponding values from a simulated physical memory.

Developed as part of COMPSCI 3SH3 (Operating Systems) at McMaster University under Dr. Neerja Mhaskar, in collaboration with Thaneesha Sivasithambaram.

## Overview

The simulator processes a sequence of virtual addresses and models how an operating system translates them into physical memory addresses.

For each virtual address, the program:

1. Extracts the page number and offset.
2. Checks the TLB for a cached page-to-frame mapping.
3. Checks the page table if the mapping is not in the TLB.
4. Handles a page fault by loading the required page from the backing store.
5. Uses FIFO page replacement when physical memory is full.
6. Computes the corresponding physical address.
7. Retrieves and displays the value stored at that address.

The program also tracks the total number of addresses processed, page faults, and TLB hits.

## Operating Systems Concepts

* **Virtual Memory:** Simulates the translation of virtual addresses into physical addresses.
* **Paging:** Divides virtual memory into fixed-size pages and physical memory into frames.
* **Page Tables:** Maintains mappings between virtual pages and physical frames.
* **TLB:** Caches recently used address translations to reduce page-table lookups.
* **Page Faults:** Loads pages from the backing store when they are not currently in physical memory.
* **Page Replacement:** Uses a FIFO strategy to replace pages when physical memory is full.
* **Memory-Mapped Files:** Uses `mmap()` to access the backing store.
* **Bitwise Operations:** Extracts page numbers and offsets from virtual addresses.

## Implementation

The simulator uses:

* **256 logical pages**, with each page containing 256 bytes.
* **128 physical frames**.
* A **16-entry TLB** for cached address translations.
* A page table to track which pages are currently loaded into physical memory.
* A circular queue to implement FIFO page replacement.

The virtual address is divided into a page number and offset:

```text
Virtual Address
┌──────────────┬──────────┐
│ Page Number  │  Offset  │
│    8 bits    │  8 bits  │
└──────────────┴──────────┘
```

When a page is not present in physical memory, it is loaded from `BACKING_STORE.bin`. If all physical frames are occupied, the oldest page is replaced.

## Compilation and Execution

Compile using GCC:

```bash
gcc virtual_memory.c -o virtual-memory
```

Run:

```bash
./virtual-memory
```

The program expects `addresses.txt` and `BACKING_STORE.bin` to be available in the working directory. These files were provided as course materials and are not included in this repository.

## Technologies

* **Language:** C
* **Memory Management:** Paging, page tables, TLBs
* **System Functions:** `mmap()`, `munmap()`
* **Concepts:** Virtual memory, address translation, page faults, FIFO page replacement
* **Libraries:** Standard C library, POSIX file and memory-mapping functions
