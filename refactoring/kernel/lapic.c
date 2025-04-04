/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

void kernel_lapic_accept( void ) {
	// about handling current hardware interrupt, properly
	kernel -> lapic_base_address -> eoi = EMPTY;
}

uint8_t kernel_lapic_id( void ) {
	// return BSP/logical CPU ID
	return kernel -> lapic_base_address -> id >> 24;
}

void kernel_lapic_reload( void ) {
	// to call internal clock interrupt in about N cycles
	kernel -> lapic_base_address -> tic = KERNEL_APIC_Hz;
}
