#include "gamelib.h"
#include <stdio.h>
#include <stdlib.h>

extern void svuotaBuffer();
extern void color(char c);

int main() {
    printf("\033[1;33mATTENZIONE: Per una migliore esperienza, imposta il terminale a SCHERMO INTERO!\n");
    printf("Premi INVIO per continuare...\033[0m");
    svuotaBuffer(); 

    system("clear");

    printf("\n                                     \033[1;36mBENVENUTI IN\033[0m \n");
    printf("\033[1;36m+---------------------------------------------------------------------------------------+\n");
    printf("|        \033[1;31m_____\033[1;32m  ____  \033[1;33m _____ \033[1;34m______ \033[1;35m _____ \033[1;33m_______ \033[1;31m_____ \033[1;33m           _   _ \033[1;34m ______ \033[1;36m      |\n");
    printf("|       \033[1;31m/ ____|\033[1;32m/ __ \\ \033[1;33m/ ____\033[1;34m|  ____|\033[1;35m/ ____|\033[1;33m__   __\033[1;31m|  __ \\\033[1;32m     /\\   \033[1;33m| \\ | |\033[1;34m|  ____|\033[1;36m      |\n");
    printf("|      \033[1;31m| |    \033[1;32m| |  | |\033[1;33m (___ \033[1;34m| |__  \033[1;35m| (___ \033[1;33m   | |  \033[1;31m| |__) |\033[1;32m   /  \\  \033[1;33m|  \\| |\033[1;34m| |__   \033[1;36m      |\n");
    printf("|      \033[1;31m| |    \033[1;32m| |  | |\033[1;33m\\___ \\\033[1;34m|  __| \033[1;35m \\___ \\\033[1;33m   | |  \033[1;31m|  _  / \033[1;32m  / /\\ \\ \033[1;33m| . ` |\033[1;34m|  __|  \033[1;36m      |\n");
    printf("|      \033[1;31m| |____\033[1;32m| |__| |\033[1;33m____)|\033[1;34m| |____\033[1;35m ____) |\033[1;33m  | |  \033[1;31m| | \\ \\ \033[1;32m / ____ \\\033[1;33m| |\\  |\033[1;34m| |____ \033[1;36m      |\n");
    printf("|       \033[1;31m\\_____|\033[1;32m\\____/\033[1;33m|_____/\033[1;34m|______|\033[1;35m|_____/ \033[1;33m |_|  \033[1;31m|_|  \\_\\\033[1;32m/_/    \\_\\\033[1;33m_| \\_|\033[1;34m|______|\033[1;36m      |\n");
    printf("+---------------------------------------------------------------------------------------+\033[0m\n");

    int scelta = 0;
    do {
        printf("\033[1;36m\n+---------------------------------------------------------------------------------------+\n");
        printf("|                                   MENU PRINCIPALE                                     |\n");
        printf("+---------------------------------------------------------------------------------------+\033[0m\n");

        printf("\n\033[1;32m1) Imposta Gioco\033[0m\n");
        printf("\033[1;34m2) Gioca\033[0m\n");
        printf("\033[1;31m3) Termina Gioco\033[0m\n");
        printf("\033[1;35m4) Crediti\033[0m\n");
        
        printf("\nInserire un valore compreso tra 1 e 4: ");
        
        if (scanf("%d", &scelta) != 1) {
            printf("\033[1;31mErrore: Inserire un numero!\033[0m\n");
            svuotaBuffer();
            scelta = -1;
            continue;
        }
        svuotaBuffer();

        switch (scelta) {
            case 1: 
                imposta_gioco(); 
                break;
            case 2: 
                gioca(); 
                break;
            case 3: 
                termina_gioco();
                scelta = 0; 
                break;
            case 4: 
                crediti(); 
                break;
            default:
                printf("\033[1;31mComando non riconosciuto! Inserire un valore compreso tra 1 e 4\033[0m\n");
                break;
        }
    } while (scelta != 3);

    return 0;
}
