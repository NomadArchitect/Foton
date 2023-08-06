/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

#ifndef	KERNEL_LIBRARY
	#define	KERNEL_LIBRARY

	#define	KERNEL_LIBRARY_base_address	0x0000700000000000

	#define	KERNEL_LIBRARY_limit		16

	#define	KERNEL_LIBRARY_FLAG_active	0b00000001
	#define	KERNEL_LIBRARY_FLAG_reserved	0b10000000

	struct	KERNEL_LIBRARY_STRUCTURE {
		uintptr_t	pointer;
		uint64_t	size_page;
		uint8_t		flags;
		uint8_t		*strtab;
		struct LIB_ELF_STRUCTURE_DYNAMIC_SYMBOL	*dynamic_linking;
		uint64_t	d_entry_count;
		uint8_t		length;
		uint8_t		name[ 255 ];
	};

	// loads libraries required by executable
	void kernel_library( struct LIB_ELF_STRUCTURE *elf );

	// configure and initialize library
	void kernel_library_load( uint8_t *string, uint64_t length );

	// reserving entry for new library
	struct KERNEL_LIBRARY_STRUCTURE *kernel_library_register();
#endif