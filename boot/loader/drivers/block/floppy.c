#include <stdbool.h>

#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/arch/x86/isr.h"
#include "boot/loader/drivers/block/block.h"
#include "boot/loader/drivers/block/floppy.h"
#include "boot/loader/drivers/cmos.h"
#include "boot/loader/stdio.h"
#include "boot/loader/string.h"

#define C_FDC_CMOS_MAX_DRIVE_COUNT 2
#define C_FDC_MAX_DRIVE_COUNT 4
#define C_FDC_IO_BASE 0x3f0
#define C_FDC_IRQ 6

#define C_FDC_IOPORT_DOR 2
#define C_FDC_IOPORT_MSR 4
#define C_FDC_IOPORT_DATA 5
#define C_FDC_IOPORT_CCR 7
#define C_FDC_IOPORT_DIR 7

#define C_FDC_DOR_ENABLED (1 << 2)
#define C_FDC_DOR_DMA_IRQ (1 << 3)
#define C_FDC_DOR_MOTOR_0 (1 << 4)
#define C_FDC_DOR_MOTOR_1 (1 << 5)
#define C_FDC_DOR_MOTOR_2 (1 << 6)
#define C_FDC_DOR_MOTOR_3 (1 << 7)

#define C_FDC_MSR_BUSY (1 << 4)
#define C_FDC_MSR_DMA (1 << 5)
#define C_FDC_MSR_DIO (1 << 6)
#define C_FDC_MSR_RQM (1 << 7)

#define C_FDC_DSR_DATARATE_1MBPS 3
#define C_FDC_DSR_DATARATE_500KBPS 0
#define C_FDC_DSR_DATARATE_250KBPS 1
#define C_FDC_DSR_DATARATE_300KBPS 2
#define C_FDC_MOTOR_DELAY_5_25 500
#define C_FDC_MOTOR_DELAY_3_5 300

#define C_FDC_CMD_SPECIFY 0x03
#define C_FDC_CMD_READ_SECTOR 0x06
#define C_FDC_CMD_CALIBRATE 0x07
#define C_FDC_CMD_CHECK_INT 0x08
#define C_FDC_CMD_SEEK 0x0f
#define C_FDC_CMD_CONFIGURE 0x13

#define C_FDC_CMD_OPTION_MF 0x40

struct ts_floppyDriveType {
    unsigned int cylinders;
    unsigned int heads;
    unsigned int sectors;
    unsigned int turnOnMotorDelay;
    unsigned int dataRateSetting;
    unsigned int stepRateTime;
    unsigned int headLoadTime;
    unsigned int headUnloadTime;
    uint64_t sectorCount;
};

static const struct ts_floppyDriveType s_floppyDriveTypes[] = {
    // 0x00: No drive
    {
        .cylinders = 0,
        .heads = 0,
        .sectors = 0,
        .turnOnMotorDelay = 0,
        .dataRateSetting = 0,
        .stepRateTime = 0,
        .headLoadTime = 0,
        .headUnloadTime = 0,
        .sectorCount = 0
    },
    // 0x01: 5.25" 360KB
    {
        .cylinders = 40,
        .heads = 2,
        .sectors = 9,
        .turnOnMotorDelay = C_FDC_MOTOR_DELAY_5_25,
        .dataRateSetting = C_FDC_DSR_DATARATE_300KBPS,
        .stepRateTime = 11,
        .headLoadTime = 9,
        .headUnloadTime = 9,
        .sectorCount = 720
    },
    // 0x02: 5.25" 1.2MB
    {
        .cylinders = 80,
        .heads = 2,
        .sectors = 15,
        .turnOnMotorDelay = C_FDC_MOTOR_DELAY_5_25,
        .dataRateSetting = C_FDC_DSR_DATARATE_500KBPS,
        .stepRateTime = 8,
        .headLoadTime = 15,
        .headUnloadTime = 15,
        .sectorCount = 2400
    },
    // 0x03: 3.5" 720KB
    {
        .cylinders = 80,
        .heads = 2,
        .sectors = 9,
        .turnOnMotorDelay = C_FDC_MOTOR_DELAY_3_5,
        .dataRateSetting = C_FDC_DSR_DATARATE_250KBPS,
        .stepRateTime = 12,
        .headLoadTime = 8,
        .headUnloadTime = 8,
        .sectorCount = 1440
    },
    // 0x04: 3.5" 1.44MB
    {
        .cylinders = 80,
        .heads = 2,
        .sectors = 18,
        .turnOnMotorDelay = C_FDC_MOTOR_DELAY_3_5,
        .dataRateSetting = C_FDC_DSR_DATARATE_500KBPS,
        .stepRateTime = 8,
        .headLoadTime = 15,
        .headUnloadTime = 15,
        .sectorCount = 2880
    },
    // 0x05: 3.5" 2.88MB
    {
        .cylinders = 80,
        .heads = 2,
        .sectors = 36,
        .turnOnMotorDelay = C_FDC_MOTOR_DELAY_3_5,
        .dataRateSetting = C_FDC_DSR_DATARATE_1MBPS,
        .stepRateTime = 0,
        .headLoadTime = 30,
        .headUnloadTime = 15,
        .sectorCount = 5760
    }
};

struct ts_fdc;

struct ts_floppyDrive {
    struct ts_fdc *fdc;
    int driveIndex;
    bool present;
    int type;
};

struct ts_fdc {
    uint16_t ioBase;
    int irq;
    volatile bool irqOccurred;
    int currentSetting;
    uint8_t dor;

    struct ts_floppyDrive drives[C_FDC_MAX_DRIVE_COUNT];
};

static struct ts_fdc s_fdc;

static int fdc_init(struct ts_fdc *p_fdc, uint16_t p_ioBase, int p_irq);
static int fdd_init(struct ts_floppyDrive *p_fdd);
static void fdc_reset(struct ts_fdc *p_fdc);
static void fdd_specify(struct ts_floppyDrive *p_drive);
static int fdd_seek(struct ts_floppyDrive *p_drive, unsigned int p_cylinder);
static void fdc_interrupt(struct ts_isrRegisters *p_registers, void *p_arg);
static void fdc_waitIrq(struct ts_fdc *p_fdc);
static void fdc_writeData(struct ts_fdc *p_fdc, uint8_t p_value);
static uint8_t fdc_readData(struct ts_fdc *p_fdc);
static void fdc_checkInt(struct ts_fdc *p_fdc, uint8_t *p_st0, uint8_t *p_cyl);
static int fdd_calibrate(struct ts_floppyDrive *p_drive);
static ssize_t fdd_read(struct ts_block *p_block, lba_t p_lba, void *p_buffer);
static void fdc_setDor(struct ts_fdc *p_fdc, uint8_t p_dor);
static uint8_t fdc_getDor(struct ts_fdc *p_fdc);
static void fdc_startMotor(struct ts_fdc *p_fdc, int p_motor);
static void fdc_stopMotor(struct ts_fdc *p_fdc, int p_motor);
static void fdd_startMotor(struct ts_floppyDrive *p_fdd);
static void fdd_stopMotor(struct ts_floppyDrive *p_fdd);

int floppy_init(void) {
    // Initialize fdc structure
    memset(&s_fdc, 0, sizeof(s_fdc));

    // Get drive info from CMOS
    uint8_t l_driveInfo = cmos_read(C_CMOS_REGISTER_FLOPPY);

    unsigned int l_driveTypes[] = {
        l_driveInfo >> 4U,
        l_driveInfo & 0x0fU
    };

    for(int l_drive = 0; l_drive < C_FDC_CMOS_MAX_DRIVE_COUNT; l_drive++) {
        int l_driveType = l_driveTypes[l_drive];

        if(l_driveType > 6) {
            // Invalid or unsupported drives
            l_driveType = 0;
        }

        // Initialize drive
        s_fdc.drives[l_drive].type = l_driveType;
        s_fdc.drives[l_drive].present = l_driveType != 0;
        s_fdc.drives[l_drive].fdc = &s_fdc;
        s_fdc.drives[l_drive].driveIndex = l_drive;
    }

    int l_returnValue = fdc_init(&s_fdc, C_FDC_IO_BASE, C_FDC_IRQ);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    // Initialize drives
    for(int l_drive = 0; l_drive < C_FDC_CMOS_MAX_DRIVE_COUNT; l_drive++) {
        if(s_fdc.drives[l_drive].present) {
            l_returnValue = fdd_init(&s_fdc.drives[l_drive]);
        }
    }

    return 0;
}

static int fdc_init(struct ts_fdc *p_fdc, uint16_t p_ioBase, int p_irq) {
    printf("floppy: Initializing controller...\n");

    p_fdc->ioBase = p_ioBase;
    p_fdc->irq = p_irq;
    p_fdc->irqOccurred = false;
    p_fdc->dor = 0xff; // Invalid value

    // Request IRQ
    isr_setHandlerIrq(p_irq, fdc_interrupt, p_fdc);

    // Reset controller
    fdc_reset(p_fdc);

    // Configure
    fdc_writeData(p_fdc, C_FDC_CMD_CONFIGURE);
    fdc_writeData(p_fdc, 0);
    fdc_writeData(p_fdc, 0x48);
    fdc_writeData(p_fdc, 0);

    return 0;
}

static int fdd_init(struct ts_floppyDrive *p_fdd) {
    printf("fdd_init: Initializing drive %d\n", p_fdd->driveIndex);

    // Calibrate drive
    fdd_specify(p_fdd);
    
    int l_returnValue = fdd_calibrate(p_fdd);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    struct ts_block *l_block = block_alloc();

    if(l_block == NULL) {
        return -1;
    }

    memset(l_block, 0, sizeof(*l_block));

    sprintf(l_block->name, "floppy%d", p_fdd->driveIndex);
    l_block->driverData = p_fdd;
    l_block->read = fdd_read;

    return block_register(l_block);
}

static void fdc_reset(struct ts_fdc *p_fdc) {
    printf("floppy: Resetting controller...\n");

    // Reset controller
    fdc_setDor(p_fdc, 0x00);
    fdc_setDor(p_fdc, C_FDC_DOR_ENABLED);
    fdc_waitIrq(p_fdc);

    printf("floppy: Controller reset complete.\n");

    // Send CHECK_INT to all drives
    for(int l_drive = 0; l_drive < 2; l_drive++) {
        uint8_t l_st0;
        uint8_t l_cyl;

        if(p_fdc->drives[l_drive].present) {
            fdc_checkInt(p_fdc, &l_st0, &l_cyl);
        }
    }
}

static void fdd_specify(struct ts_floppyDrive *p_drive) {
    if(p_drive->fdc->currentSetting == p_drive->type) {
        return;
    }

    printf("floppy: Sending SPECIFY command...\n");

    fdc_writeData(p_drive->fdc, C_FDC_CMD_SPECIFY);
    fdc_writeData(
        p_drive->fdc,
        (s_floppyDriveTypes[p_drive->type].stepRateTime << 4)
            | s_floppyDriveTypes[p_drive->type].headUnloadTime
    );
    fdc_writeData(
        p_drive->fdc,
        (s_floppyDriveTypes[p_drive->type].headLoadTime << 1) | 1
    );
    
    p_drive->fdc->currentSetting = p_drive->type;
}

static int fdd_seek(struct ts_floppyDrive *p_drive, unsigned int p_cylinder) {
    fdc_writeData(p_drive->fdc, C_FDC_CMD_SEEK);
    fdc_writeData(p_drive->fdc, p_drive->driveIndex);
    fdc_writeData(p_drive->fdc, p_cylinder);
    fdc_waitIrq(p_drive->fdc);

    uint8_t l_st0;
    uint8_t l_cyl;

    fdc_checkInt(p_drive->fdc, &l_st0, &l_cyl);

    if(l_cyl == p_cylinder) {
        return 0;
    } else {
        return 1;
    }
}

static void fdc_interrupt(
    struct ts_isrRegisters *p_registers,
    void *p_arg
) {
    printf("fdc_interrupt()\n");

    struct ts_fdc *l_fdc = (struct ts_fdc *)p_arg;

    (void)p_registers;

    l_fdc->irqOccurred = true;
}

static void fdc_waitIrq(struct ts_fdc *p_fdc) {
    while(!p_fdc->irqOccurred) {
        asm("hlt");
    }

    p_fdc->irqOccurred = false;
}

static void fdc_writeData(struct ts_fdc *p_fdc, uint8_t p_value) {
    while((inb(p_fdc->ioBase + C_FDC_IOPORT_MSR) & C_FDC_MSR_RQM) == 0);

    outb(p_fdc->ioBase + C_FDC_IOPORT_DATA, p_value);
}

static uint8_t fdc_readData(struct ts_fdc *p_fdc) {
    while((inb(p_fdc->ioBase + C_FDC_IOPORT_MSR) & C_FDC_MSR_RQM) == 0);

    return inb(p_fdc->ioBase + C_FDC_IOPORT_DATA);
}

static void fdc_checkInt(
    struct ts_fdc *p_fdc,
    uint8_t *p_st0,
    uint8_t *p_cyl
) {
    outb(p_fdc->ioBase + C_FDC_IOPORT_DATA, C_FDC_CMD_CHECK_INT);

    *p_st0 = fdc_readData(p_fdc);
    *p_cyl = fdc_readData(p_fdc);
}

static int fdd_calibrate(struct ts_floppyDrive *p_drive) {
    uint8_t l_st0;
    uint8_t l_cyl;
    int l_returnValue = -1;

    printf("floppy: Sending CALIBRATE command...\n");

    // Turn on the motor
    fdd_startMotor(p_drive);

    for(int l_attempt = 0; l_attempt < 10; l_attempt++) {
        printf("floppy: Calibrate attempt %d\n", l_attempt);
        fdc_writeData(p_drive->fdc, C_FDC_CMD_CALIBRATE);
        fdc_writeData(p_drive->fdc, p_drive->driveIndex);
        fdc_waitIrq(p_drive->fdc);
        fdc_checkInt(p_drive->fdc, &l_st0, &l_cyl);

        printf("Current cylinder: %d\n", l_cyl);

        if(l_cyl == 0) {
            l_returnValue = 0;
            break;
        }
    }

    // Turn off the motor
    fdd_stopMotor(p_drive);

    return l_returnValue;
}

static ssize_t fdd_read(
    struct ts_block *p_block,
    lba_t p_lba,
    void *p_buffer
) {
    printf("fdd_read: called\n");

    struct ts_floppyDrive *l_fdd =
        (struct ts_floppyDrive *)p_block->driverData;
    const struct ts_floppyDriveType *l_type = &s_floppyDriveTypes[l_fdd->type];

    printf("fdd_read: type=0x%02x\n", l_fdd->type);

    if(p_lba > l_type->sectorCount) {
        return -1;
    }

    unsigned int l_temp = p_lba / l_type->sectors;
    unsigned int l_sector = (p_lba % l_type->sectors) + 1;
    unsigned int l_head = l_temp % l_type->heads;
    unsigned int l_cylinder = l_temp / l_type->heads;

    fdd_specify(l_fdd);

    printf("fdd_read: SPECIFY done.\n");

    // Turn on the motor
    fdd_startMotor(l_fdd);

    printf("floppy: Motor turned on.\n");

    fdc_writeData(l_fdd->fdc, C_FDC_CMD_READ_SECTOR | C_FDC_CMD_OPTION_MF);
    fdc_writeData(l_fdd->fdc, (l_head << 2) | l_fdd->driveIndex);
    fdc_writeData(l_fdd->fdc, l_cylinder);
    fdc_writeData(l_fdd->fdc, l_head);
    fdc_writeData(l_fdd->fdc, l_sector);
    fdc_writeData(l_fdd->fdc, 2); // 512 bytes per sector
    fdc_writeData(l_fdd->fdc, l_type->sectors);
    fdc_writeData(l_fdd->fdc, 27); // GAP1
    fdc_writeData(l_fdd->fdc, 0xff); // 512 bytes per sector

    printf("floppy: Sent READ_SECTOR command.\n");

    fdc_waitIrq(l_fdd->fdc);
    
    printf("fdd_read: Reading 512 bytes...\n");

    for(int l_i = 0; l_i < 512; l_i++) {
        printf("%02x ", fdc_readData(l_fdd->fdc));
    }

    printf("\n");

    for(int l_i = 0; l_i < 7; l_i++) {
        printf("%02x ", fdc_readData(l_fdd->fdc));
    }

    printf("\n");

    // TODO
    return 0;
}

static void fdc_setDor(struct ts_fdc *p_fdc, uint8_t p_dor) {
    if(p_fdc->dor != p_dor) {
        outb(p_fdc->ioBase + C_FDC_IOPORT_DOR, p_dor);
        p_fdc->dor = p_dor;
    }
}

static uint8_t fdc_getDor(struct ts_fdc *p_fdc) {
    return p_fdc->dor;
}

static void fdc_startMotor(struct ts_fdc *p_fdc, int p_motor) {
    fdc_setDor(p_fdc, fdc_getDor(p_fdc) | (C_FDC_DOR_MOTOR_0 << p_motor));
}

static void fdc_stopMotor(struct ts_fdc *p_fdc, int p_motor) {
    fdc_setDor(p_fdc, fdc_getDor(p_fdc) & ~(C_FDC_DOR_MOTOR_0 << p_motor));
}

static void fdd_startMotor(struct ts_floppyDrive *p_fdd) {
    fdc_startMotor(p_fdd->fdc, p_fdd->driveIndex);
}

static void fdd_stopMotor(struct ts_floppyDrive *p_fdd) {
    fdc_stopMotor(p_fdd->fdc, p_fdd->driveIndex);
}
