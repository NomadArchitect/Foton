/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

void kernel_init_page( void ) {
	// allow BSP to write on read-only pages inside ring0
	__asm__ volatile( "movq %cr0, %rax\nandq $~(1 << 16), %rax\nmovq %rax, %cr0" );

	// alloc page for PML4 kernel environment array
	kernel -> page_base_address = (uint64_t *) kernel_memory_alloc( TRUE );

	// --------------------------------------------------------------------

	// debug
	kernel_log( (uint8_t *) "CR3 content:\n" );

	// map required memory area
	for( uint64_t i = 0; i < limine_memmap_request.response -> entry_count; i++ ) {
		switch( limine_memmap_request.response -> entries[ i ] -> type ) {
			case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			case LIMINE_MEMMAP_FRAMEBUFFER:
			case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			case LIMINE_MEMMAP_USABLE: {
				// map memory area to kernel paging array
				kernel_page_map( kernel -> page_base_address, limine_memmap_request.response -> entries[ i ] -> base, limine_memmap_request.response -> entries[ i ] -> base | KERNEL_MEMORY_mirror, MACRO_PAGE_ALIGN_UP( limine_memmap_request.response -> entries[ i ] -> length ) >> STD_SHIFT_PAGE, KERNEL_PAGE_FLAG_present | KERNEL_PAGE_FLAG_write );

				// debug
				kernel_log( (uint8_t *) "0x%16X -> 0x%16X ", limine_memmap_request.response -> entries[ i ] -> base, limine_memmap_request.response -> entries[ i ] -> base | KERNEL_MEMORY_mirror );
				switch( limine_memmap_request.response -> entries[ i ] -> type ) {
					case LIMINE_MEMMAP_ACPI_RECLAIMABLE: { kernel_log( (uint8_t *) "ACPI (reclaimable)\n" ); break; }
					case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: { kernel_log( (uint8_t *) "Bootloader\n" ); break; }
					case LIMINE_MEMMAP_FRAMEBUFFER: { kernel_log( (uint8_t *) "Framebuffer\n" ); break; }
					case LIMINE_MEMMAP_KERNEL_AND_MODULES: { kernel_log( (uint8_t *) "Kernel and Modules\n" ); break; }
					case LIMINE_MEMMAP_USABLE: { kernel_log( (uint8_t *) "Usable\n" ); break; }
					default: kernel_log( (uint8_t *) "\n" );
				}

				// done
				break;
			}
		}
	}

	// --------------------------------------------------------------------

	// debug
	kernel_log( (uint8_t *) "--- {external}\n" );

	// map APIC controller area
	kernel_page_map( kernel -> page_base_address, (uintptr_t) kernel -> apic_base_address & ~KERNEL_MEMORY_mirror, (uintptr_t) kernel -> apic_base_address, TRUE, KERNEL_PAGE_FLAG_present | KERNEL_PAGE_FLAG_write );

	// debug
	kernel_log( (uint8_t *) "0x%16X -> 0x%16X APIC\n", (uintptr_t) kernel -> apic_base_address & ~KERNEL_MEMORY_mirror, (uintptr_t) kernel -> apic_base_address );

	// map I/O APIC controller area
	kernel_page_map( kernel -> page_base_address, (uintptr_t) kernel -> io_apic_base_address & ~KERNEL_MEMORY_mirror, (uintptr_t) kernel -> io_apic_base_address, TRUE, KERNEL_PAGE_FLAG_present | KERNEL_PAGE_FLAG_write );

	// debug
	kernel_log( (uint8_t *) "0x%16X -> 0x%16X I/O APIC\n", (uintptr_t) kernel -> io_apic_base_address & ~KERNEL_MEMORY_mirror, (uintptr_t) kernel -> io_apic_base_address );

	// now something harder ------------------------------------------------

	// kernel file ELF64 properties
	struct LIB_ELF_STRUCTURE *elf = (struct LIB_ELF_STRUCTURE *) ((struct limine_file *) limine_kernel_file_request.response -> kernel_file) -> address;

	// kernel file ELF64 header properties
	struct LIB_ELF_STRUCTURE_HEADER *elf_header = (struct LIB_ELF_STRUCTURE_HEADER *) ((uint64_t) elf + elf -> headers_offset);

	// calculate loaded kernel limit
	uint64_t kernel_limit = EMPTY;
	for( uint16_t i = 0; i < elf -> h_entry_count; i++ ) {
		// ignore blank entry or not loadable
 		if( elf_header[ i ].type != LIB_ELF_HEADER_TYPE_load || ! elf_header[ i ].memory_size ) continue;

		// update kernel limit?
		if( kernel_limit < MACRO_PAGE_ALIGN_UP( elf_header[ i ].virtual_address + elf_header[ i ].memory_size ) - KERNEL_BASE_address ) kernel_limit = MACRO_PAGE_ALIGN_UP( elf_header[ i ].virtual_address + elf_header[ i ].memory_size ) - KERNEL_BASE_address;
	}

	// prepare area for kernel segments
	uint64_t *source = (uint64_t *) KERNEL_BASE_address;
	uint64_t *target = (uint64_t *) kernel_memory_alloc( kernel_limit >> STD_SHIFT_PAGE );
	for( uint64_t i = 0; i < kernel_limit >> STD_SHIFT_8; i++ ) target[ i ] = source[ i ];

	// debug
	kernel_log( (uint8_t *) "--- {allocated}\n" );

	// connect kernel area to destination
	kernel_page_map( (uint64_t *) kernel -> page_base_address, (uintptr_t) target & ~KERNEL_MEMORY_mirror, KERNEL_BASE_address, MACRO_PAGE_ALIGN_UP( kernel_limit ) >> STD_SHIFT_PAGE, KERNEL_PAGE_FLAG_present | KERNEL_PAGE_FLAG_write ); 

	// debug
	kernel_log( (uint8_t *) "0x%16X -> 0x%16X Kernel\n", (uintptr_t) target & ~KERNEL_MEMORY_mirror, KERNEL_BASE_address );

	// and last thing, create kernel stack area
	kernel_page_alloc( (uint64_t *) kernel -> page_base_address, KERNEL_STACK_address, KERNEL_STACK_LIMIT_page, KERNEL_PAGE_FLAG_present | KERNEL_PAGE_FLAG_write );

	
	// debug
	kernel_log( (uint8_t *) "0x%16X -> 0x%16X Stack\n", kernel_page_address( kernel -> page_base_address, KERNEL_STACK_address ), KERNEL_STACK_address );
}
