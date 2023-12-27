#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
	uint32_t		signature; 
	uint16_t		attribute; 
	uint16_t		comp_attribute; 
	uint8_t		module_ver_lo;	
	uint8_t		module_ver_hi;	
	char	modname[28];
	uint8_t		version; 
	uint8_t		nsegments; 
	int		elf_size; 
	int		psp_size; 
	uint32_t		entry;	
	uint32_t		modinfo_offset; 
	int		bss_size; 
	uint16_t		seg_align[4]; 
	uint32_t		seg_address[4];
	int		seg_size[4]; 
	uint32_t		reserved[5]; 
	uint32_t		devkitversion; 
	uint8_t		decrypt_mode; 
	uint8_t		padding; 
	uint16_t		overlap_size; 
	uint8_t		key_data0[0x30]; 
	int		comp_size; 
	int		_80;
	int		reserved2[2];	
	uint8_t		key_data1[0x10];
	uint32_t		tag; 
	uint8_t		scheck[0x58];
	uint32_t		key_data2;
	uint32_t		oe_tag; 
	uint8_t		key_data3[0x1C]; 
}  PSP_Header;

int kirk1_decrypt(unsigned char *dst, const unsigned char *src, size_t len);
int kirk5_encrypt(unsigned char *dst, const unsigned char *src, size_t len);
int kirk7_decrypt(unsigned char *dst, const unsigned char *src, size_t len, int keyid);
