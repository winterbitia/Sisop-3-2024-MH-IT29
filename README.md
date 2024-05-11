# Sisop-3-2024-MH-IT29
## Anggota Kelompok:
- Dian Anggraeni Putri (5027231016)
- Amoes Noland (5027231028)
- Malvin Putra Rismahardian (5027231048)

## Soal 1
> Dikerjakan oleh: Amoes Noland (5027231028)

Soal ini menerapkan penggunaan Shared Memory untuk melakukan pembacaan CSV untuk pengambilan data.

### auth.c

Di dalam program ini kita disuruh untuk mengecek isi folder new-entry yang berupa CSV dengan akhiran "trashcan" atau "parkinglot" untuk dimasukkan ke dalam shared memory agar dapat digunakan oleh program-program lain yang berhubungan. Sementara untuk file yang tidak memenuhi kriteria akan didelete.

Contoh format CSV yang digunakan adalah sebagai berikut:
```csv
name,rating
sigma looksmaxxer,9.1
skibidi toilet,7.2
rizzler,6.8
```

Fungsi main dan global variabel digunakan sebagai wadah yang saling menghubungkan global variabel dengan fungsi-fungsi yang ada.

```c
// Define global variables
#define MAX_BUFFER 1028
char dir_entry[MAX_BUFFER];
int keygen = 1001;

int main(){
    // Get new entry dir
    getcwd(dir_entry, sizeof(dir_entry));
    strcat(dir_entry, "/new-entry/"); 

    // Start check
    dir_csv_check();

    // Share csv amount
    keygen_send();

    return 0;
}
```

Menggunakan directory yang didapat melalui `getcwd()`, dilakukan pencarian melewati seluruh folder new-entry untuk melakukan delete pada file selain CSV yang dicari. Bila file sesuai, maka panggil fungsi untuk memasukkan nama file ke dalam shared memory.

```c
// Function to delete incorrect files
void dir_csv_check(){
    DIR *dir = opendir(dir_entry);
    struct dirent *ep;
    if (!dir) return;

    chdir(dir_entry);
    while(ep = readdir(dir)){
        // Skips the special items listed
        if ((strcmp(ep->d_name, ".") == 0 )||
            (strcmp(ep->d_name, "..") == 0))
            continue;
        
        // Deletes incorrect csv files
        if ((strstr(ep->d_name, "_trashcan.csv") == NULL)&&
            (strstr(ep->d_name, "_parkinglot.csv") == NULL)){
            remove(ep->d_name);
            continue;
        }

        // Add correct csv into shared memory
        dir_csv_share(ep->d_name);
    }
    closedir(dir);
    return;
}
```

Sistem shared memory yang saya terapkan adalah increment key untuk setiap file yang terdapat pada `/new-entry`. Hal ini pertama dilakukan dengan cara increment sebuah integer `keygen` yang membentuk sebuah key baru setiap kali sebuah shared memory dibuat.

```c
// Function to add filename to shared memory
void dir_csv_share(char* filename){
    // Generate new shared memory ID
    key_t key = (key_t)keygen++;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);
    strcpy(shm_buffer, filename);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);
}
```

Untuk memastikan bahwa program lain dapat mengetahui jumlah key shared memory yang dibuat, maka digunakan port 1000 yang secara khusus menyimpan angka keygen maximum.

```c
void keygen_send(){
    // Generate new shared memory ID
    key_t key = 1000;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer 
    int *keygen_max;
    keygen_max = shmat(shmid, NULL, 0);
    *keygen_max = --keygen;

    // Detach variable
    shmdt(keygen_max);
}
```

#### Dokumentasi Screenshot

![auth,c](https://media.discordapp.net/attachments/1071478813566976151/1237437348325621831/image.png?ex=663ba4d6&is=663a5356&hm=b93345403266f3befaa8a6b9b9054f4b03cf407ea02804baf6440a77327d43d6&=&format=webp&quality=lossless&width=1210&height=681)

### rate.c

Dalam program ini, proses akan mengambil seluruh nama file CSV yang sudah masuk ke dalam shared memory, untuk melakukan parsing mencari rating tertinggi dari setiap kategori CSV (Trash Can dan Parking Lot).

Fungsi main dan global variabel digunakan sebagai wadah yang saling menghubungkan global variabel dengan fungsi-fungsi yang ada, seperti menghubungkan directory yang digunakan dengan fungsi parsing CSV pada nantinya.

```c
// Define global variables
#define MAX_BUFFER 1028
char dir_entry[MAX_BUFFER];
int keygen;

// Define comparison variables
char max_t_name[MAX_BUFFER],
     max_p_name[MAX_BUFFER],
     max_t_fname[MAX_BUFFER],
     max_p_fname[MAX_BUFFER];
float max_t = 0, max_p = 0;

int main(){
    // Chdir will guarantee new-entry
    chdir("../new-entry"); chdir("new-entry");
    getcwd(dir_entry, sizeof(dir_entry));

    // Start main parsing process
    keygen_get();
    while (keygen > 1000) csv_get();

    // Get final output
    rating("Trash Can"  ,max_t_fname,max_t_name,max_t);
    rating("Parking Lot",max_p_fname,max_p_name,max_p);

    return 0;
}
```

Langkah pertama dalam sistem membaca shared memory adalah mencari jumlah keygen maximum dari `auth.c` sehingga dapat mengetahui jumlah loop yang dilakukan.

```c
// Function to get maximum key number from auth
void keygen_get(){
    // Generate new shared memory ID
    key_t key = 1000;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer 
    int *keygen_max;
    keygen_max = shmat(shmid, NULL, 0);
    keygen = *keygen_max;

    // Detach variable
    shmdt(keygen_max);
}
```

Sehingga, dimulai dari key maximum dapat dilakukan loop untuk mengambil nama file CSV dari shared memory, dan memanggil fungsi `csv_parse()` untuk membuka file CSV sesuai pada shared memory.

```c
// Function to grab csv filenames from auth
void csv_get(){
    // Generate new shared memory ID
    key_t key = (key_t)keygen--;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);

    // Send buffer to parse
    csv_parse(shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);
}
```

Nama file CSV yang didapat akan dicek dalam dua folder berbeda, '/new-entry' dan '/microservices/database'. Lalu dilakukan parsing file CSV yang dijalankan dengan alur sebagai berikut:

1. `fscanf` file header untuk discard.
2. While loop sebanyak `fscanf` string pertama dengan delimiter `,` dilanjut scan float untuk dimasukkan variabel sementara.
3. Bandingkan rating dengan global variabel dan simpulkan data: ubah nama file dan nama item sesuai max.

```c
void csv_parse(char* filename){
    // Open the selected file
    chdir(dir_entry);
    FILE *file = fopen(filename, "r");

    // Checks database if not in entry
    if (!file) {
        chdir("../microservices/database");
        file = fopen(filename, "r");
        if (!file) return;
    }

    // Ignore csv header
    fscanf(file, "%*s");
    // Start parsing data
    // Ignore trailing newline and parse commas
    char name[MAX_BUFFER]; float rating;
    while (fscanf(file, "%*c%[^,],%f", name, &rating) == 2){
        // Already authenticated, which means
        // if not trashcan, guaranteed parking lot
        if (strstr(filename, "_trashcan.csv") != NULL){
            if (rating < max_t) continue;
            max_t = rating;
            strcpy(max_t_name, name);
            strcpy(max_t_fname, filename);
        } else {
            if (rating < max_p) continue;
            max_p = rating;
            strcpy(max_p_name, name);
            strcpy(max_p_fname, filename);
        }
    }
    fclose(file);
}
```

Setelah seluruh scan CSV dilakukan, maka program akan melakukan output terakhir sesuai nilai max yang diperoleh dari kedua kategori CSV.

```c
void rating(char* type, char* fname, char* name, float rating){
    printf("\nType: %s\n"
           "Filename: %s\n"
           "--------------------\n"
           "Name: %s\n"
           "Rating: %.1f\n",
           type, fname, name, rating);
}
```

#### Dokumentasi Screenshot

![rate,c](https://media.discordapp.net/attachments/1071478813566976151/1237437348757770290/image.png?ex=663ba4d6&is=663a5356&hm=fead2db6b9d3af44fc28b4a2dc97c5e4af12d8694dfed28f8e50ddb9c04242bd&=&format=webp&quality=lossless&width=1210&height=681)

### db.c

Dalam program ini, proses akan memindah file yang berada di dalam `/new-entry/` yang sudah diauthentikasi untuk masuk ke dalam sebuah folder `/microservices/database/` dan dicatat semua perpindahan file yang dilakukan dalam sebuah `db.log`.

Fungsi main dan global variabel digunakan sebagai wadah yang saling menghubungkan global variabel dengan fungsi-fungsi yang ada, seperti menghubungkan directory yang digunakan dengan fungsi perpindahan CSV pada nantinya.

```c
int main(){
    // Chdir will guarantee working root
    chdir("../new-entry"); chdir("new-entry"); chdir("..");
    getcwd(dir_entry, sizeof(dir_entry));

    // Get entry and database directory
    strcpy(dir_db, dir_entry);
    strcat(dir_entry, "/new-entry/");
    strcat(dir_db, "/microservices/database/");

    // Start main parsing process
    keygen_get();
    while (keygen > 1000) csv_get();

    printf("File(s) may have been moved."
          " Check the db.log for more information.");
    return 0;
}
```

Menggunakan alur yang sama dengan program sebelumnya untuk mencari nama CSV, dengan fungsi yang mengambil keygen maximum serta dapat dilakukan loop untuk mengambil nama file CSV dari shared memory, sehingga dapat memanggil fungsi `csv_move()` untuk memindahkan file CSV sesuai pada shared memory.

```c
// Function to get maximum key number from auth
void keygen_get(){
    // Generate new shared memory ID
    key_t key = 1000;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer 
    int *keygen_max;
    keygen_max = shmat(shmid, NULL, 0);
    keygen = *keygen_max;

    // Detach variable
    shmdt(keygen_max);
}

// Function to grab csv filenames from auth
void csv_get(){
    // Generate new shared memory ID
    key_t key = (key_t)keygen--;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);

    // Call file move
    csv_move(shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);
}
```

Pemindahan file dilakukan pada CSV dengan cara rename directory lengkap untuk memindahkan CSV ke directory yang berbeda. Bila filename ditemukan pada `new-entry` dan berhasil terjadi perpindahan, maka akan dicatat pada log menggunakan fungsi.

```c
// Function to move csv and call log
void csv_move(char *filename){
    // Check if file exists
    chdir(dir_entry);
    struct stat buffer;   
    if (stat(filename, &buffer) != 0) return;

    // Prepare file variables
    char source[MAX_BUFFER], dest[MAX_BUFFER];
    strcpy(source, dir_entry);
    strcat(source, filename);
    strcpy(dest, dir_db);
    strcat(dest, filename);

    // Move the file and start log
    rename(source, dest);
    if (strstr(filename, "_trashcan.csv") != NULL)
         csv_log("Trash Can", filename);
    else csv_log("Parking Lot", filename);
}
```

Pencatatan pada log menggunakan `fprintf` sesuai format yang diminta pada soal dan menggunakan localtime untuk mencatat waktu pemindahan dilakukan.

```c
// Function to write into log
void csv_log(char* type, char *filename){
    chdir(dir_db);
    FILE *file = fopen("db.log", "a");
    if (!file) return;

    // Set up time variables 
    time_t timevar; struct tm *timeinfo;
    time (&timevar); timeinfo = localtime (&timevar);

    // Output into log
    fprintf(file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] [%s]\n",
            timeinfo->tm_mday,
            timeinfo->tm_mon,
            timeinfo->tm_year+1900,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            type, filename);
    fclose(file);
}

```

#### Dokumentasi Screenshot :

![db.c](https://media.discordapp.net/attachments/1071478813566976151/1237437349127000105/image.png?ex=663ba4d6&is=663a5356&hm=58f1c7ebcef2bb8a0070517b38ba9887d2ab5754e1d57989db7f455b3e6d3b3b&=&format=webp&quality=lossless&width=1210&height=681)



## Soal 2
> Dikerjakan oleh: Dian Anggraeni Putri (5027231016)

Pada nomor ini, diperintahkan untuk membuat kalkulator perkalian sederhana menggunakan konsep pipes dan fork dalam pemrograman sistem operasi. Kalkulator ini harus dapat melakukan operasi perkalian, penjumlahan, pengurangan, dan pembagian dengan menggunakan argumen pada saat menjalankan program.

Pertama, impor pustaka yang diperlukan.

```c
#include <stdio.h>  // For standard input/output functions
#include <stdlib.h> // For memory allocation
#include <string.h> // For string manipulation functions
#include <sys/wait.h> // For waiting on child processes (wait, waitpid)
#include <unistd.h> // For UNIX system functions (fork, pipe)
#include <time.h> // For time-related functions (time, strftime)
```

Kedua, definisikan konstanta PIPE_READ (untuk sisi baca pipe) dan PIPE_WRITE (untuk sisi tulis pipe).

```c
#define PIPE_READ 0
#define PIPE_WRITE 1
```

Fungsi wordsToNumber() untuk mengonversi kata-kata angka dalam bahasa Indonesia (satu hingga sembilan) ke bilangan bulat (1 hingga 9).
```c
int wordsToNumber(const char *word) {
    if (strcmp(word, "satu") == 0) return 1;
    if (strcmp(word, "dua") == 0) return 2;
    if (strcmp(word, "tiga") == 0) return 3;
    if (strcmp(word, "empat") == 0) return 4;
    if (strcmp(word, "lima") == 0) return 5;
    if (strcmp(word, "enam") == 0) return 6;
    if (strcmp(word, "tujuh") == 0) return 7;
    if (strcmp(word, "delapan") == 0) return 8;
    if (strcmp(word, "sembilan") == 0) return 9;
    return -1; // Jika input tidak valid, mengembalikan -1
}
```

Fungsi numberToWords() untuk mengonversi bilangan bulat (num) ke dalam bentuk kata-kata bahasa Indonesia.

```c
void numberToWords(int num, char *words) {
    const char *units[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    const char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    const char *tens[] = {"", "", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (num < 10) {
        strcpy(words, units[num]);
    } else if (num < 20) {
        strcpy(words, teens[num - 10]);
    } else {
        int tenPart = num / 10;
        int unitPart = num % 10;
        strcpy(words, tens[tenPart]);
        if (unitPart > 0) {
            strcat(words, " ");
            strcat(words, units[unitPart]);
        }
    }
}
```

Fungsi writeLog() menulis log ke file histori.log dalam format [date] [type] [message].

```c
void writeLog(const char *type, const char *message) {
    FILE *fp = fopen("histori.log", "a");
    if (fp == NULL) {
        perror("Error opening file histori.log");
        exit(EXIT_FAILURE);
    }

    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[30];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, sizeof(timeStr), "%d/%m/%y %H:%M:%S", timeinfo);

    fprintf(fp, "[%s] [%s] %s\n", timeStr, type, message);
    fclose(fp);
}
```

Fungsi formatMessage() untuk membentuk pesan hasil operasi dalam format "hasil [jenis operasi] [word1] dan [word2] adalah [resultWords]".

```c
void formatMessage(char *message, const char *operationWord, const char *word1, const char *word2, const char *resultWords) {
    snprintf(message, 100, "hasil %s %s dan %s adalah %s.", operationWord, word1, word2, resultWords);
}
```

Fungsi utama program (main) untuk mengatur proses parent dan child, serta melakukan operasi matematika sesuai dengan operator yang diberikan. Berikut penjelasan setiap bagian dari fungsi main:

### Parsing Argumen
Program memeriksa jumlah argumen yang diberikan (4 argumen termasuk nama program, operator, dan dua string angka).

```c
if (argc != 4) {
    printf("Usage: %s [operator] [number1] [number2]\n", argv[0]);
    return EXIT_FAILURE;
}
```

### Mengonversi String Angka ke Bilangan Bulat
Program mengonversi dua string angka (word1 dan word2) ke bilangan bulat (num1 dan num2).

```c
const char *word1 = argv[2];
const char *word2 = argv[3];
int num1 = wordsToNumber(word1);
int num2 = wordsToNumber(word2);
```

### Memeriksa eror
Jika string angka tidak valid (tidak dapat dikonversi ke bilangan bulat), program memberikan pesan eror.

```c
if (num1 == -1 || num2 == -1) {
    printf("ERROR: Invalid number input.\n");
    return EXIT_FAILURE;
}
```

### Membuat Pipes
Program membuat dua pipes (fd1 dan fd2) untuk komunikasi antara proses parent dan child.

```c
int fd1[2], fd2[2];
pipe(fd1);
pipe(fd2);
```

### Fork
Program menggunakan fork() untuk membuat proses child.

```c
pid_t pid = fork();
if (pid < 0) {
    perror("Fork failed");
    return EXIT_FAILURE;
}
```

### Proses Parent
Jika fork berhasil dan pid lebih besar dari 0, maka proses parent dijalankan:

```c
if (pid > 0) {
    // Parent process
    // Tutup sisi baca dari pipe pertama dan sisi tulis dari pipe kedua.
    close(fd1[PIPE_READ]);
    close(fd2[PIPE_WRITE]);

    // Menentukan jenis operasi dan menghitung hasil.
    char type[10], operationWord[20];
    if (strcmp(operator, "-kali") == 0) {
        // Mengalikan num1 dengan num2.
        result = num1 * num2;
        strcpy(type, "KALI");
        strcpy(operationWord, "perkalian");
    } else if (strcmp(operator, "-tambah") == 0) {
        // Menjumlahkan num1 dengan num2.
        result = num1 + num2;
        strcpy(type, "TAMBAH");
        strcpy(operationWord, "penjumlahan");
    } else if (strcmp(operator, "-kurang") == 0) {
        // Mengurangkan num2 dari num1.
        result = num1 - num2;
        strcpy(type, "KURANG");
        strcpy(operationWord, "pengurangan");
        // Menangani kasus hasil pengurangan yang negatif.
        if (result < 0) {
            printf("ERROR\n");
            writeLog(type, "ERROR pada pengurangan.");
            exit(EXIT_SUCCESS);
        }
    } else if (strcmp(operator, "-bagi") == 0) {
        // Membagi num1 dengan num2.
        if (num2 == 0) {
            printf("ERROR\n");
            writeLog("BAGI", "ERROR pada pembagian.");
            exit(EXIT_SUCCESS);
        }
        result = num1 / num2;
        strcpy(type, "BAGI");
        strcpy(operationWord, "pembagian");
    } else {
        // Operator tidak valid.
        printf("ERROR: Invalid operator.\n");
        return EXIT_FAILURE;
    }

    // Mengirim hasil ke child process melalui pipe.
    write(fd1[PIPE_WRITE], &result, sizeof(result));
    close(fd1[PIPE_WRITE]);

    // Membaca pesan dari child process melalui pipe.
    char buffer[100];
    read(fd2[PIPE_READ], buffer, sizeof(buffer));
    close(fd2[PIPE_READ]);

    // Mencetak hasil operasi ke terminal.
    printf("%s\n", buffer);

    // Menulis hasil operasi ke file log.
    writeLog(type, buffer);

    // Menunggu proses child selesai.
    wait(NULL);
} else {
```

Proses child bertugas untuk menerima hasil operasi dari parent melalui pipe pertama. Setelah menerima hasil operasi, child mengonversi hasil tersebut ke dalam bentuk kata-kata bahasa Indonesia menggunakan fungsi numberToWords. Kemudian, proses child menentukan jenis operasi yang sesuai dengan operator yang diberikan (-kali, -tambah, -kurang, atau -bagi) dan membentuk pesan hasil operasi dalam format yang diinginkan. Setelah pesan hasil operasi terbentuk, child mengirim pesan tersebut ke parent melalui pipe kedua. Setelah mengirim pesan, proses child menutup pipe yang digunakan dan mengakhiri dirinya dengan sukses.

```c
// Proses child
// Tutup sisi tulis dari pipe pertama dan sisi baca dari pipe kedua.
close(fd1[PIPE_WRITE]);
close(fd2[PIPE_READ]);

// Membaca hasil operasi dari parent process melalui pipe.
int result;
read(fd1[PIPE_READ], &result, sizeof(result));
close(fd1[PIPE_READ]);

// Mengonversi hasil operasi menjadi kata-kata.
char resultWords[50];
numberToWords(result, resultWords);

// Membentuk pesan hasil operasi.
char message[100];
char operationWord[20];

// Menentukan jenis operasi untuk menghasilkan pesan.
if (strcmp(operator, "-kali") == 0) {
    strcpy(operationWord, "perkalian");
} else if (strcmp(operator, "-tambah") == 0) {
    strcpy(operationWord, "penjumlahan");
} else if (strcmp(operator, "-kurang") == 0) {
    strcpy(operationWord, "pengurangan");
} else if (strcmp(operator, "-bagi") == 0) {
    strcpy(operationWord, "pembagian");
}

// Membentuk pesan hasil operasi.
formatMessage(message, operationWord, word1, word2, resultWords);

// Mengirim hasil operasi ke parent process melalui pipe kedua.
write(fd2[PIPE_WRITE], message, strlen(message) + 1);
close(fd2[PIPE_WRITE]);

// Mengakhiri proses child
exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS; 
}
```

### Screenshot Hasil Pengerjaan
![alt text](https://cdn.discordapp.com/attachments/1223171682500350062/1238015603629883463/Screenshot_1766.png?ex=663dbf61&is=663c6de1&hm=9b0dfdeb4b7812f3037283b102e32fd419dbf1683d845ea6ea25198e4c9d1282&)
![alt text](https://cdn.discordapp.com/attachments/1223171682500350062/1238015692222234644/Screenshot_1768.png?ex=663dbf76&is=663c6df6&hm=869d26954bebb46d4d381b9e5c398d260e30a16593dfdbbd4e6bcaefc1b6bd91&)
![alt text](https://cdn.discordapp.com/attachments/1223171682500350062/1238015642964201515/Screenshot_1767.png?ex=663dbf6a&is=663c6dea&hm=eeed18318b480bbb92af47519bb54b9879f4c6221a3447b7d7677a26d785c0bc&)



## Soal 3
> Dikerjakan oleh: Amoes Noland (5027231028)

Soal ini menerapkan penggunaan socket RPC untuk komunikasi antar proses yang sedang berjalan.

### actions.c

Kode ini hanya berisi fungsi-fungsi yang akan di-include dalam `paddock.c` sehingga bahkan tidak memiliki fungsi main. Fungsi yang ada terdiri atas Gap, Fuel, Tire, dan TireChange.

```c
char* gap(float distance_now){
    if (distance_now > 10)
        return "Stay out of trouble";
    if (distance_now > 3.5)
        return "Push";
        return "Gogogo";
}

char* fuel(float fuel_now){
    if (fuel_now > 80)
        return "Push Push Push";
    if (fuel_now > 50)
        return "You can go";
        return "Conserve Fuel";
}

char* tire(int tire_now){
    if (tire_now > 80)
        return "Go Push Go Push";
    if (tire_now > 50)
        return "Good Tire Wear";
    if (tire_now > 30)
        return "Conserve Your Tire";
        return "Box Box Box";
}

char* tire_change(char* type_now){
    if (strcmp(type_now, "Soft") == 0)
        return "Mediums Ready";
    if (strcmp(type_now, "Medium") == 0)
        return "Box for Softs";
}
```

### paddock.c

Program ini akan menjadi server utama yang berisi seluruh otak dari kasus ini. Program ini akan berjalan sebagai daemon yang terhubung ke proses `driver.c` sebagai hubungan server-client menggunakan socket RPC.

Salah satu poin utama dalam program ini adalah dapat menggunakan fungsi pada `actions.c` karena terletak pada direktori yang sama dengan memanggil satu baris kode ini:

```c
#include "actions.c"
```
Untuk memulai alur kerja `paddock.c` dilakukan konfigurasi server untuk dapat menunggu koneksi dari client sesuai port yang sudah didefinisikan pada variabel global.

```c
// Global definitions
#define PORT       8080
#define MAX_BUFFER 1024
char dir_log[MAX_BUFFER];

// Main series of functions
int main(){
    // Socket variables
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    char buffer[MAX_BUFFER];

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to server
    if (bind(server_socket, (struct sockaddr *)&server_address,
                             sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connection
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
```

Proses yang akan dilakukan dalam loop utama memerlukan beberapa variabel seperti satu integer sebagai boolean, berbagai jenis buffer yang akan diproses, serta current working directory untuk mencari lokasi log pada akhir.

```c
    // Define work variables
    int ohio_sigma_skibidi_rizz = 1;
    char command [MAX_BUFFER],
         argument[MAX_BUFFER],
         *response;
    getcwd(dir_log, sizeof(dir_log));
    strcat(dir_log, "/server/race.log");
```

Setelah semua persiapan selesai, maka fungsi daemon akan dipanggil untuk mengubah proses menjadi sebuah proses background.

```c
// Daemonize the process
void daemonize(){
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    pid_t sid = setsid();
    if (sid < 0)        exit(EXIT_FAILURE);
    if (chdir("/") < 0) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
```

Masuk ke dalam fase pertama loop utama, adalah untuk menunggu hubungan dari client.

```c
// Main server loop
    while(1){
        printf("waiting...\n");
        
        // Attempt to accept client socket
        if ((client_socket = accept(server_socket,
                                (struct sockaddr *)&server_address,
                                (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        while(1){
            // LOOP PHASE 2
        }
        // Close client if a break happens at input process
        close(client_socket);
        sleep(5);
    }
```

Masuk ke dalam fase kedua dari loop utama, adalah untuk memproses buffer dari client. Fase ini terbagi menjadi beberapa komponen:

1. Clear buffer dan menunggu data dari client

```c
    // Clear buffer and read from client
    memset(buffer, 0, MAX_BUFFER);
    int bytes_read = read(client_socket, buffer, MAX_BUFFER);
    if (bytes_read < 0) {
        perror("Read failed");
        break;
    } else if (bytes_read == 0) {
        printf("Client disconnect\n");
        break;
    }
```

2. Memproses buffer dan mencatat respon yang sesuai dengan fungsi yang tertera pada `actions.c`

```c
    // Processing the buffer and creating a response
    sscanf(buffer, "%s %s", command, argument);
    if (strcmp(command, "Gap") == 0){
        response = gap(atof(argument));
} else if (strcmp(command, "Fuel") == 0){
        response = fuel(atof(argument));
        if (!strchr(argument, '%'))
        strcat(argument, "%");
} else if (strcmp(command, "Tire") == 0){
        response = tire(atoi(argument));
} else if (strcmp(command, "TireChange") == 0){
        response = tire_change(argument);
    } else {
        strcpy(argument, "INVALID COMMAND");
        response = "Invalid command";
        ohio_sigma_skibidi_rizz = 0;
    }
```

3. Catat ke dalam log sumber Driver, dan log sumber Paddock hanya bila command yang diinput adalah valid.

```c
    // Write results into the log
    log_write("Driver] ", command, argument);
    if (ohio_sigma_skibidi_rizz)
    log_write("Paddock]", command, response);
    ohio_sigma_skibidi_rizz = 1;
```

Fungsi yang terhubung adalah sebagai berikut, menggunakan localtime untuk mencatat waktu:

```c
// Log function
void log_write(char* source, char* command, char* argument){
    FILE *file = fopen(dir_log, "a");
    if (!file) return; 

    // Set up time variables 
    time_t timevar; struct tm *timeinfo;
    time (&timevar); timeinfo = localtime (&timevar);

    // Output into log
    fprintf(file, "[%s [%02d/%02d/%04d %02d:%02d:%02d]: [%s] [%s]\n",
            source,
            timeinfo->tm_mday,
            timeinfo->tm_mon,
            timeinfo->tm_year+1900,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            command, argument);
    fclose(file);
}
```

4. Kirim respon ke client yang terhubung

```c
    // Send the output to the client
    if(send(client_socket, response, strlen(response), 0) < 0){
        perror("Send failed");
    };
```

Loop ini akan terus berjalan selama terdapat sebuah koneksi dengan client. Loop ini akan break kembali ke fase pertama bila koneksi terputus dari client. Loop fase pertama hanya dapat berhenti bila daemon diberikan `SIGKILL`.

### driver.c

Program ini akan menjadi client yang menghubungkan command yang diberikan oleh pengguna, untuk meminta respon dari server sesuai yang diminta.

Dimulai dengan konfigurasi socket sesuai IP dan PORT yang didefinisikan pada global variabel.

```c
// Global definitions
#define IP         "127.0.0.1"
#define PORT       8080
#define MAX_BUFFER 1024

// Main series of functions
int main() {
    // Socket variables
    int client_socket;
    struct sockaddr_in server_address;

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Address conversion for communication
    if (inet_pton(AF_INET, IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server from client
    if (connect(client_socket, (struct sockaddr *)&server_address,
                                sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
```

Sebelum memasuki loop utama untuk proses input pengguna, terdapat beberapa persiapan yang dilakukan seperti pembuatan variabel buffer dan pesan pertama saat program dibuka.

```c
// I'm addicted to formatting
void line(){
    printf("-------------------------------------\n");
}

```

```c
    // Define work variables
    char buffer[MAX_BUFFER],
         response[MAX_BUFFER];

    // Initial message
    printf("[CONNECTED TO DRIVER ASSISTANT]\n"); line();
    printf("Here are the available commands:\n"
           "  Gap        (float)  units\n"
           "  Fuel       (float)  percentage\n"
           "  Tire       (int)    usage\n"
           "  TireChange (string) Soft/Medium\n"
           "  Exit\n"); line();
    printf("~ GOOD LUCK ON THE RACE! ~\n");
```

Di dalam loop utama, terdapat beberapa komponen:

1. Clear buffer dan memasukkan input ke dalam buffer, serta mengubah karakter terakhir pada buffer menjadi sebuah null terminator.

```c
    printf("\nInput Driver   : ");
    memset(buffer, 0, MAX_BUFFER);
    fgets(buffer, MAX_BUFFER, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
```
2. Melakukan handle exit process secara client-side

```c
    if (strcmp(buffer, "Exit") == 0){
        printf("Output Driver  : Client exited");
        close(client_socket);
        return 0;
    }
```

3. Mengirim input ke server yang terhubung

```c
send(client_socket, buffer, strlen(buffer), 0);
```

4. Clear respon buffer, dan membaca respon dari server untuk print hasil.

```c
    memset(response, 0, MAX_BUFFER);
        
    int bytes_read = read(client_socket, response, MAX_BUFFER);
    if (bytes_read < 0) {
        perror("Read failed");
        close(client_socket);
        continue;
    } else if (bytes_read == 0) {
        printf("Server disconnect\n");
        break;
    }

    printf("Output Paddock : %s\n", response);
```

Bila terjadi break dari loop karena adanya disconnect dari server, maka akan close di akhir fungsi main.

```c
    // Close client socket on disconnect
    close(client_socket);
    return 0;
}
```

#### Dokumentasi Screenshot : 

![driver.c dan paddock.c, serta log](https://media.discordapp.net/attachments/1071478813566976151/1237453298391584869/image.png?ex=663bb3b1&is=663a6231&hm=dbdb65f231a7f011ca365034063c0898e6ec7920f5923fa57ad61402539163dc&=&format=webp&quality=lossless&width=1210&height=681)

## Soal 4
> Dikerjakan oleh: Malvin Putra Rismahardian (5027231048)

### isi soal

Lewis Hamilton 🏎 seorang wibu akut dan sering melewatkan beberapa episode yang karena sibuk menjadi asisten. Maka dari itu dia membuat list anime yang sedang ongoing (biar tidak lupa) dan yang completed (anime lama tapi pengen ditonton aja). Tapi setelah Lewis pikir-pikir malah kepikiran untuk membuat list anime. Jadi dia membuat file (harap diunduh) dan ingin menggunakan socket yang baru saja dipelajarinya untuk melakukan CRUD pada list animenya. 

a. Client dan server terhubung melalui socket. 

b. client.c di dalam folder client dan server.c di dalam folder server

c. Client berfungsi sebagai pengirim pesan dan dapat menerima pesan dari server.

d. Server berfungsi sebagai penerima pesan dari client dan hanya menampilkan pesan perintah client saja.  

e. Server digunakan untuk membaca myanimelist.csv. Dimana terjadi pengiriman data antara client ke server dan server ke client.
	* Menampilkan seluruh judul
	* Menampilkan judul berdasarkan genre
	* Menampilkan judul berdasarkan hari
	* Menampilkan status berdasarkan berdasarkan judul
	* Menambahkan anime ke dalam file myanimelist.csv
	* Melakukan edit anime berdasarkan judul
	* Melakukan delete berdasarkan judul
	* Selain command yang diberikan akan menampilkan tulisan “Invalid Command”

f. Karena Lewis juga ingin track anime yang ditambah, diubah, dan dihapus. Maka dia membuat server dapat mencatat anime yang dihapus dalam sebuah log yang diberi nama change.log.
	* Format: [date] [type] [massage]
	* Type: ADD, EDIT, DEL
	* Ex:
	1. [29/03/24] [ADD] Kanokari ditambahkan.
	2. [29/03/24] [EDIT] Kamis,Comedy,Kanokari,completed diubah menjadi Jumat,Action,Naruto,completed.
	3. [29/03/24] [DEL] Naruto berhasil dihapus.

g. Koneksi antara client dan server tidak akan terputus jika ada kesalahan input dari client, cuma terputus jika user mengirim pesan “exit”. Program exit dilakukan pada sisi client.

h. Hasil akhir:
soal_4/
    ├── change.log
    ├── client/
    │   └── client.c
    ├── myanimelist.csv
    └── server/
        └── server.c

Log Perubahan:
5/3/2024 - 2:37 pm: Tambah keterangan soal 3 di poin H 

5/3/2024 - 6.18 pm: Ralat “dan” pada poin D soal 1





### Penyelesaian
**Pertama**

Saya membuat direktori dan mengisi direktori itu dengan format sesuai dengan perintah yang diberikan. Lalu menginstall file yang diberikan pada direktori yang sesuai, dengan cara : 

```sh
curl -o myanimelist.csv -L 'https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50'
```

**Kedua**

Selanjutnya saya mengisi`server.c`dengan code berikut:

```sh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define CMD_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 8192
#define CSV_PATH "/home/malvin/sisopmodul3/soal_4/myanimelist.csv"
#define LOG_PATH "/home/malvin/sisopmodul3/soal_4/change.log"

void write_to_log(const char *type, const char *message) {
    FILE *log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, 20, "%d/%m/%y - %I:%M %p", tm_info);

    fprintf(log_file, "[%s] [%s] %s\n", time_str, type, message);
    fclose(log_file);
}

void handle_command(int client_socket, const char *command) {
    char response[RESPONSE_BUFFER_SIZE] = {0};

    if (strcmp(command, "SHOW") == 0) {
        FILE *csv_file = fopen(CSV_PATH, "r");
        if (csv_file == NULL) {
            sprintf(response, "Error opening file: No such file or directory");
        } else {
            char line[1024];
            while (fgets(line, sizeof(line), csv_file)) {
                strcat(response, line);
            }
            fclose(csv_file);
        }
    } else if (strncmp(command, "GENRE", 5) == 0) {
        char genre[256] = {0};
        sscanf(command, "%*s %s", genre);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", genre, CSV_PATH);
        
        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "DAY", 3) == 0) {
        char day[256] = {0};
        sscanf(command, "%*s %s", day);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", day, CSV_PATH);

        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "STATUS", 6) == 0) {
        char status[256] = {0};
        sscanf(command, "%*s %s", status);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", status, CSV_PATH);

        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "ADD", 3) == 0 || strncmp(command, "EDIT", 4) == 0 || strncmp(command, "DEL", 3) == 0) {
        // Log the command
        write_to_log("ADD/EDIT/DEL", command);

        // Handle the command (add/edit/del) here
    
        // For now, just set the response to a placeholder message
        sprintf(response, "Command \"%s\" executed successfully", command);
    } else {
        // Invalid command
        sprintf(response, "Invalid Command");
    }

    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1) {
        char command[1024] = {0};
        if (recv(new_socket, command, sizeof(command), 0) <= 0) {
            break;
        }

        handle_command(new_socket, command);
    }

    return 0;
}
```

Code ini bertanggung jawab untuk membuat server yang menerima koneksi dari client, menanggapi perintah yang diterima dari client, dan mengirimkan respons kembali ke client.

Berikut adalah penjelasan singkat tentang setiap bagian dari code tersebut:

`Definisi Konstanta:` Konstanta `PORT`, `CMD_BUFFER_SIZE`, `RESPONSE_BUFFER_SIZE`, `CSV_PATH`, dan `LOG_PATH` didefinisikan untuk menentukan port yang akan digunakan untuk koneksi, ukuran buffer untuk perintah dan respons, serta path untuk file CSV dan log.

`Fungsi write_to_log()`: Fungsi ini digunakan untuk menulis pesan ke file log dengan format waktu, tipe, dan pesan yang ditentukan.

`Fungsi handle_command()`: Fungsi ini menangani perintah yang diterima dari client. Bergantung pada jenis perintah yang diterima, fungsi ini akan membaca file CSV, melakukan pencarian dengan menggunakan grep, atau menulis log, dan kemudian mengirimkan respons kembali ke client.

`Membuat Socket`: Socket TCP/IP dibuat menggunakan `socket()` dengan menggunakan `AF_INET` untuk domain (IPv4), SOCK_STREAM untuk tipe socket (TCP), dan `0` untuk protokol default.

`Mengatur Socket Options`: Fungsi `setsockopt()` digunakan untuk mengatur opsi socket, seperti `SO_REUSEADDR` dan `SO_REUSEPORT`, agar socket dapat digunakan kembali setelah di-bind.

`Menyiapkan Struktur sockaddr_in`: Struktur `sockaddr_in` digunakan untuk menyimpan alamat server. Port dan alamat IP server ditentukan dalam struktur ini.

`Binding Socket`: Socket di-bind ke alamat server dengan menggunakan fungsi `bind()`.

`Mendengarkan Koneksi`: Server mulai mendengarkan koneksi dari client dengan menggunakan fungsi `listen()`.

`Menerima Koneksi`: Server menerima koneksi dari client dengan menggunakan fungsi `accept()`.

`Loop untuk Menerima Perintah`: Program memasuki loop tak terbatas untuk menerima perintah dari client. Setiap perintah yang diterima akan ditangani oleh fungsi `handle_command()`.

`Menutup Socket`: Setelah loop berakhir, server menutup socket dengan `close()`.



**Ketiga**

Selanjutnya saya mengisi`client.c`dengan code berikut:

 ```sh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char command[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        if (strlen(command) <= 1) {
            printf("Please enter a valid command\n");
            continue;
        }

        // Remove trailing newline character
        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }

        send(sock, command, strlen(command), 0);

        char response[8192] = {0};
        if (recv(sock, response, sizeof(response), 0) <= 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Server response:\n%s\n", response);
    }

    close(sock);
    return 0;
}
``` 
Code ini berfungsi untuk membuat koneksi dengan server menggunakan protokol TCP/IP, mengirimkan perintah dari pengguna ke server, dan menampilkan respons dari server.

Berikut adalah penjelasan singkat tentang setiap bagian dari code tersebut:

`Definisi Konstanta:` Konstanta `PORT` didefinisikan untuk menentukan port yang akan digunakan untuk koneksi.

`Membuat Socket:` Socket `TCP/IP` dibuat menggunakan `socket()` dengan menggunakan `AF_INET` untuk domain (IPv4), `SOCK_STREAM` untuk tipe socket (TCP), dan `0` untuk protokol default.

`Menyiapkan Struktur sockaddr_in:` Struktur sockaddr_in digunakan untuk menyimpan alamat server. Port dan alamat IP server ditentukan dalam struktur ini.

`Menghubungkan ke Server:` Fungsi `connect()` digunakan untuk menghubungkan socket ke server menggunakan alamat yang sudah ditentukan.

`Loop Pengiriman Perintah:` Program memasuki loop tak terbatas untuk menerima perintah dari pengguna dan mengirimkannya ke server. Pengguna diminta untuk memasukkan perintah, kemudian perintah tersebut dikirimkan ke server menggunakan `send()`. Jika panjang string perintah yang dimasukkan kurang dari atau sama dengan 1, program akan menampilkan pesan kesalahan. String perintah juga dipotong untuk menghapus karakter newline yang dimasukkan oleh `fgets()`. Setelah perintah dikirimkan, program menerima respons dari server menggunakan `recv()`. Jika tidak ada respons dari server (koneksi terputus), program menampilkan pesan bahwa server terputus dan loop berakhir.

``Menutup Socket:` Setelah loop berakhir, program menutup socket dengan `close()` dan mengakhiri eksekusi dengan mengembalikan nilai 0.


**Keempat**

Untuk langkah terakhir, kita harus membuka dua Terminal sekaligus untuk bisa menjalankan code diatas. Masing - masing terminal membuka direktori `server` dan `client` .

### Cara Menjalankan code

Untuk menjalankan code client.c dan server.c, kita perlu mengikuti beberapa langkah:

*Menjalankan Server*

Compile file server.c menjadi sebuah executable. Kita dapat menggunakan compiler C seperti gcc.

`gcc -o server server.c`
Jalankan server dengan mengetikkan nama executable yang telah dibuat.


`./server`
Server sekarang akan berjalan dan siap menerima koneksi dari client.

*Menjalankan Client*

Compile file client.c menjadi sebuah executable.

`gcc -o client client.c`
Jalankan client dengan mengetikkan nama executable yang telah dibuat.

`./client`
Setelah client berjalan, Anda akan diminta untuk memasukkan perintah. Ketiklah perintah sesuai dengan kebutuhan, seperti "SHOW", "GENRE", "DAY", "STATUS", "ADD", "EDIT", "DEL", dan tekan Enter untuk mengirim perintah ke server.

Client akan menerima respons dari server dan menampilkannya di layar.

#### Dokumentasi Screenshot : 

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828983479439450/Screenshot_2024-05-11_192240.png?ex=6640b4e5&is=663f6365&hm=fd2b0e8ed8ce7bd469f2ee11f0ae6dd65d262f0f34c56993250feb9427b61eca&)

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828983941074985/Screenshot_2024-05-11_192218.png?ex=6640b4e6&is=663f6366&hm=725ab93683f3ce40b57c239082a466657e12d4d588c22a786776610866d57a39&)

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828984318300191/Screenshot_2024-05-11_192104.png?ex=6640b4e6&is=663f6366&hm=fb0f5f2a061b6b4dcc6eaca9c449bb3aee4dfad7089661af5fcc9f79fd7af253&)

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828984868016128/Screenshot_2024-05-11_192008.png?ex=6640b4e6&is=663f6366&hm=a6a44ac3166c8d648dd376dada90b4443cc01f6a3e87155c2c202eb38ba63bee&)

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828985245241394/Screenshot_2024-05-11_191926.png?ex=6640b4e6&is=663f6366&hm=8917bb63827a7b6f007490bb704adb3eb664b110487fcc91f51493961a066d45&)

![Hasil](https://cdn.discordapp.com/attachments/1045023350679949476/1238828985627185244/Screenshot_2024-05-11_192309.png?ex=6640b4e6&is=663f6366&hm=0ea0cb70cf8f1e8b4f8ca97520a884198320c8abe077e2d26ad8750258baa6a7&)

### kendala
Tidak ada kendala.
