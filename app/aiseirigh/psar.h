#pragma once

#include "workbuffer.h"

#include <ff.h>
#include <stdint.h>

typedef struct
{
    uint32_t unk0;
    uint32_t unk4;
    uint32_t num_entries;
    uint32_t unk_block_size;
    char version[128];
    uint32_t unk90;
    char unk94[124];
} PsarMetadata;

typedef struct
{
	uint32_t unk0; //0
	char filename[252]; //4
	uint32_t unk1; //0x100
	uint32_t size; //0x104
	uint32_t expanded_size; // 0x108
	char unk2; // 0x10C
	char unk3; // 0x10D
	char is_file; // 0x10E
	char signcheck; // 0x10F
} PSAR_File_Header;

typedef struct
{
    PSAR_File_Header header;
    size_t file_offset;
} PsarEntry;

typedef struct {
    const char *name;
    const char *path;
    PsarEntry *entry;
} TableEntry;

typedef struct {
    FIL fp;
    unsigned int version;
    unsigned int table_version;
    const WorkBuffer *wb1, *wb2;
    PsarEntry *list;
    size_t num_entries;
    TableEntry *table_list;
    size_t num_table_entries;
    PsarMetadata metadata;
} PsarHandle;

typedef enum {
    PSAR_RESULT_OK,
    PSAR_ERR_OPEN,
    PSAR_ERR_READ_HEADER,
    PSAR_ERR_SEEK_HEADER,
    PSAR_ERR_NOT_PSAR,
    PSAR_ERR_READ_DATA,
    PSAR_ERR_DECRYPT_HEADER,
    PSAR_ERR_UNKNOWN_FIRMWARE,
    PSAR_ERR_SEEK_DATA,
    PSAR_ERR_PARSE_FILE_HEADER,
    PSAR_ERR_OOM,
    PSAR_ERR_ENTRY_NOT_FOUND,
    PSAR_ERR_ENTRY_NOT_FILE,
    PSAR_ERR_WORK_BUFFER_TOO_SMALL,
    PSAR_ERR_DECOMPRESSION,
    PSAR_ERR_PARSE_TABLE,
    PSAR_ERR_DECRYPT_TABLE,
    PSAR_ERR_HANDLER,
    PSAR_ERR_DECRYPT_IPL,
    PSAR_ERR_IPL_NOT_FOUND
} PsarResult;

typedef enum {
    PSAR_FILE_NORMAL,
    PSAR_FILE_REQ_PERSONALISED
} Personalisation;

typedef enum {
    PSAR_FILE_TYPE_NORMAL,
    PSAR_FILE_TYPE_IPL
} FileType;

typedef struct {
    Personalisation personalisation;
    FileType file_type;
} FileProperties;

typedef int (* ON_DIRECTORY_FUNC)(const char *path, void *arg);
typedef int (* ON_FILE_FUNC)(const char *path, unsigned char *data, size_t len, const FileProperties *prop);

PsarResult psar_init(PsarHandle *ph, const WorkBuffer *wb1, const WorkBuffer *wb2);
PsarResult psar_open(PsarHandle *ph, const char *path);
PsarResult psar_read_each_directory(PsarHandle *ph, ON_DIRECTORY_FUNC func, void *arg);
PsarResult psar_read_each_file(PsarHandle *ph, ON_FILE_FUNC func);