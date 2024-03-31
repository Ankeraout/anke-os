#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/drivers/block/pata/pata.h"
#include "klibc/debug.h"
#include "klibc/stdlib.h"

#define C_PATA_MAX_CHANNELS 4

enum te_pataCommand {
    E_PATA_COMMAND_IDENTIFY = 0xec
};

struct ts_pataDevice {
    bool m_present;
};

struct ts_pataChannel {
    bool m_present;
    int m_id;
    struct ts_pataChannelOperations *m_operations;
    struct ts_pataDevice m_devices[2];
};

static void pataProbeChannel(struct ts_pataChannel *p_channel);
static void pataProbeDevice(struct ts_pataChannel *p_channel, int p_device);

static struct ts_pataChannel s_pataChannels[C_PATA_MAX_CHANNELS];

int pataInit(void) {
    for(int l_i = 0; l_i < C_PATA_MAX_CHANNELS; l_i++) {
        s_pataChannels[l_i].m_present = false;
    }

    return 0;
}

int pataRegisterController(struct ts_pataChannelOperations *p_operations) {
    int l_i = 0;

    while(s_pataChannels[l_i].m_present && l_i < C_PATA_MAX_CHANNELS) {
        l_i++;
    }

    if(l_i == C_PATA_MAX_CHANNELS) {
        // TODO: change the error code
        return -1;
    }

    s_pataChannels[l_i].m_id = l_i;
    s_pataChannels[l_i].m_operations = p_operations;
    s_pataChannels[l_i].m_devices[0].m_present = false;
    s_pataChannels[l_i].m_devices[1].m_present = false;
    s_pataChannels[l_i].m_present = true;

    kernelDebug("pata: Registered new channel with ID %d.\n", l_i);

    pataProbeChannel(&s_pataChannels[l_i]);

    return l_i;
}

static void pataProbeChannel(struct ts_pataChannel *p_channel) {
    kernelDebug("pata: Probing channel %d.\n", p_channel->m_id);
    pataProbeDevice(p_channel, 0);
    pataProbeDevice(p_channel, 1);
}

static void pataProbeDevice(struct ts_pataChannel *p_channel, int p_device) {
    kernelDebug(
        "pata: Channel %d: Probing drive %d.\n",
        p_channel->m_id,
        p_device
    );
}
