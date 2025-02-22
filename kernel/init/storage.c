/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

void kernel_init_storage( void ) {
	// allocate area for list of available storages
	kernel -> storage_base_address = (struct KERNEL_STRUCTURE_STORAGE *) kernel_memory_alloc( MACRO_PAGE_ALIGN_UP( KERNEL_STORAGE_limit * sizeof( struct KERNEL_STRUCTURE_STORAGE ) ) >> STD_SHIFT_PAGE );

	// register modules of Virtual File System as storages
	for( uint64_t i = 0; i < limine_module_request.response -> module_count; i++ ) {
		// module type of VFS?
		if( ! kernel_vfs_identify( (uintptr_t) limine_module_request.response -> modules[ i ] -> address, limine_module_request.response -> modules[ i ] -> size ) ) continue;	// no

		// register device of type VFS
		struct KERNEL_STRUCTURE_STORAGE *storage = kernel_storage_register( KERNEL_STORAGE_TYPE_vfs );

		// address of VFS main block location
		storage -> device_block = (uintptr_t) limine_module_request.response -> modules[ i ] -> address;

		// default block size in Bytes
		storage -> device_byte = STD_PAGE_byte;

		// length of storage in Blocks
		storage -> device_limit = MACRO_PAGE_ALIGN_UP( limine_module_request.response -> modules[ i ] -> size ) >> STD_SHIFT_PAGE;

		// attach read/write functions
		storage -> read = (void *) kernel_vfs_read;
	}
}
