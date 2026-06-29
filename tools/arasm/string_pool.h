#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <arazu/core/context.h>
#include <stdlib.h>
#include <string.h>

#define SP__DEFAULT_CAPACITY     64u
#define SP__LOAD_FACTOR_NUM       3u   /* max load = 3/4 */
#define SP__LOAD_FACTOR_DEN       4u
#define SP__ARENA_INITIAL_BYTES  (16u * 1024u)

typedef struct
{
    const char* ptr;   /* NULL == empty slot */
    unsigned int hash;
} sp__Bucket;

typedef struct sp__Arena
{
    char*            base;
    Arazu_Size       used;
    Arazu_Size       capacity;
    struct sp__Arena* next;
} sp__Arena;

typedef struct
{
    sp__Bucket* buckets;
    Arazu_Size  bucketCount;   /* always a power of two */
    Arazu_Size  entryCount;
    sp__Arena*  arenaHead;
} sp__Data;


static unsigned int sp__hash(const char* s)
{
    unsigned int h = 2166136261u;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h;
}

static sp__Arena* sp__arena_new(Arazu_Size minBytes)
{
    Arazu_Size cap = SP__ARENA_INITIAL_BYTES;
    while (cap < minBytes) cap *= 2u;

    sp__Arena* a = (sp__Arena*)malloc(sizeof(sp__Arena));
    if (!a) return ARAZU_NULL;
    a->base = (char*)malloc((size_t)cap);
    if (!a->base) { free(a); return ARAZU_NULL; }
    a->used     = 0;
    a->capacity = cap;
    a->next     = ARAZU_NULL;
    return a;
}

static const char* sp__arena_push(sp__Arena** head, const char* str, Arazu_Size len)
{
    Arazu_Size needed = len + 1u;
    sp__Arena* a = *head;

    if (!a || a->used + needed > a->capacity)
    {
        sp__Arena* fresh = sp__arena_new(needed);
        if (!fresh) return ARAZU_NULL;
        fresh->next = *head;
        *head = fresh;
        a = fresh;
    }

    char* dst = a->base + a->used;
    memcpy(dst, str, (size_t)len);
    dst[len] = '\0';
    a->used += needed;
    return dst;
}

static void sp__arena_destroy(sp__Arena* head)
{
    while (head) { sp__Arena* n = head->next; free(head->base); free(head); head = n; }
}

static void sp__table_insert(sp__Bucket* buckets, Arazu_Size count,
                              const char* ptr, unsigned int hash)
{
    Arazu_Size mask = count - 1u;
    Arazu_Size idx  = (Arazu_Size)hash & mask;
    for (Arazu_Size i = 0; i < count; ++i)
    {
        if (!buckets[idx].ptr) { buckets[idx].ptr = ptr; buckets[idx].hash = hash; return; }
        idx = (idx + 1u) & mask;
    }
}

static Arazu_Bool sp__grow(sp__Data* d)
{
    Arazu_Size    newCount   = d->bucketCount * 2u;
    sp__Bucket*   newBuckets = (sp__Bucket*)calloc((size_t)newCount, sizeof(sp__Bucket));
    if (!newBuckets) return ARAZU_FALSE;

    for (Arazu_Size i = 0; i < d->bucketCount; ++i)
        if (d->buckets[i].ptr)
            sp__table_insert(newBuckets, newCount, d->buckets[i].ptr, d->buckets[i].hash);

    free(d->buckets);
    d->buckets     = newBuckets;
    d->bucketCount = newCount;
    return ARAZU_TRUE;
}

static Arazu_String sp__intern(Arazu_StringPool* pool, const char* str)
{
    if (!str) return ARAZU_NULL;
    sp__Data*    d    = (sp__Data*)pool->userdata;
    unsigned int hash = sp__hash(str);
    Arazu_Size   mask = d->bucketCount - 1u;
    Arazu_Size   idx  = (Arazu_Size)hash & mask;

    for (Arazu_Size i = 0; i < d->bucketCount; ++i)
    {
        sp__Bucket* b = &d->buckets[idx];
        if (!b->ptr) break;
        if (b->hash == hash && strcmp(b->ptr, str) == 0)
            return (Arazu_String)(Arazu_PointerSize)b->ptr;
        idx = (idx + 1u) & mask;
    }

    Arazu_Size maxLoad = (d->bucketCount * SP__LOAD_FACTOR_NUM) / SP__LOAD_FACTOR_DEN;
    if (d->entryCount >= maxLoad && !sp__grow(d)) return ARAZU_NULL;

    Arazu_Size  len      = (Arazu_Size)strlen(str);
    const char* interned = sp__arena_push(&d->arenaHead, str, len);
    if (!interned) return ARAZU_NULL;

    sp__table_insert(d->buckets, d->bucketCount, interned, hash);
    d->entryCount++;
    return (Arazu_String)(Arazu_PointerSize)interned;
}

static Arazu_Bool sp__compare(Arazu_StringPool* pool, Arazu_String a, Arazu_String b)
{
    (void)pool;
    return (a == b) ? ARAZU_TRUE : ARAZU_FALSE;
}

static const char* sp__to_cstring(Arazu_StringPool* pool, Arazu_String str)
{
    (void)pool;
    return (const char*)(Arazu_PointerSize)str;
}

static void sp__destroy(Arazu_StringPool* pool)
{
    sp__Data* d = (sp__Data*)pool->userdata;
    if (!d) return;
    sp__arena_destroy(d->arenaHead);
    free(d->buckets);
    free(d);
    pool->userdata = ARAZU_NULL;
}

static inline Arazu_StringPool make_string_pool(Arazu_Size initialCapacity)
{
    Arazu_StringPool pool;
    memset(&pool, 0, sizeof(pool));

    if (initialCapacity == 0) initialCapacity = SP__DEFAULT_CAPACITY;

    {
        Arazu_Size v = initialCapacity - 1u;
        v |= v >> 1u; v |= v >> 2u; v |= v >> 4u; v |= v >> 8u; v |= v >> 16u;
#if defined(ARAZU_DETAIL_X86_64) || defined(ARAZU_DETAIL_ARM64)
        v |= v >> 32u;
#endif
        initialCapacity = v + 1u;
    }

    sp__Data* d = (sp__Data*)calloc(1, sizeof(sp__Data));
    if (!d) return pool;

    d->buckets = (sp__Bucket*)calloc((size_t)initialCapacity, sizeof(sp__Bucket));
    if (!d->buckets) { free(d); return pool; }

    d->bucketCount = initialCapacity;
    d->arenaHead   = ARAZU_NULL;

    pool.intern    = sp__intern;
    pool.compare   = sp__compare;
    pool.toCString = sp__to_cstring;
    pool.destroy   = sp__destroy;
    pool.userdata  = d;
    return pool;
}

#endif
