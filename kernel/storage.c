/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

struct KERNEL_STRUCTURE_STORAGE *kernel_storage_register( uint8_t type ) {
	// lock exclusive access
	MACRO_LOCK( kernel -> storage_semaphore );

	// check for available storage entry
	for( uint64_t i = 0; i < KERNEL_STORAGE_limit; i++ ) {
		// entry available?
		if( kernel -> storage_base_address[ i ].device_type ) continue;	// no

		// mark entry as occupied
		kernel -> storage_base_address[ i ].device_type = type;

		// unlock access
		MACRO_UNLOCK( kernel -> storage_semaphore );

		// return pointer to device entry
		return (struct KERNEL_STRUCTURE_STORAGE *) &kernel -> storage_base_address[ i ];
	}

	// unlock access
	MACRO_UNLOCK( kernel -> storage_semaphore );

	// no available entry
	return EMPTY;
}
