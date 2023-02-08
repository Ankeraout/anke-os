#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "dev/parallel.h"
#include "debug.h"

static bool parallelIsBusy(struct ts_devParallel *p_dev);
static void parallelWaitBusy(struct ts_devParallel *p_dev);
static void parallelWaitAck(struct ts_devParallel *p_dev);

int parallelInit(struct ts_devParallel *p_dev) {
    outb(p_dev->a_basePort + 2, 0x0c);
    return 0;
}

void parallelSend(struct ts_devParallel *p_dev, uint8_t p_value) {
    parallelWaitBusy(p_dev);

    outb(p_dev->a_basePort, p_value);
    outb(p_dev->a_basePort + 2, 0x0d);

    parallelWaitAck(p_dev);

    outb(p_dev->a_basePort + 2, 0x0c);
}

uint8_t parallelReceive(struct ts_devParallel *p_dev) {
    return inb(p_dev->a_basePort);
}

static bool parallelIsBusy(struct ts_devParallel *p_dev) {
    return (inb(p_dev->a_basePort + 1) & 0x80) == 0;
}

static void parallelWaitBusy(struct ts_devParallel *p_dev) {
    while(parallelIsBusy(p_dev));
}

static void parallelWaitAck(struct ts_devParallel *p_dev) {
    while((inb(p_dev->a_basePort + 1) & 0x40) == 0);
}
