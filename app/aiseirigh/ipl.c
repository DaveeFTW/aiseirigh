#include "ipl.h"
#include "log.h"

#include <model.h>
#include <emcsm.h>
#include <string.h>

typedef struct {
    uint8_t unk0;
    uint8_t unk1;
    uint16_t xor_id;
    uint32_t tag;
    uint32_t ecc;
} IplSpareData;

int clear_ipl(void)
{
        // TODO: constants!
    for (size_t block = 0; block < 0x30; ++block)
    {
        // TODO: constants!
        size_t ppn = block*32;
        int res = emcsm_erase_block(ppn);
    }

    return 0;
}

int write_ipl(const void *data, size_t length)
{
    // TODO: replace with calls?
    size_t ppb = 32;
    size_t page_size = 512;

    if (!length) {
        return -1;
    }

    // check if IPL can fit in the designated region
    if (length > 32 * ppb * page_size) {
        return -1;
    }

    uint16_t xor_id = 0xFFFF;

    switch (model_get_identity()->model) {
        case PSP_MODEL_01G:
        case PSP_MODEL_02G:
            xor_id = 0;
            break;
        case PSP_MODEL_03G:
        case PSP_MODEL_04G:
        case PSP_MODEL_07G:
        case PSP_MODEL_09G:
        case PSP_MODEL_11G:
            xor_id = 1;
            break;
        case PSP_MODEL_05G:
            xor_id = 2;
            break;
    }

    LOG("writing IPL with xor id: %02X", xor_id);

    // calculate the number of pages the IPL will consume
    size_t num_ipl_pages = (length / page_size) + (length % page_size ? (1) : (0));

    LOG("num ipl pages %i", num_ipl_pages);

    // TODO: constants
    unsigned char block_data[32 * 512];
    IplSpareData spare[32];
    uint16_t block_index[32];
    memset(block_index, 0, sizeof(block_index));
    
    // write all the IPL pages to the NAND start at block 16. if we reach block 48
    // without writing all the pages then we are in a VERY bad situation and
    // cannot continue
    for (size_t block_num = 16, num_written_pages = 0; num_written_pages < num_ipl_pages; num_written_pages += ppb) {
        // get the next non-bad block
        while (1) {
            if (block_num >= 48) {
                return -1;
            }

            if (emcsm_is_bad_block(block_num * ppb) == 0) {
                break;
            }

            block_num += 1;
        }

        // write IPL pages and fill empty pages with all FFs
        for (size_t i = 0; i < ppb; ++i) {
            if (num_written_pages + i < num_ipl_pages) {
                memcpy(&block_data[i * page_size], data + (num_written_pages + i) * page_size, page_size);
                memset(&spare[i], 0xFF, sizeof(IplSpareData));
                spare[i].xor_id = xor_id;
                spare[i].tag = 0x6DC64A38;
            } else {
                memset(&block_data[i * page_size], 0xFF, page_size);
                memset(&spare[i], 0xFF, sizeof(IplSpareData));
            }
        }

        block_index[num_written_pages/ppb] = block_num;
        LOG("writing IPL chunk to block %i. num written pages: %i", block_num, num_written_pages);
        int res = emcsm_write_block_with_verify(block_num * ppb, block_data, spare);

        if (res < 0) {
            return -1;
        }

        block_num += 1;
    }

    memset(block_data, 0, sizeof(block_data));
    memcpy(block_data, block_index, sizeof(block_index));
    memset(spare, 0xFF, sizeof(spare));

    spare[0].xor_id = xor_id;
    spare[0].tag = 0x6DC64A38;

    int written = 0;

    // TODO: constants
    for (size_t block_num = 4; block_num < 12; ++block_num) {
        if (emcsm_is_bad_block(block_num * ppb)) {
            continue;
        }

        LOG("writing block index %i", block_num);
        emcsm_write_block_with_verify(block_num * ppb, block_data, spare);
        written = 1;
    }


    if (!written) {
        return -1;
    }

    return 0;
}
