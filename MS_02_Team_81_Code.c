#include <stdio.h>
#include <stdlib.h> // Correct header for standard library functions
#include <errno.h>  // for err function
#include <string.h> // for strerror function

#define MEMORY_SIZE 60
// note to self
int globalQuantum;
int p1Done = 0;
int p2Done = 0;
int p3Done = 0;

typedef struct
{
    char name[20];   // exp= instruction1
    char value[200]; // exp= print x
} word;
// our memory word structure will be (instruction1 : semwait userinput)

typedef struct
{
    int processID;
    char ProcessState[20]; // blocked OR ready
    int CurrentPriority;
    int ProgramCounter;
    int lower;
    int upper;
} Pcb;

Pcb p1;
Pcb p2;
Pcb p3;
// initialization of all pcbs for each process

typedef struct
{
    word words[MEMORY_SIZE]; // an array of 60 memory words
} memory;

typedef struct
{
    int quantum;
    int executiontime;
} programVar;

programVar program1;
programVar program2;
programVar program3;
// initialization of variables for each process

// start of queue data structure
typedef struct
{
    int q[3];
    int end;
} queue;

queue readyQueue;
queue generalBlockQueue;

int queueEmpty(queue *q)
{
    if ((*q).end == -1)
    {
        // printf("the queue is empty");
        return 1;
    }
    else
    {
        // printf("the queue is not empty");
        return 0;
    }
}

int queuefull(queue *q)
{
    if ((*q).end == 2)
    {
        // printf("the queue is full");
        return 1;
    }
    else
    {
        // printf("the queue is not full");
        return 0;
    }
}

void enqueue(queue *q, int value)
{
    if (queuefull(q) == 1)
    {
        printf("can't add any more values because the queue is full.\n");
        return;
    }

    (*q).end++;
    (*q).q[(*q).end] = value;
    printf("enqueued %d\n", value);
    printf("end is: %d\n", (*q).end);
}

int dequeue(queue *q)
{

    if (queueEmpty(q))
    {
        printf("can't remove values because the queue is empty");
        return -1;
    }
    int value = (*q).q[0];
    for (int i = 0; i < (*q).end; i++)
    {
        (*q).q[i] = (*q).q[i + 1];
    }
    (*q).end--;
    return value;
}

void printQueue(queue *q)
{
    if (queueEmpty(q))
    {
        printf("Queue is empty\n");
        return;
    }

    printf("elements in queue are : ");
    for (int i = 0; i <= q->end; i++)
    {
        printf("%d ", q->q[i]);
    }
    printf("\n");
}

int valueExistsInQueue(const queue *q_ptr, int value)
{
    // Iterate through the elements in the queue
    for (int i = 0; i < q_ptr->end; i++)
    {
        // If the value matches an element in the queue, return 1 (true)
        if (q_ptr->q[i] == value)
        {
            return 1;
        }
    }
    // If the value is not found in the queue, return 0 (false)
    return 0;
}

// end of queue structure

// start of mutex code
typedef struct
{
    enum
    {
        zero,
        one
    } value;
    queue q;
    int ownerID;
} mutex;

mutex userInput;
mutex userOutput;
mutex file;
// initialization of mutex for each process

int semWaitB(mutex *m, int processID, memory *mem)
{
    if ((*m).value == one)
    {
        (*m).ownerID = processID;
        (*m).value = zero;
        return 1; // indicates that the resource was available
    }
    else
    {
        printf("mutex is already locked\n");
        if (processID == 1)
        {
            strcpy(mem->words[1].value, "blocked");
        }
        else if (processID == 2)
        {
            strcpy(mem->words[21].value, "blocked");
        }
        else if (processID == 3)
        {
            strcpy(mem->words[41].value, "blocked");
        }

        // enqueue(&(m.queue), processID);

        dequeue(&readyQueue);
        enqueue(&(m->q), processID);
        enqueue(&generalBlockQueue, processID);

        return 0; // indicates that the resource was not allocated
    }
}

void semSignalB(mutex *m, int processID)
{
    if ((*m).ownerID == processID)
    {

        if ((queueEmpty(&(m->q))) == 1)
        {
            (*m).value = one;
        }
        else
        {
            /* remove a process P from m.queue and place it on ready list*/ /* update ownerID to be equal to Process P’s ID */
            (*m).ownerID = dequeue(&(m->q));
        }

        // check if there is a process in the queue

        if (queueEmpty(&(m->q)) == 0)
        {
            // not an empty queue
            // remove from blocked and add to ready
            enqueue(&readyQueue, dequeue(&(m->q)));
        }
    }
    else
    {
        printf("the processID is not the owner of the mutex being signalled");
    }
}

// end of mutex code

// helpers to use in the project

char *readvalueFromMemory(memory *mem, int address)
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        return (*mem).words[address].value;
    }
    else
    {
        printf("the memory address is wrong\n");
        return NULL;
    }
}

char *readnameFromMemory(memory *mem, int address)
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        return (*mem).words[address].name;
    }
    else
    {
        printf("the memory address is wrong\n");
        return NULL;
    }
}

void writevalueintoMemory(memory *mem, int address, char value[])
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        strcpy((*mem).words[address].value, value);
    }
    else
    {
        printf("the memory address is wrong\n");
    }
}

void writenameintoMemory(memory *mem, int address, char value[])
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        strcpy((*mem).words[address].name, value);
    }
    else
    {
        printf("the memory address is wrong\n");
    }
}

void printFullMemory(memory *mem)
{
    int i = 0;
    while (i < (MEMORY_SIZE))
    {
        printf("address %d: \n", i);
        printf("%s :", (readnameFromMemory(mem, i)));
        printf("%s \n", (readvalueFromMemory(mem, i)));
        i++;
    }
}

void intializeMemory(memory *mem)
{

    int i = 0;
    while (i < (MEMORY_SIZE))
    {
        writenameintoMemory(mem, i, "empty");
        writevalueintoMemory(mem, i, "empty");
        i++;
    }
}

int readInstructionsFromTxtfile(char file_name[], memory *mem)
{
    FILE *input = fopen(file_name, "r");

    char instruction[200];
    int min_address;
    int max_address;
    int instructionNo = 1;
    int programid;

    // this if condition will check if i am reading program 1 or 2 or 3 so that i can insert in the right place in memory
    if (file_name[8] == '1')
    {
        min_address = 9;
        programid = 1;
    }
    else if (file_name[8] == '2')
    {
        min_address = 29;
        programid = 2;
    }
    else if (file_name[8] == '3')
    {
        min_address = 49;
        programid = 3;
    }

    if (!input)
    {
        printf("error reading from file ");
        exit(EXIT_FAILURE);
    }

    while (fgets(instruction, 200, input))
    {
        char name[20];
        snprintf(name, sizeof(name), "instruction %d ", instructionNo);

        writevalueintoMemory(mem, min_address, instruction);
        writenameintoMemory(mem, min_address, name);

        instructionNo++;
        min_address++;

        if (programid == 1)
        {
            program1.executiontime++;
        }
        else if (programid == 2)
        {
            program2.executiontime++;
        }
        else if (programid == 3)
        {
            program3.executiontime++;
        }
    }

    return 0;
}

void updatePcbmem(int programId, memory *mem)
{
    char id[30];
    char ProcessState[30];
    char CurrentPriority[30];
    char ProgramCounter[50];
    char LowerBoundary[30];
    char UpperBoundary[30];
    int i = 0;

    if (programId == 1)
    {
        while (i < (p1.lower + 6))
        {
            if (i == p1.lower)
            {
                writenameintoMemory(mem, i, "Process ID");
                sprintf(id, "%d", p1.processID);
                writevalueintoMemory(mem, i, id);
            }
            else if (i == p1.lower + 1)
            {
                writenameintoMemory(mem, i, "Process State");
                writevalueintoMemory(mem, i, p1.ProcessState);
            }
            else if (i == p1.lower + 2)
            {
                writenameintoMemory(mem, i, "Current Priority");
                sprintf(CurrentPriority, "%d", p1.CurrentPriority);
                writevalueintoMemory(mem, i, CurrentPriority);
            }
            else if (i == p1.lower + 3)
            {
                writenameintoMemory(mem, i, "Program Counter");
                sprintf(ProgramCounter, "%d", p1.ProgramCounter);
                writevalueintoMemory(mem, i, ProgramCounter);
            }
            else if (i == p1.lower + 4)
            {
                writenameintoMemory(mem, i, "Lower boundary");
                sprintf(LowerBoundary, "%d", p1.lower);
                writevalueintoMemory(mem, i, LowerBoundary);
            }
            else if (i == p1.lower + 5)
            {
                writenameintoMemory(mem, i, "Upper boundary");
                sprintf(UpperBoundary, "%d", p1.upper);
                writevalueintoMemory(mem, i, UpperBoundary);
            }

            i++;
        }
    }
    else if (programId == 2)
    {
        while (i < (p2.lower + 6))
        {
            if (i == p2.lower)
            {
                writenameintoMemory(mem, i, "Process ID");
                sprintf(id, "%d", p2.processID);
                writevalueintoMemory(mem, i, id);
            }
            else if (i == p2.lower + 1)
            {
                writenameintoMemory(mem, i, "Process State");
                writevalueintoMemory(mem, i, p2.ProcessState);
            }
            else if (i == p2.lower + 2)
            {
                writenameintoMemory(mem, i, "Current Priority");
                sprintf(CurrentPriority, "%d", p2.CurrentPriority);
                writevalueintoMemory(mem, i, CurrentPriority);
            }
            else if (i == p2.lower + 3)
            {
                writenameintoMemory(mem, i, "Program Counter");
                sprintf(ProgramCounter, "%d", p2.ProgramCounter);
                writevalueintoMemory(mem, i, ProgramCounter);
            }
            else if (i == p2.lower + 4)
            {
                writenameintoMemory(mem, i, "Lower boundary");
                sprintf(LowerBoundary, "%d", p2.lower);
                writevalueintoMemory(mem, i, LowerBoundary);
            }
            else if (i == p2.lower + 5)
            {
                writenameintoMemory(mem, i, "Upper boundary");
                sprintf(UpperBoundary, "%d", p2.upper);
                writevalueintoMemory(mem, i, UpperBoundary);
            }

            i++;
        }
    }
    else if (programId == 3)
    {
        while (i < (p3.lower + 6))
        {
            if (i == p3.lower)
            {
                writenameintoMemory(mem, i, "Process ID");
                sprintf(id, "%d", p3.processID);
                writevalueintoMemory(mem, i, id);
            }
            else if (i == p3.lower + 1)
            {
                writenameintoMemory(mem, i, "Process State");
                writevalueintoMemory(mem, i, p3.ProcessState);
            }
            else if (i == p3.lower + 2)
            {
                writenameintoMemory(mem, i, "Current Priority");
                sprintf(CurrentPriority, "%d", p3.CurrentPriority);
                writevalueintoMemory(mem, i, CurrentPriority);
            }
            else if (i == p3.lower + 3)
            {
                writenameintoMemory(mem, i, "Program Counter");
                sprintf(ProgramCounter, "%d", p3.ProgramCounter);
                writevalueintoMemory(mem, i, ProgramCounter);
            }
            else if (i == p3.lower + 4)
            {
                writenameintoMemory(mem, i, "Lower boundary");
                sprintf(LowerBoundary, "%d", p3.lower);
                writevalueintoMemory(mem, i, LowerBoundary);
            }
            else if (i == p3.lower + 5)
            {
                writenameintoMemory(mem, i, "Upper boundary");
                sprintf(UpperBoundary, "%d", p3.upper);
                writevalueintoMemory(mem, i, UpperBoundary);
            }

            i++;
        }
    }
}

void createPcb(int programNo, memory *mem)
{
    char id[30];
    char ProcessState[30];
    char CurrentPriority[30];
    char ProgramCounter[50];
    char LowerBoundary[30];
    char UpperBoundary[30];
    int low;
    int upper;
    int pc;

    if (programNo == 1)
    {
        p1.processID = 1;
        strcpy(id, "1");
        strcpy(LowerBoundary, "0");
        p1.lower = 0;
        low = 0;
        upper = ((low + 20) - 1);
        p1.upper = upper;
        sprintf(UpperBoundary, "%d", upper);
        pc = low + 9;
        p1.ProgramCounter = pc;
        sprintf(ProgramCounter, "%d", pc);
        strcpy(p1.ProcessState, "ready");
        p1.CurrentPriority = 1;
        updatePcbmem(1, mem);
    }
    else if (programNo == 2)
    {
        p2.processID = 2;
        strcpy(id, "2");
        strcpy(LowerBoundary, "20");
        p2.lower = 20;
        low = 20;
        upper = ((low + 20) - 1);
        p2.upper = upper;
        sprintf(UpperBoundary, "%d", upper);
        pc = low + 9;
        p2.ProgramCounter = pc;
        sprintf(ProgramCounter, "%d", pc);
        strcpy(p2.ProcessState, "ready");
        p2.CurrentPriority = 1;
        updatePcbmem(2, mem);
    }
    else if (programNo == 3)
    {
        p3.processID = 3;
        strcpy(id, "3");
        strcpy(LowerBoundary, "40");
        p3.lower = 40;
        low = 40;
        upper = ((low + 20) - 1);
        p3.upper = upper;
        sprintf(UpperBoundary, "%d", upper);
        pc = low + 9;
        p3.ProgramCounter = pc;
        sprintf(ProgramCounter, "%d", pc);
        strcpy(p3.ProcessState, "ready");
        p3.CurrentPriority = 1;
        updatePcbmem(3, mem);
    }
    else
    {
        printf("there doesn't exist a program with this number");
    }

    // strcpy(ProcessState, "ready");
    // strcpy(CurrentPriority, "1");

    // int i = 0;
    // while (i < (low + 6))
    // {
    //     if (i == low)
    //     {
    //         writenameintoMemory(mem, i, "Process ID");
    //         writevalueintoMemory(mem, i, id);
    //     }
    //     else if (i == low + 1)
    //     {
    //         writenameintoMemory(mem, i, "Process State");
    //         writevalueintoMemory(mem, i, ProcessState);
    //     }
    //     else if (i == low + 2)
    //     {
    //         writenameintoMemory(mem, i, "Current Priority");
    //         writevalueintoMemory(mem, i, CurrentPriority);
    //     }
    //     else if (i == low + 3)
    //     {
    //         writenameintoMemory(mem, i, "Program Counter");
    //         writevalueintoMemory(mem, i, ProgramCounter);
    //     }
    //     else if (i == low + 4)
    //     {
    //         writenameintoMemory(mem, i, "Lower boundary");
    //         writevalueintoMemory(mem, i, LowerBoundary);
    //     }
    //     else if (i == low + 5)
    //     {
    //         writenameintoMemory(mem, i, "Upper boundary");
    //         writevalueintoMemory(mem, i, UpperBoundary);
    //     }

    //     i++;
    // }
}

void createProcess(int programNo, memory *mem)
{
    // create pcb for the process
    // create the variables struct for the process
    // read instructions from text file to memory
    // put the instruction in the first level queue
    if (programNo == 1)
    {

        createPcb(1, mem);
        program1.executiontime = 0;
        program1.quantum = globalQuantum;
        readInstructionsFromTxtfile("Program_1.txt", mem);
    }
    else if (programNo == 2)
    {
        createPcb(2, mem);
        program2.executiontime = 0;
        program2.quantum = globalQuantum;

        readInstructionsFromTxtfile("Program_2.txt", mem);
    }
    else if (programNo == 3)
    {

        createPcb(3, mem);
        program3.executiontime = 0;
        program3.quantum = globalQuantum;

        readInstructionsFromTxtfile("Program_3.txt", mem);
    }
}

void executeOneLine(int programID, memory *mem)
{
    // get the pc
    //  get the instruction from memory
    // decode it
    int currentpc;
    int variablesfirstaddress;
    int variableslastaddress;
    if (programID == 1)
    {
        currentpc = atoi(readvalueFromMemory(mem, 3));
        variablesfirstaddress = atoi(readvalueFromMemory(mem, 4)) + 6;
        variableslastaddress = variablesfirstaddress + 3;
        // this is the last address of variables + 1
    }
    else if (programID == 2)
    {
        currentpc = atoi(readvalueFromMemory(mem, 23));
        variablesfirstaddress = atoi(readvalueFromMemory(mem, 24)) + 6;
        variableslastaddress = variablesfirstaddress + 3;
    }
    else if (programID == 3)
    {
        currentpc = atoi(readvalueFromMemory(mem, 43));
        variablesfirstaddress = atoi(readvalueFromMemory(mem, 44)) + 6;
        variableslastaddress = variablesfirstaddress + 3;
    }

    // this extracts the instruction from the memory
    char extracted[300];

    sprintf(extracted, "%s", readvalueFromMemory(mem, currentpc));
    printf("\n-------------\ninstruction currently executing is: %s--------------\n", extracted);

    //  printf("%s \n", extracted);
    //  printf("%c \n", extracted[7]);
    //   this print was to test and it is working as intended

    // below are the cases for translation

    // case 1 is printing
    if (extracted[0] == 'p')
    {
        char tobeprinted[500];
        // print x case
        if (extracted[5] == ' ')
        {
            char variablename[2];
            variablename[0] = extracted[6];
            variablename[1] = '\0';
            int i = variablesfirstaddress;
            while (i < variableslastaddress)
            {

                if (strcmp(readnameFromMemory(mem, i), variablename) == 0)
                {
                    strcpy(tobeprinted, readvalueFromMemory(mem, i));
                    printf("%s", tobeprinted);
                    return;
                }
                else
                {
                    printf("can't print a value that doesn't exist");
                }
                i++;
            }
            printf("print executed\n");
        }
        // printfromto x y
        else if (extracted[9] == 'T')
        {
            char variablename1[2];
            char variablename2[2];

            variablename1[0] = extracted[12];
            variablename1[1] = '\0';
            variablename2[0] = extracted[14];
            variablename2[1] = '\0';

            int var1;
            int var2;

            int i = variablesfirstaddress;
            while (i < variableslastaddress)
            {
                if (strcmp(readnameFromMemory(mem, i), variablename1) == 0)
                {
                    var1 = atoi(readvalueFromMemory(mem, i));
                }
                else
                {
                }
                i++;
            }
            i = variablesfirstaddress;
            while (i < variableslastaddress)
            {
                if (strcmp(readnameFromMemory(mem, i), variablename2) == 0)
                {
                    var2 = atoi(readvalueFromMemory(mem, i));
                }
                else
                {
                }
                i++;
            }

            int x;
            x = var1;
            while (x <= var2)
            {
                printf("%d ,", x);
                x++;
            }
            printf("printfromto executed\n");
        }
    }
    // case 2 semWait and semSignal
    else if (extracted[0] == 's')
    {
        // semWait
        if (extracted[3] == 'W')
        {
            // file
            if (extracted[8] == 'f')
            {
                semWaitB(&file, programID, mem);
            }
            else if (extracted[8] == 'u')
            {
                // userinput
                if (extracted[12] == 'I')
                {
                    semWaitB(&userInput, programID, mem);
                }
                // useroutput
                else if (extracted[12] == 'O')
                {
                    semWaitB(&userOutput, programID, mem);
                }
            }
            printf("semwait executed\n");
        }
        // semsignal
        else if (extracted[3] == 'S')
        {

            // file
            if (extracted[10] == 'f')
            {
                semSignalB(&file, programID);
            }
            else if (extracted[8] == 'u')
            {
                // userinput
                if (extracted[14] == 'I')
                {
                    semSignalB(&userInput, programID);
                }
                // useroutput
                else if (extracted[14] == 'O')
                {
                    semSignalB(&userOutput, programID);
                }
            }
            printf("semsingal executed\n");
        }
    }
    // case assign
    else if (extracted[0] == 'a')
    {
        // first variable extraction
        char destVariable[2];
        destVariable[0] = extracted[7];
        destVariable[1] = '\0';

        int i = variablesfirstaddress;
        char value[10];

        while (i < variableslastaddress)
        {
            if (strcmp(readnameFromMemory(mem, i), destVariable) == 0)
            {
                break;
            }
            else if (strcmp(readnameFromMemory(mem, i), "empty") == 0)
            {
                writenameintoMemory(mem, i, destVariable);
                break;
            }
            else
            {
                i++;
            }
        }
        if (extracted[9] == 'r')
        // defected case
        {
            char filenameVar[2];
            filenameVar[0] = extracted[18];
            filenameVar[1] = '\0';
            int i = variablesfirstaddress;
            char extractedFilename[200];
            int x;
            while (i < variableslastaddress)
            {
                if ((strcmp(readnameFromMemory(mem, i), filenameVar)) == 0)
                {

                    strcpy(extractedFilename, readvalueFromMemory(mem, i));
                    x = i;
                }
                else
                {
                }
                i++;
            }

            FILE *input = fopen(extractedFilename, "r");

            if (!input)
            {
                printf("error reading from file ");
                exit(EXIT_FAILURE);
            }

            char readdata[200];
            while (fgets(readdata, 200, input))
            {
                // strcat(instructions, buffer);
                writevalueintoMemory(mem, x, readdata);
                printf("******* SUCCESS %s ********   \n", readdata);
            }

            // char filename[200];
            // strncpy(filename, readvalueFromMemory(mem, i), sizeof(readvalueFromMemory(mem, i)) - 1);
            // FILE *fptr;
            // fptr = fopen(filename, "r");
            // if (fptr == NULL)
            // {
            //     printf("Error opening file %s\n", filename);
            //     return; // Exit function if file opening fails
            // }
            // char myString[300];
            // fgets(myString, 300, fptr);
            // printf("%s was read from file\n", myString);
            // fclose(fptr);
            // writevalueintoMemory(mem, i, myString);
            // printf("assign readfile executed\n");
        }
        if (extracted[9] == 'i')
        {

            printf("please enter a value:");
            fgets(value, sizeof(value), stdin);
            writevalueintoMemory(mem, i, value);
            printf("* %s written into address:%s **\n", value, destVariable);
        }
        else
        {
            int j = variablesfirstaddress;
            while (j < variableslastaddress)
            {
                char temp[2];
                temp[0] = extracted[9];
                temp[1] = '\0';
                if (strcmp(temp, readnameFromMemory(mem, i)) == 0)
                {
                    writevalueintoMemory(mem, i, readvalueFromMemory(mem, j));
                }
                j++;
            }
            printf("assign executed\n");
        }
    }
    // case writefile
    else if (extracted[0] == 'w')
    {
        char filenameVar[2];
        filenameVar[0] = extracted[10];
        filenameVar[1] = '\0';
        char filename[100];
        char toBeWritten[300];
        int i = variablesfirstaddress;
        while (i < variableslastaddress)
        {
            if (strcmp(filenameVar, readnameFromMemory(mem, i)) == 0)
            {
                strcpy(filename, readvalueFromMemory(mem, i));
                // filename = readvalueFromMemory(mem, i);
                break;
            }
            i++;
        }
        int j = variablesfirstaddress;
        while (j < variableslastaddress)
        {
            char tempstring[2];
            tempstring[0] = extracted[12];
            tempstring[1] = '\0';
            if (strcmp(tempstring, readnameFromMemory(mem, i)) == 0)
            {
                break;
            }
            j++;
        }
        FILE *fptr;
        fptr = fopen(filename, "w");
        const char *content = readvalueFromMemory(mem, j);
        fprintf(fptr, "%s", content);
        fclose(fptr);
        printf("writeFile executed\n");
    }

    // case readFile
    // not needed
    else if (extracted[0] == 'r')
    {
        char filenameVar[2];
        filenameVar[0] = extracted[9];
        filenameVar[1] = '\0';
        int i = variablesfirstaddress;
        char extractedFilename[300];
        while (i < variableslastaddress)
        {

            if ((strcmp(readnameFromMemory(mem, i), filenameVar)) == 0)
            {

                strcpy(extractedFilename, readvalueFromMemory(mem, i));
            }
            else
            {
            }
            i++;
        }

        FILE *input = fopen(extractedFilename, "r");

        if (!input)
        {
            printf("error reading from file ");
            exit(EXIT_FAILURE);
        }

        // char filename[100];
        // strncpy(filename, readvalueFromMemory(mem, i), sizeof(readvalueFromMemory(mem, i)) - 1);
        // // = readvalueFromMemory(mem, i);
        // FILE *fptr;
        // fopen(filename, "r");
        // char myString[300];
        // fgets(myString, 300, fptr);
        // printf("%s was read from file\n", myString);
        // fclose(fptr);
    }
}
// ready q // 3 blocked // general blocked
// if a program is in it's last clock cycle and another program arrives at the same clock cycle , the program that was excuting will be the tail
//  (last in q)

// for pcb , when setting , change in the variable and call updatePcb . for getting , you must access memory

int getPc(int processID, memory *mem)
{

    if (processID == 1)
    {
        return atoi(mem->words[3].value);
    }
    else if (processID == 2)
    {
        return atoi(mem->words[23].value);
    }
    else if (processID == 3)
    {
        return atoi(mem->words[43].value);
    }
    else
    {
        printf("error");
    }
}

char **stringSplit(char *str, int *word_count)

{
    char *temp = strdup(str); // Duplicate the input string to avoid modifying the original
    if (temp == NULL)
    {
        perror("Failed to duplicate string");
        exit(EXIT_FAILURE);
    }

    // Calculate the maximum number of words (just a heuristic)
    int max_words = strlen(temp) / 2 + 1; // +1 to account for worst case
    char **words = malloc(max_words * sizeof(char *));
    if (words == NULL)
    {
        perror("Failed to allocate memory");
        free(temp);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char *token = strtok(temp, " ");
    while (token != NULL && count < max_words)
    {
        words[count] = strdup(token); // Duplicate each token to store in words array
        if (words[count] == NULL)
        {
            perror("Failed to duplicate token");
            for (int i = 0; i < count; i++)
            {
                free(words[i]);
            }
            free(words);
            free(temp);
            exit(EXIT_FAILURE);
        }
        count++;
        token = strtok(NULL, " ");
    }

    // Reallocate array to fit the actual number of words
    char **resized_words = realloc(words, count * sizeof(char *));
    if (resized_words == NULL)
    {
        perror("Failed to reallocate memory");
        for (int i = 0; i < count; i++)
        {
            free(words[i]);
        }
        free(words);
        free(temp);
        exit(EXIT_FAILURE);
    }
    words = resized_words;

    free(temp);
    *word_count = count;
    return words;
}

void runAll(int quanta, int arrival1, int arrival2, int arrival3, memory *mem)
{
    int clock = 0;

    //     typedef struct
    // {
    //     int processID;
    //     char ProcessState[20]; // BLOCKED OR READY
    //     int CurrentPriority;
    //     int ProgramCounter;
    //     int lower;
    //     int upper;
    // } Pcb;

    // in each clock cycle print the running process
    // print full memory
    printf("------------------------------------\n");

    while (clock >= 0 && clock < 30)
    {
        printf("clock cycle: %d \n", clock);
        // no 2 processes arrive at the same time

        if (clock == arrival1)
        {
            printf("arr1\n");
            createProcess(1, mem);
            enqueue(&readyQueue, atoi(mem->words[0].value));
        }
        if (clock == arrival2)
        {
            printf("arr2\n");

            createProcess(2, mem);
            enqueue(&readyQueue, atoi(mem->words[20].value));
        }
        if (clock == arrival3)
        {
            printf("arr3\n");

            createProcess(3, mem);
            enqueue(&readyQueue, atoi(mem->words[40].value));
        }

        if (queueEmpty(&readyQueue) != 1)
        {

            if (readyQueue.q[0] == 1)
            {
                printf("currently running process is 1 \n");

                executeOneLine(1, mem);

                char newPC[3];
                sprintf(newPC, "%d", atoi(mem->words[3].value) + 1);
                strcpy(mem->words[3].value, newPC);

                program1.executiontime = program1.executiontime - 1;
                if (program1.executiontime == 0)
                {
                    dequeue(&readyQueue);
                    p1Done = 1;
                }
                else
                {
                    program1.quantum = program1.quantum - 1;
                    printf("program 1 quantum is now %d \n", program1.quantum);

                    if (program1.quantum == 0)
                    {
                        enqueue(&readyQueue, dequeue(&readyQueue));
                        program1.quantum = globalQuantum;
                    }
                }

                // in create program we need to set the quantum of the programVars
                //  update quanta
                //  update service time
                //  move to the end of queue if quanta done
                // change while condition
            }
            else if (readyQueue.q[0] == 2)
            {

                printf("currently running process is 2 \n");

                executeOneLine(2, mem);

                char newPC[3];
                sprintf(newPC, "%d", atoi(mem->words[23].value) + 1);
                strcpy(mem->words[23].value, newPC);

                program2.executiontime = program2.executiontime - 1;
                if (program2.executiontime == 0)
                {
                    dequeue(&readyQueue);
                    p2Done = 1;
                }
                else
                {
                    program2.quantum = program2.quantum - 1;
                    printf("program 2 quantum is now %d \n", program2.quantum);

                    if (program2.quantum == 0)
                    {
                        enqueue(&readyQueue, dequeue(&readyQueue));
                        program2.quantum = globalQuantum;
                    }
                }
            }
            else if (readyQueue.q[0] == 3)
            {

                printf("currently running process is 3 \n");

                executeOneLine(3, mem);

                char newPC[3];
                sprintf(newPC, "%d", atoi(mem->words[43].value) + 1);
                strcpy(mem->words[43].value, newPC);

                program3.executiontime = program3.executiontime - 1;
                if (program3.executiontime == 0)
                {
                    dequeue(&readyQueue);
                    p3Done = 1;
                }
                else
                {
                    program3.quantum = program3.quantum - 1;
                    printf("program 3 quantum is now %d \n", program3.quantum);

                    if (program3.quantum == 0)
                    {
                        enqueue(&readyQueue, dequeue(&readyQueue));
                        program3.quantum = globalQuantum;
                    }
                }
            }
        }

        // find out if i should stop the excution
        if (p1Done == 1 && p2Done == 1 && p3Done == 1)
        {
            clock = -3;
        }
        clock++;
        // printFullMemory(mem);
        // printf("\n");
        // printf("ready queue \n");
        // printQueue(&readyQueue);
        // printf("\n");
        // printf("userInput block queue \n");
        // printQueue(&userInput.q);
        // printf("\n");
        // printf("userOutput block queue \n");
        // printQueue(&userOutput.q);
        // printf("\n");
        // printf("file block queue \n");
        // printQueue(&file.q);
        // printf("\n");
        // printf("generalBlockQueue \n");
        // printQueue(&generalBlockQueue);
        // printf("\n");

        printf("------------------------------------\n");
    }
}

void initializeQueueEnds()
{
    readyQueue.end = -1;
    userInput.q.end = -1;
    userOutput.q.end = -1;
    file.q.end = -1;
    generalBlockQueue.end = -1;
}

int main()
{
    memory mem;

    intializeMemory(&mem);
    createProcess(1, &mem);
    createProcess(2, &mem);
    createProcess(3, &mem);
    printFullMemory(&mem);

    // intializeMemory(&mem);
    // initializeQueueEnds();
    // userOutput.value = one;
    // userInput.value = one;
    // file.value = one;

    // int arrival1 = 0;
    // int arrival2 = 1;
    // int arrival3 = 4;

    // printf("Please enter the quantum: ");
    // scanf("%d", &globalQuantum);

    // printf("Please enter arrival time of process1: ");
    // scanf("%d", &arrival1);

    // printf("Please enter arrival time of process2: ");
    // scanf("%d", &arrival2);

    // printf("Please enter arrival time of process3: ");
    // scanf("%d", &arrival3);
    // program1.quantum = globalQuantum;
    // program2.quantum = globalQuantum;
    // program3.quantum = globalQuantum;

    // runAll(globalQuantum, arrival1, arrival2, arrival3, &mem);

    // readInstructionsFromTxtfile("Program_1.txt", &memory);

    // queue data structure tests
    //  queue q = {{}, -1};
    // enqueue(&q, 10);
    // enqueue(&q, 20);
    // enqueue(&q, 30);
    // dequeue(&q);
    // printQueue(&q);

    // mutex tests
    // userInput.value = one;
    // userInput.ownerID = -1;
    // userInput.q.end = -1;

    // semWaitB(&userInput, 1);
    // semWaitB(&userInput, 2);
    // semWaitB(&userInput, 3);
    // printQueue(&(userInput.q));
    // semSignalB(&userInput, 1);
    // semWaitB(&userInput, 3);
    // printQueue(&(userInput.q));

    return 0;
}

//   int pc = getPc(1, mem);
//                     char instruction[100];
//                     strcpy(instruction, mem->words[pc].value);
//                     int wordCount;
//                     char **wordsOfInstruction = stringSplit(instruction, wordCount);

//                     if (strcmp(wordsOfInstruction[0], "semWait") == 0)
//                     {
//                         char *resourceName = wordsOfInstruction[1];

//                         if (strcmp(resourceName, "userInput") == 0)
//                         {
//                             if()userInput.value

//                         }
//                         else if (strcmp(resourceName, "userOutput") == 0)
//                         {
//                         }
//                         else if (strcmp(resourceName, "file") == 0)
//                         {
//                         }
//                     }