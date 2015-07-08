#include "utils.h"

#define ALPHABET_LEN 256
#define NOT_FOUND patlen
#define MAX_MEMSTR_PATH_LENGTH 30 * sizeof(int)
#define max(a, b) ((a < b) ? b : a)

size_t strlen (const char *str)
{
        return (*str) ? strlen(++str) + 1 : 0;
}

void * memcpy(void * dst, const void * src, size_t len)
{
        char *dst_ptr = (char*)dst;
        char *src_ptr = (char*)src;

        for(int i = 0; i < len; i++)
        {
                dst_ptr[i] = src_ptr[i];
        }

        return dst;
}

void * memset(void * s, int c, size_t n)
{
        int* p=s;

        for(int i = 0; i < n; i+=sizeof(int))
        {
                *p++ = c;
        }

        return s;
}

int strcpy(char *dst, const char *src)
{
        size_t len = strlen(src);
        memcpy(dst, src, len);
        return len;
}

int substr(char *dst, const char *src, int start, size_t len)
{
        size_t srcLen = strlen(src);
        if(start + len > srcLen) len = srcLen - start;
        memcpy(dst, &src[start], len);
        return len;
}

int strcat(char *dst, const char *src, size_t copyLen)
{
    size_t len = strlen(dst);
    memcpy(&dst[len], src, copyLen);
    return copyLen;
}

void make_delta1(int *delta1, char *pat, int patlen) {
        int i;
        for (i=0; i < ALPHABET_LEN; i++) {
                delta1[i] = NOT_FOUND;
        }
        for (i=0; i < patlen-1; i++) {
                delta1[pat[i]] = patlen-1 - i;
        }
}

// true if the suffix of word starting from word[pos] is a prefix
// of word
int is_prefix(char *word, int wordlen, int pos) {
        int i;
        int suffixlen = wordlen - pos;
        // could also use the strncmp() library function here
        for (i = 0; i < suffixlen; i++) {
                if (word[i] != word[pos+i]) {
                        return 0;
                }
        }
        return 1;
}

// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
int suffix_length(char *word, int wordlen, int pos) {
        int i;
        // increment suffix length i to the first mismatch or beginning
        // of the word
        for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i < pos); i++);
        return i;
}

void make_delta2(int *delta2, char *pat, int patlen) {
        int p;
        int last_prefix_index = patlen-1;

        // first loop
        for (p=patlen-1; p>=0; p--) {
                if (is_prefix(pat, patlen, p+1)) {
                        last_prefix_index = p+1;
                }
                delta2[p] = last_prefix_index + (patlen-1 - p);
        }

        // second loop
        for (p=0; p < patlen-1; p++) {
                int slen = suffix_length(pat, patlen, p);
                if (pat[p - slen] != pat[patlen-1 - slen]) {
                        delta2[patlen-1 - slen] = patlen-1 - p + slen;
                }
        }
}

char* memstr (char *string, SceUInt stringlen, char *pat, SceUInt patlen) {
        int i;
        int delta1[ALPHABET_LEN];
        int delta2 [MAX_MEMSTR_PATH_LENGTH];
        make_delta1(delta1, pat, patlen);
        make_delta2(delta2, pat, patlen);

        // The empty pattern must be considered specially
        if (patlen == 0) return string;

        i = patlen-1;
        while (i < stringlen) {
                int j = patlen-1;
                while (j >= 0 && (string[i] == pat[j])) {
                        --i;
                        --j;
                }
                if (j < 0) {
                        return (string + i+1);
                }

                i += max(delta1[string[i]], delta2[j]);
        }
        return NULL;
}
