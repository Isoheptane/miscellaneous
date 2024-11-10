#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

/*
    (64, 57) Hamming-Code
    0, 1, 2, 4, 8, 16, 32
*/

#define __HARDWARE_POPCNT__

#ifdef __HARDWARE_POPCNT__
//  https://github.com/kimwalisch/libpopcnt/blob/master/libpopcnt.h#L190
static inline uint64_t popcnt64(uint64_t x) {
    __asm__ ("popcnt %1, %0" : "=r" (x) : "0" (x));
    return x;
}
#else
const uint64_t m1  = 0x5555555555555555; //binary: 0101...
const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t m8  = 0x00ff00ff00ff00ff; //binary:  8 zeros,  8 ones ...
const uint64_t m16 = 0x0000ffff0000ffff; //binary: 16 zeros, 16 ones ...
const uint64_t m32 = 0x00000000ffffffff; //binary: 32 zeros, 32 ones
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...
static inline uint64_t popcnt64(uint64_t x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}
#endif

static inline uint64_t generate_rand (uint64_t* state) {
    uint64_t x = *state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return *state = x;
}

const static uint64_t masks[] = {
    0xAAAAAAAAAAAAAAAA,
    0xCCCCCCCCCCCCCCCC,
    0xF0F0F0F0F0F0F0F0,
    0xFF00FF00FF00FF00,
    0xFFFF0000FFFF0000,
    0xFFFFFFFF00000000,
};

// Reposition data
static inline uint64_t reposition(uint64_t x) {
    return 
        (x & (0x00000001ull <<  0)) << 3 |   // Skip 0, 1, 2
        (x & (0x00000007ull <<  1)) << 4 |   // Skip 4
        (x & (0x0000007Full <<  4)) << 5 |   // Skip 8
        (x & (0x00007FFFull << 11)) << 6 |   // Skip 16
        (x & (0x7FFFFFFFull << 26)) << 7;    // Skip 32
}

// Un-reposition data
static inline uint64_t unreposition(uint64_t x) {
    return
        (x & (0x00000001ull <<  3)) >> 3 |
        (x & (0x00000007ull <<  5)) >> 4 |
        (x & (0x0000007Full <<  9)) >> 5 |
        (x & (0x00007FFFull << 17)) >> 6 |
        (x & (0x7FFFFFFFull << 33)) >> 7;
}

// Set parity
static inline uint64_t set_parity(uint64_t x) {
    x ^= ((popcnt64(x & masks[0]) & 1) << (1ull << 0)); // Set 1
    x ^= ((popcnt64(x & masks[1]) & 1) << (1ull << 1)); // Set 2
    x ^= ((popcnt64(x & masks[2]) & 1) << (1ull << 2)); // Set 4
    x ^= ((popcnt64(x & masks[3]) & 1) << (1ull << 3)); // Set 8
    x ^= ((popcnt64(x & masks[4]) & 1) << (1ull << 4)); // Set 16
    x ^= ((popcnt64(x & masks[5]) & 1) << (1ull << 5)); // Set 32
    x ^= popcnt64(x) & 1; // Set 0 (extend bit)
    return x;
}

// Find position
static inline uint64_t find_error_position(uint64_t x) {
    return
        (popcnt64(x & masks[0]) & 1) << 0 |
        (popcnt64(x & masks[1]) & 1) << 1 |
        (popcnt64(x & masks[2]) & 1) << 2 |
        (popcnt64(x & masks[3]) & 1) << 3 |
        (popcnt64(x & masks[4]) & 1) << 4 |
        (popcnt64(x & masks[5]) & 1) << 5;
}

// Correct block, return true if success
static inline bool correct_block(uint64_t* x) {
    uint64_t ep = find_error_position(*x);
    // Found 1 error
    if (popcnt64(*x) & 1) {
        *x ^= (1ull << ep);
        return true;
    } else if (!ep) {
        return true;
    } else {
        return false;
    }
}

// Encode a block
static inline uint64_t encode(uint64_t x) {
    return set_parity(reposition(x));
}

// Decode a block, return true if success
static inline bool decode(uint64_t* x) {
    if (correct_block(x)) {
        *x = unreposition(*x);
        return true;
    } else {
        return false;
    }
}

void print_binary(uint64_t x) {
    for (uint64_t i = 0; i < 64; i++) {
        putchar('0' + (x >> i & 1));
    }
    putchar('\n');
}

void print_block(uint64_t x, bool maskout) {
    for (uint64_t i = 0; i < 64; i++) {
        // If parity check bit
        if (maskout && popcnt64(i) <= 1)
            putchar('#');
        else 
            putchar('0' + (x >> i & 1));
        putchar(' ');
        if ((i & 0x07) == 0x07)
        putchar('\n');
    }
    putchar('\n');
}

bool test() {
    struct timeval rand_time;
    gettimeofday(&rand_time, NULL);
    uint64_t seed = rand_time.tv_sec * 1000000 + rand_time.tv_usec;
    // tests
    for (size_t i = 0; i < 10000; i++) {
        uint64_t data = generate_rand(&seed) % (1ull << 57);

        uint64_t blocked_data = encode(data);
        uint64_t unrepo_data = unreposition(blocked_data);
        if (data != unrepo_data) {
            print_block(blocked_data, false);
            print_binary(data);
            print_binary(unrepo_data);
            printf("%llu: %llu\n", i, data);
            return false;
        }

        uint64_t err_pos = generate_rand(&seed) % 64;
        uint64_t err1_data = blocked_data ^ (1ull << err_pos);
        uint64_t found_err_pos = find_error_position(err1_data);
        if (err_pos != found_err_pos) {
            print_block(blocked_data, false);
            print_block(err1_data, false);
            printf("Error pos: %llu (found %llu)\n", err_pos, found_err_pos);
            printf("%llu\n", (popcnt64(err1_data & masks[0]) & 1));
            printf("%llu\n", (popcnt64(err1_data & masks[1]) & 1));
            printf("%llu\n", (popcnt64(err1_data & masks[2]) & 1));
            printf("%llu\n", (popcnt64(err1_data & masks[3]) & 1));
            printf("%llu\n", (popcnt64(err1_data & masks[4]) & 1));
            printf("%llu\n", (popcnt64(err1_data & masks[5]) & 1));
            return false;
        }

        if (!decode(&blocked_data)) {
            printf("Decode failed on %llu: %llu\n", i, data);
            return false;
        } else if (blocked_data != data) {
            print_binary(data);
            print_binary(blocked_data);
            printf("Decode unmatch on %llu: %llu\n", i, data);
            return false;
        }

        uint64_t err_pos1 = generate_rand(&seed) % 64;
        uint64_t err_pos2 = 0;
        blocked_data = encode(data);
        while (err_pos2 == err_pos1)
            err_pos2 = generate_rand(&seed) % 64;
        uint64_t unrecoverable = blocked_data ^ ((1ull << err_pos1) | (1ull << err_pos2));
        uint64_t found_pos = find_error_position(unrecoverable);
        uint64_t block_parity = popcnt64(unrecoverable) | 1;
        if (decode(&unrecoverable) != false) {
            printf("Decode success but expeced failed on %llu: %llu\n", i, data);
            return false;
        }
    }
    return true;
}

int main() {
    struct timeval rand_time;
    gettimeofday(&rand_time, NULL);
    uint64_t seed = rand_time.tv_sec * 1000000 + rand_time.tv_usec;

    if (!test()) {
        printf("Test failed.\n");
        return 0;
    }

    printf("Test PASS. Preparing performance bench...\n");
    uint64_t count = 1 << 24;
    double megabytes = count * 57.0 / 8.0 / 1024.0 / 1024.0;

    uint64_t* original = malloc(count * sizeof(uint64_t));
    uint64_t* data = malloc(count * sizeof(uint64_t));
    
    for (uint64_t i = 0; i < count; i++)
        original[i] = generate_rand(&seed) % (1ull << 57);
    memcpy(data, original, count * sizeof(uint64_t));

    printf("Running encode test bench...\n");
    struct timeval t_start, t_end;
    double milis;

    gettimeofday(&t_start, NULL);
    for (uint64_t i = 0; i < count; i++)
        data[i] = encode(data[i]);
    gettimeofday(&t_end, NULL);
    milis = 1000.0 * (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("Encode bench run complete. %.3lf MiB in %.3lf ms.\n", megabytes, milis);

    printf("Adding noises on blocks...\n");
    for (uint64_t i = 0; i < count; i++) {
        uint64_t err_pos = generate_rand(&seed) % 64;
        data[i] ^= (1ull << err_pos);
    }
    uint64_t extra_error_i = generate_rand(&seed) % count;
    uint64_t extra_error_pos = generate_rand(&seed) % count;
    printf("Added extra noise on block %llu.\n", extra_error_i);
    data[extra_error_i] ^= (1ull << extra_error_pos);

    printf("Running decode test bench...\n");
    gettimeofday(&t_start, NULL);
    for (uint64_t i = 0; i < count; i++) {
        if (!decode(&data[i])) {
            printf("Decode failed on %llu block!\n", i);
            continue;
        }
    }
    gettimeofday(&t_end, NULL);
    milis = 1000.0 * (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("Decode bench run complete. %.3lf MiB in %.3lf ms.\n", megabytes, milis);
    for (uint64_t i = 0; i < count; i++) {
        if (original[i] != data[i]) {
            printf("Data not match on %llu block!\n", i);
        }
    }

    return 0;
}