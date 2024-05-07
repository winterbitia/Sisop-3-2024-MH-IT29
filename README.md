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
