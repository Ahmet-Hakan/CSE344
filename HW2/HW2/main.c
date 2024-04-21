#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#define BUF_SIZE 256
#define FIFO_PERMS (S_IRUSR | S_IWUSR)
void sigHandler(int sigNum);
int *stringToArray(const char *str);
void intArrayToString(int *array, int size, char *buffer, int bufferSize);
int dofifochildsecond(FILE *fp1, int numOfArray);
int dofifochildfirst(FILE *fp1, const char *fifo2, int numOfArray);

sig_atomic_t child_count = 0;

int main()
{
    int numOfArray=0;
    printf("Enter number of elements in array:");
    scanf("%d", &numOfArray);
    if (numOfArray <= 0)
    {
        printf("Error: Invalid number of elements\n");
        exit(0);
    }
    else if (numOfArray > 10)
    {
        printf("Error: Number of elements should be less than or equal to 10\n");
        exit(0);
    }
    
    int *arr = (int *)malloc(numOfArray * sizeof(int));
    printf("Choose the array you want to craete.\n");
    printf("Press 1 for Array: 1,2,3,...n\n");
    printf("Press 2 for Random Array. \n");
    int choice;
    scanf("%d", &choice);
    if (choice == 1)
    {
        for (int i = 0; i < numOfArray; i++)
        {
            arr[i] = i + 1;
        }
    }
    else if (choice == 2)
    {
        srand(time(NULL));
        for (int i = 0; i < numOfArray; i++)
        {
            arr[i] = rand() % 10 + 1;
        }

    }
    else
    {
        printf("Invalid choice\n");
        exit(0);
    }

    printf("Array: ");
    for (int i = 0; i < numOfArray; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
    fflush(stdout);
    char *fifo1 = "fifo1";
    char *fifo2 = "fifo2";
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        fprintf(stderr, "[%ld]:failed to register signal handler: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }

    if (mkfifo(fifo1, FIFO_PERMS) == -1)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
                    (long)getpid(), fifo1, strerror(errno));
            exit(1);
        }
    }

    if (mkfifo(fifo2, FIFO_PERMS) == -1)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
                    (long)getpid(), fifo2, strerror(errno));
            exit(1);
        }
    }

    pid_t child_1, child_2;
    int fd1, fd2;

    child_1 = fork();
    switch (child_1)
    {
    case -1:
        fprintf(stderr, "[%ld]:failed to fork: %s\n", (long)getpid(), strerror(errno));
        exit(1);
    case 0:
    {
        // write(1, "Opening child-1\n", strlen("Opening child-1\n"));
        FILE *fp1 = fopen(fifo1, "r");
        // write(1, "Opened child-1\n", strlen("Opened child-1\n"));
        sleep(10);
        dofifochildfirst(fp1, fifo2, numOfArray);
        exit(EXIT_SUCCESS);
    }
    default:
        // write(1, "Opening par-1\n", strlen("Opening par-1\n"));
        fd1 = open(fifo1, O_WRONLY);
        // write(1, "Opened par-1\n", strlen("Opened par-1\n"));
        if (fd1 == -1)
        {
            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
                    (long)getpid(), fifo1, strerror(errno));
            exit(1);
        }
        char buffer[BUF_SIZE];
        intArrayToString(arr, numOfArray, buffer, BUF_SIZE);
        strcat(buffer, "\n");
        // printf("Array: %s", buffer);
        if (write(fd1, buffer, strlen(buffer)) == -1)
        {
            fprintf(stderr, "[%ld]:failed to write to named pipe %s: %s\n",
                    (long)getpid(), fifo1, strerror(errno));
            exit(1);
        }
    }

    child_2 = fork();
    switch (child_2)
    {
    case -1:
        fprintf(stderr, "[%ld]:failed to fork: %s\n", (long)getpid(), strerror(errno));
        exit(1);
    case 0:
    {
        //// printf("Opening child-2\n");
        FILE *fp1 = fopen(fifo2, "r");
        //// printf("Opened child-2\n");
        sleep(10);
        dofifochildsecond(fp1, numOfArray);
        fclose(fp1);
        exit(EXIT_SUCCESS);
    }
    default:
        //// printf("Opening fifo2 parent\n");
        fd2 = open(fifo2, O_WRONLY);
        //// printf("Opened fifo2 parent\n");
        if (fd2 == -1)
        {
            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
                    (long)getpid(), fifo2, strerror(errno));
            exit(1);
        }
        char buffer[BUF_SIZE];
        intArrayToString(arr, numOfArray, buffer, BUF_SIZE);
        strcat(buffer, "\n");
        // printf("Array: %s\n", buffer);

        if (write(fd2, buffer, strlen(buffer)) == -1)
        {
            fprintf(stderr, "[%ld]:failed to write to named pipe %s: %s\n",
                    (long)getpid(), fifo2, strerror(errno));
            exit(1);
        }
        if (write(fd2, "multiply\n", strlen("multiply\n")) == -1)
        {
            fprintf(stderr, "[%ld]:failed to write to named pipe %s: %s\n",
                    (long)getpid(), fifo2, strerror(errno));
            exit(1);
        }
    }

    while (1)
    {
        if (child_count >= 2)
        {
            printf("All children terminated\n");
            free(arr);
            exit(0);
        }
        printf("Proceeding...\n");
        sleep(2);
    }
    return 0;
}

int dofifochildfirst(FILE *fp1, const char *fifo2, int numOfArray)
{
    // printf("here 1\n");
    fflush(stdout);
    if (fp1 == NULL)
    {
        fprintf(stderr, "[%ld]:failed to open file fifo1: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }
    char line[256];
    // printf("here 2\n");
    fflush(stdout);
    if (fgets(line, sizeof(line), fp1) == NULL)
    {
        fprintf(stderr, "[%ld]:failed to read from file fifo1: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }
    // printf("here 3\n");
    fflush(stdout);
    // printf("Line child-1: %s\n", line);
    fflush(stdout);
    int *arr = stringToArray(line);
    for (int i = 0; i < numOfArray; i++)
    {
        //// printf("%d ", arr[i]);
    }
    //// printf("\n");
    int sum = 0;
    for (int i = 0; i < numOfArray; i++)
    {
        sum += arr[i];
    }
    // printf("Sum child-1: %d\n", sum);
    fflush(stdout);
    fclose(fp1);

    // printf("Opening child-1-2\n");
    int fd2 = open(fifo2, O_WRONLY | O_APPEND);
    // printf("Opened chield-1-2\n");
    if (fd2 == -1)
    {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
                (long)getpid(), fifo2, strerror(errno));
        exit(1);
    }

    char buffer[BUF_SIZE];
    sprintf(buffer, "%d\n", sum);
    if (write(fd2, buffer, strlen(buffer)) == -1)
    {
        fprintf(stderr, "[%ld]:failed to write to named pipe %s: %s\n",
                (long)getpid(), fifo2, strerror(errno));
        exit(1);
    }
    free(arr);
    return 0;
}

int dofifochildsecond(FILE *fp1, int numOfArray)
{
    if (fp1 == NULL)
    {
        fprintf(stderr, "[%ld]:failed to ----1----open file fifo2: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }

    char line[256];

    if (fgets(line, sizeof(line), fp1) == NULL)
    {
        fprintf(stderr, "[%ld]:failed to ----2---read from file fifo2: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }
    // printf("Line: %s\n", line);
    int *arr = stringToArray(line);
    //// printf("Array child-2: ");
    for (int i = 0; i < numOfArray; i++)
    {
        //// printf("%d ", arr[i]);
    }
    if (fgets(line, sizeof(line), fp1) == NULL)
    {
        fprintf(stderr, "[%ld]:failed to ----3----read from file fifo2: %s\n",
                (long)getpid(), strerror(errno));
        exit(1);
    }
    // printf("Line: %s\n", line);
    fflush(stdout);
    int product = 1;
    if (strcmp(line, "multiply\n") == 0)
    {
        for (int i = 0; i < numOfArray; i++)
        {
            product *= arr[i];
        }
    }

    int check = 0;
    while (!check)
    {
        if (fgets(line, sizeof(line), fp1) == NULL)
        {
            // fprintf(stderr, "[%ld]:failed to ------4------read from file fifo2: %s\n",
            //         (long)getpid(), strerror(errno));
            // exit(1);
            check = 0;
            // printf("Waiting\n");
            fflush(stdout);
            sleep(10);
        }
        else
            check = 1;
    }
    //// printf("Line: %s\n", line);
    int sum = atoi(line);

    sum = sum + product;
    printf("Sum child-2: %d\n", sum);
    free(arr);
    return 0;
}

void intArrayToString(int *array, int size, char *buffer, int bufferSize)
{
    char *bufferEnd = buffer + bufferSize;
    for (int i = 0; i < size && buffer < bufferEnd; i++)
    {
        buffer += snprintf(buffer, bufferEnd - buffer, "%d ", array[i]);
    }
    // if(buffer != bufferEnd) {
    //     *buffer = '\0';
    // }
}

int *stringToArray(const char *string)
{
    int *arr = (int *)malloc(BUF_SIZE * sizeof(int));
    char str[BUF_SIZE];
    strcpy(str, string);
    char *token = strtok(str, " ");
    int i = 0;
    while (token != NULL)
    {
        arr[i] = atoi(token);
        i++;
        token = strtok(NULL, " ");
    }
    return arr;
}

void sigHandler(int sigNum)
{
    int status = sigNum;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        child_count++;
        printf("Child %ld terminated with status %d\n", (long)child_pid, status);
    }
}

// gcc -o main main.c
// ./main
