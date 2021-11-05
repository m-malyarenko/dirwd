/**
 * @file fentry.h
 * @date 3 Nov 2021
 * @brief Linux file entry utility
 */

#ifndef __UTIL_FENTRY_H__
#define __UTIL_FENTRY_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/stat.h>

/* Define -------------------------------------------------------------------*/

#define FENTRY_VEC_DEFAULT_CAP ((size_t) 8)

/* Constants ----------------------------------------------------------------*/

/* Structures ---------------------------------------------------------------*/

struct fentry_t {
    char* file_name;
    struct stat file_stat;
};

struct fentry_set_t {
    size_t cap;
    size_t len;
    struct fentry_t** buffer;
};

/* Function definitions -----------------------------------------------------*/

struct fentry_t* fentry_new(const char* file_name, const struct stat* file_stat);

struct fentry_t* fentry_clone(const struct fentry_t* other);

void fentry_drop(struct fentry_t** self);

bool fentry_equals(const struct fentry_t* a, const struct fentry_t* b);


struct fentry_set_t* fentry_set_new();

struct fentry_set_t* fentry_set_clone(const struct fentry_set_t* other);

void fentry_set_drop(struct fentry_set_t** self);

void fentry_set_insert(struct fentry_set_t* self, struct fentry_t* entry);

void fentry_set_remove(struct fentry_set_t* self, const char* file_name);

bool fentry_set_contains(const struct fentry_set_t* self, const char* file_name);

const struct fentry_t* fentry_set_get(const struct fentry_set_t* self, const char* file_name);

struct fentry_t* fentry_set_pop(struct fentry_set_t* self);

bool fentry_set_is_empty(const struct fentry_set_t* self);

size_t fentry_set_len(const struct fentry_set_t* self);

/* a / b */
struct fentry_set_t* fentry_set_diff(const struct fentry_set_t* a, const struct fentry_set_t* b);

#endif /* __UTIL_FENTRY_H__ */