#include "errno.h"
#include "irq.h"
#include "list.h"
#include "stdlib.h"
#include "string.h"

struct ts_irqHandler {
    tf_irqHandler *m_handler;
    void *m_arg;
};

struct ts_listNode *s_handlerList[C_IRQ_MAX];

int irq_init(void) {
    memset(s_handlerList, 0, sizeof(s_handlerList));
}

int irq_service(int p_irq) {
    struct ts_listNode *l_node = s_handlerList[p_irq];

    while(l_node != NULL) {
        struct ts_irqHandler *l_handler =
            (struct ts_irqHandler *)l_node->m_data;

        l_handler->m_handler(l_handler->m_arg);

        l_node = l_node->m_next;
    }

    return 0;
}

int irq_addHandler(int p_irq, tf_irqHandler *p_handler, void *p_arg) {
    struct ts_irqHandler *l_handler = malloc(sizeof(struct ts_irqHandler));

    if(l_handler == NULL) {
        return -ENOMEM;
    }

    l_handler->m_handler = p_handler;
    l_handler->m_arg = p_arg;

    int l_result = list_insertBeginning(&s_handlerList[p_irq], l_handler);

    if(l_result < 0) {
        free(l_handler);
    }

    return l_result;
}

int irq_removeHandler(int p_irq, tf_irqHandler *p_handler, void *p_arg) {
    struct ts_irqHandler l_handler = {
        .m_handler = p_handler,
        .m_arg = p_arg
    };

    struct ts_listNode *l_node = s_handlerList[p_irq];

    while(l_node != NULL) {
        struct ts_listNode *l_nextNode = l_node->m_next;

        if(memcmp(l_node->m_data, &l_handler, sizeof(l_handler)) == 0) {
            list_remove(&s_handlerList[p_irq], l_node->m_data);
        }

        l_node = l_nextNode;
    }

    return 0;
}
