#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Test counter for summary
static int test_count = 0;
static int passed_count = 0;

// Function to execute a command and return its exit status
int execute_command(const char* command) {
    int status = system(command);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Assert function that mirrors the bash script functionality
void assert(int expected, const char* input) {
    test_count++;

    // Compile test helper
    if (execute_command("cc -c ./test/test_helper.c -o test_helper.o") != 0) {
        fprintf(stderr, "Failed to compile test_helper.c\n");
        exit(1);
    }

    // Generate assembly with ycc compiler
    char ycc_cmd[1024];
    snprintf(ycc_cmd, sizeof(ycc_cmd), "./ycc \"%s\" > tmp.s", input);
    if (execute_command(ycc_cmd) != 0) {
        fprintf(stderr, "Failed to run ycc compiler for: %s\n", input);
        exit(1);
    }

    // Compile the generated assembly with test helper
    if (execute_command("cc -o tmp tmp.s test_helper.o") != 0) {
        fprintf(stderr, "Failed to compile assembly for: %s\n", input);
        exit(1);
    }

    // Run the compiled program and get exit code
    int actual = execute_command("./tmp");

    // Check the result
    if (actual == expected) {
        printf("%s => %d\n", input, actual);
        fflush(stdout);
        passed_count++;
    } else {
        printf("%s => %d expected, but got %d\n", input, expected, actual);
        fflush(stdout);
        exit(1);
    }
}

int main() {
    printf("Running YCC Compiler Tests...\n\n");

    // Basic arithmetic tests
    assert(0, "main() { return 0; }");
    assert(42, "main() { return 42; }");
    assert(21, "main() { return 5+20-4; }");
    assert(41, "main() { return  12 + 34 - 5 ; }");
    assert(47, "main() { return 5+6*7; }");
    assert(15, "main() { return 5*(9-6); }");
    assert(4, "main() { return (3+5)/2; }");
    assert(10, "main() { return -10+20; }");
    assert(10, "main() { return - -10; }");
    assert(10, "main() { return - - +10; }");

    // Comparison operators
    assert(0, "main() { return 0==1; }");
    assert(1, "main() { return 42==42; }");
    assert(1, "main() { return 0!=1; }");
    assert(0, "main() { return 42!=42; }");

    assert(1, "main() { return 0<1; }");
    assert(0, "main() { return 1<1; }");
    assert(0, "main() { return 2<1; }");
    assert(1, "main() { return 0<=1; }");
    assert(1, "main() { return 1<=1; }");
    assert(0, "main() { return 2<=1; }");

    assert(1, "main() { return 1>0; }");
    assert(0, "main() { return 1>1; }");
    assert(0, "main() { return 1>2; }");
    assert(1, "main() { return 1>=0; }");
    assert(1, "main() { return 1>=1; }");
    assert(0, "main() { return 1>=2; }");

    // Variable assignment
    assert(3, "main() { a=3; return a; }");
    assert(19, "main() { a=3*5;return a+4; }");
    assert(13, "main() { foo=3;bar=5;return foo+bar*2; }");

    // Return statements
    assert(8, "main() { return 8; }");
    assert(21, "main() { a=3;return a*7; }");
    assert(14, "main() { foo = 3;bar = 5 * 6 - 8;return foo + bar / 2; }");

    // If statements
    assert(3, "main() { if (0) return 2; return 3; }");
    assert(2, "main() { if (1-1) return 3; return 2; }");
    assert(3, "main() { if (1) return 3; return 2; }");
    assert(3, "main() { if (2-1) return 3; return 2; }");

    // While loops
    assert(10, "main() { i=0;while(i<10)i=i+1;return i; }");
    assert(55, "main() { i=0;j=0;while(i<10){i=i+1;j=j+i;}return j; }");

    // For loops
    assert(2, "main() { i=0;for(i=0;i<3;i=i+1)j=i;return j; }");
    assert(5, "main() { i=0;for(i=0;i<3;i=i+1)j=i;return j+i; }");
    assert(19, "main() { for(i=0;i<10;i=i+1)j=i;return j+i; }");

    // Function calls
    assert(5, "main() { return foo(); }");
    assert(8, "main() { return bar(8); }");
    assert(6, "main() { return baz(1,2,3); }");

    // Define functions and call them
    assert (32, "main() { return ret32(); } ret32() { return 32; }");
    assert (7, "main() { return add2(3,4); } add2(x,y) { return x+y; }");
    assert (1, "main() { return sub2(4,3); } sub2(x,y) { return x-y; }");
    assert (55, "main() { return fib(9); } fib(x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }");
    assert (21, "main() { return add6(1,2,3,4,5,6); } add6(a,b,c,d,e,f) { return a+b+c+d+e+f; }");

    // Pointer and address-of tests
    assert(3, "main() { a=3; return *&a; }");
    assert(3, "main() { a=3; b=&a; return *b; }");
    assert(5, "main() { a=3; b=&a; *b=5; return a; }");
    assert(7, "main() { a=3; b=&a; c=&b; **c=7; return a; }");

    // Print summary
    printf("\n========================================\n");
    printf("OK - All tests passed! (%d/%d)\n", passed_count, test_count);
    printf("========================================\n");

    // Clean up temporary files
    execute_command("rm -f test_ycc tmp.s tmp test_helper.o");

    return 0;
}
