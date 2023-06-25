#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

bool daemonRunning = false;
pthread_t daemonThread;

void ProzessStatusF() {
char statusDateiAbruf[256];
sprintf(statusDateiAbruf, "/proc/%d/status", getpid());
// Datei für Status wird geöffnet:
FILE *statusDatei = fopen(statusDateiAbruf, "r");
if (statusDatei == NULL) {
    perror("Fehler: Status-Datei kann nicht geöffnet werden");
    return;
}

// Extrahieren des Prozessstatuses:
char line[256];
char statusText[256];  // Variable für den Status-Text
while (fgets(line, sizeof(line), statusDatei) != NULL) {
    if (strncmp(line, "State:", 6) == 0) {
        char state[16];
        sscanf(line + 7, "%s", state);

        // Status-Text festlegen:
        if (strcmp(state, "R") == 0) {
            strcpy(statusText, "Status: running");
        } else if (strcmp(state, "S") == 0) {
            strcpy(statusText, "Status: sleeping");
        } else if (strcmp(state, "D") == 0) {
            strcpy(statusText, "Status: blocked");
        } else {
            strcpy(statusText, "Status: unknown");
        }

        break;
    }
}

// Schließen der Status-Datei:
fclose(statusDatei);

// Ergebnis in die Datei "daemon.info" schreiben:
FILE *logfile = fopen("/home/schwobbelheini/Schreibtisch/Bitte_Funktioniere/daemon.info", "a");
if (logfile == NULL) {
    perror("Fehler: Log-Datei kann nicht geöffnet werden");
    return;
}
fprintf(logfile, "%s\n", statusText);
fclose(logfile);
}

void LoescheDateiInhalt() {
    FILE *logfile = fopen("/home/schwobbelheini/Schreibtisch/Bitte_Funktioniere/daemon.info", "w");
    if (logfile == NULL) {
        perror("Fehler: Log-Datei kann nicht geöffnet werden");
        return;
    }

    // Dateiinhalt löschen
    fclose(logfile);
}

void *daemonTask(void *arg) {
    while (daemonRunning) {

     for (int i = 1; i <= 3; i++) {
            ("%d\n", i);
            sleep(1); // Eine Sekunde warten
            if (i == 3) {
        syslog(LOG_INFO, "Der Zaehler hat 3 erreicht");
            }
        }

    }
        return NULL;
}

//Funktion zum Beenden des Programmes
void beendeProgramm() {
    exit(EXIT_SUCCESS);
}


// Funktion zum Erstellen des Daemon-Prozesses
void createDaemon() {
    pid_t pid = fork();

    // Fehler beim Forken
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    // Elternprozess beenden
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Neue Sitzung und Gruppe erstellen
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // Syslog öffnen
    openlog("DaemonExample", LOG_PID, LOG_DAEMON);

// Protokollierung der PID in syslog
    syslog(LOG_INFO, "Daemon started.");
    daemonRunning = true;

   if (pthread_create(&daemonThread, NULL, daemonTask, NULL) != 0) {
        syslog(LOG_ERR, "Failed to create daemon thread");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Daemon thread created.");

    closelog();
}

// Funktion zum Beenden des Daemons
void stopDaemon(pthread_t thread) {
    daemonRunning = false;
    pthread_join(thread, NULL);
}

// Funktion zum Erstellen der Datei mit der PID des Daemons
void createPidFile() {
    FILE *file = fopen("daemon.info", "a");
    if (file == NULL) {
        perror("Failed to create information file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "PID: %d\nParent PID: %d\nReale User ID: %d\nEffektive User ID: %d\nReale Gruppen ID: %d\nEffektive Gruppen ID: %d", getpid(), getppid(), getuid(),geteuid(), getgid(), getegid());

    ProzessStatusF();

    fclose(file);
}

// Funktion zum Lesen der PID-Datei
void readPidFile() {
    FILE *file = fopen("/home/schwobbelheini/Schreibtisch/Bitte_Funktioniere/daemon.info", "r");
    if (file == NULL) {
        perror("Failed to open information file");
        exit(EXIT_FAILURE);
    }
    int pid, ppid, uid, euid, gid, egid;
    char statusText[256];
    fscanf(file, "Status: %[^\n\r]\nPID: %d\nParent PID: %d\nReale User ID: %d\nEffektive User ID: %d\nReale Gruppen ID: %d\nEffektive Gruppen ID: %d",statusText, &pid, &ppid, &uid, &euid, &gid, &egid);
    fclose(file);
    printf("Status: %s\nPID: %d\nParent PID: %d\nReale User ID: %d\nEffektive User ID: %d\nReale Gruppen ID: %d\nEffektive Gruppen ID: %d\n",statusText, pid, ppid, uid, euid, gid, egid);
}

int main() {

     //Ändern des Puffermodus der Standardeingabe
    setvbuf(stdin, NULL, _IONBF, 0);

    while(1){

    int choice;

    printf("1. Create Daemon\n");
    printf("2. Stop Daemon\n");
    printf("3. Read Daemon Info\n");
    printf("4. Terminate\n");
    printf("Enter your choice: ");

     scanf("%d", &choice);

    switch (choice) {
        case 1:
            if (!daemonRunning) {
                    createDaemon();
                    createPidFile();
                    daemonRunning = true;
                } else {
                    printf("Daemon is already running.\n");
                }
            break;
        case 2:
           if (daemonRunning) {
                    LoescheDateiInhalt();
                    stopDaemon(daemonThread);
                }
                  else {
                    printf("Daemon is not running.\n");
                }
            break;
        case 3:
            readPidFile();
            break;
        case 4:
            beendeProgramm();
            break;
        default:
            printf("Invalid choice. Please choose between 1, 2, 3 & 4\n");
            break;
    }
    }

    return 0;
}

