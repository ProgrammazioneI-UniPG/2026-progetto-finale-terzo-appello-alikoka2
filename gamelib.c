#include "gamelib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

int c; // Variabile di controllo input
void svuotaBuffer();
void color(char color);

bool impostato = false; 
Giocatore* giocatori[4];
unsigned short num_giocatori = 0;

// Puntatori per le due liste speculari
Zona_mondoreale* prima_zona_mondoreale = NULL;
Zona_soprasotto* prima_zona_soprasotto = NULL;
unsigned short numero_zone_create = 0;

// Variabili di stato specifiche per il progetto Cosestrane
static int mappa_chiusa = 0; 
static int undici_presente = 0;
static int demotorzone_sconfitto = 0;
static int partite_effettuate = 0;
static bool vincitore_comparso = false;
unsigned short turno = 1;

// Funzioni per imposta_gioco()
static void menu_impostazione_giocatori();
static void creazione_giocatori();
static void stampa_giocatori();
static void deallocazione_giocatori();
static void menu_impostazione_mappa();
static void genera_mappa(); 
static void cancella_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void chiudi_mappa();
static void menu_impostazione_tempo_pausa();
static void investiga_zona(Giocatore* giocatore, unsigned short* azione);

// Funzioni verifica (Enum -> Stringa)
static char* verifica_tipo_zona_reale(Zona_mondoreale* pZona);
static char* verifica_tipo_zona_sotto(Zona_soprasotto* pZona);
static char* verifica_tipo_nemico_reale(Zona_mondoreale* pZona);
static char* verifica_tipo_nemico_sotto(Zona_soprasotto* pZona);
static char* verifica_tipo_oggetto(Zona_mondoreale* pZona);
static char* nome_oggetto(Tipo_oggetto oggetto);

// Funzioni per gioca()
static void genera_ordine_casuale(unsigned short ordine_giocatori[]);
static void avanza(Giocatore* giocatore, unsigned short* movimento);
static void indietreggia(Giocatore* giocatore, unsigned short* movimento);
static void cambia_mondo(Giocatore* giocatore, unsigned short* movimento);
static void combatti(Giocatore* giocatore, unsigned short* azione);
static unsigned short lancia_dado();
static void raccogli_oggetto(Giocatore* giocatore, unsigned short* azione);
static void utilizza_oggetto(Giocatore* giocatore, unsigned short* azione);
static bool presenza_nemico_nella_zona(Giocatore* giocatore);
static void passa(int prossimo_indice, unsigned short ordine[]);
static void stampa_giocatore(int i);
static void stampa_zaino(Giocatore* giocatore);
static void stampa_zona_gioco(Giocatore* giocatore);
static int nemici_sconfitti_totali = 0; 
static int oggetti_raccolti_totali = 0;
static char ultimi_vincitori[3][100] = {"Nessuno", "Nessuno", "Nessuno"};
static void registra_vincitore(char* nome);

static Tipo_nemico genera_nemico_reale(int probabilita);
static Tipo_nemico genera_nemico_soprasotto(int probabilita, int indice_zona);

unsigned short durata_intervallo = 2;
unsigned short num_giocatori_morti = 0;

void imposta_gioco() {
    if (impostato == false) {
        creazione_giocatori();
        genera_mappa();
        printf("\033[1;32mMappa generata con successo.\033[0m\n");
        menu_impostazione_mappa();
        printf("\033[1;32mConfigurazione iniziale terminata!\033[0m\n");
    }

    int scelta = 0;
    do {
        printf("\033[1;36m\n+----------------------------------------------------------+\n");
        printf("|                    MENU IMPOSTAZIONE                     |\n");
        printf("+----------------------------------------------------------+\033[0m\n");
        printf("0. Torna al menu principale\n");
        printf("1. Impostazione Giocatori (Modifica/Visualizza)\n");
        printf("2. Impostazione Mappa (Inserisci/Cancella/Chiudi)\n");
        printf("3. Impostazione Velocità Testi (Pausa)\n");
        printf("\n>> Inserire uno dei comandi: ");
        
        // Gestione errore: l'utente inserisce una lettera
        if (scanf("%d", &scelta) != 1) {
            color('r');
            printf("\n--- ERRORE: Devi inserire un NUMERO! ---\n");
            color('0');
            svuotaBuffer(); 
            printf("Premi INVIO per continuare...");
            getchar(); // Pausa per far leggere l'errore
            continue; 
        }
        svuotaBuffer();

        // Gestione scelta numerica
        switch (scelta) {
            case 0:
                printf("Torno al menu principale...\n");
                break;
            case 1:
                menu_impostazione_giocatori();
                break;
            case 2:
                menu_impostazione_mappa();
                break;
            case 3:
                menu_impostazione_tempo_pausa();
                break;
            default:
                // Numero valido ma non presente nel menu
                color('r');
                printf("\n--- COMANDO [%d] NON VALIDO! Scegli tra 0 e 3. ---\n", scelta);
                color('0');
                printf("Premi INVIO per continuare...");
                getchar();
                break;
        }
    } while (scelta != 0);
}

static void stampa_zaino(Giocatore* giocatore) {
  printf("\nZaino di %s:\n", giocatore->nome);
  bool vuoto = true;
  for (int i = 0; i < 3; i++) {
    if (giocatore->zaino[i] != nessun_oggetto) {
      printf("\t[%d] %s\n", i + 1, nome_oggetto(giocatore->zaino[i]));
      vuoto = false;
    } else {
      printf("\t[%d] Vuoto \n", i + 1);
    }
  }
  if (vuoto) printf("\t(Lo zaino non contiene oggetti)\n");
}

void gioca() {
    srand((unsigned)time(NULL));
    if (!impostato || mappa_chiusa == 0) {
        color('r');
        printf("Errore: Imposta il gioco e chiudi la mappa prima di giocare!\n");
        color('0');
        return;
    }

    printf("\033[1;36m\n+---------------------------------------------------------------------------------------+\n");
    printf("|                          GIOCO INIZIATO! BENVENUTI A OCCHINZ!                         |\n");
    printf("+---------------------------------------------------------------------------------------+\033[0m\n");

    // Backup e posizionamento iniziale
    Giocatore* backup_giocatori[num_giocatori];
    for(int i = 0; i < num_giocatori; i++) {
        backup_giocatori[i] = (Giocatore*) malloc(sizeof(Giocatore));
        backup_giocatori[i]->attacco_psichico = giocatori[i]->attacco_psichico;
        backup_giocatori[i]->difesa_psichica = giocatori[i]->difesa_psichica;
        backup_giocatori[i]->fortuna = giocatori[i]->fortuna;
        
        giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
        giocatori[i]->pos_soprasotto = prima_zona_soprasotto;
        giocatori[i]->mondo = 0; 
    }
    unsigned short ordine_giocatori[num_giocatori];
    demotorzone_sconfitto = 0;
    vincitore_comparso = false;
    num_giocatori_morti = 0;
    turno = 1;

    do {
        genera_ordine_casuale(ordine_giocatori);
        for (int i = 0; i < num_giocatori; i++) {
            int corrente = ordine_giocatori[i];
            if (giocatori[corrente]->fortuna <= 0) continue;

            color('c');
            printf("\n--- ROUND %hu | Turno di: %s ---\n", turno, giocatori[corrente]->nome);
            color('0');

            unsigned short scelta;
            unsigned short movimento = 0; 
            unsigned short azione = 0; 

            do {
                if (vincitore_comparso) {
                    scelta = 0; 
                    break; 
                }
                color('p');
                printf("\n[%s] Mondo: %s | Fortuna: %d | Movimento: %hu/1\n", 
                       giocatori[corrente]->nome, 
                       (giocatori[corrente]->mondo == 0) ? "REALE" : "SOPRASOTTO",
                       giocatori[corrente]->fortuna, movimento);
                color('0');
                
                printf("0. Passa il turno (Termina)\n");
                printf("1. Avanzare\n");
                printf("2. Indietreggiare\n");
                printf("3. Cambiare mondo\n");
                printf("4. Stato Giocatore & Zaino\n");
                printf("5. Info zona attuale\n");
                printf("6. Raccogli oggetto\n");
                printf("7. Combatti\n");
                printf("8. Usa oggetto dello zaino\n");
                printf("9. Investiga zona\n");
                printf("666. Arrendersi (Elimina giocatore)\n");
                printf("\nInserire una delle opzioni sopra indicate: ");

                if (scanf("%hu", &scelta) != 1) {
                    svuotaBuffer();
                    scelta = 99;
                } else {
                    svuotaBuffer();
                }
                switch (scelta) {
                    case 0: passa(i, ordine_giocatori); break;
                    case 1: avanza(giocatori[corrente], &movimento); break;
                    case 2: indietreggia(giocatori[corrente], &movimento); break;
                    case 3: cambia_mondo(giocatori[corrente], &movimento); break;
                    case 4: stampa_giocatore(corrente); stampa_zaino(giocatori[corrente]); break;
                    case 5: stampa_zona_gioco(giocatori[corrente]); break;
                    case 6: raccogli_oggetto(giocatori[corrente], &azione); break;
                    case 7: 
                        combatti(giocatori[corrente], &azione); 
                        break;
                    case 8: utilizza_oggetto(giocatori[corrente], &azione); break;
                    case 9: investiga_zona(giocatori[corrente], &azione); break;
                    case 666: 
                        giocatori[corrente]->fortuna = 0; 
                        num_giocatori_morti++;
                        scelta = 0;
                        break;
                    default: 
                    color('r');
                    printf("Scelta non valida!\nInserire di nuovo un numero presente nelle opzioni sopra indicate.\n"); 
                    color('0'); 
                    break;
                    
                }
                
                if (!vincitore_comparso && demotorzone_sconfitto == 1) {
                    system("clear");
                    color('g');
                    printf("+----------------------------------------------------------+\n");
                    printf("|  __     ______  _    _  __          ______  _   _  _     |\n");
                    printf("|  \\ \\   / / __ \\| |  | | \\ \\        / / __ \\| \\ | || |    |\n");
                    printf("|   \\ \\_/ / |  | | |  | |  \\ \\  /\\  / / |  | |  \\| || |    |\n");
                    printf("|    \\   /| |  | | |  | |   \\ \\/  \\/ /| |  | | . ` || |    |\n");
                    printf("|     | | | |__| | |__| |    \\  /\\  / | |__| | |\\  ||_|    |\n");
                    printf("|     |_|  \\____/ \\____/      \\/  \\/   \\____/|_| \\_|(_)    |\n");
                    printf("|                                                          |\n");
                    printf("+----------------------------------------------------------+\n");
                    color('0');
                    registra_vincitore(giocatori[corrente]->nome);
                    vincitore_comparso = true;
                    scelta = 0; 
                    
                    printf("\nPremi INVIO per visualizzare i crediti e terminare la partita");
                    svuotaBuffer();
                }
                if (num_giocatori_morti == num_giocatori) scelta = 0;
            } while (scelta != 0 && giocatori[corrente]->fortuna > 0 && !vincitore_comparso);
            if (vincitore_comparso || num_giocatori_morti == num_giocatori) break;
        }
        if (vincitore_comparso) break;
        turno++;
    } while (!vincitore_comparso && num_giocatori_morti < num_giocatori);

    if (num_giocatori_morti == num_giocatori && !vincitore_comparso) {
        printf("\033[1;31m\n");
        printf("+----------------------------------------------------------------------------+\n");    
        printf("|            _____                         ____                   _          |\n");
        printf("|           / ____|                       / __ \\                 | |         |\n");
        printf("|          | |  __  __ _ _ __ ___   ___  | |  | |_   _____ _ __  | |         |\n");
        printf("|          | | |_ |/ _` | '_ ` _ \\ / _ \\ | |  | \\ \\ / / _ \\ '__| | |         |\n");
        printf("|          | |__| | (_| | | | | | |  __/ | |__| |\\ V /  __/ |    |_|         |\n");
        printf("|           \\_____|\\__,_|_| |_| |_|\\___|  \\____/  \\_/ \\___|_|    (_)         |\n");
        printf("+----------------------------------------------------------------------------+");
        printf("\033[0m\n");
        registra_vincitore("Nessun Vincitore (Tutti Morti)");
        printf("Premi INVIO per continuare...");
        svuotaBuffer();
    }
    // Ripristino e pulizia
    for(int i = 0; i < num_giocatori; i++) {
        giocatori[i]->attacco_psichico = backup_giocatori[i]->attacco_psichico;
        giocatori[i]->difesa_psichica = backup_giocatori[i]->difesa_psichica;
        giocatori[i]->fortuna = backup_giocatori[i]->fortuna;
        free(backup_giocatori[i]);
    }
    partite_effettuate++;
    crediti();
}

static void dealloca_mappa() {
    struct Zona_mondoreale* temp_reale;
    struct Zona_soprasotto* temp_sotto;
    // 1. Libera il Mondo Reale
    while (prima_zona_mondoreale != NULL) {
        temp_reale = prima_zona_mondoreale;
        prima_zona_mondoreale = prima_zona_mondoreale->avanti;
        free(temp_reale);
    }
    // 2. Libera il Soprasotto
    while (prima_zona_soprasotto != NULL) {
        temp_sotto = prima_zona_soprasotto;
        prima_zona_soprasotto = prima_zona_soprasotto->avanti;
        free(temp_sotto);
    }
    // 3. Reset variabili di stato
    numero_zone_create = 0;
    impostato = 0;
    mappa_chiusa = 0;
}

void termina_gioco() {
    if (partite_effettuate == 0) {
        printf("\033[1;31m\n+----------------------------------------------------------+\n");
        printf("|              ERRORE: USCITA NON CONSENTITA!              |\n");
        printf("|      Non hai ancora giocato nessuna partita a Occhinz!   |\n");
        printf("|          Per uscire correttamente devi prima giocare     |\n");
        printf("+----------------------------------------------------------+\033[0m\n");
        return; 
    } 
    // Se arriviamo qui, significa che partite_effettuate > 0
    printf("\nGRAZIE PER AVER GIOCATO A COSESTRANE!\n");
    dealloca_mappa(); 
    printf("Chiusura programma in corso. Alla prossima!\n");
    exit(0); // Chiude il programma definitivamente
}

static void registra_vincitore(char* nome) {
    // Spostiamo i nomi esistenti per fare spazio al nuovo (logica LIFO)
    // Il 2° diventa 3°, il 1° diventa 2°
    strcpy(ultimi_vincitori[2], ultimi_vincitori[1]);
    strcpy(ultimi_vincitori[1], ultimi_vincitori[0]);
    
    // Il vincitore attuale diventa il 1° della lista
    strncpy(ultimi_vincitori[0], nome, 99);
}

void crediti() {
    // Puliamo lo schermo appena entrano nei crediti
    system("clear");
    printf("\n\n");

    // Banner Unico Crediti e Info Sviluppatore
    printf("\033[1;36m+----------------------------------------------------------+\n");
    printf("|                                                          |\n");
    
    // Titolo dinamico
    if (partite_effettuate > 0) {
        printf("|          GRAZIE PER AVER GIOCATO A COSESTRANE!           |\n");
    } else {
        printf("|                         CREDITI                          |\n");
    }
    
    printf("|                                                          |\n");
    printf("+----------------------------------------------------------+\n");
    printf("|                                                          |\n");
    printf("|        Sviluppato da: Ali Koka                           |\n");
    printf("|        Matricola: 346288                                 |\n");
    printf("|        Corso: Programmazione Procedurale 2025/2026       |\n");
    printf("|                                                          |\n");
    printf("+----------------------------------------------------------+\033[0m\n");

    color('y');
    printf("\n+----------------------------------------------------------+\n");
    printf("|                STORICO ULTIMI 3 VINCITORI                |\n");
    printf("+----------------------------------------------------------+\n");    for (int i = 0; i < 3; i++) {
        printf("  %d. %s\n", i + 1, ultimi_vincitori[i]);
    }
    color('0');

    // Informazioni aggiuntive dinamiche
    if (partite_effettuate > 0) {
        color('p');
        printf("\n+----------------------------------------------------------+\n");
        printf("|                   STATISTICHE SESSIONE                   |\n");
        printf("+----------------------------------------------------------+\n");
        printf("Partite totali giocate: %d\n", partite_effettuate);
        printf("Totale nemici sconfitti: %d\n", nemici_sconfitti_totali);
        printf("Totale oggetti raccolti: %d\n", oggetti_raccolti_totali);
        color('0');
    } else {
        color('y');
        printf("\nNessuna partita giocata ancora. Torna al menu per iniziare!\n");
        color('0');
    }
    int scelta_fine = 0;
    while (scelta_fine != 1) {
        printf("\nDigitare 1 per tornare al menu principale: ");
        // Se l'utente inserisce una lettera o un numero diverso da 1
        if (scanf("%d", &scelta_fine) != 1) {
            svuotaBuffer();
            color('r');
            printf("Errore: Inserire un numero valido. ");
            color('0');
        } else if (scelta_fine != 1) {
            color('r');
            printf("Comando non riconosciuto. ");
            color('0');
        }
    }
    svuotaBuffer();
    system("clear");
}

void svuotaBuffer() {
  int c; 
  while ((c = getchar()) != '\n' && c != EOF);
}

void color(char color) { 
  switch(color) {
    case 'r': // Rosso
      printf("\033[0;91m");
      break;
    case 'g': // Verde
      printf("\033[0;92m");
      break;
    case 'y': // Giallo
      printf("\033[0;93m");
      break;
    case 'b': // Blu
      printf("\033[0;94m");
      break;
    case 'p': // Viola
      printf("\033[0;95m");
      break;
    case 'c': // Ciano
      printf("\033[0;96m");
      break;
    default: // Reset
      printf("\033[0m");
  }
}

static void menu_impostazione_giocatori() {
    unsigned short scelta; // Non serve nemmeno inizializzarla a 3
    do {
        printf("\033[1;36m\n+----------------------------------------------------------+\n");
        printf("|                MENU IMPOSTAZIONE GIOCATORI               |\n");
        printf("+----------------------------------------------------------+\033[0m\n");
        
        printf("\033[1;35m\t0. Torna al menu precedente\033[0m\n");
        printf("\033[1;32m\t1. Stampa la lista dei giocatori attuali\033[0m\n");
        printf("\033[1;33m\t2. Reset e nuova creazione giocatori\033[0m\n");
        
        printf("\nInserire una delle opzioni sopra indicate: ");

        // Se scanf fallisce (restituisce 0), entriamo qui
        if (scanf("%hu", &scelta) != 1) {
            color('r');
            printf("\nERRORE: Devi inserire un numero intero!\n");
            color('0');
            svuotaBuffer(); // Pulisce le lettere o i simboli errati
            continue;       // SALTA TUTTO E TORNA ALL'INIZIO DEL DO
        }
        
        svuotaBuffer(); // Pulisce l'invio rimasto dopo un numero corretto

        // Ora gestiamo i numeri
        switch (scelta) {
            case 0:
                printf("Torno al menu impostazione...\n");
                break;
            case 1:
                stampa_giocatori();
                break;
            case 2:
                creazione_giocatori();
                break;
            default:
                // Se arriva qui è un numero, ma non è tra 0 e 2
                color('r');
                printf("\n--- COMANDO [%hu] NON VALIDO! Scegli tra 0, 1 o 2. ---\n", scelta);
                color('0');
                break;
        }
    } while (scelta != 0);
}

static void creazione_giocatori() {
    deallocazione_giocatori();
    // BLOCCO 1: NUMERO GIOCATORI
    do {
        printf("Inserisci il numero di giocatori (da 1 a 4): ");
        if (scanf("%hu", &num_giocatori) != 1) {
            color('r');
            printf("Errore: Non hai inserito un numero! Riprova.\n");
            color('0');
            svuotaBuffer(); 
            num_giocatori = 0; 
        } 
        else if (num_giocatori < 1 || num_giocatori > 4) {
            color('r');
            printf("Errore: Il numero deve essere compreso tra 1 e 4\n");
            color('0');
            svuotaBuffer(); 
        } 
        else {
            svuotaBuffer();
            break; 
        }
    } while (1);

    for(int i = 0; i < num_giocatori; i++) {
        giocatori[i] = (Giocatore*) malloc(sizeof(Giocatore));
        
        printf("\nGiocatore %d, inserisci il tuo nome: ", i+1);
        fgets(giocatori[i]->nome, 100, stdin);
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = 0;
        giocatori[i]->attacco_psichico = (rand() % 20) + 1;
        giocatori[i]->difesa_psichica = (rand() % 20) + 1;
        giocatori[i]->fortuna = (rand() % 20) + 1;
        
        for(int j = 0; j < 3; j++) giocatori[i]->zaino[j] = nessun_oggetto;

        // BLOCCO 2: SPECIALIZZAZIONE
        int scelta_stat = 0;
        do {
            printf("\nScegli la tua specializzazione:\n");
            printf("1. Focalizzato sull'Attacco (+3 Att / -3 Dif)\n");
            printf("2. Focalizzato sulla Difesa (-3 Att / +3 Dif)\n>> ");
            if (scanf("%d", &scelta_stat) != 1) {
                color('r');
                printf("Errore: inserimento non valido. Inserire il numero 1 o 2.\n");
                color('0');
                svuotaBuffer();
                scelta_stat = 0; 
            } 
            else if (scelta_stat == 1) {
                svuotaBuffer();
                giocatori[i]->attacco_psichico += 3;
                giocatori[i]->difesa_psichica = (giocatori[i]->difesa_psichica > 3) ? giocatori[i]->difesa_psichica - 3 : 1;
                printf("Specializzazione scelta: Attacco!\n");
            } 
            else if (scelta_stat == 2) {
                svuotaBuffer();
                giocatori[i]->difesa_psichica += 3;
                giocatori[i]->attacco_psichico = (giocatori[i]->attacco_psichico > 3) ? giocatori[i]->attacco_psichico - 3 : 1;
                printf("Specializzazione scelta: Difesa!\n");
            } 
            else {
                color('r');
                printf("Comando non riconosciuto! Scegli tra 1 e 2\n");
                color('0');
                svuotaBuffer();
            }
        } while (scelta_stat != 1 && scelta_stat != 2);

        // BLOCCO 3: PERSONAGGIO SPECIALE
        if (undici_presente == 0) {
            int scelta_undici = 0;
            do {
                printf("\nVuoi diventare 'Undici VirgolaCinque'? (+4 Att/Dif, -7 Fortuna)\n");
                printf("1. Sì\n2. No\n>> Inserire una delle opzioni: ");
                if (scanf("%d", &scelta_undici) != 1) {
                    color('r');
                    printf("Errore: inserimento non valido. Digita 1 per Sì o 2 per No\n");
                    color('0');
                    svuotaBuffer();
                    scelta_undici = 0;
                } 
                else if (scelta_undici == 1) {
                    svuotaBuffer();
                    giocatori[i]->attacco_psichico += 4;
                    giocatori[i]->difesa_psichica += 4;
                    giocatori[i]->fortuna = (giocatori[i]->fortuna > 7) ? giocatori[i]->fortuna - 7 : 1;
                    strcat(giocatori[i]->nome, " (Undici VirgolaCinque)");
                    undici_presente = 1;
                    printf("Trasformazione completata!\n");
                } 
                else if (scelta_undici == 2) {
                    svuotaBuffer();
                    printf("Hai scelto di restare un cittadino comune.\n");
                } 
                else {
                    color('r');
                    printf("Scelta non valida! Inserire 1 o 2\n");
                    color('0');
                    svuotaBuffer();
                }
            } while (scelta_undici != 1 && scelta_undici != 2);
        }
        printf("Giocatore %s pronto: Att %d, Dif %d, Fort %d\n", 
                giocatori[i]->nome, giocatori[i]->attacco_psichico, 
                giocatori[i]->difesa_psichica, giocatori[i]->fortuna);
    }
}

static void stampa_giocatori() {
    if (num_giocatori == 0) {
        printf("\nNessun giocatore registrato.\n");
        return;
    }
    printf("\033[1;36m\n+----------------------------------------------------------+\n");
    printf("|                    LISTA DEI GIOCATORI                   |\n");
    printf("+----------------------------------------------------------+\033[0m\n\n");
    for(int i = 0; i < num_giocatori; i++) {
        printf("%d° Giocatore: %s\n", i+1, giocatori[i]->nome);
        printf("\t[STAT] Attacco: %d | Difesa: %d | Fortuna: %d\n", 
               giocatori[i]->attacco_psichico, 
               giocatori[i]->difesa_psichica, 
               giocatori[i]->fortuna);
        printf("\t[ZAINO]: ");
        for(int j = 0; j < 3; j++) {
            // nome_oggetto è la funzione che restituisce la stringa dall'enum
            printf("[%s] ", nome_oggetto(giocatori[i]->zaino[j]));
        }
        printf("\n");
    }
}

static void deallocazione_giocatori() {
    if (num_giocatori == 0) return;
    printf("Eliminazione dei giocatori precedenti in corso...\n");
    for(int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
        free(giocatori[i]);
        giocatori[i] = NULL;
        }
    }
    num_giocatori = 0;
    undici_presente = 0; // Resettiamo anche il flag del personaggio speciale
}

static void menu_impostazione_mappa() {
    unsigned short scelta = 10;
    do {
        printf("\033[1;36m\n+----------------------------------------------------------+\n");
        printf("|                  MENU IMPOSTAZIONE MAPPA                 |\n");
        printf("+----------------------------------------------------------+\033[0m\n");
        
        printf("\033[1;35m\t0. Torna al menu precedente (Chiudi Mappa)\033[0m\n");
        printf("\033[1;32m\t1. Genera nuova mappa (15 zone Reali + 15 Soprasotto)\033[0m\n");
        printf("\033[1;34m\t2. Inserisci una zona (in entrambi i mondi)\033[0m\n");
        printf("\033[1;33m\t3. Cancella una zona (da entrambi i mondi)\033[0m\n");
        printf("\033[1;37m\t4. Stampa la mappa attuale\033[0m\n");
        
        printf("\nInserire una delle opzioni sopra indicate: ");

        if (scanf("%hu", &scelta) != 1) {
            color('r');
            printf("\nERRORE: Hai inserito un carattere non valido! Inserisci un NUMERO.\n");
            color('0');
            svuotaBuffer();
            scelta = 99;
        } else {
            svuotaBuffer();
        }
        switch(scelta) {
        case 0:
            // chiudi_mappa() deve verificare la presenza del Demotorzone per validare
            chiudi_mappa(); 
            if (mappa_chiusa == 0) {
                // Se il boss manca, forziamo il menu a restare aperto
                scelta = 99; 
            }
            break;
        case 1:
            cancella_mappa();
            genera_mappa();
            break;
        case 2:
            inserisci_zona();
            break;
        case 3:
            cancella_zona();
            break;
        case 4:
            stampa_mappa();
            break;
        case 99:
            // Caso gestito dall'errore di input, non fa nulla e ripete il ciclo
            break;
        default:
            color('r');
            printf("\nComando non riconosciuto! Inserire un valore tra 0 e 4.\n");
            color('0');
            break;
        }
    } while(scelta != 0);
}

static Tipo_nemico genera_nemico_reale(int probabilita) {
    int r = rand() % 100;
    if (r < probabilita) {
        // 70% Billi, 30% Democane (che può stare in entrambi)
        return (rand() % 10 < 7) ? billi : democane;
    }
    return nessun_nemico;
}

static Tipo_nemico genera_nemico_soprasotto(int probabilita, int indice_zona) {
    // Se è l'ultima zona, forziamo Demotorzone se non è già apparso
    if (indice_zona == 14) return demotorzone; 
    int r = rand() % 100;
    if (r < probabilita) {
        return democane; // Nel soprasotto solo Democane o Demotorzone
    }
    return nessun_nemico;
}

static void genera_mappa() {
    if (prima_zona_mondoreale != NULL) {
        cancella_mappa();
    }

    struct Zona_mondoreale* ultima_reale = NULL;
    struct Zona_soprasotto* ultima_soprasotto = NULL;

    for (int i = 0; i < 15; i++) {
        struct Zona_mondoreale* nuova_reale = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuova_sotto = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));
        // Tipo Zona speculare
        nuova_reale->zona = rand() % 10;
        nuova_sotto->zona = nuova_reale->zona; 
        // Oggetti (Solo Reale)
        nuova_reale->oggetto = rand() % 5; 
        // Nemici
        nuova_reale->nemico = genera_nemico_reale(20);
        nuova_sotto->nemico = genera_nemico_soprasotto(30, i);
        // Collegamenti Orizzontali (Avanti/Indietro)
        nuova_reale->avanti = NULL;
        nuova_reale->indietro = ultima_reale;
        nuova_sotto->avanti = NULL;
        nuova_sotto->indietro = ultima_soprasotto;
        // Questi permettono a cambia_mondo() di funzionare correttamente
        nuova_reale->link_soprasotto = nuova_sotto; 
        nuova_sotto->link_mondoreale = nuova_reale;

        if (i == 0) {
            prima_zona_mondoreale = nuova_reale;
            prima_zona_soprasotto = nuova_sotto;
        } else {
            ultima_reale->avanti = nuova_reale;
            ultima_soprasotto->avanti = nuova_sotto;
        }
        ultima_reale = nuova_reale;
        ultima_soprasotto = nuova_sotto;
    }
    numero_zone_create = 15;
    mappa_chiusa = 0; 
    
    color('g');
    printf("Mappa 15+15 generata con collegamenti speculari e Boss finale!\n");
    color('0');
}

static void cancella_mappa() {
    // Deallocazione Mondo Reale
    struct Zona_mondoreale* corr_reale = prima_zona_mondoreale;
    while (corr_reale != NULL) {
        struct Zona_mondoreale* temp = corr_reale;
        corr_reale = corr_reale->avanti;
        free(temp);
    }
    prima_zona_mondoreale = NULL;

    // Deallocazione Soprasotto
    struct Zona_soprasotto* corr_sotto = prima_zona_soprasotto;
    while (corr_sotto != NULL) {
        struct Zona_soprasotto* temp = corr_sotto;
        corr_sotto = corr_sotto->avanti;
        free(temp);
    }
    prima_zona_soprasotto = NULL;

    // Reset variabili di stato
    numero_zone_create = 0;
    mappa_chiusa = 0; 
    color('g');
    printf("Memoria dinamica deallocata: la mappa e' stata cancellata con successo.\n");
    color('0');
}

static void inserisci_zona() {
    // CONTROLLO LIMITE MASSIMO
    if (numero_zone_create >= 50) { 
        color('r');
        printf("Errore: La mappa ha raggiunto il limite massimo di 50 zone!\n");
        color('0');
        return; 
    }
    int temp_scelta = 0;
    unsigned short posizione = 0;
    int input_valido = 0;

    printf("\nIl numero attuale di zone speculari è %hu\n", numero_zone_create);
    
    // SCELTA POSIZIONE (CON ERRORE SE > NUMERO_ZONE)
    do {
        printf("In quale posizione vuoi inserire la nuova zona? (da 0 a %hu): ", numero_zone_create);
        if (scanf("%hu", &posizione) != 1) {
            color('r');
            printf("Errore: Inserisci un numero valido!\n");
            color('0');
            svuotaBuffer();
        } else if (posizione > numero_zone_create) {
            color('r');
            printf("Errore: La posizione %hu non esiste. Range valido: 0-%hu.\n", posizione, numero_zone_create);
            color('0');
            svuotaBuffer();
        } else {
            svuotaBuffer();
            input_valido = 1;
        }
    } while (!input_valido);

    // Allocazione memoria per i nuovi nodi
    struct Zona_mondoreale* nuova_reale = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_sotto = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));
    do {
        printf("\nScegli il tipo di zona (0-9):\n");
        printf("0:Bosco, 1:Scuola, 2:Laboratorio, 3:Caverna, 4:Strada,\n");
        printf("5:Giardino, 6:Supermercato, 7:Centrale, 8:Deposito, 9:Polizia\n >> ");
        
        if (scanf("%d", &temp_scelta) != 1 || temp_scelta < 0 || temp_scelta > 9) {
            color('r'); printf("Errore: Inserire un numero tra 0 e 9\n"); color('0');
            svuotaBuffer();
            temp_scelta = -1;
        } else {
            svuotaBuffer();
        }
    } while (temp_scelta < 0);
    
    nuova_reale->zona = (Tipo_zona)temp_scelta;
    nuova_sotto->zona = (Tipo_zona)temp_scelta;

    // OGGETTO (Solo Mondo Reale)
    do {
        printf("Scegli oggetto nel Mondo Reale (0:Nessuno, 1:Bicicletta, 2:Maglietta, 3:Bussola, 4:Schitarrata): ");
        if (scanf("%d", &temp_scelta) != 1 || temp_scelta < 0 || temp_scelta > 4) {
            color('r'); printf("Errore: Scelta non valida!\n"); color('0');
            svuotaBuffer();
            temp_scelta = -1;
        } else {
            svuotaBuffer();
        }
    } while (temp_scelta < 0);
    nuova_reale->oggetto = (Tipo_oggetto)temp_scelta;

    // NEMICO MONDO REALE
    do {
        printf("Nemico nel Mondo Reale (0:Nessuno, 1:Billi): ");
        if (scanf("%d", &temp_scelta) != 1 || (temp_scelta != 0 && temp_scelta != 1)) {
            color('r'); printf("Errore: Inserire un numero compreso tra le opzioni sopra indicate (0 o 1)\n"); color('0');
            svuotaBuffer();
            temp_scelta = -1;
        } else {
            svuotaBuffer();
        }
    } while (temp_scelta == -1);
    nuova_reale->nemico = (Tipo_nemico)temp_scelta;

    // NEMICO SOPRASOTTO
    do {
        printf("Nemico nel Soprasotto (0:Nessuno, 2:Democane, 3:Demotorzone): ");
        if (scanf("%d", &temp_scelta) != 1 || (temp_scelta < 0 || temp_scelta > 3 || temp_scelta == 1)) {
            color('r'); printf("Errore: Scelta non valida (1 non ammesso qui)!\n"); color('0');
            svuotaBuffer();
            temp_scelta = -1;
        } else {
            svuotaBuffer();
        }
    } while (temp_scelta == -1);
    nuova_sotto->nemico = (Tipo_nemico)temp_scelta;

    // Inizializzazione puntatori e collegamenti trasversali
    nuova_reale->avanti = NULL; nuova_reale->indietro = NULL;
    nuova_sotto->avanti = NULL; nuova_sotto->indietro = NULL;
    nuova_reale->link_soprasotto = nuova_sotto; 
    nuova_sotto->link_mondoreale = nuova_reale; 

    // LOGICA DI INSERIMENTO PUNTATORI
    if (numero_zone_create == 0 || posizione == 0) {
        // Inserimento in testa
        if (prima_zona_mondoreale != NULL) {
            nuova_reale->avanti = prima_zona_mondoreale;
            prima_zona_mondoreale->indietro = nuova_reale;
            nuova_sotto->avanti = prima_zona_soprasotto;
            prima_zona_soprasotto->indietro = nuova_sotto;
        }
        prima_zona_mondoreale = nuova_reale;
        prima_zona_soprasotto = nuova_sotto;
    } else {
        // Inserimento in mezzo o in coda
        struct Zona_mondoreale* corr_reale = prima_zona_mondoreale;
        struct Zona_soprasotto* corr_sotto = prima_zona_soprasotto;
        
        // Scorriamo fino alla posizione desiderata
        for (int i = 0; i < posizione - 1 && corr_reale->avanti != NULL; i++) {
            corr_reale = corr_reale->avanti;
            corr_sotto = corr_sotto->avanti;
        }
        // Collegamento Mondo Reale
        nuova_reale->avanti = corr_reale->avanti;
        nuova_reale->indietro = corr_reale;
        if (corr_reale->avanti != NULL) corr_reale->avanti->indietro = nuova_reale;
        corr_reale->avanti = nuova_reale;

        // Collegamento Soprasotto
        nuova_sotto->avanti = corr_sotto->avanti;
        nuova_sotto->indietro = corr_sotto;
        if (corr_sotto->avanti != NULL) corr_sotto->avanti->indietro = nuova_sotto;
        corr_sotto->avanti = nuova_sotto;
    }
    numero_zone_create++;
    mappa_chiusa = 0; // La mappa è cambiata, va ri-validata
    color('g');
    printf("\nZona speculare inserita correttamente in posizione %hu!\n", posizione);
    color('0');
}

static void cancella_zona() {
    if (numero_zone_create == 0) {
        color('r');
        printf("Mappa vuota, nulla da cancellare!\n");
        color('0');
        return;
    }
    int posizione = -1; // Usiamo int per gestire meglio i controlli sui negativi
    int input_valido = 0;

    do {
        printf("Inserisci la posizione della zona da rimuovere (0 a %hu): ", numero_zone_create - 1);
        // scanf restituisce 1 se legge un numero, 0 se legge una lettera
        if (scanf("%d", &posizione) != 1) {
        color('r');
        printf("Errore: devi inserire un numero intero!\n");
        color('0');
        svuotaBuffer();
        } else {
        svuotaBuffer();
        
        // Controlliamo se il numero è nel range
        if (posizione < 0 || posizione >= numero_zone_create) {
            color('r');
            printf("Errore: Posizione %d non valida. Inserire un numero tra 0 e %hu\n", 
                posizione, numero_zone_create - 1);
            color('0');
        } else {
            input_valido = 1; // Il numero è perfetto, possiamo uscire dal ciclo
        }
        }
    } while (!input_valido);

    struct Zona_mondoreale* da_canc_reale = prima_zona_mondoreale;
    struct Zona_soprasotto* da_canc_sotto = prima_zona_soprasotto;

    // Raggiungiamo la posizione (posizione è ora sicuramente valida)
    for (int i = 0; i < posizione; i++) {
        da_canc_reale = da_canc_reale->avanti;
        da_canc_sotto = da_canc_sotto->avanti;
    }

    // LOGICA DI CANCELLAZIONE (Gestione puntatori)
    // Mondo Reale
    if (da_canc_reale->indietro != NULL) 
        da_canc_reale->indietro->avanti = da_canc_reale->avanti;
    else 
        prima_zona_mondoreale = da_canc_reale->avanti;
    if (da_canc_reale->avanti != NULL) 
        da_canc_reale->avanti->indietro = da_canc_reale->indietro;
    // Soprasotto
    if (da_canc_sotto->indietro != NULL) 
        da_canc_sotto->indietro->avanti = da_canc_sotto->avanti;
    else 
        prima_zona_soprasotto = da_canc_sotto->avanti;
    if (da_canc_sotto->avanti != NULL) 
        da_canc_sotto->avanti->indietro = da_canc_sotto->indietro;

    free(da_canc_reale);
    free(da_canc_sotto);
    numero_zone_create--;
    mappa_chiusa = 0; // Quando cancello una zona, la mappa deve essere richiusa prima di giocare
    color('g');
    printf("Zona speculare in posizione %d rimossa con successo!\n", posizione);
    color('0');
    }

static char* verifica_tipo_zona_reale(Zona_mondoreale* pZona) {
    switch (pZona->zona) {
        case bosco: return "bosco";
        case scuola: return "scuola";
        case laboratorio: return "laboratorio";
        case caverna: return "caverna";
        case strada: return "strada";
        case giardino: return "giardino";
        case supermercato: return "supermercato";
        case centrale_elettrica: return "centrale elettrica";
        case deposito_abbandonato: return "deposito abbandonato";
        case stazione_polizia: return "stazione di polizia";
    }
    return "";
}

static char* verifica_tipo_oggetto(Zona_mondoreale* pZona) {
    switch (pZona->oggetto) {
        case nessun_oggetto: return "nessun oggetto";
        case bicicletta: return "bicicletta";
        case maglietta_fuocoinferno: return "maglietta fuocoinferno";
        case bussola: return "bussola";
        case schitarrata_metallica: return "schitarrata metallica";
    }
    return "";
}

static char* verifica_tipo_nemico_reale(Zona_mondoreale* pZona) {
    switch (pZona->nemico) {
        case nessun_nemico: return "nessun nemico";
        case billi: return "billi";
        case democane: return "democane";
        case demotorzone: return "demotorzone";
    }
    return "";
}

static char* verifica_tipo_zona_sotto(Zona_soprasotto* pZona) {
    switch (pZona->zona) {
        case bosco: return "bosco";
        case scuola: return "scuola";
        case laboratorio: return "laboratorio";
        case caverna: return "caverna";
        case strada: return "strada";
        case giardino: return "giardino";
        case supermercato: return "supermercato";
        case centrale_elettrica: return "centrale elettrica";
        case deposito_abbandonato: return "deposito abbandonato";
        case stazione_polizia: return "stazione di polizia";
    }
    return "";
}

static char* verifica_tipo_nemico_sotto(Zona_soprasotto* pZona) {
    switch (pZona->nemico) {
        case nessun_nemico: return "nessun nemico";
        case billi: return "billi";
        case democane: return "democane";
        case demotorzone: return "demotorzone";
    }
    return "";
}

static char* nome_oggetto(Tipo_oggetto o) {
    switch (o) {
        case nessun_oggetto: return "vuoto";
        case bicicletta: return "bicicletta";
        case maglietta_fuocoinferno: return "maglietta fuocoinferno";
        case bussola: return "bussola";
        case schitarrata_metallica: return "schitarrata metallica";
    }
    return "";
}

static void stampa_mappa() {
    if(numero_zone_create == 0) {
        printf("La mappa e' vuota!\n");
        return;
    }
    struct Zona_mondoreale* corr_reale = prima_zona_mondoreale;
    struct Zona_soprasotto* corr_sotto = prima_zona_soprasotto;
    unsigned short posizione = 0;
    printf("\nSTATO DELLA MAPPA SPECULARE \n");
    
    // Continua finche' entrambe le liste hanno zone (sono speculari)
    while(corr_reale != NULL && corr_sotto != NULL) {
        printf("Zona %hu:\n", posizione);
        
        // Stampa Mondo Reale: passiamo il puntatore 'corr_reale'
        printf("\t[REALE] Tipo: %s | Nemico: %s | Oggetto: %s\n", 
            verifica_tipo_zona_reale(corr_reale), 
            verifica_tipo_nemico_reale(corr_reale),
            verifica_tipo_oggetto(corr_reale));
        
        // Stampa Soprasotto: passiamo il puntatore 'corr_sotto'
        printf("\t[SOPRA] Tipo: %s | Nemico: %s\n", 
            verifica_tipo_zona_sotto(corr_sotto), 
            verifica_tipo_nemico_sotto(corr_sotto));
        
        printf("\n");
        
        // Avanzamento dei puntatori
        corr_reale = corr_reale->avanti;
        corr_sotto = corr_sotto->avanti;
        posizione++;
    }
}

static void chiudi_mappa() {
    if(numero_zone_create < 15) {
        color('r');
        printf("Errore: La mappa ha solo %hu zone. Servono almeno 15 zone per giocare!\n", numero_zone_create);
        color('0');
        mappa_chiusa = 0;
        return;
    }
    // Conteggio Demotorzone nel Soprasotto
    int contatore_demotorzone = 0;
    struct Zona_soprasotto* corr = prima_zona_soprasotto;
    
    while(corr != NULL) {
        if(corr->nemico == demotorzone) {
        contatore_demotorzone++;
        }
        corr = corr->avanti;
    }

    if(contatore_demotorzone != 1) {
        color('r');
        printf("Errore: Nel Soprasotto devono esserci ESATTAMENTE 1 Demotorzone (attualmente: %d)\n", contatore_demotorzone);
        printf("Usa inserisci/cancella zona per sistemare la mappa.\n");
        color('0');
        mappa_chiusa = 0;
    } else {
        printf("Mappa validata e chiusa con successo. Pronti per giocare!\n");
        mappa_chiusa = 1;
        impostato = true; // Permette l'accesso alla funzione gioca()
    }
}

static void menu_impostazione_tempo_pausa() {
    int scelta_temp = 0;
    int input_valido = 0;

    do {
        printf("\n--- IMPOSTA VELOCITÀ TESTO ---");
        printf("\nInserire secondi di pausa tra le azioni (0-5): ");
        
        if (scanf("%d", &scelta_temp) != 1) {
            color('r');
            printf("Errore: Inserire un numero intero!\n");
            color('0');
            svuotaBuffer();
        } else {
            svuotaBuffer();
            if (scelta_temp < 0 || scelta_temp > 5) {
                color('r');
                printf("Errore: Inserire un valore tra 0 e 5 secondi!\n");
                color('0');
            } else {
                durata_intervallo = (unsigned short)scelta_temp;
                input_valido = 1;
                color('g');
                printf("Velocità testo impostata a %d secondi.\n", durata_intervallo);
                color('0');
            }
        }
    } while (!input_valido);
}

// Funzione per generare l'ordine dei turni in modo casuale ogni round
static void genera_ordine_casuale(unsigned short ordine_giocatori[]) {
    ordine_giocatori[0] = rand() % num_giocatori;
    for(int i = 1; i < num_giocatori; i++) {
        ordine_giocatori[i] = rand() % num_giocatori;
        for(int j = i - 1; j >= 0; j--) {
        if(ordine_giocatori[i] == ordine_giocatori[j]) {
            i--;
            break;
        }
        }
    }
}

static char* nome_nemico(Tipo_nemico n) {
    switch (n) {
        case nessun_nemico: return "Nessun nemico";
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "DEMOTORZONE";
        default: return "Nemico sconosciuto";
    }
}

static void avanza(Giocatore* giocatore, unsigned short* movimento) {
    if(*movimento > 0) {
        color('r');
        printf("Ti sei gia' mosso! Sciegli un'altra opzione\n");
        color('0');
        return;
    } 
    if (presenza_nemico_nella_zona(giocatore)) {
        color('r');
        printf("C'e' un nemico! Devi combattere o cambiare mondo!\n");
        color('0');
        return;
    }
    if (giocatore->mondo == 0) { // MONDO REALE
        if (giocatore->pos_mondoreale->avanti == NULL) {
            // Controllo Vittoria
            if (demotorzone_sconfitto) {
                printf("Sei all'uscita! Passa il turno per vincere!\n");
            } else {
                printf("L'uscita è chiusa! Devi prima sconfiggere il Demotorzone nel Soprasotto!\n");
            }
            return;
        }
        giocatore->pos_mondoreale = giocatore->pos_mondoreale->avanti;
        giocatore->pos_soprasotto = giocatore->pos_soprasotto->avanti;
    } 
    else { // SOPRASOTTO
        if (giocatore->pos_soprasotto->avanti == NULL) {
            printf("Sei nel cuore del Soprasotto. Non puoi andare oltre.\n");
            return;
        }
        giocatore->pos_soprasotto = giocatore->pos_soprasotto->avanti;
        giocatore->pos_mondoreale = giocatore->pos_mondoreale->avanti;
    }
    *movimento = 1;
    printf("%s avanza. Ora sei in: %s\n", giocatore->nome, 
           (giocatore->mondo == 0) ? verifica_tipo_zona_reale(giocatore->pos_mondoreale) : "Soprasotto");
}

static void indietreggia(Giocatore* giocatore, unsigned short* movimento) {
    // Controllo se ha già mosso in questo turno
    if (*movimento > 0) {
        color('r');
        printf("Hai gia' effettuato un movimento in questo turno!\n");
        color('0');
        return;
    }
    // Controllo nemico nella zona attuale
   if (presenza_nemico_nella_zona(giocatore)) {
        color('r');
        printf("Non puoi indietreggiare! C'e' un nemico che ti blocca la strada.\n");
        printf("Devi prima sconfiggerlo per poterti muovere.\n");
        color('0');
        return;
    }

    // Verifica se siamo all'inizio della mappa (niente alle spalle)
    if (giocatore->mondo == 0) { // MONDO REALE
        if (giocatore->pos_mondoreale->indietro == NULL) {
            color('y');
            printf("Sei gia' nella zona iniziale del Mondo Reale, non puoi tornare piu' indietro!\n");
            color('0');
        } else {
            // Muoviamo entrambi i puntatori per restare speculari
            giocatore->pos_mondoreale = giocatore->pos_mondoreale->indietro;
            giocatore->pos_soprasotto = giocatore->pos_soprasotto->indietro;
            printf("Sei tornato alla zona precedente nel Mondo Reale.\n");
            *movimento = *movimento + 1;
        }
    } else { // SOPRASOTTO
        if (giocatore->pos_soprasotto->indietro == NULL) {
            color('y');
            printf("Sei all'inizio del Soprasotto, non c'e' nulla alle tue spalle.\n");
            color('0');
        } else {
            giocatore->pos_soprasotto = giocatore->pos_soprasotto->indietro;
            giocatore->pos_mondoreale = giocatore->pos_mondoreale->indietro;
            printf("Sei tornato alla zona precedente nel Soprasotto.\n");
            *movimento = *movimento + 1;
        }
    }
}

static void cambia_mondo(Giocatore* giocatore, unsigned short* movimento) {
    if (*movimento > 0) {
        color('r');
        printf("Hai già effettuato un movimento in questo turno!\n");
        color('0');
        return;
    }
    if (giocatore->mondo == 0) { // Da Reale a Soprasotto
        giocatore->mondo = 1;
        color('p');
        printf("%s attraversa il portale verso il Soprasotto...\n", giocatore->nome);
        color('0');
    } else { // Da Soprasotto a Reale (Tentativo)
        printf("Provi a tornare nel Mondo Reale. Test fortuna in corso...\n");
        int test = (rand() % 100);
        
        // Se la fortuna è alta, hai più probabilità (es. fortuna * 10)
        if (test < (giocatore->fortuna * 15)) { 
            giocatore->mondo = 0;
            color('g');
            printf("Successo! Sei tornato nella realtà.\n");
            color('0');
        } else {
            giocatore->fortuna--;
            color('r');
            printf("Il portale ti respinge! Perdi 1 Fortuna e resti nel Soprasotto.\n");
            color('0');
        }
    }
    *movimento = 1; // Il cambio mondo consuma il movimento del turno
}

static bool presenza_nemico_nella_zona(Giocatore* giocatore) {
    if (giocatore->mondo == 0) { // Mondo Reale
        if (giocatore->pos_mondoreale->nemico != nessun_nemico) {
            return true;
        }
    } else { // Soprasotto
        if (giocatore->pos_soprasotto->nemico != nessun_nemico) {
            return true;
        }
    }
    return false;
}

static void rimuovi_nemico_corrente(Giocatore* giocatore) {
    if (giocatore->mondo == 0) {
        if (giocatore->pos_mondoreale != NULL) {
            giocatore->pos_mondoreale->nemico = nessun_nemico;
        }
    } else {
        if (giocatore->pos_soprasotto != NULL) {
            giocatore->pos_soprasotto->nemico = nessun_nemico;
        }
    }
}

static void stampa_giocatore(int i) {
    Giocatore* giocatore = giocatori[i];
    printf("\033[1;36m\n+----------------------------------------------------------+\n");
    printf("|              STATISTICHE GIOCATORE: %-20s |\n", giocatore->nome);
    printf("+----------------------------------------------------------+\033[0m\n");    printf("\tMondo attuale: %s\n", (giocatore->mondo == 0) ? "Mondo Reale" : "Soprasotto");
    printf("\tAttacco Psichico: %d\n", giocatore->attacco_psichico);
    printf("\tDifesa Psichica: %d\n", giocatore->difesa_psichica);
    printf("\tFortuna: %d\n", giocatore->fortuna);
    
    printf("\tZaino (3 slot): ");
    for(int j = 0; j < 3; j++) {
        printf("[%s] ", nome_oggetto(giocatore->zaino[j]));
    }
  sleep(durata_intervallo);
}

static void stampa_zona_gioco(Giocatore* giocatore) {
    printf("Il giocatore %s si trova in una zona di tipo: ", giocatore->nome);
    if (giocatore->mondo == 0) { // MONDO REALE
        printf("%s\n", verifica_tipo_zona_reale(giocatore->pos_mondoreale));
        // Oggetto nella zona
        if (giocatore->pos_mondoreale->oggetto == nessun_oggetto) {
            printf("\tOggetto: Non ci sono oggetti utili qui!\n");
        } else {
            printf("\tOggetto: Vedi a terra un/una %s!\n", nome_oggetto(giocatore->pos_mondoreale->oggetto));
        }
        // Nemico nella zona
        if (giocatore->pos_mondoreale->nemico == nessun_nemico) {
            printf("\tNemici: La zona sembra tranquilla.\n");
        } else {
            color('r');
            printf("\tATTENZIONE: C'e' %s che ti osserva!\n", (giocatore->pos_mondoreale->nemico == billi) ? "Billi" : "un nemico");
            color('0');
        }
        // Segnale che Demotorzone si trova nel Soprasotto
        if (giocatore->pos_soprasotto->nemico == demotorzone) {
            color('r');
            printf("\n\t[!!!] Un brivido ti percorre la schiena: senti una presenza \n");
            printf("\taltamente pericolosa nella versione oscura di questo luogo.\n");
            printf("\tForse dovresti Cambiare Mondo per investigare...\n");
            color('0');
        }

    } else { // SOPRASOTTO
        printf("%s (Versione Oscura)\n", verifica_tipo_zona_sotto(giocatore->pos_soprasotto));
        // Nel Soprasotto non ci sono oggetti
        printf("\tOggetto: L'oscurita' avvolge tutto, non trovi oggetti.\n");
        // Nemici nel Soprasotto
        if (giocatore->pos_soprasotto->nemico == nessun_nemico) {
            printf("\tNemici: Senti dei rumori, ma non vedi nessuno.\n");
        } else {
            Tipo_nemico n = giocatore->pos_soprasotto->nemico;
            printf("\tPERICOLO: In questa zona c'e' un %s!\n", 
                    (n == democane) ? "Democane" : (n == demotorzone) ? "DEMOTORZONE" : "Nemico");
        }
    }
    sleep(durata_intervallo);
}

static void raccogli_oggetto(struct Giocatore* giocatore, unsigned short* azione) {
    // Controllo giocatore se ha già compiuto un'azione
    if (*azione > 0) {
        color('r');
        printf("Il giocatore %s ha già compiuto un'azione in questo turno!\n", giocatore->nome);
        color('0');
        return;
    } 
    // Controllo se siamo nel Soprasotto (Gli oggetti sono solo nel Mondo Reale)
    if (giocatore->mondo == 1) { // 1 = Soprasotto
        color('y');
        printf("Ti trovi nel Soprasotto: qui non esistono oggetti materiali da raccogliere!\n");
        color('0');
        return;
    }
    // Puntatore alla zona reale per comodità
    struct Zona_mondoreale* zona = giocatore->pos_mondoreale;
    // CONTROLLO NEMICO (Vincolo PDF: "Non si può raccogliere oggetto se c'è un nemico")
    if (zona->nemico != nessun_nemico) {
        color('r');
        printf("Attenzione! Non puoi raccogliere l'oggetto finche' %s e' presente!\n", nome_nemico(zona->nemico));
        printf("Devi prima sconfiggerlo o farlo scappare.\n");
        color('0');
        return;
    }
    // Controllo se c'è effettivamente un oggetto
    if (zona->oggetto == nessun_oggetto) {
        printf("Il giocatore %s guarda intorno, ma non c'è nulla da raccogliere.\n", giocatore->nome);
        return;
    } 
    // Controllo spazio nello zaino (3 slot)
    int slot_libero = -1;
    for (int i = 0; i < 3; i++) {
        if (giocatore->zaino[i] == nessun_oggetto) {
        slot_libero = i;
        break;
        }
    }
    if (slot_libero == -1) {
        color('r');
        printf("Lo zaino di %s è pieno! Devi scartare qualcosa per raccogliere altro.\n", giocatore->nome);
        color('0');
    } else {
        // Raccolta effettiva
        Tipo_oggetto oggetto_preso = zona->oggetto;
        giocatore->zaino[slot_libero] = oggetto_preso;
        zona->oggetto = nessun_oggetto; // L'oggetto scompare dalla mappa
        oggetti_raccolti_totali++;
        color('g');
        printf("Ottimo! %s ha raccolto: %s!\n", giocatore->nome, nome_oggetto(oggetto_preso));
        color('0');
        // Incrementiamo l'azione per terminare il turno
        *azione = *azione + 1;
    }
}

static void utilizza_oggetto(Giocatore* giocatore, unsigned short* azione) {
    // Controllo se il giocatore ha già fatto un'azione
    if (*azione > 0) {
        color('r');
        printf("Il giocatore %s ha già compiuto un'azione in questo turno!\n", giocatore->nome);
        color('0');
        return;
    }
    // controllo zaino vuoto
    bool zaino_vuoto = true;
    for (int i = 0; i < 3; i++) {
        if (giocatore->zaino[i] != nessun_oggetto) {
            zaino_vuoto = false;
            break;
        }
    }
    if (zaino_vuoto) {
        color('y');
        printf("Non c'e' nessun oggetto nello zaino di %s!\n", giocatore->nome);
        color('0');
        return; // Torna al menu senza consumare l'azione
    }
    stampa_zaino(giocatore);
    printf("Quale oggetto vuoi usare? (1-3, 0 per annullare): ");
    
    int scelta;
    if (scanf("%d", &scelta) != 1) {
        svuotaBuffer();
        printf("Input non valido.\n");
        return;
    }
    svuotaBuffer();
    if (scelta == 0) return; // Annulla l'operazione

    // Controllo se lo slot selezionato è vuoto o fuori range
    if (scelta < 1 || scelta > 3 || giocatore->zaino[scelta-1] == nessun_oggetto) {
        color('r');
        printf("Scelta non valida: in questo slot non c'e' nessun oggetto!\n");
        color('0');
        return;
    }
    // Utilizzo effettivo dell'oggetto
    Tipo_oggetto oggetto_scelto = giocatore->zaino[scelta-1];
    color('g');
    if (oggetto_scelto == bicicletta) {
        printf("Il giocatore %s usa la %s! La difesa aumenta.\n", giocatore->nome, nome_oggetto(bicicletta));
        giocatore->difesa_psichica += 2;
    } 
    else if (oggetto_scelto == maglietta_fuocoinferno) {
        printf("Il giocatore %s indossa la %s! L'attacco aumenta.\n", giocatore->nome, nome_oggetto(maglietta_fuocoinferno));
        giocatore->attacco_psichico += 2;
    }
    else if (oggetto_scelto == bussola) {
        printf("Il giocatore %s consulta la %s! La fortuna aumenta.\n", giocatore->nome, nome_oggetto(bussola));
        giocatore->fortuna += 1;
    }
    else if (oggetto_scelto == schitarrata_metallica) {
        printf("Il giocatore %s lancia una %s! Forza psichica totale aumentata.\n", giocatore->nome, nome_oggetto(schitarrata_metallica));
        giocatore->attacco_psichico += 1;
        giocatore->difesa_psichica += 1;
    }
    color('0');
    giocatore->zaino[scelta-1] = nessun_oggetto; 
    *azione = *azione + 1;
}

static void subisce_danno_psichico(Giocatore* giocatore) {
    color('p');
    giocatore->fortuna--; 
    if (giocatore->fortuna <= 0) {
        giocatore->fortuna = 0; // Evita numeri negativi
        num_giocatori_morti++;
        printf("Il giocatore %s ha perso tutta la sua forza di volontà ed è fuori dai giochi!\n", giocatore->nome);
    } else {
        printf("Il giocatore %s vacilla! Fortuna rimanente: %d\n", giocatore->nome, giocatore->fortuna);
    }
    color('0');
    sleep(durata_intervallo);
}

static void combatti(Giocatore* giocatore, unsigned short* azione) {
    // Controllo se giocatore ha già agito
    if (*azione > 0) {
        color('r');
        printf("Il giocatore %s ha già compiuto un'azione in questo turno!\n", giocatore->nome);
        color('0');
        return;
    }
    // Identificazione nemico
    Tipo_nemico nemico_attuale;
    if (giocatore->mondo == 0) 
        nemico_attuale = giocatore->pos_mondoreale->nemico;
    else 
        nemico_attuale = giocatore->pos_soprasotto->nemico;
    if (nemico_attuale == nessun_nemico) {
        color('r');
        printf("Non ci sono nemici da combattere in questa zona per %s\n", giocatore->nome);
        color('0');
        return;
    }
    color('b');
    printf("\n--- INIZIO SCONTRO PSICHICO: %s VS %s ---\n", 
           giocatore->nome, 
           (nemico_attuale == billi) ? "Billi" : (nemico_attuale == democane) ? "Democane" : "DEMOTORZONE");
    color('0');

    int salute_psichica_giocatore = giocatore->difesa_psichica + giocatore->fortuna;
    int salute_psichica_nemico = (nemico_attuale == billi) ? 3 : (nemico_attuale == democane) ? 5 : 10;

    do {
        printf("\nLanciate i dadi per l'iniziativa...\n");
        unsigned short dado_giocatore = lancia_dado();
        unsigned short dado_nemico = lancia_dado();
        printf("Dado %s: %hu VS Dado Nemico: %hu\n", giocatore->nome, dado_giocatore, dado_nemico);
        sleep(durata_intervallo);
        if (dado_giocatore >= dado_nemico) {
            printf("%s attacca per primo!\n", giocatore->nome);
            // Attacco del giocatore: dado + attacco_psichico
            int danno = (lancia_dado()) + giocatore->attacco_psichico;
            salute_psichica_nemico -= danno;
            printf("Colpo psichico! Il nemico barcolla (Salute nemico: %d)\n", (salute_psichica_nemico < 0) ? 0 : salute_psichica_nemico);
        } else {
            printf("Il nemico attacca per primo!\n");
            // Attacco del nemico: forza fissa in base al tipo
            int danno_nemico = (nemico_attuale == billi) ? 2 : (nemico_attuale == democane) ? 4 : 6;
            salute_psichica_giocatore -= danno_nemico;
            printf("%s subisce un attacco mentale! (Tua salute psichica: %d)\n", giocatore->nome, (salute_psichica_giocatore < 0) ? 0 : salute_psichica_giocatore);
        }
        sleep(durata_intervallo);
    } while (salute_psichica_giocatore > 0 && salute_psichica_nemico > 0);

    // Risultato finale
    if (salute_psichica_giocatore > 0) {
        color('g');
        printf("\nVITTORIA! %s ha sconfitto il nemico!\n", giocatore->nome);
        nemici_sconfitti_totali++;
        
        if (nemico_attuale == demotorzone) {
            demotorzone_sconfitto = 1;
            printf("Il DEMOTORZONE è stato ricacciato nell'abisso!\n");
            rimuovi_nemico_corrente(giocatore); 
        } else {
            printf("Il nemico e' stato sconfitto e rimosso dalla zona!\n");
            rimuovi_nemico_corrente(giocatore);
        }
        color('0');
    } else {
        color('p');
        printf("\nSCONFITTA! %s è stato sopraffatto mentalmente\n", giocatore->nome);
        subisce_danno_psichico(giocatore); 
        color('0');
    }
    *azione = *azione + 1;
    if (nemico_attuale != demotorzone) {
        printf("Combattimento terminato per %s.\n", giocatore->nome);
    }
}

static unsigned short lancia_dado() {
    unsigned short valore = (rand() % 6) + 1; 
    printf("Lancio del dado... ");
    fflush(stdout); 
    sleep(durata_intervallo);
    printf("[%hu]\n", valore);
    
    return valore;
}

static void investiga_zona(Giocatore* giocatore, unsigned short* azione) {
    if (*azione > 0) {
        color('r');
        printf("Il giocatore %s ha già compiuto un'azione in questo turno!\n", giocatore->nome);
        color('0');
        return;
    }
    if (giocatore->mondo == 1) { // Soprasotto
        printf("Nel Soprasotto l'oscurità è troppo fitta per investigare.\n");
        return;
    }
    if (giocatore->pos_mondoreale->oggetto != nessun_oggetto) {
        printf("C'è già un oggetto qui: %s. Raccoglilo!\n", nome_oggetto(giocatore->pos_mondoreale->oggetto));
        return;
    }
    printf("%s inizia a cercare indizi nella zona...\n", giocatore->nome);
    // Chiamata alla funzione del dado
    unsigned short risultato_dado = lancia_dado();
    // Determinazione della soglia in base alla zona
    int soglia_necessaria; 
    Tipo_zona zona_attuale = giocatore->pos_mondoreale->zona;
    if (zona_attuale == laboratorio || zona_attuale == supermercato || zona_attuale == stazione_polizia) {
        soglia_necessaria = 4; // Con 4, 5, 6 hai successo (50%)
    } else {
        soglia_necessaria = 5; // Con 5, 6 hai successo (circa 30%)
    }
    if (risultato_dado >= soglia_necessaria) {
        // SUCCESSO: Scelta dell'oggetto con rand() "silenzioso"
        Tipo_oggetto trovato = (rand() % 4) + 1; 
        giocatore->pos_mondoreale->oggetto = trovato;
        color('g');
        printf("Successo! %s ha trovato un/una %s!\n", giocatore->nome, nome_oggetto(trovato));
        color('0');
    } else {
        // FALLIMENTO
        color('p');
        printf("L'investigazione non ha prodotto risultati.\n");
        // Rischio Billi (20% -> 1 su 5)
        if ((rand() % 5) == 0) {
             printf("ATTENZIONE: I rumori hanno attirato Billi!\n");
             giocatore->pos_mondoreale->nemico = billi; 
        }
        color('0');
    }
    *azione = *azione + 1;
}

static void passa(int prossimo_indice, unsigned short ordine[]) {
    color('y');
    printf("\n%s ha terminato il turno.", giocatori[ordine[prossimo_indice]]->nome);
    
    // Se c'è un giocatore successivo nel round attuale
    if (prossimo_indice + 1 < num_giocatori) {
        int successivo = ordine[prossimo_indice + 1];
        printf(" Ora tocca a: %s\n", giocatori[successivo]->nome);
    } else {
        printf(" Fine del round attuale!\n");
    }
    printf("Le energie psichiche si ricaricano...\n");
    color('0');
    sleep(durata_intervallo);
}
