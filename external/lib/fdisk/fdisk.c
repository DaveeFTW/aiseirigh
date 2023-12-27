#include "lib/fdisk.h"

#include <lflash.h>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    size_t cylinder;
    size_t head;
    size_t sector;
} CylinderHeadSector;

typedef struct __attribute__((packed)) {
    uint8_t h;
    uint8_t cs;
    uint8_t c8;
} CylinderHeadSectorAddress;

typedef struct __attribute__((packed)) {
    uint8_t boot_indicator;
    CylinderHeadSectorAddress chs_part_start;
    uint8_t part_type;
    CylinderHeadSectorAddress chs_part_end;
    uint32_t lba_part_start;
    uint32_t part_size;
} PartitionEntry;

typedef struct __attribute__((packed)) {
    uint8_t unused[0x1BE];
    PartitionEntry partitions[4];
    uint16_t signature; // 0xAA55
} BootRecord;

typedef struct {
    size_t sector_lba;
    BootRecord record;
} Partition;

typedef struct {
    size_t max_cylinders;
    size_t heads_per_cylinder;
    size_t sectors_per_track;
} DiskGeometry;

typedef struct {
    const DiskGeometry *geometry;
    size_t disk_size;
    size_t next_offset;
    size_t alignment;
    size_t ebr_base;
    size_t num_partitions;
    Partition partitions[16];
} FdiskState;

const DiskGeometry g_assumed_geometry = {
    .max_cylinders = 1023,
    .heads_per_cylinder = 2,
    .sectors_per_track = 32
};

// TODO: this should be something in the nand driver?
#define SECTOR_SIZE (512)

static FdiskResult lba_to_chs(size_t lba, CylinderHeadSector *out_chs, const DiskGeometry *geometry)
{
    size_t cylinder = lba / (geometry->heads_per_cylinder * geometry->sectors_per_track);
    size_t head = (lba / geometry->sectors_per_track) % geometry->heads_per_cylinder;
    size_t sector = (lba % geometry->sectors_per_track) + 1;

    // check if this LBA can be represented in the CHS format
    if (cylinder > geometry->max_cylinders) {
        return FDISK_ERR_LBA_OUT_OF_CHS_RANGE;
    }

    out_chs->cylinder = cylinder;
    out_chs->head = head;
    out_chs->sector = sector;

    return FDISK_RESULT_OK;
}

static CylinderHeadSectorAddress chs_to_chs_addr(const CylinderHeadSector *chs)
{
    CylinderHeadSectorAddress chs_addr;

    chs_addr.h = chs->head;
    chs_addr.c8 = chs->cylinder & 0xFF;
    chs_addr.cs = (chs->sector & 0x3F) | (((chs->cylinder >> 8) & 0x3) << 6);

    return chs_addr;
}

static void build_ebr_part_entry(PartitionEntry *pte, size_t type, size_t lba, size_t ebr_base, size_t size, const DiskGeometry *geometry)
{
    CylinderHeadSector chs_start, chs_end;

    if (lba_to_chs(lba, &chs_start, geometry) == FDISK_ERR_LBA_OUT_OF_CHS_RANGE) {
        chs_start.cylinder = geometry->max_cylinders;
        chs_start.head = geometry->heads_per_cylinder;
        chs_start.sector = geometry->sectors_per_track;
    }

    if (lba_to_chs(lba, &chs_end, geometry) == FDISK_ERR_LBA_OUT_OF_CHS_RANGE) {
        chs_end.cylinder = geometry->max_cylinders;
        chs_end.head = geometry->heads_per_cylinder;
        chs_end.sector = geometry->sectors_per_track;
    }

    // LBA addressing, EBR partition
    pte->part_type = type;
    pte->chs_part_start = chs_to_chs_addr(&chs_start);
    pte->chs_part_end = chs_to_chs_addr(&chs_end);
    pte->lba_part_start = lba - ebr_base;
    pte->part_size = size;
    printf("writing partition type 0x%X, lba: %08X, len: %08X\n", type, lba - ebr_base, size);
}

static void align_next(FdiskState *state, size_t size)
{
    state->next_offset += size;
    size_t remainder = state->next_offset % state->alignment;

    if (remainder) {
        state->next_offset += state->alignment - remainder;
    }
}

static void build_mbr(FdiskState *state, BootRecord *record)
{
    memset(record, 0, sizeof(*record));
    build_ebr_part_entry(&record->partitions[0], 0xF, 0x40, 0, state->disk_size - 0x40, state->geometry);
    record->signature = 0xAA55;
    align_next(state, 0x40);
}

static void add_partition(FdiskState *state, size_t type, size_t size)
{
    if (state->num_partitions == 0) {    
        // first partition is the EBR base
        align_next(state, 0);
        state->ebr_base = state->next_offset;
    } else {
        // add a chain from the previous partition record
        Partition *prev_part = &state->partitions[state->num_partitions - 1];
        align_next(state, 0);
        build_ebr_part_entry(&prev_part->record.partitions[1], 0xF, state->next_offset, state->ebr_base, size, state->geometry);
    }

    memset(&state->partitions[state->num_partitions].record, 0, sizeof(BootRecord));
    state->partitions[state->num_partitions].record.signature = 0xAA55;
    state->partitions[state->num_partitions].sector_lba = state->next_offset;
    printf("partition %i located at lba: %X\n", state->num_partitions, state->next_offset);
    build_ebr_part_entry(&state->partitions[state->num_partitions].record.partitions[0], type, state->alignment, 0, size - state->alignment, state->geometry);
    align_next(state, size);
    state->num_partitions += 1;
}

FdiskResult fdisk(size_t f0_size, size_t f1_size, size_t f2_size, size_t f3_size)
{
    FdiskState *state = calloc(1, sizeof(FdiskState));

    state->geometry = &g_assumed_geometry;
    state->disk_size = lflash_get_size();
    state->alignment = 32; // TODO: replace with block size

    BootRecord mbr;
    build_mbr(state, &mbr);

    // TODO: more specific error codes?
    if (lflash_write_sectors(0, &mbr, 1, NULL) != 0) {
        free(state);
        return FDISK_ERR_WRITE;
    }

    // add partitions
    add_partition(state, 0xE, f0_size);
    add_partition(state, 0xE, f1_size);
    add_partition(state, 0xE, f2_size);
    add_partition(state, 0xE, f3_size);

    for (size_t i = 0; i < state->num_partitions; ++i) {
        // TODO: more specific error codes?
        if (lflash_write_sectors(state->partitions[i].sector_lba, &state->partitions[i].record, 1, NULL) != 0) {
            free(state);
            return FDISK_ERR_WRITE;
        }
    }

    lflash_sync();
    free(state);
    return FDISK_RESULT_OK;
}
