#include <stdio.h>
#include <unistd.h>

#define loop while (1)

__attribute__((noinline)) extern void my_println(const char* str) {
    puts(str);
}

int main() {
    loop {
        my_println("Hello, world");
        sleep(1);
    }
}
