#include "string.h"
#include "task.h"

struct ts_task_context g_task_currentTaskContext;

void task_save(struct ts_task_context *p_context) {
    memcpy(
        p_context,
        &g_task_currentTaskContext,
        sizeof(struct ts_task_context)
    );
}

void task_load(struct ts_task_context *p_context) {
    memcpy(
        &g_task_currentTaskContext,
        p_context,
        sizeof(struct ts_task_context)
    );
}
