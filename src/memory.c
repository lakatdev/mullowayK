#include <memory.h>

void* memcpy(void *dest, const void *src, unsigned int size)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; size != 0; size--) *dp++ = *sp++;
    return dest;
}

void* memset(void *dest, char val, int count)
{
    char *temp = (char *)dest;
    for(; count != 0; count--) *temp++ = val;
    return dest;
}

unsigned int strlen(const char *str)
{
    unsigned int len = 0;
    while (*str++) len++;
    return len;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int memcmp(const void *str1, const void *str2, unsigned int size)
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    for (; size != 0; size--) {
        if (*s1 != *s2) {
            return (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    return 0;
}

char* strtok_r(char* str, const char* delim, char** saveptr)
{
    char* token;
    if (str == (void*)0) {
        str = *saveptr;
    }

    while (*str) {
        int is_delim = 0;
        for (const char* d = delim; *d; ++d) {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
        }
        if (!is_delim) break;
        ++str;
    }
    if (*str == '\0') {
        *saveptr = str;
        return (void*)0;
    }
    token = str;

    while (*str) {
        int is_delim = 0;
        for (const char* d = delim; *d; ++d) {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
        }
        if (is_delim) {
            *str = '\0';
            ++str;
            break;
        }
        ++str;
    }
    *saveptr = str;
    return token;
}

char* strchr(const char* s, int c)
{
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : (void*)0;
}

char* strcpy(char* dest, const char* src)
{
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, unsigned int n)
{
    unsigned int i = 0;
    for (; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

int isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}

long strtol(const char* nptr, char** endptr, int base)
{
    const char* s = nptr;
    long acc = 0;
    int neg = 0, any = 0, c;

    while (isspace(*s)) s++;

    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16; s += 2;
    }
    else if (base == 0 && s[0] == '0') {
        base = 8; s++;
    }
    else if (base == 0) {
        base = 10;
    }

    while (1) {
        c = *s;
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
        else break;
        if (digit >= base) break;
        acc = acc * base + digit;
        any = 1;
        s++;
    }
    if (any == 0) s = nptr;
    if (endptr) *endptr = (char*)s;
    return neg ? -acc : acc;
}

float strtof(const char* nptr, char** endptr)
{
    float result = 0.0f, sign = 1.0f, frac = 0.1f;
    int seen_dot = 0;
    const char* s = nptr;

    while (isspace(*s)) s++;

    if (*s == '-') {
        sign = -1.0f; s++;
    }
    else if (*s == '+') {
        s++;
    }

    while ((*s >= '0' && *s <= '9') || *s == '.') {
        if (*s == '.') {
            if (seen_dot) break;
            seen_dot = 1;
            s++;
            continue;
        }
        if (!seen_dot) {
            result = result * 10.0f + (*s - '0');
        }
        else {
            result = result + (*s - '0') * frac;
            frac *= 0.1f;
        }
        s++;
    }

    if (*s == 'e' || *s == 'E') {
        s++;
        int exp_sign = 1, exp = 0;
        if (*s == '-') {
            exp_sign = -1; s++;
        }
        else if (*s == '+') {
            s++;
        }
        while (*s >= '0' && *s <= '9') {
            exp = exp * 10 + (*s - '0');
            s++;
        }
        float exp_factor = 1.0f;
        for (int i = 0; i < exp; i++) {
            exp_factor *= 10.0f;
        }
        if (exp_sign == 1) {
            result *= exp_factor;
        }
        else {
            result /= exp_factor;
        }
    }

    if (endptr) *endptr = (char*)s;
    return result * sign;
}

void* memmove(void* dest, const void* src, int n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d < s) {
        for (int i = 0; i < n; i++) {
            d[i] = s[i];
        }
    }
    else if (d > s) {
        for (int i = n - 1; i >= 0; i--) {
            d[i] = s[i];
        }
    }
    return dest;
}
