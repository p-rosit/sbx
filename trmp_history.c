#ifndef SBXP_HISTORY_H
#define SBXP_HISTORY_H

#include <stdlib.h>
#include <string.h>

#include "sbxp_data.c"

void sbxp_free_history(sbxp_history_t* history);
void sbxp_free_history_item(sbxp_history_item_t* item);

int sbxp_history_size(sbxp_history_t*);
sbxp_history_item_t* sbxp_new_history_item(sbxp_history_t*);
sbxp_history_item_t* sbxp_get_history_item(sbxp_history_t*, int);


void sbxp_free_history(sbxp_history_t* history) {
    sbxp_history_item_t* item;

    for (int i = 0; i < sbxp_history_size(history); i++) {
        item = sbxp_get_history_item(history, i);   
        sbxp_free_history_item(item);
    }
    free(history->items);
}

void sbxp_free_history_item(sbxp_history_item_t* item) {
    free(item->function_name);
    free(item->command);
}

sbxp_history_item_t* sbxp_new_history_item(sbxp_history_t* history) {
    history->last_index = (history->last_index + 1) % history->size;

    if (history->first_index == history->last_index) {
        sbxp_free_history_item(&history->items[history->last_index]);
        history->first_index = (history->first_index + 1) % history->size;
    }

    if (history->first_index == -1 && history->last_index != -1) {
        history->first_index = 0;
    }

    return &history->items[history->last_index];
}

sbxp_history_item_t* sbxp_get_history_item(sbxp_history_t* history, int index) {
    int size;
    if (index < 0) {
        return NULL;
    }

    size = sbxp_history_size(history);

    if (index > size - 1) {
        return NULL;
    }

    index = history->last_index - index;
    if (index < 0) {
        index += size;
    }

    return &history->items[index];
}

int sbxp_history_size(sbxp_history_t* history) {
    if (history->last_index == -1) {
        return 0;
    } else if ((history->last_index + 1) % history->size != history->first_index) {
        return history->last_index - history->first_index + 1;
    } else {
        return history->size;
    }
}

#endif

