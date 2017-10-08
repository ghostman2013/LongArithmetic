#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#define ARRAY_INC 80

#define SUM '+'
#define SUB '-'
#define MUL '*'
#define DIV '/'
#define NEG 'n'
#define LBR '('
#define RBR ')'

typedef struct BInt {
    int sign;
    unsigned int * mods;
    int size;
} BInt;

typedef struct Token {
    union data {
        char ch;
        char* str;
    } data;
    int size;
} Token;

/*
 * Returns the base of long numbers
 * --------------------------------------------------------------------------------------
 * @return Optimal base for long numbers
 */
short get_base() {
    unsigned int max = UINT_MAX;
    short count = -1;

    while ((max /= 10) != 0) {
        ++count;
    }

    return count;
}

/*
 * Prints an error in default output stream
 */
void print_err() {
    printf("[error]");
}

/*
 * Adds a new element to array
 * ----------------------------------------------------------------------------
 * @param arr Pointer to array pointer
 * @param val Pointer to a new element
 * @param t_size Size of array element
 * @param Pointer to array size
 * @param Pointer to array capacity
 * @return Not 0 if it has errors
 */
int add_element(void** arr, void* val, int t_size, int* size, int* capacity) {

    if ((*size) == (*capacity)) {
        *capacity += ARRAY_INC;
        *arr = realloc(*arr, (*capacity) * t_size);

        if (*arr == NULL) {
            return 1;
        }
    }

    memcpy((char*)(*arr) + (*size) * t_size, (char*)val, t_size);
    ++(*size);

    return 0;
}

/*
 * Returns an inputed line by the user
 * ----------------------------------------------------------------------------
 * @param Pointer to string
 * @param Pointer to string size
 * @return Not 0 if it has errors
 */
int get_line(char** str, int* size) {
    int capacity = ARRAY_INC;
    *size = 0;
    char* s = (char*)malloc(capacity * sizeof(char));
    char ch;

    while (scanf("%c", &ch) == 1
           && !(ch == '\0' || ch == '\n')) {

        if (ch != ' ' && add_element(&s, &ch, sizeof(char), size, &capacity)) {
            return 1;
        }
    }

    *str = (char*)malloc(*size * sizeof(char));
    memcpy(*str, s, *size * sizeof(char));

    free(s);
    s = NULL;

    return 0;
}

/*
 * Prints the string (/substring) in default output stream
 * ----------------------------------------------------------------------------
 * @param Pointer to the start position of the substring
 * @param Pointer to the end position of the substring
 * @return Not 0 if it has errors
 */
int print_arr(char* start, char* end) {

    if (start == NULL || end < start) {
        return 1;
    }

    for (char* i = start; i < end; ++i) {
        printf("%c", *i);
    }

    printf("%c", *end);

    return 0;
}

/*
 * Returns the token array from the inputed string
 * ----------------------------------------------------------------------------
 * @param Pointer to token array
 * @param Pointer to token array size
 * @return Not 0 if it has errors
 */
int scan_tokens(Token** p_tokens, int* size) {
    char* line;
    int len;

    if (get_line(&line, &len)) {
        return 1;
    }

    int capacity = ARRAY_INC;
    *size = 0;
    *p_tokens = (Token*)malloc(capacity * sizeof(Token));
    int start = -1;

    for (int i = 0; i < len; ++i) {
        char ch = line[i];

        if (isdigit(ch)) {

            if(start == -1) {
                start = i;
            }
        } else {
            if (start != -1) {
                Token t;
                int count = i - start;
                t.size = count;
                t.data.str = (char*)malloc(count * sizeof(char));
                memcpy(t.data.str, line + start, count * sizeof(char));
                start = -1;
                add_element(p_tokens, &t, sizeof(Token), size, &capacity);
            }

            Token t;

            switch(ch) {
                case SUM:
                case SUB:
                case MUL:
                case DIV:
                case LBR:
                case RBR:
                    t.size = 0;
                    t.data.ch = ch;
                    add_element(p_tokens, &t, sizeof(Token), size, &capacity);
                    break;
                default:
                    return 1;
            }
        }
    }

    if (start != -1) {
        Token t;
        int count = len - start;
        t.size = count;
        t.data.str = (char*)malloc(count * sizeof(char));
        memcpy(t.data.str, line + start, count * sizeof(char));
        start = -1;
        add_element(p_tokens, &t, sizeof(Token), size, &capacity);
    }

    free(line);
    line = NULL;

    return 0;
}

int print_tokens(Token* start, Token* end) {

    if (start == NULL || end < start) {
        return 1;
    }

    for (Token* i = start; i < end; ++i) {

        if (i->size == 0) {
            printf("%c", i->data.ch);
        } else {
            print_arr(i->data.str, i->data.str + i->size - 1);
        }

        printf("\n");
    }

    if (end->size == 0) {
        printf("%c\n", end->data.ch);
    } else {
        print_arr(end->data.str, end->data.str + end->size - 1);
    }

    return 0;
}

void free_tokens(Token** p_tokens, int size) {
    Token* p = *p_tokens;

    for (int i = 0; i < size; ++i) {

        if (p[i].size > 0) {
            free(p[i].data.str);
        }
    }

    free(p);
    *p_tokens = NULL;
}

int to_int(char* start, char* end, unsigned int* num) {

    if (start == NULL || end < start) {
        return 1;
    }

    *num = 0;
    unsigned int mul = 1;

    for (char* i = end; i >= start; --i) {
        unsigned int d = (*i) - '0';

        if (d > 9) {
            return 1;
        }

        *num += d * mul;
        mul *= 10;
    }

    return 0;
}

BInt* new_bint(Token* token, short base) {
    int size = token->size;
    char* str = token->data.str;

    if (size <= 0 && str == NULL) {
        return NULL;
    }

    BInt* bn = NULL;
    int count = size / base;

    if (size % base != 0) {
        ++count;
    }

    if (count > 1) {
        bn = malloc(sizeof(BInt));
        bn->size = count;
        bn->sign = 1;
        bn->mods = malloc(count * sizeof(unsigned int));

        int old_i = size - 1;

        for (int i = size - base, index = 0; index < count; ++index) {

            if (to_int(str + i, str + old_i, bn->mods + index)) {
                free(bn->mods);
                free(bn);

                break;
            }

            old_i = i - 1;
            i -= base;

            if (i < 0) {
                i = 0;
            }
        }
    } else {
        unsigned int num;

        if (!to_int(str, str + size - 1, &num)) {
            bn = malloc(sizeof(BInt));
            bn->sign = (int)num;
            bn->size = 0;
        }
    }

    return bn;
}

int build_tree(Token* start, Token* end, BInt* tree) {

    if (start == NULL || end < start) {
        return 1;
    }

    typedef struct Rest {
        Token* l;
        Token* r;
    } Rest;

    int capacity = ARRAY_INC;
    int size = 0;
    Rest* rest = malloc(capacity * sizeof(Rest));

    Token* l = start;
    Token* r = end;

    for (Token* i = l; i != r; ++i) {

    }

    free(rest);

    return 0;
}

int main() {
    Token* p_tokens = NULL;
    int size = 0;

    if (scan_tokens(&p_tokens, &size)) {
        print_err();
        return 0;
    }

    short base = get_base();

    print_tokens(p_tokens, p_tokens + size - 1);

    BInt* bn = new_bint(p_tokens, base);

    if (bn != NULL) {
        free(bn);
    }

    free_tokens(&p_tokens, size);

    return 0;
}
