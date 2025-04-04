/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

void kernel_network_ethernet_encapsulate( struct KERNEL_STRUCTURE_NETWORK_SOCKET *socket, struct KERNEL_STRUCTURE_NETWORK_HEADER_ETHERNET *ethernet, uint16_t length ) {
	// set target and host MAC addresses
	for( uint8_t i = 0; i < 6; i++ ) ethernet -> target[ i ] = socket -> ethernet_address[ i ];
	for( uint8_t i = 0; i < 6; i++ ) ethernet -> source[ i ] = kernel -> network_interface.ethernet_address[ i ];

	// set type of Ethernet header
	switch( socket -> protocol ) {
		case STD_NETWORK_PROTOCOL_arp: { ethernet -> type = MACRO_ENDIANNESS_WORD( KERNEL_NETWORK_HEADER_ETHERNET_TYPE_arp ); break; }
		default: { ethernet -> type = MACRO_ENDIANNESS_WORD( KERNEL_NETWORK_HEADER_ETHERNET_TYPE_ipv4 ); break; }
	}

	// block access to stack modification
	MACRO_LOCK( kernel -> network_tx_semaphore );

	// TODO, make sure that frame was placed inside transfer queue

	// free entry available?
	if( kernel -> network_tx_limit < KERNEL_NETWORK_YX_limit )
		// insert frame properties
		kernel -> network_tx_base_address[ kernel -> network_tx_limit++ ] = ((uintptr_t) ethernet & ~KERNEL_MEMORY_mirror) | length + sizeof( struct KERNEL_STRUCTURE_NETWORK_HEADER_ETHERNET );

	// unlock
	MACRO_UNLOCK( kernel -> network_tx_semaphore );
}

uint8_t kernel_network_ethernet_resolve( struct KERNEL_STRUCTURE_NETWORK_SOCKET *socket ) {
	// wait for ARP thread to resolve destination MAC address
	volatile uint64_t timeout = kernel -> time_rtc + DRIVER_RTC_Hz;
	while( timeout > kernel -> time_rtc && ! socket -> ethernet_lease_time ) kernel_time_sleep( TRUE );

	// IPv4 resolved?
	if( socket -> ethernet_lease_time ) return TRUE;	// yes

	// no
	return FALSE;
}
