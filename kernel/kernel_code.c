/*
 * kernel_code.c - Project 2, CMPSC 473
 * Copyright 2023 Ruslan Nikolaev <rnikola@psu.edu>
 * Distribution, modification, or usage without explicit author's permission
 * is not allowed.
 */

#include <kernel.h>
#include <types.h>
#include <printf.h>
#include <malloc.h>
#include <string.h>

void *page_table = NULL; /* TODO: Must be initialized to the page table address */
void *user_stack = NULL; /* TODO: Must be initialized to a user stack virtual address */
void *user_program = NULL; /* TODO: Must be initialized to a user program virtual address */

void kernel_init(void *ustack, void *uprogram, void *memory, size_t memorySize)
{
	// 'memory' points to the place where memory can be used to create
	// page tables (assume 1:1 initial virtual-to-physical mappings here)
	// 'memorySize' is the maximum allowed size, do not exceed that (given just in case)
	printf("Hello from sgroup45, J-Y-abc\n");

	// TODO: Create a page table here
	struct page_pte{
		uint64_t present:1;
		uint64_t writable:1;
		uint64_t user_mode:1;
		uint64_t permission: 9;
		uint64_t page_address:40;
		uint64_t avail:7;
		uint64_t pke:4;
		uint64_t nonexecute:1;
	};

	// struct page_pde{
	// 	uint64_t present:1;
	// 	uint64_t writable:1;
	// 	uint64_t user_mode:1;
	// 	uint64_t pwt: 1;
	// 	uint64_t pcd: 1;
	// 	uint64_t a: 1;
	// 	uint64_t ign: 1;
	// 	uint64_t o: 1;
	// 	uint64_t ign1: 1;
	// 	uint64_t avl: 3;
	// 	uint64_t page_address:40;
	// 	uint64_t avail:7;
	// 	uint64_t pke:4;
	// 	uint64_t nonexecute:1;
	// };

	// struct page_pml4e{
	// 	uint64_t present:1;
	// 	uint64_t writable:1;
	// 	uint64_t user_mode:1;
	// 	uint64_t pwt: 1;
	// 	uint64_t pcd: 1;
	// 	uint64_t a: 1;
	// 	uint64_t ign: 1;
	// 	uint64_t mbz: 1;
	// 	uint64_t mbz1: 1;
	// 	uint64_t avl: 3;
	// 	uint64_t page_address:40;
	// 	uint64_t avail:7;
	// 	uint64_t pke:4;
	// 	uint64_t nonexecute:1;
	// };

	struct page_pte *p = memory;

	for (size_t i = 0; i < 1048576; i++){
		(p+i)->present = 1;
		(p+i)->writable = 1;
		(p+i)->user_mode = 0;
		(p+i)->permission = 0;
		(p+i)->page_address = i;
		(p+i)->avail= 0;
		(p+i)->pke= 0;
		(p+i)->nonexecute= 0;
	}


	struct page_pte *pd = (p+1048576);

	for (size_t j = 0; j < 2048; j++){
		struct page_pte *start_pte = p + 512 * j;
		(pd+j)->present = 1;
		(pd+j)->writable = 1;
		(pd+j)->user_mode = 0;
		(pd+j)->permission = 0;
		(pd+j)->page_address = (uint64_t)start_pte >> 12;
		(pd+j)->avail = 0;
		(pd+j)->pke = 0;
		(pd+j)->nonexecute = 0;
	}

	struct page_pte *pdpe = (pd + 2048);
	for (size_t k = 0; k < 4; k++){
		struct page_pte *start_pte2 = pd + 512 * k;
		(pdpe+k)->present = 1;
		(pdpe+k)->writable = 1;
		(pdpe+k)->user_mode = 0;
		(pdpe+k)->permission = 0;
		(pdpe+k)->page_address = (uint64_t)start_pte2 >> 12;
		(pdpe+k)->avail = 0;
		(pdpe+k)->pke = 0;
		(pdpe+k)->nonexecute = 0;
	}

	for (size_t k = 4; k < 512; k++){
		(pdpe+k)->present = 0;
		(pdpe+k)->writable = 0;
		(pdpe+k)->user_mode = 0;
		(pdpe+k)->permission = 0;
		(pdpe+k)->page_address = 0;
		(pdpe+k)->avail = 0;
		(pdpe+k)->pke = 0;
		(pdpe+k)->nonexecute = 0;
	}


	struct page_pte *pml4e = (pdpe + 512);
	//struct page_pml4e *start_pte = (struct page_pml4e *)pdpe;
	pml4e->present=1;
	pml4e->writable =1;
	pml4e->user_mode = 0;
	pml4e->permission = 0;
	pml4e->page_address = (uint64_t)pdpe >> 12;
	pml4e->avail= 0;
	pml4e->pke= 0;
	pml4e->nonexecute= 0;

	for (size_t l = 1; l < 512; l++){
		(pml4e+l)->present=0;
		(pml4e+l)->writable =0;
		(pml4e+l)->user_mode = 0;
		(pml4e+l)->permission = 0;
		(pml4e+l)->page_address = 0;
		(pml4e+l)->avail= 0;
		(pml4e+l)->pke= 0;
		(pml4e+l)->nonexecute= 0;
	}
	

	page_table = (void *)pml4e;
	


	// TODO: It is OK for Q1-Q3, but should changed
	// for the final part's Q4 when creating your own user page table
	user_stack = ustack;

	// TODO: It is OK for Q1-Q3, but should changed
	// for the final part's Q4 when creating your own user page table
	user_program = uprogram;

	// The remaining portion just loads the page table,
	// this does not need to be changed:
	// load 'page_table' into the CR3 register
	const char *err = load_page_table(page_table);
	if (err != NULL) {
		printf("ERROR: %s\n", err);
	}


	// The extra credit assignment
	mem_extra_test();
}

/* The entry point for all system calls */
long syscall_entry(long n, long a1, long a2, long a3, long a4, long a5)
{
	// TODO: the system call handler to print a message (n = 1)
	// the system call number is in 'n', make sure it is valid!

	// Arguments are passed in a1,.., a5 and can be of any type
	// (including pointers, which are casted to 'long')
	// For the project, we only use a1 which will contain the address
	// of a string, cast it to a pointer appropriately 

	// For simplicity, assume that the address supplied by the
	// user program is correct
	//
	// Hint: see how 'printf' is used above, you want something very
	// similar here
	if (n == 1){
		
		printf("%s", (char *)a1);
		return 0;
	}


	return -1; /* Success: 0, Failure: -1 */
}
