/**
 * @file fentry.c
 * @date 3 Nov 2021
 * @brief Linux file entry utility
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <sys/stat.h>

#include "fentry.h"

struct fentry_t* fentry_new(const char* file_name, const struct stat* file_stat) {
    if ((file_name == NULL) || (file_stat == NULL)) {
        return NULL;
    }

    struct fentry_t* new_entry = (struct fentry_t*) malloc(sizeof(struct fentry_t));
    new_entry->file_name = (char*) malloc((strlen(file_name) + 1) * sizeof(char));
    strcpy(new_entry->file_name, file_name);
    memcpy(&new_entry->file_stat, file_stat, sizeof(struct stat));

    return new_entry;
}

struct fentry_t* fentry_clone(const struct fentry_t* other) {
    if ((other == NULL) || (other->file_name == NULL)) {
        return NULL;
    }

    struct fentry_t* new_entry = (struct fentry_t*) malloc(sizeof(struct fentry_t));
    new_entry->file_name = (char*) malloc((strlen(other->file_name) + 1) * sizeof(char));
    strcpy(new_entry->file_name, other->file_name);
    memcpy(&new_entry->file_stat, &other->file_stat, sizeof(struct stat));

    return new_entry;
}

void fentry_drop(struct fentry_t** self) {
    if ((self == NULL) || (*self == NULL)) {
        return;
    }

    free((*self)->file_name);
    free(*self);
    *self = NULL; 
}

bool fentry_equals(const struct fentry_t* a, const struct fentry_t* b) {
    if ((a == NULL) || (b == NULL)) {
        return false;
    }
    
    const struct stat* a_stat = &a->file_stat;
    const struct stat* b_satt = &b->file_stat;

    const bool are_equal = (a_stat->st_size == b_satt->st_size)  // Size
        && (a_stat->st_mtime == b_satt->st_mtime);               // Modification time

    return are_equal;
}

struct fentry_set_t* fentry_set_new() {
    struct fentry_set_t* new_set = (struct fentry_set_t*) malloc(sizeof(struct fentry_set_t));
    new_set->cap = FENTRY_VEC_DEFAULT_CAP;
    new_set->len = 0;
    new_set->buffer = (struct fentry_t**) malloc(new_set->cap * sizeof(struct fentry_t*));
    for (size_t i = 0; i < new_set->cap; i++) {
        new_set->buffer[i] = NULL;
    }

    return new_set;
}

struct fentry_set_t* fentry_set_clone(const struct fentry_set_t* other) {
    if ((other == NULL) || (other->buffer == NULL)) {
        return fentry_set_new();
    }
    struct fentry_set_t* new_set = (struct fentry_set_t*) malloc(sizeof(struct fentry_set_t));
    new_set->cap = other->len;
    new_set->len = other->len;
    new_set->buffer = (struct fentry_t**) malloc(new_set->cap * sizeof(struct fentry_t*));

    for (size_t i = 0; i < new_set->len; i++) {
        new_set->buffer[i] = fentry_clone(other->buffer[i]);
    }

    return new_set;
}

void fentry_set_drop(struct fentry_set_t** self) {
    if ((self == NULL) || (*self == NULL)) {
        return;
    }

    for (size_t i = 0; i < (*self)->len; i++) {
        fentry_drop(&(*self)->buffer[i]);
    }
    free((*self)->buffer);
}

void fentry_set_insert(struct fentry_set_t* self, struct fentry_t* entry) {
    if ((self == NULL) || (entry == NULL)) {
        return;
    }

    const struct fentry_t* twin_entry = fentry_set_get(self, entry->file_name);

    if (twin_entry != NULL) {
        /* Ignore if already exists */
        fentry_drop(&entry);
    } else {
        /* Insert new entry if not exists */
        if (self->len == self->cap) {
            self->cap += FENTRY_VEC_DEFAULT_CAP;
            self->buffer = (struct fentry_t**) realloc(self->buffer, self->cap * sizeof(struct fentry_t*));
        }

        self->buffer[self->len++] = entry;
    }
}

void fentry_set_remove(struct fentry_set_t* self, const char* file_name) {
    if ((self == NULL) || (file_name == 0)) {
        return;
    }

    for (size_t i = 0; i < self->len; i++) {
        struct fentry_t* entry = self->buffer[i];

        if (strcmp(entry->file_name, file_name) == 0) {
            fentry_drop(&entry);
            for (size_t j = i; j < self->len; j++) {
                self->buffer[j] = self->buffer[j + 1]; 
            }

            self->buffer[self->len - 1] = NULL;
            self->len--;
            break;
        }
    }
}

bool fentry_set_contains(const struct fentry_set_t* self, const char* file_name) {
    return fentry_set_get(self, file_name) != NULL;
}

const struct fentry_t* fentry_set_get(const struct fentry_set_t* self, const char* file_name) {
    if ((self == NULL) || (file_name == NULL)) {
        return NULL;
    }

    for (size_t i = 0; i < self->len; i++) {
        const struct fentry_t* entry = self->buffer[i];

        if (strcmp(entry->file_name, file_name) == 0) {
            return entry;
        }
    }

    return NULL;
}

struct fentry_t* fentry_set_pop(struct fentry_set_t* self){
    if ((self == NULL) || (self->len == 0)) {
        return NULL;
    }

    struct fentry_t* entry = self->buffer[self->len - 1];
    self->buffer[--self->len] = NULL;
    return entry;
}

bool fentry_set_is_empty(const struct fentry_set_t* self) {
    return (self != NULL) ? (self->len == 0) : false;
}

struct fentry_set_t* fentry_set_diff(const struct fentry_set_t* a, const struct fentry_set_t* b) {
    if ((a == NULL) || (b == NULL)) {
        return NULL;
    }

    const bool a_is_empty = fentry_set_is_empty(a);
    const bool b_is_empty = fentry_set_is_empty(b);

    if (a_is_empty) {
        return fentry_set_new();
    } else if (b_is_empty) {
        return fentry_set_clone(a);
    }

    struct fentry_set_t* diff_set = fentry_set_new();

    for (size_t i = 0; i < a->len; i++) {
        if (!fentry_set_contains(b, a->buffer[i]->file_name)) {
            fentry_set_insert(diff_set, fentry_clone(a->buffer[i]));
        }
    }

    return diff_set;
}

size_t fentry_set_len(const struct fentry_set_t* self) {
    return (self != NULL) ? self->len : 0;
}
