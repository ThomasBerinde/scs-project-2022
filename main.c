#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NR_OF_TESTS 1000000
#define NR_OF_OPERATIONS 1073741824
#define ARRAY_SIZE 100000

long nr_of_threads = 1;

void get_cpu_info() {
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("nr. of cpus: %ld\n", num_cpus);
}

void *calculate_integer_operations() {
    long long int a = 124235250001, b = 21241201001, c = 2342234320000, d = 233240000000001, e = 2341200000100000;
    long long int g = 123112003001, h = 56740012007, k = 240010001001, l = 34224003003006, m = 3242352500060006;
    long long int n = 512412487236, o = 24223432, p = 11234142738462, r = 8532114414, s = 31125646724, t = 67451241564;
    long long int u = 34447892317, w = 3244427389, x = 8913189230, y = 12323782349, v1 = 0000001000023, v2 = 3000009000009;
    long long int v3 = 565000000204, v4 = 30000090009, v5 = 78743777777;

    for (int i = 0; i < NR_OF_OPERATIONS / nr_of_threads; i++) {
        c *= ((((d + e) + (v5 * v5)) * ((a * (v1 + v2)) + (b * (o + p))) + ((g * h) + (x * y)) * (t + u) +
               ((k * l) + (g * k)) * (w * l)) * (((m * n) + (k + n)) + ((r * s) + (v3 * v4))));
    }

    return NULL;
}

void *calculate_floating_point_operations() {
    double a = 124.235250001, b = 21.241201001, c = 2342.234320000, d = 23.3240000000001, e = 2.341200000100000;
    double g = 1231.12003001, h = 567.40012007, k = 24.0010001001, l = 342.24003003006, m = 324.2352500060006;
    double n = 51.2412487236, o = 242.23432, p = 112.34142738462, r = 853.2114414, s = 31.125646724, f = 67.451241564;
    double u = 34.447892317, w = 3.244427389, x = 89.13189230, y = 123.23782349, v1 = 0.000001000023;
    double v2 = 3.000009000009, v3 = 56.5000000204, v4 = 0.00000090009, v5 = 787.43777777;

    for (int i = 0; i < NR_OF_OPERATIONS / nr_of_threads; i++) {
        c *= ((((d + e) + (v5 * v5)) * ((a * (v1 + v2)) + (b * (o + p))) + ((g * h) + (x * y)) * (f + u) +
               ((k * l) + (g * k)) * (w * l)) * (((m * n) + (k + n)) + ((r * s) + (v3 * v4))));
    }

    return NULL;
}

void test_flops(int floats) {
    pthread_t t[nr_of_threads];
    struct timeval tp_before, tp_after;
    unsigned long number_of_operations = NR_OF_OPERATIONS;
    char *operations_type = floats == 1 ? "floating point" : "integer";

    gettimeofday(&tp_before, NULL);
    for (int i = 0; i < nr_of_threads; i++) {
        if (0 != pthread_create(&t[i], NULL,
                                floats == 1 ? calculate_floating_point_operations : calculate_integer_operations, NULL))
            perror("failed to create threads");
    }
    for (int i = 0; i < nr_of_threads; i++) {
        if (0 != pthread_join(t[i], NULL))
            perror("failed to join threads");
    }
    gettimeofday(&tp_after, NULL);

    printf("Did %lu %s operations in %.2g seconds with %ld threads\n", number_of_operations * 30,
           operations_type,
           (double) tp_after.tv_sec - (double) tp_before.tv_sec + 1E-6 * (tp_after.tv_usec - tp_before.tv_usec),
           nr_of_threads);
}


void test_memory_access() {
    // Declare variables
    int write_index, read_index, value;
    int *array = (int *) malloc(ARRAY_SIZE * sizeof(int));
    struct timeval tp_before, tp_after;
    double total_time = 0;

    // Allocate array
    if (NULL == array)
        perror("memory allocation fail");

    for (int i = 0; i < NR_OF_TESTS; i++) {

        // Generate random indices and values
        write_index = rand() % ARRAY_SIZE;
        read_index = rand() % ARRAY_SIZE;
        value = rand();

        // Perform one write and one read operation, measure the time and add it to the total time
        gettimeofday(&tp_before, NULL);
        array[write_index] = value;
        value = array[read_index];
        gettimeofday(&tp_after, NULL);
        total_time +=
                (double) tp_after.tv_sec - (double) tp_before.tv_sec + 1E-6 * (tp_after.tv_usec - tp_before.tv_usec);
    }

    if (NULL != array)
        free(array);

    // Multiply by 2 because in each loop cycle we perform one read and one write operation
    printf("Did %d memory reads and writes in %.2g seconds\n", NR_OF_TESTS * 2, total_time);
}

int get_test_to_run(char* test) {
    if (strcmp(test, "--cpu") == 0) {
        return 1;
    } else if (strcmp(test, "--flops") == 0) {
        return 2;
    } else if (strcmp(test, "--iops") == 0) {
        return 3;
    } else if (strcmp(test, "--mem") == 0) {
        return 4;
    } else if (strcmp(test, "--all") == 0) {
        return 5;
    }
    return 0;
}

int main(int args, char **argv) {
    int test_to_run = -1;
    if (args != 4 || strcmp(argv[1], "--threads") != 0 || (test_to_run = get_test_to_run(argv[3])) == 0) {
        printf("./cpubm [--threads <n>] --[cpu/flops/iops/mem/all]\n"); // cpubm comes from cpu benchmark
        exit(1);
    }

    char *end_ptr;
    nr_of_threads = strtol(argv[2], &end_ptr, 10);

    srand((unsigned int) time(0));

    switch(test_to_run) {
        case 1: // cpu
            get_cpu_info();
            break;
        case 2: // flops
            test_flops(1);
            break;
        case 3: // iops
            test_flops(0);
            break;
        case 4: // mem
            test_memory_access();
            break;
        case 5: // all
            get_cpu_info();
            test_memory_access();
            test_flops(1); // test floating point operations
            test_flops(0); // test integer point operations
            break;
        default:
            printf("./cpubm [--threads <n>] --[cpu/flops/iops/mem/all]\n");
            exit(1);
    }

    return 0;
}

