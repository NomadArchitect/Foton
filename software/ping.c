/*===============================================================================
 Copyright (C) Andrzej Adamczyk (at https://blackdev.org/). All rights reserved.
===============================================================================*/

	//----------------------------------------------------------------------
	// variables, routines, procedures
	//----------------------------------------------------------------------
	#include	"../library/network.h"

#define	PING_ICMP_TYPE_request	0x08
#define	PING_ICMP_TYPE_reply	0x00

#define	PING_ICMP_DATA_length	32

struct PING_STRUCTURE_ICMP {
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	identificator;
	uint16_t	sequence;
} __attribute__((packed));

int64_t _main( uint64_t argc, uint8_t *argv[] ) {
	// debug, do not use
	return 0;

	// nothing to do?
	if( argc < 2 ) return 0;	// yes

	// retrieve IPv4 address
	uint32_t ipv4 = lib_network_string_to_ipv4( (uint8_t *) argv[ 1 ] );

	// IPv4 invalid?
	if( ! ipv4 ) {
		// show error message
		printf( "Invalid IPv4 address: \e[38;5;250m%s", argv[ 1 ] );

		// end
		return 0;
	}

	// prepare connection with selected IPv4 address
	int64_t socket = std_network_open( STD_NETWORK_PROTOCOL_icmp, ipv4, EMPTY, EMPTY );

	// create ICMP request
	struct PING_STRUCTURE_ICMP *icmp = (struct PING_STRUCTURE_ICMP *) malloc( sizeof( struct PING_STRUCTURE_ICMP ) + PING_ICMP_DATA_length );	// default 32 Bytes of data inside ICMP frame

	// ICMP type: request
	icmp -> type = PING_ICMP_TYPE_request;
	
	// ICMP code: EMPTY
	icmp -> code = EMPTY;

	// identificator
	icmp -> identificator = MACRO_ENDIANNESS_WORD( TRUE );

	// sequence number
	icmp -> sequence = MACRO_ENDIANNESS_WORD( TRUE );

	// insert data
	uint8_t *data = (uint8_t *) ((uintptr_t) icmp + sizeof( struct PING_STRUCTURE_ICMP ));
	for( uint16_t i = 0; i < PING_ICMP_DATA_length; i++ ) data[ i ] = i + 0x20;

	// calculate checksum
	icmp -> checksum = EMPTY;	// always
	icmp -> checksum = lib_network_checksum( (uint16_t *) icmp, sizeof( struct PING_STRUCTURE_ICMP ) + PING_ICMP_DATA_length );

	// send request outside
	std_network_send( socket, (uint8_t *) icmp, sizeof( struct PING_STRUCTURE_ICMP ) + PING_ICMP_DATA_length );

	// release ICMP request
	free( icmp );

	// done
	return 0;
}