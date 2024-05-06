#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

// Mengimpor pustaka-pustaka yang diperlukan untuk operasi input/output,
// alokasi memori, manipulasi string, proses fork, menunggu proses, dan waktu.

#define PIPE_READ 0
#define PIPE_WRITE 1

// Mendefinisikan konstanta untuk sisi baca dan tulis dari sebuah pipe.

// Fungsi untuk mengubah kata-kata angka ke angka
int wordsToNumber(const char *word) {
    // Mengonversi kata-kata angka dalam bahasa Indonesia ke bilangan bulat.
    if (strcmp(word, "satu") == 0) return 1;
    if (strcmp(word, "dua") == 0) return 2;
    if (strcmp(word, "tiga") == 0) return 3;
    if (strcmp(word, "empat") == 0) return 4;
    if (strcmp(word, "lima") == 0) return 5;
    if (strcmp(word, "enam") == 0) return 6;
    if (strcmp(word, "tujuh") == 0) return 7;
    if (strcmp(word, "delapan") == 0) return 8;
    if (strcmp(word, "sembilan") == 0) return 9;
    return -1; 
    // Kembalikan -1 jika input tidak valid.
}

// Fungsi untuk mengubah angka menjadi kata-kata dalam bahasa Indonesia.
void numberToWords(int num, char *words) {
    // Mendefinisikan array konstanta untuk angka satuan, puluhan, dan belasan.
    const char *units[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    const char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    const char *tens[] = {"", "", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    // Mengonversi angka menjadi kata-kata berdasarkan rentang nilainya.
    if (num < 10) {
        strcpy(words, units[num]);
    } else if (num < 20) {
        strcpy(words, teens[num - 10]);
    } else {
        int tenPart = num / 10; 
        // Mendapatkan nilai puluhan.
        int unitPart = num % 10; 
        // Mendapatkan nilai satuan.
        strcpy(words, tens[tenPart]);
        if (unitPart > 0) {
            strcat(words, " ");
            strcat(words, units[unitPart]);
        }
    }
}

// Fungsi untuk menulis log ke file histori.log.
void writeLog(const char *type, const char *message) {
    // Membuka file histori.log dalam mode append.
    FILE *fp = fopen("histori.log", "a");
    if (fp == NULL) {
        perror("Error opening file histori.log");
        // Menangani eror saat membuka file log.
        exit(EXIT_FAILURE);
    }

    // Mendapatkan waktu saat ini.
    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[30];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, sizeof(timeStr), "%d/%m/%y %H:%M:%S", timeinfo);

    // Menulis log dalam format [tanggal] [jenis] [pesan].
    fprintf(fp, "[%s] [%s] %s\n", timeStr, type, message);

    // Menutup file setelah menulis log.
    fclose(fp);
}

// Fungsi untuk membuat kalimat hasil operasi.
void formatMessage(char *message, const char *operationWord, const char *word1, const char *word2, const char *resultWords) {
    // Bentuk pesan hasil operasi sesuai format yang diinginkan.
    snprintf(message, 100, "hasil %s %s dan %s adalah %s.", operationWord, word1, word2, resultWords);
}

int main(int argc, char *argv[]) {
    // Memeriksa jumlah argumen yang diberikan.
    if (argc != 4) {
        // Memberikan instruksi penggunaan jika jumlah argumen tidak sesuai.
        printf("Usage: %s [operator] [number1] [number2]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Mengambil operator dan angka dari input argumen.
    const char *operator = argv[1];
    const char *word1 = argv[2];
    const char *word2 = argv[3];
    int num1 = wordsToNumber(word1);
    int num2 = wordsToNumber(word2);
    int result;

    // Memastikan bahwa input valid (angka dapat dikonversi).
    if (num1 == -1 || num2 == -1) {
        printf("ERROR: Invalid number input.\n");
        return EXIT_FAILURE;
    }

    // Membuat dua pipes untuk komunikasi antara proses parent dan child.
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);
    
    // Menggunakan fork() untuk membuat proses child.
    pid_t pid = fork();

    // Menangani kesalahan saat melakukan fork().
    if (pid < 0) {
        perror("Fork failed");
        return EXIT_FAILURE;
    }

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
        // Child process
        // Tutup sisi tulis dari pipe pertama dan sisi baca dari pipe kedua.
        close(fd1[PIPE_WRITE]);
        close(fd2[PIPE_READ]);

        // Membaca hasil dari parent process melalui pipe.
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

        // Mengirim hasil operasi ke parent process melalui pipe.
        write(fd2[PIPE_WRITE], message, strlen(message) + 1);
        close(fd2[PIPE_WRITE]);

        // Mengakhiri proses child dengan sukses.
        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS; 
}
