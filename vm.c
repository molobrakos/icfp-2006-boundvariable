#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <byteswap.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

static int verbosity = 0;
static int echo = 0;

#define VERBOSITY_INFO 1<<1
#define VERBOSITY_DEBUG 1<<2

inline static void info(const char* format, ...) {
    if (!(verbosity & VERBOSITY_INFO))
        return;
    fprintf(stderr, "> ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/*
   --------------------------------------------------------------------------------------
   MEMORY MANAGEMENT
   --------------------------------------------------------------------------------------
*/

#define MAX_ALLOC_SIZE ((1<<24) - 1) // 16MB items per block


typedef struct {
    uint32_t* buf;
    uint32_t size:24; /* size of buffer in items */
    uint32_t len:24;  /* number of actual items in array */
    uint32_t use:1;
} mem_t;

static mem_t* mem = NULL;
static uint32_t mem_size = 0;

static inline size_t byte_size(uint32_t len) {
    return sizeof(uint32_t) * len;
}

static void mem_resize_buf(uint32_t id, uint32_t size) {
    assert(byte_size(size) <= MAX_ALLOC_SIZE);

    mem[id].size = size;

    if (size) {
        mem[id].buf = realloc(mem[id].buf, byte_size(size));
        assert(mem[id].buf);
    } else {
        free(mem[id].buf);
        mem[id].buf = NULL;
    }
}

static void mem_heap_grow(uint32_t new_size) {
    assert(new_size != 0 || mem_size != 0);

    // 0 means just double the current size
    if (new_size == 0 && mem_size != 0)
        new_size = mem_size * 2;

    mem = realloc(mem, new_size * sizeof(mem_t));
    assert(mem);

    for (uint32_t i = mem_size; i<new_size; i++) {
        mem[i].use = 0;
        mem[i].len = 0;
        mem[i].size = 0;
        mem[i].buf = NULL;
        mem_resize_buf(i, 0); /* change to preallocate space */
    }

    mem_size = new_size;
}

static void mem_init() {
    info("Overhead of each memory block: %d bytes", sizeof(mem_t));
    mem = NULL;
    mem_size = 0;
    mem_heap_grow(1);
}

static void mem_cleanup() {
    for (uint32_t i = 0; i<mem_size; i++)
        if (mem[i].size && mem[i].buf)
            free(mem[i].buf);
    free(mem);
}

static void mem_ensure(int id, uint32_t size) {
    if (size > mem[id].size)
        mem_resize_buf(id, size);
}

static uint32_t candidate = 0;

static void mem_free(uint32_t id) {
    assert(id != 0);
    assert(mem[id].use);
    mem_resize_buf(id, 0);
    mem[id].len = 0;
    mem[id].use = 0;
    candidate = id;
}

static uint32_t mem_find_free_item() {
    for (unsigned i = 0; i < mem_size; i++) {
        if (!mem[candidate].use) {
            return candidate;
        }
        candidate++;
        candidate %= mem_size;
    }

    candidate = mem_size;
    mem_heap_grow(0); /* 0 means double heap (number of platters) */

    return candidate;
}

static void mem_alloc_at(uint32_t idx, uint32_t len) {
    mem_ensure(idx, len);

    mem[idx].len = len;
    mem[idx].use = 1;
    memset(mem[idx].buf, 0, byte_size(mem[idx].len));
}

static uint32_t mem_alloc(uint32_t len) {
    int idx = mem_find_free_item(len);
    assert(idx != 0);
    mem_alloc_at(idx, len);
    return idx;
}

static inline uint32_t* mem_get_buf(uint32_t id, uint32_t offset) {
    assert(id < mem_size);

    mem_t item = mem[id];

    assert(item.use);
    assert(offset < MAX_ALLOC_SIZE);
    assert(offset < item.size);
    assert(offset < item.len);

    return &item.buf[offset];
}

static inline uint32_t mem_get(uint32_t id, uint32_t offset) {
    return *(mem_get_buf(id, offset));
}

static inline void mem_set(uint32_t id, uint32_t offset, uint32_t val) {
    *(mem_get_buf(id, offset)) = val;
}

/*
   --------------------------------------------------------------------------------------
   PROGRAM
   --------------------------------------------------------------------------------------
*/

static void swap_byte_order(uint32_t* buf, size_t size) {
    while (size--) {
        *buf = __bswap_32(*buf);
        buf++;
    }
}

static void load(const char* fname) {
    info("Loading program %s", fname);

    FILE* fp = fopen(fname, "r");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    long byte_size = ftell(fp);
    assert(byte_size > 0);
    assert(byte_size % 4 == 0);
    rewind(fp);

    uint32_t len = byte_size / sizeof(uint32_t);

    mem_alloc_at(0, len);

    size_t res = fread(mem[0].buf, 4, len, fp);
    res = res;
    assert(res == len);
    swap_byte_order(mem[0].buf, len);
    fclose(fp);
    info("Loaded program %s (%d/%x items, %d bytes) into memory", fname, len, len, byte_size);
}

static void load_program(uint32_t id) {
    if (id == 0)
        return;

    assert(mem[id].use);
    assert(mem[id].len);

    mem_ensure(0, mem[id].len);

    assert(mem[0].size >= mem[id].size);

    memcpy(mem[0].buf, mem[id].buf, byte_size(mem[id].len));
    mem[0].len = mem[id].len;
    mem[0].use = 1;
}

/*
   --------------------------------------------------------------------------------------
   MAIN
   --------------------------------------------------------------------------------------
*/

static uint32_t reg[]  = {0,0,0,0,0,0,0,0};
static uint32_t ip = 0;

enum {
    MOV = 0,
    IDX = 1,
    AMEND = 2,
    ADD = 3,
    MUL = 4,
    DIV = 5,
    NAND = 6,
    HALT = 7,
    ALLOC = 8,
    FREE = 9,
    OUTP = 10,
    INP = 11,
    LOAD = 12,
    ORTO = 13,
};

void print_state(uint32_t ip) {

    static uint32_t instr_count = 0;

    static const char* STR_OP[] = {
        "mov", "get", "set", "add", "mul", "div", "nand",
        "halt", "allo", "del", "out", "in", "load", "orto"
    };

    uint32_t instr = mem[0].buf[ip];
    uint32_t op = instr;
    uint32_t C = op & 0x7;
    op >>= 3;
    uint32_t B = op & 0x7;
    op >>= 3;
    uint32_t A = op & 0x7;
    op >>= 3;
    op >>= 19;

    fprintf(stderr,
            "%08x %08x %08x>%08x "
            "[%02d|%4s] ",
            instr_count++, mem[0].len, ip, mem[0].buf[ip],
            op, STR_OP[op]);

    if (op == ORTO) {
        uint32_t val = instr & 0x1ffffff;
        A = (instr >> 25) & 0x7;
        fprintf(stderr,
                "|%01x|     "
                "|%08x|          %07x| ",
                A, reg[A], val);
    } else {
        fprintf(stderr,
                "|%01x|%01x|%01x| "
                "|%08x|%08x|%08x| ",
                A, B, C,
                reg[A], reg[B], reg[C]);
    }

    fprintf(stderr,
            "|%08x|%08x|%08x|%08x"
            "|%08x|%08x|%08x|%08x|\n",
            reg[0], reg[1], reg[2], reg[3],
            reg[4], reg[5], reg[6], reg[7]);
}


int run() {
    info("Running");

    uint32_t instr;
    uint32_t A, B, C;
    uint32_t op;
    int ch;

    while (1) {

        assert(ip < mem[0].len);
        instr = mem[0].buf[ip];

        op = instr;

        C = op & 0x7;
        op >>= 3;

        B = op & 0x7;
        op >>= 3;

        A = op & 0x7;
        op >>= 3;

        op >>= 19;

#if DEBUG
        if (verbosity & VERBOSITY_DEBUG)
            print_state(ip);
#endif

        ip++;

        switch (op) {
        case MOV:
            if (reg[C] != 0)
                reg[A] = reg[B];
            break;
        case IDX:
            reg[A] = mem_get(reg[B], reg[C]);
            break;
        case AMEND:
            mem_set(reg[A], reg[B], reg[C]);
            break;
        case ADD:
            reg[A] = reg[B] + reg[C];
            break;
        case MUL:
            reg[A] = reg[B] * reg[C];
            break;
        case DIV:
            assert(reg[C] != 0);
            reg[A] = reg[B] / reg[C];
            break;
        case NAND:
            reg[A] = ~(reg[B] & reg[C]);
            break;
        case HALT:
            fflush(stdout);
            mem_cleanup();
            info("Halted");
            exit(EXIT_SUCCESS);
            break;
        case ALLOC:
            reg[B] = mem_alloc(reg[C]);
            break;
        case FREE:
            mem_free(reg[C]);
            break;
        case OUTP:
            assert(reg[C] <= 255);
            putchar(reg[C]);
            break;
        case INP:
            ch = getchar();
	    if (echo)
		putchar(ch);
            assert(ch == EOF || ch <= 255);
            reg[C] = (ch == EOF ? 0xffffffff : (uint32_t)ch);
            break;
        case LOAD:
            ip = reg[C];
            load_program(reg[B]);
            break;
        case ORTO:
            A = (instr >> 25) & 0x7;
            B = instr & 0x1ffffff;
            reg[A] = B;
            break;
        default:
            fprintf(stderr, "invalid opcode\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv) {
    const char* fname = "sandmark.umz";
    int opt;
    while ((opt = getopt(argc, argv, "dve")) != -1) {
        switch (opt) {
        case 'd': verbosity |= VERBOSITY_DEBUG; break;
        case 'v': verbosity |= VERBOSITY_INFO; break;
        case 'e': echo = 1; break;
        default:
            printf("Usage: %s [-d] [-v] [-e] [file]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        fname = argv[optind];

    info("Starting VM");
    mem_init();

    load(fname);
    run();
}
