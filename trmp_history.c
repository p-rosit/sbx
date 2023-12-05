#ifndef TRMP_HISTORY_H
#define TRMP_HISTORY_H

#include <stdlib.h>
#include <string.h>

#include "trmp_data.c"

void trmp_free_history(trmp_history_t* history);
void trmp_free_history_item(trmp_history_item_t* item);

int trmp_history_size(trmp_history_t*);
trmp_history_item_t* trmp_new_history_item(trmp_history_t*);
trmp_history_item_t* trmp_get_history_item(trmp_history_t*, int);


void trmp_free_history(trmp_history_t* history) {
    trmp_history_item_t* item;

    for (int i = 0; i < trmp_history_size(history); i++) {
        item = trmp_get_history_item(history, i);   
        trmp_free_history_item(item);
    }
    free(history->items);
}

void trmp_free_history_item(trmp_history_item_t* item) {
    free(item->function_name);
    free(item->command);
}

trmp_history_item_t* trmp_new_history_item(trmp_history_t* history) {
    history->last_index = (history->last_index + 1) % history->size;

    if (history->first_index == history->last_index) {
        trmp_free_history_item(&history->items[history->last_index]);
        history->first_index = (history->first_index + 1) % history->size;
    }

    if (history->first_index == -1 && history->last_index != -1) {
        history->first_index = 0;
    }

    return &history->items[history->last_index];
}

trmp_history_item_t* trmp_get_history_item(trmp_history_t* history, int index) {
    int size;
    if (index < 0) {
        return NULL;
    }

    size = trmp_history_size(history);

    if (index > size - 1) {
        return NULL;
    }

    index = history->last_index - index;
    if (index < 0) {
        index += size;
    }

    return &history->items[index];
}

int trmp_history_size(trmp_history_t* history) {
    if (history->last_index == -1) {
        return 0;
    } else if ((history->last_index + 1) % history->size != history->first_index) {
        return history->last_index - history->first_index + 1;
    } else {
        return history->size;
    }
}

#endif

