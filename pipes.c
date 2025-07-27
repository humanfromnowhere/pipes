#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

#define XXX 123 // Last 3 digits of your ID + 100 (xxx + 100 )

int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

void generate_random_numbers(const char* filename, int N) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < N; i++) {
        fprintf(fp, "%d\n", rand() % XXX);
    }
    fclose(fp);
}

int count_primes_in_file(const char* filename) {
    int count = 0, num;
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    while (fscanf(fp, "%d", &num) == 1) {
        if (is_prime(num)) {
            count++;
        }
    }
    fclose(fp);
    return count;
}

int main() {
    srand(time(NULL));
    int N;
    printf("Enter the number of integers N: ");
    scanf("%d", &N);

    char file1[] = "File1.txt", file2[] = "File2.txt";
    generate_random_numbers(file1, N);
    generate_random_numbers(file2, N);

    int pipe_p1_c1[2], pipe_c1_p1[2], pipe_p1_c2[2], pipe_c2_p1[2];
    pipe(pipe_p1_c1);
    pipe(pipe_c1_p1);
    pipe(pipe_p1_c2);
    pipe(pipe_c2_p1);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe_p1_c1[1]); close(pipe_c1_p1[0]);
        close(pipe_p1_c2[0]); close(pipe_p1_c2[1]);
        close(pipe_c2_p1[0]); close(pipe_c2_p1[1]);

        int n, other_prime;
        read(pipe_p1_c1[0], &n, sizeof(n)); 
        int prime_count = count_primes_in_file(file1);
        write(pipe_c1_p1[1], &prime_count, sizeof(prime_count));  

        read(pipe_p1_c1[0], &other_prime, sizeof(other_prime));  

        if (prime_count > other_prime)
            printf("I am Child process P1: The winner is child process P1\n");
        else if (prime_count < other_prime)
            printf("I am Child process P1: The winner is child process P2\n");
        else
            printf("I am Child process P1: The result is a tie\n");

        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe_p1_c2[1]); close(pipe_c2_p1[0]);
        close(pipe_p1_c1[0]); close(pipe_p1_c1[1]); close(pipe_c1_p1[0]); close(pipe_c1_p1[1]);

        int n, other_prime;
        read(pipe_p1_c2[0], &n, sizeof(n));  
        int prime_count = count_primes_in_file(file2);
        write(pipe_c2_p1[1], &prime_count, sizeof(prime_count));  

        read(pipe_p1_c2[0], &other_prime, sizeof(other_prime)); 

        if (prime_count > other_prime)
            printf("I am Child process P2: The winner is child process P2\n");
        else if (prime_count < other_prime)
            printf("I am Child process P2: The winner is child process P1\n");
        else
            printf("I am Child process P2: The result is a tie\n");

        exit(0);
    }

    close(pipe_p1_c1[0]);
    close(pipe_c1_p1[1]); 
    close(pipe_p1_c2[0]); 
    close(pipe_c2_p1[1]);

    write(pipe_p1_c1[1], &N, sizeof(N));
    write(pipe_p1_c2[1], &N, sizeof(N));

    int prime_count1, prime_count2;
    read(pipe_c1_p1[0], &prime_count1, sizeof(prime_count1));
    read(pipe_c2_p1[0], &prime_count2, sizeof(prime_count2));

    write(pipe_p1_c1[1], &prime_count2, sizeof(prime_count2));  
    write(pipe_p1_c2[1], &prime_count1, sizeof(prime_count1)); 

    wait(NULL);
    wait(NULL);

    printf("The number of positive integers in each file: %d\n", N);
    printf("The number of prime numbers in File1: %d\n", prime_count1);
    printf("The number of prime numbers in File2: %d\n", prime_count2);

    return 0;
}
