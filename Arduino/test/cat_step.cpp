/*
 * Stdin: lines of five hex bytes (e.g. "00 00 00 00 00").
 * After each command, print a 32-byte cat_map_debug_dump as hex.
 */
#include "../firmware/cat_map.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

int main(void)
{
    char line[256];
    unsigned int b[5];
    uint8_t cmd[5];
    uint8_t dump[CAT_DEBUG_LEN];
    cat_result_t result;
    int i;
    int n;

    cat_map_init();
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        n = std::sscanf(line, "%x %x %x %x %x", &b[0], &b[1], &b[2], &b[3], &b[4]);
        if (n != 5) {
            std::fprintf(stderr, "bad line: %s", line);
            return 1;
        }
        for (i = 0; i < 5; i++) {
            cmd[i] = (uint8_t)b[i];
        }
        cat_map_dispatch(cmd, &result);
        if (result.n_host == CAT_DEBUG_LEN) {
            memcpy(dump, result.host, CAT_DEBUG_LEN);
        } else {
            cat_map_debug_dump(dump);
        }
        for (i = 0; i < CAT_DEBUG_LEN; i++) {
            std::printf("%02X%s", dump[i], i + 1 == CAT_DEBUG_LEN ? "\n" : " ");
        }
        std::fflush(stdout);
    }
    return 0;
}
