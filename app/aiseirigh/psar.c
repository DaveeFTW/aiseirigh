#include "psar.h"
#include "crypt.h"
#include "log.h"
#include "des.h"
#include "prx.h"

#include <lk/debug.h>

#include <lib/miniz.h>

#include <kirk.h>
#include <uart.h>
#include <model.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
	uint32_t magic;
	int version;
	int size;
	uint32_t unk0;
} PSAR_Header;

static PsarResult get_header(FIL* fp, PSAR_Header *header)
{
    while (1) {
        unsigned int bytes_read = 0;
        FRESULT res = f_read(fp, header, sizeof(PSAR_Header), &bytes_read);
        
        if (res != FR_OK) {
            LOG("%s: err %i reading PSAR header/PBP header", __FUNCTION__, res);
            return PSAR_ERR_READ_HEADER;
        }

        // check if we're dealing with a PBP. technically, yes we expect that
        // it is a PBP, but if its a PSAR it'll be fine
        if (header->magic == 0x50425000) {
            uint32_t psar_offset = 0;

            res = f_lseek(fp, 0x24);

            if (res != FR_OK) {
                LOG("%s: err %i seeking to PSAR header offset", __FUNCTION__, res);
                return PSAR_ERR_SEEK_HEADER;
            }

            res = f_read(fp, &psar_offset, sizeof(psar_offset), &bytes_read);

            if (res != FR_OK) {
                LOG("%s: err %i reading PSAR header offset", __FUNCTION__, res);
                return PSAR_ERR_READ_HEADER;
            }

            res = f_lseek(fp, psar_offset);

            if (res != FR_OK) {
                LOG("%s: err %i seeking to PSAR header", __FUNCTION__, res);
                return PSAR_ERR_SEEK_HEADER;
            }
        }

        else if (header->magic == 0x52415350) {
            return PSAR_RESULT_OK;
        }

        // we have neither PBP nor PSAR
        else {
            return PSAR_ERR_NOT_PSAR;
        }
    }
}

static int get_block_header(PsarHandle *ph, PSP_Header *block_header, int position)
{
    // go to the position provided
    if (position > 0) {
        if (f_lseek(&ph->fp, position) != FR_OK) {
            return -1;
        }
    }

    unsigned int bytes_read = 0;
    if (f_read(&ph->fp, block_header, sizeof(*block_header), &bytes_read) != FR_OK) {
        return -2;
    }

    // PSAR version 2 onward has an encrypted block
    if (ph->version >= 2) {
        // kirk_hwreset();
        volatile Kirk47Header *hdr = (volatile Kirk47Header *)0xBFC00C00;
        memcpy((void *)(0xBFC00C00 + sizeof(Kirk47Header)), (char *)block_header+0x20, 0x130);
        
        hdr->mode = 5;
        hdr->unk_4 = 0;
        hdr->unk_8 = 0;
        hdr->keyslot = 0x55;
        hdr->size = 0x130;
    
        if (kirk7((void *)0xBFC00C00, (void *)0xBFC00C00) < 0) {
            return -3;
        }

        memcpy((char *)block_header+0x20, (void *)0xBFC00C00, 0x130);
    }

    return 0;
}

static int get_block_data(PsarHandle *ph, PSP_Header *header, unsigned char *data, size_t len, int position)
{
    // go to the position provided
    if (position > 0) {
        if (f_lseek(&ph->fp, position) != FR_OK) {
            return -1;
        }
    }

    memcpy(data, header, sizeof(*header));

    unsigned int bytes_read = 0;
    if (f_read(&ph->fp, data+sizeof(*header), len-sizeof(*header), &bytes_read) != FR_OK) {
        return -2;
    }

    size_t dec_length = 0;
    PrxDecryptResult res = prx_decrypt(data, &dec_length);

    switch (res) {
        case PRX_DECRYPT_RESULT_OK:
            break;

        case PRX_DECRYPT_ERR_KIRK1:
            LOG("%s: prx_decrypt kirk 1 failure", __FUNCTION__);
            return -3;
        case PRX_DECRYPT_ERR_KIRK7:
            LOG("%s: prx_decrypt kirk 7 failure", __FUNCTION__);
            return -4;
        case PRX_DECRYPT_ERR_UNSUPPORTED:
            LOG("%s: prx_decrypt unsupported tag: 0x%08X", __FUNCTION__, header->tag);
            return -5;
    }

    return 0;
}

typedef struct
{
    uint8_t key[8];
    uint8_t iv[8];
} PsarTableKeys;

const PsarTableKeys *get_table_keys(unsigned int version) {
    static const PsarTableKeys table_keys[6] = {
        {
            .key = { 0x95, 0x62, 0x0B, 0x49, 0xB7, 0x30, 0xE5, 0xC7 },
            .iv = { 0x9E, 0xA4, 0x33, 0x81, 0x86, 0x0C, 0x52, 0x85 }
        },
        {
            .key = { 0x5A, 0x7B, 0x3D, 0x9D, 0x45, 0xC9, 0xDC, 0x95 },
            .iv = { 0xB2, 0xFE, 0xD9, 0x79, 0x8A, 0x02, 0xB1, 0x87 }
        },
        {
            .key = { 0x4C, 0xCE, 0x49, 0x5B, 0x6F, 0x20, 0x58, 0x5A },
            .iv = { 0x81, 0x08, 0xC1, 0xF2, 0x35, 0x98, 0x69, 0xB0 }
        },
        {
            .key = { 0x73, 0xF4, 0x52, 0x62, 0x62, 0x0B, 0xF1, 0x5A },
            .iv = { 0x6D, 0x52, 0x1B, 0xA3, 0xC2, 0x36, 0xF9, 0x2B }
        },
        {
            .key = { 0xA6, 0x64, 0xC8, 0xF8, 0xFD, 0x9D, 0x44, 0x98 },
            .iv = { 0xDB, 0x4E, 0x79, 0x41, 0xF5, 0x97, 0x30, 0xAD }
        },
        {
            .key = { 0xD7, 0xBD, 0x74, 0x81, 0x3D, 0x64, 0x26, 0xE7 },
            .iv = { 0xA6, 0x83, 0x0C, 0x2F, 0x63, 0x0B, 0x96, 0x29 }
        },
    };

    if (version >= sizeof(table_keys)/sizeof(PsarTableKeys)) {
        FATAL("table key version unsupported! table version %i", version);
    }

    return &table_keys[version];
}

static int is_table(const char *name, unsigned int version)
{
    if (version == 1 || version == 2) {
        return (strcmp(name, "01g:00000") == 0 || strcmp(name, "02g:00000") == 0);
    }
    else if (version == 3 || version == 4) {
        unsigned int table_num = atoi(name);
        return table_num <= 12;
    }

    return 0;
}

#include <mbedtls/des.h>

static int decrypt_table(void *dst, const void *src, size_t len, unsigned int version)
{
    LOG("decrypting table of size %i bytes. table_version = %i", len, version);

    const PsarTableKeys *keys = get_table_keys(version);

    unsigned char iv[8];
    memcpy(iv, keys->iv, sizeof(iv));

    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_dec(&ctx, keys->key);
    mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_DECRYPT, len, iv, src, dst);
    mbedtls_des_free(&ctx);

    PSP_Header *header = (PSP_Header *)dst;

    size_t dec_length = 0;
    PrxDecryptResult res = prx_decrypt(dst, &dec_length);

    switch (res) {
        case PRX_DECRYPT_RESULT_OK:
            break;

        case PRX_DECRYPT_ERR_KIRK1:
            LOG("%s: prx_decrypt kirk 1 failure", __FUNCTION__);
            return -3;
        case PRX_DECRYPT_ERR_KIRK7:
            LOG("%s: prx_decrypt kirk 7 failure", __FUNCTION__);
            return -4;
        case PRX_DECRYPT_ERR_UNSUPPORTED:
            LOG("%s: prx_decrypt unsupported tag: 0x%08X", __FUNCTION__, header->tag);
            return -5;
    }

    return 0;
}

static PsarResult decrypt_header(PsarHandle *ph, unsigned char *header)
{
    if (ph->version == 5) {
        static const unsigned char go_pre_key[] = { 0xD8, 0x69, 0xB8, 0x95, 0x33, 0x6B, 0x63, 0x34, 0x98, 0xB9, 0xFC, 0x3C, 0xB7, 0x26, 0x2B, 0xD7 };
        for (size_t i = 0; i < sizeof(PSP_Header) - 0x20; ++i) {
            header[i + 0x20] ^= go_pre_key[i % sizeof(go_pre_key)];
        }
    }

    int res = kirk7_decrypt(header + 0x20, header + 0x20, sizeof(PSP_Header) - 0x20, 0x55);

    if (res != 0) {
        return PSAR_ERR_DECRYPT_HEADER;
    }

    if (ph->version == 5) {
        static const unsigned char go_post_key[] = { 0x0D, 0xA0, 0x90, 0x84, 0xAF, 0x9E, 0xB6, 0xE2, 0xD2, 0x94, 0xF2, 0xAA, 0xEF, 0x99, 0x68, 0x71 };
        for (size_t i = 0; i < sizeof(PSP_Header) - 0x20; ++i) {
            header[i + 0x20] ^= go_post_key[i % sizeof(go_post_key)];
        }
    }

    return PSAR_RESULT_OK;
}

static PsarResult decrypt_block_in_wb1(PsarHandle *ph, size_t *len)
{
    PSP_Header *header = (PSP_Header *)ph->wb1->ptr;
    size_t dec_length = 0;
    PrxDecryptResult res = prx_decrypt(header, &dec_length);

    switch (res) {
        case PRX_DECRYPT_RESULT_OK:
            break;

        case PRX_DECRYPT_ERR_KIRK1:
            LOG("%s: prx_decrypt kirk 1 failure", __FUNCTION__);
            return -3;
        case PRX_DECRYPT_ERR_KIRK7:
            LOG("%s: prx_decrypt kirk 7 failure", __FUNCTION__);
            return -4;
        case PRX_DECRYPT_ERR_UNSUPPORTED:
            LOG("%s: prx_decrypt unsupported tag: 0x%08X", __FUNCTION__, header->tag);
            return -5;
    }

    *len = dec_length;
    return PSAR_RESULT_OK;
}

static PsarResult read_next_block_wb1(PsarHandle *ph, size_t block_size, size_t *len)
{
    unsigned int bytes_read = 0;
    FRESULT res = f_read(&ph->fp, ph->wb1->ptr, block_size, &bytes_read);
    
    if (res != FR_OK) {
        LOG("%s: err %i reading PSAR data", __FUNCTION__, res);
        return PSAR_ERR_READ_DATA;
    }

    if (bytes_read != block_size) {
        return PSAR_ERR_READ_DATA;
    }

    if (ph->version != 1) {
        PsarResult res = decrypt_header(ph, ph->wb1->ptr);

        if (res != PSAR_RESULT_OK) {
            return res;
        }
    }

    return decrypt_block_in_wb1(ph, len);
}


static PsarResult read_psar_contents_list(PsarHandle *ph)
{
	PSAR_Header psar_header;
    PsarResult res = get_header(&ph->fp, &psar_header);
    
    if (res != PSAR_RESULT_OK) {
        LOG("%s: could not get PSAR header", __FUNCTION__);
        return res;
    }

    ph->version = psar_header.version;
    LOG("got PSAR: %08X", psar_header.version);

    // read the metadata block into wb1
    size_t len = 0;
    res = read_next_block_wb1(ph, sizeof(PSP_Header) + sizeof(PsarMetadata), &len);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: could not read next block", __FUNCTION__);
        return res;
    }

    PsarMetadata metadata;

    if (len != sizeof(metadata)) {
        LOG("%s: unexpected size for metadata. expected %x got %x", __FUNCTION__, sizeof(metadata), len);
        return PSAR_ERR_PARSE_FILE_HEADER;
    }

    memcpy(&metadata, ph->wb1->ptr, sizeof(metadata));

    // there is a block before the main contents list. not sure what it does,
    // and it is not necessary. just skip past it
    FRESULT fres = f_lseek(&ph->fp, f_tell(&ph->fp) + metadata.unk_block_size);
    
    if (fres != FR_OK) {
        LOG("%s: error %i seeking to file list offset", __FUNCTION__, fres);
        return PSAR_ERR_SEEK_DATA;
    }

    // allocate storage for all the files. the metadata entry count includes
    // the skipped block, so allocate one less
    size_t num_contents = metadata.num_entries - 1;
    PsarEntry *list = malloc(sizeof(PsarEntry) * num_contents);

    // check for OOM conditions
    if (!list) {
        LOG("%s: error allocating %i bytes. oom", __FUNCTION__, sizeof(PsarEntry) * num_contents);
        return PSAR_ERR_OOM;
    }

    // now populate the list
    for (size_t i = 0; i < num_contents; ++i) {
        // the PSAR is constructed in two block chunks. one block is the
        // file metadata, the next is the file contents. parse through the list
        // and read all the metadata
        size_t len = 0;
        res = read_next_block_wb1(ph, sizeof(PSP_Header) + sizeof(PSAR_File_Header), &len);

        if (res != PSAR_RESULT_OK) {
            LOG("%s: could not read next block", __FUNCTION__);
            return res;
        }

        // check that we have read enough data to create an entry
        if (len != sizeof(PSAR_File_Header)) {
            LOG("%s: unexpected size for file header. expected %x got %x", __FUNCTION__, sizeof(PSAR_File_Header), len);
            return PSAR_ERR_PARSE_FILE_HEADER;
        }

        // copy the data into our list
        memcpy(&list[i].header, ph->wb1->ptr, sizeof(list[i].header));
        list[i].file_offset = f_tell(&ph->fp);

        // check that the file can fit in the assigned work buffers
        if (list[i].header.size > ph->wb1->len || list[i].header.size > ph->wb2->len) {
            LOG("%s: psar file %s is too big for work buffer. %i bytes into %i and %i", __FUNCTION__, list[i].header.filename, list[i].header.size, ph->wb1->len, ph->wb2->len);
            return PSAR_ERR_WORK_BUFFER_TOO_SMALL;
        }

        // check that the decompressed file can fit in the assigned work buffers
        if (list[i].header.expanded_size > ph->wb1->len || list[i].header.expanded_size > ph->wb2->len) {
            LOG("%s: psar file %s is too big for work buffer when decompressed. %i bytes into %i and %i", __FUNCTION__, list[i].header.filename, list[i].header.expanded_size, ph->wb1->len, ph->wb2->len);
            return PSAR_ERR_WORK_BUFFER_TOO_SMALL;
        }

        // seek beyond this data to the next entry
        fres = f_lseek(&ph->fp, f_tell(&ph->fp) + list[i].header.size);
        
        if (fres != FR_OK) {
            LOG("%s: error %i seeking to next file entry", __FUNCTION__, fres);
            return PSAR_ERR_SEEK_DATA;
        }
    }

    ph->list = list;
    ph->num_entries = num_contents;
    ph->metadata = metadata;

    return PSAR_RESULT_OK;
}

static void *alloc_decompressor(size_t len)
{
    static WorkBuffer wb = { 0 };

    if (wb.len == 0) {
        wb = work_buffer_create(len, 16);
    }

    return wb.ptr;
}

static void free_decompressor(void *)
{
    // no-op
}

static ssize_t zlib_decompress(void *dst, size_t output_size, const void *src, size_t input_size)
{
    return tinfl_decompress_mem_to_mem(dst, output_size, src, input_size, TINFL_FLAG_PARSE_ZLIB_HEADER);
}

static int is_entry_a_file(PsarEntry *entry)
{
    return entry->header.is_file == 1;
}

static PsarEntry *lookup_entry_by_name(PsarHandle *ph, const char *name)
{
    for (size_t i = 0; i < ph->num_entries; ++i) {
        if (strcmp(ph->list[i].header.filename, name) == 0) {
            return &ph->list[i];
        }
    }

    return NULL;
}

static PsarResult read_file_by_entry(PsarHandle *ph, PsarEntry *entry, unsigned char **out_data, size_t *out_len)
{
    if (!is_entry_a_file(entry)) {
        return PSAR_ERR_ENTRY_NOT_FILE;
    }

    // seek to the file location and read the contents into wb1
    FRESULT fres = f_lseek(&ph->fp, entry->file_offset);
    
    if (fres != FR_OK) {
        LOG("%s: error %i seeking to file list offset", __FUNCTION__, fres);
        return PSAR_ERR_SEEK_DATA;
    }

    size_t len = 0;
    PsarResult res = read_next_block_wb1(ph, entry->header.size, &len);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: error %i reading file data", __FUNCTION__, res);
        return res;
    }

    // the file contents now sit in wb1, but they are zlib compressed
    // decompress into wb2
    switch (zlib_decompress(ph->wb2->ptr, entry->header.expanded_size, ph->wb1->ptr, len)) {
        default:
            break;

        case TINFL_DECOMPRESS_MEM_TO_MEM_FAILED:
            LOG("%s: error decompressing data", __FUNCTION__);
            return PSAR_ERR_DECOMPRESSION;
    }

    *out_data = ph->wb2->ptr;
    *out_len = entry->header.expanded_size;
    return PSAR_RESULT_OK;
}

static PsarResult read_file_by_name(PsarHandle *ph, const char *name, unsigned char **out_data, size_t *out_len)
{
    PsarEntry *entry = lookup_entry_by_name(ph, name);

    if (!entry) {
        return PSAR_ERR_ENTRY_NOT_FOUND;
    }

    return read_file_by_entry(ph, entry, out_data, out_len);
}

static const char *get_table_name_for_model(const PspModelIdentity *identity)
{
    switch (identity->model) {
        case PSP_MODEL_01G:
            return "00001";
        case PSP_MODEL_02G:
            return "00002";
        case PSP_MODEL_03G:
            return "00003";
        case PSP_MODEL_04G:
            return "00004";
        case PSP_MODEL_05G:
            return "00005";
        case PSP_MODEL_07G:
            return "00007";
        case PSP_MODEL_09G:
            return "00009";
        case PSP_MODEL_11G:
            return "00011";
    }

    panic("unknown model?");
}

static size_t count_table_entries(const char *table)
{
    size_t num_entries = 0;

    while ((table = strstr(table, "\r\n")) != NULL) {
        table += 2;
        num_entries += 1;
    }

    return num_entries;
}

static PsarResult build_table_entry_list(PsarHandle *ph, const char *temp_table)
{
    // the table is within a work buffer. copy it to some more
    // persistent and mutable memory
    char *table = malloc(strlen(temp_table)+1);

    if (!table) {
        LOG("%s: failed to allocate %i bytes for table data", __FUNCTION__, strlen(temp_table)+1);
        return PSAR_ERR_OOM;
    }

    strcpy(table, temp_table);

    // count the number of entries within the table
    size_t num_entries = count_table_entries(table);

    // we can allocate proper table entry data now
    TableEntry *list = (TableEntry *)malloc(num_entries * sizeof(TableEntry));

    if (!list) {
        LOG("%s: failed to allocate %i bytes for table entry list", __FUNCTION__, num_entries * sizeof(TableEntry));
        return PSAR_ERR_OOM;
    }

    // parse the input line by line
    char *token = strtok(table, "\r\n");

    for (size_t i = 0; i < num_entries; ++i) {
        if (token == NULL) {
            return PSAR_ERR_PARSE_TABLE;
        }

        // token should point to a comma separated line in the form:
        // XXXXX,device:/path/to/file/or/directory
        char *sep = strchr(token, ',');

        if (!sep) {
            return PSAR_ERR_PARSE_TABLE;
        }

        *sep = 0;
        const char *psar_ent_name = token;
        const char *path = sep + 1;

        PsarEntry *entry = lookup_entry_by_name(ph, psar_ent_name);

        if (!entry) {
            return PSAR_ERR_PARSE_TABLE;
        }

        list[i].name = psar_ent_name;
        list[i].path = path;
        list[i].entry = entry;

        token = strtok(NULL, "\r\n");
    }

    ph->table_list = list;
    ph->num_table_entries = num_entries;
    return PSAR_RESULT_OK;
}

static PsarResult decrypt_ipl(void *data, size_t *dec_length)
{
    PSP_Header header;
    memcpy(&header, data, sizeof(header));
    PrxDecryptResult res = prx_decrypt(data, dec_length);

    switch (res) {
        case PRX_DECRYPT_RESULT_OK:
            break;

        case PRX_DECRYPT_ERR_KIRK1:
            LOG("%s: prx_decrypt kirk 1 failure", __FUNCTION__);
            return PSAR_ERR_DECRYPT_IPL;
        case PRX_DECRYPT_ERR_KIRK7:
            LOG("%s: prx_decrypt kirk 7 failure", __FUNCTION__);
            return PSAR_ERR_DECRYPT_IPL;
        case PRX_DECRYPT_ERR_UNSUPPORTED:
            LOG("%s: prx_decrypt unsupported tag: 0x%08X", __FUNCTION__, header.tag);
            return PSAR_ERR_DECRYPT_IPL;
    }

    return PSAR_RESULT_OK;
}

static PsarResult self_test_ipl_decryption(PsarHandle *ph)
{
    // IPL is not encrypted on 01G models
    if (model_get_identity()->model == PSP_MODEL_01G) {
        return PSAR_RESULT_OK;
    }

    // iterate through all the entries within the table
    for (size_t i = 0; i < ph->num_table_entries; ++i) {
        TableEntry *ent = &ph->table_list[i];

        // filter out any directories
        if (!is_entry_a_file(ent->entry)) {
            continue;
        }

        if (memcmp(ent->path, "ipl:", 4) != 0) {
            continue;
        }

        unsigned char *data = NULL;
        size_t length = 0;
        PsarResult res = read_file_by_entry(ph, ent->entry, &data, &length);
    
        if (res != PSAR_RESULT_OK) {
            return res;
        }

        size_t dec_length = 0;
        return decrypt_ipl(data, &dec_length);
    }

    return PSAR_ERR_IPL_NOT_FOUND;
}

PsarResult psar_init(PsarHandle *ph, const WorkBuffer *wb1, const WorkBuffer *wb2)
{
    memset(ph, 0, sizeof(*ph));
    ph->wb1 = wb1;
    ph->wb2 = wb2;
    return PSAR_RESULT_OK;
}

PsarResult psar_open(PsarHandle *ph, const char *path)
{
    switch (f_open(&ph->fp, path, FA_READ)) {
	case FR_OK:
		break;

	default:
        LOG("%s: f_open failed: could not open %s", __FUNCTION__, path);
        return PSAR_ERR_OPEN;
	}

    PsarResult res = read_psar_contents_list(ph);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: read_psar_contents_list failed %i", __FUNCTION__, res);
        return res;
    }

    char *p = strrchr(ph->metadata.version, ',');

    if (!p) {
        LOG("%s: could not identity PSAR firmware version", __FUNCTION__);
        return PSAR_ERR_UNKNOWN_FIRMWARE;
    }

    if (memcmp(p+1, "6.61", 4) != 0) {
        LOG("%s: only firmware 6.61 is supported. got: %s", __FUNCTION__, p);
        return PSAR_ERR_UNKNOWN_FIRMWARE;
    }

    // since we're restricting support to 6.61, we can explicitly set
    // the table version for this firmware
    ph->table_version = 4;

    // now we want to read the table data for this psp model
    const char *table_name = get_table_name_for_model(model_get_identity());

    unsigned char *table_data = NULL;
    size_t table_length = 0;

    res = read_file_by_name(ph, table_name, &table_data, &table_length);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: read_file_by_name failed %i", __FUNCTION__, res);
        return res;
    }

    int tbl_res = decrypt_table(table_data, table_data, table_length, ph->table_version);

    if (tbl_res) {
        LOG("%s: decrypt_table failed %i", __FUNCTION__, tbl_res);
        return PSAR_ERR_DECRYPT_TABLE;
    }

    res = build_table_entry_list(ph, (const char *)table_data);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: build_table_entry_list failed %i", __FUNCTION__, res);
        return res;
    }

    res = self_test_ipl_decryption(ph);

    if (res != PSAR_RESULT_OK) {
        LOG("%s: self_test_ipl_decryption failed %i", __FUNCTION__, res);
        return res;
    }

    return PSAR_RESULT_OK;
}

PsarResult psar_read_each_directory(PsarHandle *ph, ON_DIRECTORY_FUNC func, void *arg)
{
    // iterate through all the entries within the table
    for (size_t i = 0; i < ph->num_table_entries; ++i) {
        TableEntry *ent = &ph->table_list[i];

        // filter out any files
        if (is_entry_a_file(ent->entry)) {
            continue;
        }

        if (func(ent->path, arg)) {
            return PSAR_ERR_HANDLER;
        }
    }

    return PSAR_RESULT_OK;
}

PsarResult psar_read_each_file(PsarHandle *ph, ON_FILE_FUNC func)
{
    // iterate through all the entries within the table
    for (size_t i = 0; i < ph->num_table_entries; ++i) {
        TableEntry *ent = &ph->table_list[i];

        // filter out any directories
        if (!is_entry_a_file(ent->entry)) {
            continue;
        }

        FileProperties properties = {
            .file_type = (memcmp(ent->path, "ipl:", 4) == 0) ? PSAR_FILE_TYPE_IPL : PSAR_FILE_TYPE_NORMAL,
            .personalisation = (ent->entry->header.signcheck == 2) ? PSAR_FILE_REQ_PERSONALISED : PSAR_FILE_NORMAL
        };

        unsigned char *data = NULL;
        size_t length = 0;
    
        PsarResult res = read_file_by_entry(ph, ent->entry, &data, &length);
    
        if (res != PSAR_RESULT_OK) {
            return res;
        }
            
        if (memcmp(ent->path, "ipl:", 4) == 0) {
            if (model_get_identity()->model != PSP_MODEL_01G) {
                res = decrypt_ipl(data, &length);

                if (res != PSAR_RESULT_OK) {
                    return res;
                }
            }
        }

        if (func(ent->path, data, length, &properties)) {
            return PSAR_ERR_HANDLER;
        }
    }

    return PSAR_RESULT_OK;
}