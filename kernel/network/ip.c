/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

uint8_t kernel_network_ipv4( struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET *ethernet, uint16_t length ) {
	// properties of IPv4 header
	struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *ipv4 = (struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *) ((uintptr_t) ethernet + sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET ));

	// inquiry about our IPv4 address or multicast?
	if( ipv4 -> target != MACRO_ENDIANNESS_DWORD( kernel -> network_interface.ipv4_address ) && ipv4 -> target != 0xFFFFFF ) return TRUE;	// no, ignore

	// IPv4 header length
	uint16_t ipv4_header_length = (ipv4 -> version_and_header_length >> STD_SHIFT_4) << STD_SHIFT_32;

	// // packet dump
	// uint8_t *memory = (uint8_t *) ethernet;
	// for( uint8_t y = 0; y < (length / 16) + 1 ; y++ ) kernel -> log( (uint8_t *) "%8X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X\n", (uintptr_t) &memory[ y * 16 ], memory[ (y * 16) + 0 ], memory[ (y * 16) + 1 ], memory[ (y * 16) + 2 ], memory[ (y * 16) + 3 ], memory[ (y * 16) + 4 ], memory[ (y * 16) + 5 ], memory[ (y * 16) + 6 ], memory[ (y * 16) + 7 ], memory[ (y * 16) + 8 ], memory[ (y * 16) + 9 ], memory[ (y * 16) + 10 ], memory[ (y * 16) + 11 ], memory[ (y * 16) + 12 ], memory[ (y * 16) + 13 ], memory[ (y * 16) + 14 ], memory[ (y * 16) + 15 ]); kernel -> log( (uint8_t *) "\n" );

	// choose action
	switch( ipv4 -> protocol ) {
		case KERNEL_NETWORK_HEADER_IPV4_PROTOCOL_icmp: {
			// parse as ICMP frame
			kernel_network_icmp( ethernet, length );

			// done
			break;
		}

		case KERNEL_NETWORK_HEADER_IPV4_PROTOCOL_udp: {
			// parse as ICMP frame
			// kernel -> network_udp( ethernet, length );

			// done
			break;
		}

		case KERNEL_NETWORK_HEADER_IPV4_PROTOCOL_tcp: {
			// kernel -> network_tcp( ethernet, length );
		}
	}

	// debug
	return FALSE;
}

void kernel_network_ipv4_encapsulate( struct KERNEL_NETWORK_STRUCTURE_SOCKET *socket, struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET *ethernet, uint16_t length ) {
	// properties of IPv4 header
	struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *ipv4 = (struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *) ((uintptr_t) ethernet + sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET ));
	ipv4 -> version_and_header_length = KERNEL_NETWORK_HEADER_IPV4_VERSION_AND_HEADER_LENGTH_default;
	ipv4 -> ecn = KERNEL_NETWORK_HEADER_IPV4_ECN_default;
	ipv4 -> length = MACRO_ENDIANNESS_WORD( (length + ((KERNEL_NETWORK_HEADER_IPV4_VERSION_AND_HEADER_LENGTH_default & 0x0F) << STD_SHIFT_4)) );
	ipv4 -> id = socket -> ipv4_id;
	ipv4 -> flags_and_offset = KERNEL_NETWORK_HEADER_IPV4_FLAGS_AND_OFFSET_default;
	ipv4 -> ttl = KERNEL_NETWORK_HEADER_IPV4_TTL_default;
	ipv4 -> protocol = socket -> ipv4_protocol;
	ipv4 -> local = MACRO_ENDIANNESS_DWORD( kernel -> network_interface.ipv4_address );
	ipv4 -> target = socket -> ipv4_target;

	// calculate checksum
	ipv4 -> checksum = EMPTY;	// always
	ipv4 -> checksum = kernel_network_checksum( (uint16_t *) ((uintptr_t) ethernet + sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET )), sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 ) );

	// wrap data into a Ethernet frame and send
	kernel_network_ethernet_encapsulate( socket, ethernet, length + ((KERNEL_NETWORK_HEADER_IPV4_VERSION_AND_HEADER_LENGTH_default & 0x0F) << STD_SHIFT_4) );
}

void kernel_network_ipv4_exit( struct KERNEL_NETWORK_STRUCTURE_SOCKET *socket, uint8_t *data, uint16_t length ) {
	// alloc packet area
	struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET *ethernet = (struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET *) kernel_memory_alloc( TRUE );

	// properties of IPv4 header
	struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *ipv4 = (struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 *) ((uintptr_t) ethernet + sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_ETHERNET ));

	// copy IPv4 data
	uint8_t *frame_data = (uint8_t *) ((uintptr_t) ipv4 + sizeof( struct KERNEL_NETWORK_STRUCTURE_HEADER_IPV4 ));
	for( uint16_t i = 0; i < length; i++ ) frame_data[ i ] = data[ i ];

	// wrap data into a IPv4 frame and send
	kernel_network_ipv4_encapsulate( socket, ethernet, length );
}