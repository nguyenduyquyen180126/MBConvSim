#include <stdio.h>
#include <stdint.h>

int main() {
    uint16_t counter = 1;
    while(counter != 0){
        int ctz = __builtin_ctz(counter);
        int dem = ctz + 2;
        printf("counter: %d, ctz: %d, dem: %d\n", counter, ctz, dem);
        counter = counter << 1;
    }
    return 0;
}
