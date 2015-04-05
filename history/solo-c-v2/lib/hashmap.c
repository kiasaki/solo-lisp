#undef NDEBUG
#include <stdint.h>
#include <dbg.h>
#include <hashmap.h>
#include <bstrlib.h>

static int default_compare(void *a, void *b) {
        return bstrcmp((bstring) a, (bstring) b);
}

static uint32_t default_hash(void *a) {
        size_t len = blength((bstring) a);
        char *key = bdata((bstring) a);
        uint32_t hash = 0;
        uint32_t i = 0;

        for(; i < len; i++) {
                hash += key[i];
                hash += (hash << 10);
                hash ^= (hash >> 6);
        }

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash >> 15);
        return hash;
}

Hashmap *Hashmap_create(Hashmap_compare compare, Hashmap_hash hash) {
        Hashmap *map = calloc(1, sizeof(Hashmap));
        check_mem(map);

        map->compare = compare == NULL ? default_compare : compare;
        map->hash = hash == NULL ? default_hash : hash;
        map->buckets = DArray_create(sizeof(DArray *), DEFAULT_NUMBER_OF_BUCKETS);
        check_mem(map->buckets);
        return map;
error:
        if (map) {
                Hashmap_destroy(map);
        }
        return NULL;
}

void Hashmap_destroy(Hashmap *map) {
        
        if (map) {
                if (map->buckets) {
                        for (int i = 0; i < DArray_count(map->buckets); i++) {
                                DArray *bucket = DArray_get(map->buckets, i);
                                if (bucket) {
                                        for (int j = 0; j < DArray_count(bucket); j++) {
                                                free(DArray_get(bucket, j));
                                        }
                                        DArray_destroy(bucket);
                                }
                        }
                        DArray_destroy(map->buckets);
                }
                free(map);
        }

}

static inline HashmapNode *HashmapNode_create(int hash, void *key, void *value) {
        HashmapNode *node = calloc(1, sizeof(HashmapNode));
        check_mem(node);

        node->key = key;
        node->value = value;
        node->hash = hash;
        return node;
error:
        if (node) {
                free(node);
        }
        return NULL;
}

static inline DArray *Hashmap_find_bucket(Hashmap *map, void *key, int create, uint32_t *hash_out) {
        uint32_t hash = map->hash(key);
        int bucket_n = hash % DEFAULT_NUMBER_OF_BUCKETS;

        check(bucket_n >= 0, "Invalid bucket %d found.", bucket_n);

        *hash_out = hash; // store it for return so the caller can use it.

        DArray *bucket = DArray_get(map->buckets, bucket_n);

        if (!bucket && create) {
                bucket = DArray_create(sizeof(void *), DEFAULT_NUMBER_OF_BUCKETS);
                check_mem(bucket);
                DArray_set(map->buckets, bucket_n, bucket);
        }

        return bucket;
error:
        return NULL;
}

int Hashmap_set(Hashmap *map, void *key, void *value) {
        uint32_t hash = 0;
        DArray *bucket = Hashmap_find_bucket(map, key, 1, &hash);
        check_mem(bucket);

        HashmapNode *node = HashmapNode_create(hash, key, value);
        check_mem(node);
        DArray_push(bucket, node);
        return 0;
error:
        return -1;
}

static inline HashmapNode *Hashmap_get_node(Hashmap *map, uint32_t hash, DArray *bucket, void *key) {
        for (int i = 0; i < DArray_end(bucket); i++) {
                debug("TRY: %d", i);
                HashmapNode *node = DArray_get(bucket, i);
                if (node->hash == hash && map->compare(node->key, key) == 0) {
                        return node;
                }

        }
        return NULL;
}

void *Hashmap_get(Hashmap *map, void *key) {
        uint32_t hash = 0;
        DArray *bucket = Hashmap_find_bucket(map, key, 0, &hash);
        check_mem(bucket);

        HashmapNode *node = Hashmap_get_node(map, hash, bucket, key);
        if (node == NULL)  return NULL;
        return node->value;
error:
        return NULL;
}

int Hashmap_traverse(Hashmap *map, Hashmap_traverse_cb cb) {
        int rc = 0;
        int count = DArray_count(map->buckets);
        debug("total buckets_count %d", count);
        for (int i = 0; i < DArray_count(map->buckets); i++) {
                DArray *bucket = DArray_get(map->buckets, i);
                if (bucket) {
                        int buckets_count = DArray_count(bucket);
                        debug("buckets_count %d %d", i, buckets_count);
                        for (int j = 0; j < DArray_count(bucket); j++) {
                                HashmapNode *node = DArray_get(bucket, j);
                                rc = cb(node);
                                debug("traverse %d, %d", i, j);
                                if (rc != 0) return rc;
                        }
                }
        }
        return 0;
}

void *Hashmap_delete(Hashmap *map, void *key) {
        uint32_t hash = 0;
        DArray *bucket = Hashmap_find_bucket(map, key, 0, &hash);
        if (!bucket) return NULL;

        HashmapNode *node = Hashmap_get_node(map, hash, bucket, key);

        void *data = node->value;
        free(node);

        // WHY?
        //HashmapNode *ending = DArray_pop(bucket);
        //if (ending != node) {
        //        DArray_set(bucket, i, ending);
        //}

        return data;
}
