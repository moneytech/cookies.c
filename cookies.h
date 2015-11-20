#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* key;
    size_t      key_length;
    const char* value;
    size_t      value_length;
} cookies_pair_t;

typedef int (*cookie_handler_t)(const cookies_pair_t pair, void *userdata);

int cookies_parse(const char* text, cookie_handler_t handler, void *userdata) {

    const char *pos = text;
    const char *end = text + strlen(text);

    const char *eq_p;
    const char *semi_p;

    cookies_pair_t pair;

    while (pos < end) {

        // Seek to first non-whitespace character
        while (isspace(*pos)) {
            pos++;
        }

        // Find the key
        eq_p = strchr(pos, '=');
        if (!eq_p) {
            return 1;
        }
        pair.key = pos;
        pair.key_length = size_t(eq_p - pair.key);

        // Find the value
        semi_p = strchr(eq_p, ';');
        if (!semi_p) {
            return 1;
        }
        pair.value = eq_p + 1;
        pair.value_length = size_t(semi_p - pair.value - 1);

        // invoke callback
        int res = handler(pair, userdata);
        if (res) {
            return res;
        }

        // move the position to the next key/value pair
        pos = semi_p + 1;
    }

    return 0;
}

typedef struct {
    bool           found;
    const char     *key;
    cookies_pair_t result;
} cookies_lookup_t;

static int cookies_handle_find_key(const cookies_pair_t pair, void *data) {
    cookies_lookup_t *lookup = (cookies_lookup_t*)data;
    if (strncmp(lookup->key, pair.key, pair.key_length) == 0) {
        lookup->found = true;
        lookup->result = pair;
        return 1;
    }
    return 0;
}

void cookies_init_lookup(cookies_lookup_t *lookup, const char* key) {
    lookup->found = false;
    lookup->key = key;
}

char *cookies_lookup(const char* text, const char *key) {
    cookies_lookup_t lookup;
    cookies_init_lookup(&lookup, key);
    cookies_parse(text, cookies_handle_find_key, &lookup);
    if (!lookup.found) {
        return NULL;
    }
    return strndup(lookup.result.value, lookup.result.value_length);
}
