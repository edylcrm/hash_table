#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "hash_table.h"
#include "prime.c"

int HT_PRIME_1 = 163;
int HT_PRIME_2 = 199;

int HT_INITIAL_BASE_SIZE = 57;

static ht_item HT_DELETED_ITEM = {NULL, NULL};

static ht_item *ht_new_item(const char *key, const char *value) {
    ht_item *item = malloc(sizeof(ht_item));
    item->key = strdup(key);
    item->value = strdup(value);

    return item;
}

static void ht_del_item(ht_item *item) {
    free(item->key);
    free(item->value);
    free(item);
}

static int ht_hash(const char *string, const int a, const int m) {
    long hash = 0;
    const int len = strlen(string);
    for (int i = 0; i < len; i++) {
        hash += (long) pow(a, len - (i + 1)) * string[i];
        hash = hash % m;
    }
    return (int) hash;
}

static int ht_get_hash(const char *s, const int num_buckets, const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

void ht_del_hash_table(ht_hash_table *table) {
    for (int i = 0; i < table->size; ++i) {
        ht_item *item = table->items[i];
        if (item != NULL) {
            ht_del_item(item);
        }
    }

    free(table->items);
    free(table);
}

ht_hash_table *ht_new_hash_table(const int base_size) {
    ht_hash_table *table = malloc(sizeof(ht_hash_table));

    table->base_size = base_size;
    table->size = next_prime(base_size);

    table->count = 0;
    table->items = calloc((size_t) table->size, sizeof(ht_item));

    return table;
}

static void ht_resize(ht_hash_table *ht, const int new_size) {
    if (new_size < HT_INITIAL_BASE_SIZE) {
        return;
    }

    ht_hash_table *new_ht = ht_new_hash_table(new_size);

    for (int i = 0; i < ht->size; i++) {
        ht_item *current_item = ht->items[i];
        if (current_item != NULL && current_item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, current_item->key, current_item->value);
        }
    }

    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    const int tmp_size = ht->size;
    ht_item **tmp_items = ht->items;

    ht->items = new_ht->items;
    ht->size = new_ht->size;

    new_ht->items = tmp_items;
    new_ht->size = tmp_size;

    ht_del_hash_table(new_ht);
}

static void ht_resize_up(ht_hash_table *ht) {
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

static void ht_resize_down(ht_hash_table *ht) {
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

void ht_insert(ht_hash_table *ht, const char *key, const char *value) {
    const int load = ht->count * 100 / ht->size;

    if (load > 70) {
        ht_resize_up(ht);
    }

    ht_item *item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->size, 0);

    ht_item *current_item = ht->items[index];

    int i = 1;

    if (current_item != NULL && current_item != &HT_DELETED_ITEM) {
        if (strcmp(current_item->key, key) == 0) {
            ht_del_item(current_item);
            ht->items[index] = item;
            return;
        }
        index = ht_get_hash(item->key, ht->size, i);
        current_item = ht->items[index];
        i++;
    }

    ht->items[index] = item;
    ht->count++;
}

char *ht_search(ht_hash_table *ht, const char *key) {
    int index = ht_get_hash(key, ht->size, 0);
    ht_item *current_item = ht->items[index];

    int i = 1;
    while (current_item != NULL) {
        if (current_item != &HT_DELETED_ITEM) {
            if (strcmp(current_item->key, key) == 0) {
                return current_item->value;
            }
        }

        index = ht_get_hash(key, ht->size, i);
        current_item = ht->items[index];
        i++;
    }

    return NULL;
}

void ht_delete(ht_hash_table *ht, const char *key) {
    const int load = ht->count * 100 / ht->size;

    if (load < 10) {
        ht_resize_down(ht);
    }

    int index = ht_get_hash(key, ht->size, 0);
    ht_item *current_item = ht->items[index];

    int i = 1;
    while (current_item != NULL) {
        if (current_item != &HT_DELETED_ITEM) {
            if (strcmp(current_item->key, key)) {
                ht_del_item(current_item);
                ht->items[index] = &HT_DELETED_ITEM;
            }
        }

        index = ht_get_hash(key, ht->count, i);
        current_item = ht->items[index];
        i++;
    }
    ht->count--;
}

int main() {
    ht_hash_table *ht = ht_new_hash_table(HT_INITIAL_BASE_SIZE);
    char *k = "some-key";
    char *v = "some-value";
    ht_insert(ht, k, v);
    char *r = ht_search(ht, k);

    printf("%d", ht->count);

    char *k1 = "some-key";
    char *v1 = "second-value";
    ht_insert(ht, k1, v1);
    char *r1 = ht_search(ht, k1);

    printf("%d", ht->count);

    ht_delete(ht, k);

    printf("%d", ht->count);

    return 0;
}