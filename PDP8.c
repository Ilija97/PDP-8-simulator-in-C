#include<stdio.h>
#include<stdlib.h>

int RAM[4096][16]; //ram memory

int PC[12];  // program counter
int I[1], OPR[3], MAR[12];
int MBR[16];  //buffer
int AC[16];
int E;
int SC[2];
int S;
int F, R;
int ZERO[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void Decoder2x4(int x[2], int e, int y[4])
{
    y[0] = !x[0] && !x[1] && e;
    y[1] = !x[0] && x[1] && e;
    y[2] = x[0] && !x[1] && e;
    y[3] = x[0] && x[1] && e;
}

void Decoder3x8(int x[3], int e, int y[8])
{
    y[0] = !x[0] && !x[1] && !x[2] && e;
    y[1] = !x[0] && !x[1] && x[2] && e;
    y[2] = !x[0] && x[1] && !x[2] && e;
    y[3] = !x[0] && x[1] && x[2] && e;

    y[4] = x[0] && !x[1] && !x[2] && e;
    y[5] = x[0] && !x[1] && x[2] && e;
    y[6] = x[0] && x[1] && !x[2] && e;
    y[7] = x[0] && x[1] && x[2] && e;
}

void increase(int x[], int n)  //povecava vrijednost registra za 1
{
    int q=1;

    for(int i=n-1; i>=0; i--)
    {
        int qn = x[i] && q;
        x[i] = q ^ x[i];
        q = qn;
    }
}


int loadProgramFrom(char* filename) // funkcija koja cita program u memoriju
{
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        printf("Unable to open file");
        return -1;
    }

    char str[17];
    int arr[16];
    int i;
    int memory_commands;
    int memory_data;

    fgets(str, 100, f);
    charToBinary(arr, str, 16);                //prevodimo niz chara u binarni niz
    memory_commands = binaryToInt(arr, 16);
    i = memory_commands;

    while(fgets(str, 100, f)){                 //ucitava naredbe
        if(i > 4095){
            printf("Memory overflow");
            return;
        }

        charToBinary(arr, str, 16);            //ako je HEX 7777 prekidamo
        if(binaryToInt(arr, 16) == 30583)
            break;

        for(int j=0; j<16; j++){
            if(str[j] != 0 && str[j] != 10)
                RAM[i][j] = str[j] - '0';
        }
        i++;
    }

    fgets(str, 100, f);                        //ucitavamo pocetnu adresu za podatke
    charToBinary(arr, str, 16);
    memory_data = binaryToInt(arr, 16);
    i = memory_data;

    while(fgets(str, 100, f)){                 //ucitavamo podatke

        if(i>4095){
            printf("Memory overflow");
            return;
        }

        charToBinary(arr, str, 16);            //ako je HEX 7777 prekidamo
        if(binaryToInt(arr, 16) == 30583)
            break;

        for(int j=0; j<16; j++){
            if(str[j] != 0 && str[j] != 10)
                RAM[i][j] = str[j] - '0';
        }
        i++;
    }

    fgets(str, 100, f);                    //ucitavamo HEX FFFF
    fgets(str, 100, f);                    //ucitavamo PC
    charToBinary(arr, str, 16);
    write(PC, arr+4, 12);

    printf("Memory loading succesful \n----- \n");
    fclose(f);
    return 0;
}

void charToBinary(int* arr, char* str, int n){  //prevodi niz char u binarni niz int
    for(int i=0; i<n; i++){
        arr[i] = str[i] - '0';
    }
}

void write(int* w, int* r, int n) //funkcija koja prepisuje iz drugog registra u prvi registar duzine n
{
    for(int i=0; i<n; i++){
        w[i] = r[i];
    }
}

int binaryToInt(int* arr, int n) //funkcija koja konvertuje binarni niz duzine n u int
{
    int val = 0;
    for(int i=n-1; i>=0; i--){
        val = val + arr[n-1-i] * pow(2, i);
    }
    return val;
}

int integer(int* arr) //funkcija koja konvertuje binarni niz duzine 12 u int
{
    int val = 0;
    for(int i=11; i>=0; i--){
        val = val + arr[11-i] * pow(2, i);
    }
    return val;
}

void tact(int q[8], int t[4], int c[4], int B[12])
{
    //fetch cycle
    if(c[0] == 1){
        if(t[0] == 1){
            write(MAR, PC, 12);
            return;
        }
        if(t[1] == 1){
            write(MBR, RAM[integer(PC)], 16);
            increase(PC, 12);
            return;
        }
        if(t[2] == 1){
            write(I, MBR, 1);
            write(OPR, MBR+1, 3);
            return;
        }
        if(t[3] == 1){
            if((q[7] == 1 && I == 0) || (q[7] == 0 && I == 1)){
                R = 1;
            } else {
                F = 1;
            }
            return;
        }
        return;
    }
    //indirect cycle
    if(c[1] == 1){
        if(t[0] == 1){
            write(MAR, MBR+4, 12);
            return;
        }
        if(t[1] == 1){
            write(MBR, RAM[integer(PC)], 16);
            return;
        }
        if(t[2] == 1)
            return;

        if(t[3] == 1){;
            F = 1;
            R = 0;
            return;
        }
    }

    //execute cycle
    if(c[2] == 1){
        //AND
        if(q[0]){
            if(t[0]){
                write(MAR, MBR+4, 12);
                printf("Exec AND \n----- \n");
                printf("MAR <- MBR(AD) \n");
                return;
            }
            if(t[1]){
                write(MBR, RAM[integer(MAR)], 16);
                printf("MBR <- M \n");
                return;
            }
            if(t[2]){
                for(int i=15; i>=0; i--){
                    AC[i] = AC[i] & MBR[i];
                }
                printf("AC <- AC AND MBR \n");
                return;
            }
            if(t[3]){
                F = 0;
                printf("F <- 0 \n");
                printf("----- \n");
                return;
            }

        }
        //ADD
        if(q[1]){
            if(t[0] == 1){
                write(MAR, MBR+4, 12);
                printf("\nExec ADD \n----- \n");
                printf("MAR <- MBR \n");
                return;
            }
            if(t[1] == 1){
                write(MBR, RAM[integer(MAR)], 16);
                printf("MBR <- M \n");
                return;
            }
            if(t[2] == 1){
                for(int i=15; i>=0; i--){
                    int tmp = 0;
                    if(AC[i] + MBR[i] + E > 1){
                        tmp = 1;
                    }
                    AC[i] = (AC[i] ^ MBR[i]) ^ E;

                    E = tmp;
                }
                printf("E-AC <- AC + MBR \n");
                return;
            }
            if(t[3] == 1){
                F = 0;
                printf("F <- 0 \n");
                printf("----- \n");
                return;
            }
        }

        //LDA
        if(q[2]){
            if(t[0]){
                write(MAR, MBR+4, 12);
                printf("\nExec LDA \n----- \n");
                printf("MAR <- MBR \n");
                return;
            }
            if(t[1]){
                write(AC, ZERO, 16);
                write(MBR, RAM[integer(MAR)], 16);
                printf("MBR <- M, AC <- 0 \n");
                return;
            }
            if(t[2]){
                for(int i=0; i<16; i++){
                    AC[i] = AC[i] ^ MBR[i];
                }
                printf("AC <- AC + MBR \n");
            }

            if(t[3]){
                F = 0;
                printf("F <- 0 \n");
                printf("----- \n");
                return;
            }
        }

        //STA
        if(q[3]){
            if(t[0]){
                write(MAR, MBR+4, 12);
                printf("\nExec STA \n----- \n");
                printf("MAR <- MBR \n");
                return;
            }
            if(t[1]){
                write(MBR, AC, 16);
                printf("MBR <- AC \n");
                return;
            }
            if(t[2]){
                write(RAM[integer(MAR)], MBR, 16);
                printf("M <- MBR \n");
                return;
            }
            if(t[3]){
                printf("F <- 0 \n");
                printf("----- \n");
                F = 0;
                return;
            }
        }
        //BUN
        if(q[4]){
            if(t[0]){
                write(PC, MBR+4, 12);
                printf("\nExec BUN\n ----- \n");
                printf("PC <- MBR \n");
                return;
            }
            if(t[1]){
                printf("NOP \n");
                return;
            }
            if(t[2]){
                printf("NOP \n");
                return;
            }
            if(t[3]){
                printf("F <- 0 \n");
                printf("----- \n");
                F = 0;
                return;
            }
        }

        //BSA
        if(q[5] == 1){
            if(t[0]){
                write(MAR, MBR+4, 12);
                write(MBR+4, PC, 12);
                printf("\nExec BSA\n ----- \n");
                printf("MAR <- MBR, MBR <- PC \n");
                return;
            }
            if(t[1]){
                write(RAM[integer(MAR)], MBR, 16);
                printf("M <- MBR \n");
                return;
            }
            if(t[2]){
                increase(MAR, 12);
                write(PC, MAR, 12);
                printf("PC <- MAR+1 \n");
                return;
            }
            if(t[3]){
                printf("F <- 0 \n");
                printf("----- \n");
                F = 0;
                return;
            }
        }

        //ISZ
        if(q[6] == 1){
            if(t[0]){
                write(MAR, MBR+4, 12);
                write(MBR+4, PC, 12);
                printf("\nExec ISZ\n ----- \n");
                printf("MAR <- MBR \n");
                return;
            }
            if(t[1]){
                write(MBR, RAM[integer(MAR)], 16);
                printf("MBR <- M \n");
                return;
            }
            if(t[2]){
                printRegisters(MBR, 16);
                increase(MBR, 16);
                printf("MBR <- MBR + 1 \n");
                printRegisters(MBR, 16);
                return;
            }
            if(t[3]){
                write(RAM[integer(MAR)], MBR, 16);
                int f = 1;
                for(int i=0; i<16; i++){ //da li je MBR=0
                    if(MBR[i] == 1){
                        f = 0;
                        break;
                    }
                }
                if(f == 1){
                    increase(PC, 12);
                    printf("PC <- PC + 1, ");
                }
                printf("F <- 0 \n");
                printf("----- \n");
                F = 0;
                return;
            }
        }

        //register commands
        if(q[7] == 1){
            //CLA
            if(B[0]){
                if(t[0]){
                    printf("\nExec CLA \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    write(AC, ZERO, 16);
                    F = 0;
                    printf("AC <- 0, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //CLE
            if(B[1]){
                if(t[0]){
                    printf("\nExec CLE \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    E = 0;
                    F = 0;
                    printf("E <- 0, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //CMA
            if(B[2]){
                if(t[0]){
                    printf("\nExec CMA \n----- \n");
                    printf("NOP \n");
                    printRegisters(AC,16);
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    for(int i=15; i>=0; i--){
                        if(AC[i]==0){
                            AC[i] = 1;
                        } else {
                            AC[i] = 0;
                        }
                    }
                    F = 0;
                    printf("AC <- AC', ");
                    printf("F <- 0 \n");
                    printRegisters(AC,16);
                    printf("----- \n");
                    return;
                }
            }
            //CME
            if(B[3]){
                if(t[0]){
                    printf("\nExec CME \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    if(E == 0){
                        E = 1;
                    } else {
                        E = 0;
                    }
                    F = 0;
                    printf("E <- E', ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //CIR
             if(B[4]){
                if(t[0]){
                    printf("\nExec CIR \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    E = AC[15];
                    for(int i=15; i>0; i--){
                        AC[i] = AC[i-1];
                    }
                    AC[0] = E;
                    F = 0;
                    printf("EAC <- cirEAC , ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //CIL
            if(B[5]){
                if(t[0]){
                    printf("\nExec CIL \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    E = AC[0];
                    for(int i=1; i<16; i++){
                        AC[i-1] = AC[i];
                    }
                    AC[15] = E;
                    F = 0;
                    printf("EAC <- cilEAC, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //INC
            if(B[6]){
                if(t[0]){
                    printf("\nExec INC \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    int f = 1;
                    for(int i=0; i<16; i){ //da li je AC=11...1 - prekoracenja
                        if(AC[i] == 0){
                            f = 0;
                            break;
                        }
                    }
                    if(f == 1){
                        write(AC, ZERO, 16);
                        E = 1;
                    } else {
                        increase(AC, 16);
                    }
                    F = 0;
                    printf("E-AC <- E-AC + 1, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //SPA
            if(B[7]){
                if(t[0]){
                    printf("\nExec SPA \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    if(AC[0] == 0){  //ako je AC>0
                        increase(PC, 12);
                    }
                    F = 0;
                    printf("if(AC>0) : PC <- PC+1, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
            //SNA
            if(B[8]){
                if(t[0]){
                    printf("\nExec SNA \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    if(AC[0] == 1){  //ako je AC<0
                        increase(PC, 12);
                    }
                    F = 0;
                    printf("if(AC<0) : PC <- PC+1, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }

            //SZA
            if(B[9]){
                if(t[0]){
                    printf("\nExec SZA \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    int f = 1;
                    for(int i=0; i<16; i++){ //da li je AC=0
                        if(AC[i] == 1){
                            f = 0;
                            break;
                        }
                    }
                    if(f == 1){
                        increase(PC, 12);
                    }
                    F = 0;
                    printf("if(AC=0) : PC <- PC+1, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }

            //SZE
            if(B[10]){
                if(t[0]){
                    printf("\nExec SZE \n----- \n");
                    printf("NOP \n");
                    return;
                }
                if(t[1]){
                    printf("NOP \n");
                    return;
                }
                if(t[2]){
                    printf("NOP \n");
                    return;
                }
                if(t[3]){
                    if(E == 0){
                        increase(PC, 12);
                    }
                    F = 0;
                    printf("if(E=0) : PC <- PC+1, ");
                    printf("F <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }

            //HLT
            if(B[11]){
                if(t[0])
                    return;
                if(t[1])
                    return;
                if(t[2])
                    return;
                if(t[3]){
                    printf("\nExec HLT \n----- \n");
                    S = 0;
                    printf("S <- 0 \n");
                    printf("----- \n");
                    return;
                }
            }
        }


    if(c[3] == 1)
        return;

    }
}


void runComputer()  //generator taktova
{
    printf("\nComputer has started \n-----\n");
    while(S)
    {
        int t[4];
        Decoder2x4(SC, 1, t);

        int FR[] = {F, R};
        int c[4];
        Decoder2x4(FR, 1, c);

        int q[8];
        Decoder3x8(OPR, 1, q);

        tact(q, t, c, MBR+4); //izvrsi se jedan takt
        increase(SC, 2);
    }
}

void printRegisters(int *a, int n)
{
    for(int i=0; i<n; i++)
        printf("%d", a[i]);

    printf("\n");
}

void printRAM(char* filename) // funkcija koja stampa ram
{
    FILE *f = fopen(filename, "w");
    if(f == NULL){
        printf("Unable to create a file");
        return;
    }
    for(int i=0; i<4096; i++){
        for(int j=0; j<16; j++){
            fprintf(f,"%d", RAM[i][j]);
        }
        fprintf(f,"\n");
    }
    printf("\nRAM data is saved in RAM.txt \n-----\n");
}

int createProgramFile()   //prevodi heksadicimalan kod u binarni i zapisuje ga u txt fajlu
{
        char str[4];

        FILE *fp = fopen("program_w.txt", "w+");
        if(fp == NULL){
            printf("Unable to create a file");
            return -1;
        }

        while(scanf(" %c%c%c%c",&str[0], &str[1], &str[2], &str[3])){
            if(str[0] == 'q' || str[0] == 'Q')
                break;
            for(int i=0; i<4; i++){
                    switch(str[i])
                    {
                        case '0':
                            fprintf(fp,"0000"); break;
                        case '1':
                            fprintf(fp,"0001"); break;
                        case '2':
                            fprintf(fp,"0010"); break;
                        case '3':
                            fprintf(fp,"0011"); break;
                        case '4':
                            fprintf(fp,"0100"); break;
                        case '5':
                            fprintf(fp,"0101"); break;
                        case '6':
                            fprintf(fp,"0110"); break;
                        case '7':
                            fprintf(fp,"0111"); break;
                        case '8':
                            fprintf(fp,"1000"); break;
                        case '9':
                            fprintf(fp,"1001"); break;
                        case 'A':
                            fprintf(fp,"1010"); break;
                        case 'B':
                            fprintf(fp,"1011"); break;
                        case 'C':
                            fprintf(fp,"1100"); break;
                        case 'D':
                            fprintf(fp,"1101"); break;
                        case 'E':
                            fprintf(fp,"1110"); break;
                        case 'F':
                            fprintf(fp,"1111"); break;
                        case 'a':
                            fprintf(fp,"1010"); break;
                        case 'b':
                            fprintf(fp,"1011"); break;
                        case 'c':
                            fprintf(fp,"1100"); break;
                        case 'd':
                            fprintf(fp,"1101"); break;
                        case 'e':
                            fprintf(fp,"1110"); break;
                        case 'f':
                            fprintf(fp,"1111"); break;
                        case '\n':
                            continue;
                        case 'q':
                            continue;
                        case 'Q':
                            continue;
                        default:
                            printf("invalid input");
                            return -1;
                    }
                }
                fprintf(fp,"\n");
            }
        fclose(fp);
    return 0;
}



int main()
{
    S=1;
    memset(RAM, 0, sizeof(RAM[0][0]) * 16 * 4096);

    int x;
    printf("PDP-8 Simulator\nIlija Gracanin 24/20\n-----\n\n");
    printf("Select loading method: \n  1  program.txt - binary \n  2  keyboard input - hexadecimal\n\n");
    scanf("%d", &x);
    printf("-----\n\n");
    if(x == 2){
        printf("Type in hexdecimal code. To end type 'QQQQ'\n\n");

        if(createProgramFile() == -1)
            return -1;

        printf("-----\n\n");
        if(loadProgramFrom("program_w.txt") == -1)
            return -3;
    } else if(x == 1){
        if(loadProgramFrom("program.txt") == -1)
            return -4;
    } else {
        printf("Invalid option. Please run the program again and type 1 or 2.\n");
        return -2;
    }

    runComputer();
    printRAM("RAM.txt");
    printf("\nShutting down\n-----\n");
    return 0;
}
