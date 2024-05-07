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

## Soal 4
> Dikerjakan oleh: Malvin Putra Rismahardian (5027231048)
