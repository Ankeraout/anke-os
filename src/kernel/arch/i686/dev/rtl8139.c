#include <stdbool.h>

#include "irq.h"
#include "arch/i686/io.h"
#include "arch/i686/pci.h"
#include "arch/i686/mm/pmm.h"
#include "arch/i686/mm/vmm.h"
#include "dev/net/eth.h"
#include "dev/net/netif.h"
#include "libk/stdio.h"
#include "libk/stdlib.h"
#include "libk/string.h"

typedef struct {
    netif_t netif;
    uint16_t io_base;
    uint8_t *rxBuffer_v;
    uint8_t *rxBuffer_p;
    uint8_t *txBuffer_v;
    uint8_t *txBuffer_p;
    volatile bool rts;
} rtl8139_t;

int rtl8139_init(const pci_dev_t *dev);
static void rtl8139_irq_handler(rtl8139_t *rtl);
static void rtl8139_registerInterface(rtl8139_t *rtl);
static void rtl8139_api_getLinkAddress(netif_t *netif, netif_addr_t *linkAddress);
static int rtl8139_api_send(netif_t *netif, const void *buffer, size_t size);
static int rtl8139_api_setLinkAddress(netif_t *netif, const netif_addr_t *linkAddress);

static netif_api_t rtl8139_api = {
    .send = rtl8139_api_send,
    .setLinkAddress = rtl8139_api_setLinkAddress,
    .getLinkAddress = rtl8139_api_getLinkAddress
};

int rtl8139_init(const pci_dev_t *dev) {
    // Allocate netif
    rtl8139_t *rtl = malloc(sizeof(rtl8139_t));

    rtl->rts = true;

    // Find I/O space address
    rtl->io_base = pci_csam_read32(dev, 0x10) & 0xfffffffc;
    printf("rtl8139: I/O base address: %#08x\n", rtl->io_base);

    // Allocate receive buffer
    rtl->rxBuffer_p = pmm_alloc(3);
    rtl->rxBuffer_v = vmm_map(rtl->rxBuffer_p, 3, true);

    printf("rtl8139: receive buffer physical address: %#08x\n", (uint32_t)rtl->rxBuffer_p);
    printf("rtl8139: receive buffer virtual address: %#08x\n", (uint32_t)rtl->rxBuffer_v);

    // Allocate send buffer
    rtl->txBuffer_p = pmm_alloc(2);
    rtl->txBuffer_v = vmm_map(rtl->txBuffer_p, 2, true);

    printf("rtl8139: transmit buffer physical address: %#08x\n", (uint32_t)rtl->txBuffer_p);
    printf("rtl8139: transmit buffer virtual address: %#08x\n", (uint32_t)rtl->txBuffer_v);

    // Get IRQ number
    uint8_t irq = pci_csam_read8(dev, 0x3f);

    // Register IRQ
    irq_register(irq, rtl8139_irq_handler, rtl);

    printf("rtl8139: registered IRQ %d\n", irq);

    // Enable bus mastering
    uint16_t commandRegister = pci_csam_read16(dev, 6);

    commandRegister |= (1 << 2);

    pci_csam_write16(dev, 6, commandRegister);

    // Start
    outb(rtl->io_base + 0x52, 0);

    // Reset
    outb(rtl->io_base + 0x37, 0x10);
    while(inb(rtl->io_base + 0x37) & 0x10);

    // Configure IMR + ISR
    outw(rtl->io_base + 0x3c, 0x0005);

    // Configure receive buffer
    outl(rtl->io_base + 0x30, (uint32_t)rtl->rxBuffer_p);
    outb(rtl->io_base + 0x44, 0x8f);

    // Enable receiver and transmitter
    outb(rtl->io_base + 0x37, 0x0c);

    // Register interface
    rtl->netif.type = NETIF_ETHERNET;
    rtl8139_api_getLinkAddress(&rtl->netif, &rtl->netif.link_addr);
    memset(&rtl->netif.net_addr, 0, sizeof(netif_addr_t));
    rtl->netif.api = rtl8139_api;

    netif_register((netif_t *)rtl);

    return 0;
}

static void rtl8139_irq_handler(rtl8139_t *rtl) {
    printf("rtl8139: interrupt\n");

    uint16_t isr = inw(rtl->io_base + 0x3e);

    if(isr & (1 << 2)) {
        rtl->rts = true;
        outw(rtl->io_base + 0x3e, (1 << 2));
    } else {
        // Packet received
        outw(rtl->io_base + 0x3e, (1 << 0));
    }
}

static void rtl8139_api_getLinkAddress(netif_t *netif, netif_addr_t *linkAddress) {
    rtl8139_t *rtl = (rtl8139_t *)netif;
    
    for(int i = 0; i < 6; i++) {
        linkAddress->mac[i] = inb(rtl->io_base + i);
    }
}

static int rtl8139_api_send(netif_t *netif, const void *buffer, size_t size) {
    rtl8139_t *rtl = (rtl8139_t *)netif;

    while(!rtl->rts) {

    }

    rtl->rts = false;

    memcpy(rtl->txBuffer_v, buffer, size);
    outl(rtl->io_base + 0x20, rtl->txBuffer_p);
    outl(rtl->io_base + 0x10, size);
}

static int rtl8139_api_setLinkAddress(netif_t *netif, const netif_addr_t *linkAddress) {
    rtl8139_t *rtl = (rtl8139_t *)netif;
}
