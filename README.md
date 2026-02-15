[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/5fsIc7xe)
# Progetto-finale-2025-Cosestrane
Progetto finale Programmazione Procedurale UniPG Informatica

## Nome: Ali

## Cognome: Koka

## Matricola: 346288

## Commenti/modifiche al progetto:

## Introduzione
Ho realizzato questo progetto seguendo le specifiche del gioco "Cosestrane". Il programma permette di simulare l'esplorazione della città di Occhinz, gestendo due mappe speculari (Mondo Reale e Soprasotto) tramite l'uso di liste concatenate doppie. Ho cercato di curare non solo la logica dei turni, ma soprattutto la robustezza del codice e l'esperienza dell'utente durante la partita.

## Scelte Progettuali e Migliorie Apportate
Oltre a quanto richiesto dal PDF originale, ho deciso di introdurre alcune modifiche che ritengo fondamentali per un software ben fatto:

### 1. Gestione Robusta degli Input (Anti-Crash)
Una delle mie priorità è stata impedire che il programma crashasse a causa di inserimenti errati da tastiera. 
- Ho implementato un sistema di controllo su ogni `scanf`: se inserisco una lettera al posto di un numero, il programma non va in loop infinito.
- Ho creato la funzione `svuotaBuffer()` per pulire il canale di input dopo ogni errore, permettendo all'utente di riprovare immediatamente.
- Ho inserito messaggi d'errore colorati in rosso per segnalare subito quando un comando non è valido.

### 2. Uscita di Sicurezza e Controllo Partite
Ho modificato la funzione `termina_gioco()` per evitare chiusure accidentali. Se provo a uscire (opzione 3 del menu principale) senza aver giocato nemmeno una partita, il programma mi avverte che l'uscita rapida non è consentita e mi invita a provare il gioco. I crediti finali cambiano dinamicamente: se ho giocato mi ringraziano, altrimenti mi danno il benvenuto nel sistema.

### 3. Gestione del Ritmo di Gioco
Ho inserito una funzione per impostare il "tempo di pausa" (`menu_impostazione_tempo_pausa`). Questo permette di decidere quanto velocemente devono apparire i messaggi a schermo. Ho usato la funzione `sleep()` per fare in modo che l'utente abbia il tempo di leggere i risultati dei dadi o dei combattimenti prima che il terminale venga pulito.

### 4. Interfaccia Pulita e Colori
Per rendere il gioco più piacevole, ho usato la funzione `color()` per evidenziare i momenti salienti:
- **Ciano** per i titoli e i menu.
- **Verde** per i successi e i ritrovamenti.
- **Rosso** per errori e pericoli.
- **Viola** per le azioni che avvengono nel Soprasotto.
Inoltre, uso spesso `system("clear")` per evitare che il terminale si riempia di testo inutile, mantenendo il menu sempre in cima.

### 5. Gestione della Memoria e Liste
Ho dedicato molta attenzione alla pulizia della memoria. La funzione `dealloca_mappa()` si assicura di scorrere tutti i nodi delle liste (sia del Mondo Reale che del Soprasotto) e di liberarli con `free()` prima di chiudere il programma. Ho testato questa parte per assicurarmi che non ci siano Memory Leak.

### 6. Statistiche di Sessione e Memoria Storica
A differenza di una versione base, il gioco mantiene una "memoria" che persiste tra una partita e l'altra all'interno della stessa sessione. Questo permette di avere dei Crediti finali dinamici:

- **Storico degli Ultimi 3 Vincitori:** Ho implementato un sistema di gestione dei nomi (logica LIFO) che registra i vincitori delle ultime 3 partite giocate. Nel caso in cui una partita termini con la sconfitta di tutti i giocatori, il sistema registra l'evento come "Nessun Vincitore", garantendo la coerenza dei dati.
- **Contatori Globali di Sessione:** Il programma traccia statistiche cumulative che non si resettano a fine partita:
    - *Partite totali giocate:* per monitorare l'attività.
    - *Totale nemici sconfitti:* somma di tutti i combattimenti vinti da tutti i giocatori.
    - *Totale oggetti raccolti:* conteggio effettivo degli oggetti prelevati dalla mappa e messi nello zaino.
    
### 7. Sistema di "Hinting" per la Localizzazione del Demotorzone
Durante lo sviluppo, è emersa una criticità logica: poiché il sistema permette di generare mappe dinamiche, il **Demotorzone** (boss finale) potrebbe non trovarsi necessariamente nell'ultima zona del Soprasotto (con le modifiche in impostazioni mappa). Un giocatore nel Mondo Reale potrebbe quindi avanzare fino alla fine della mappa, ignorando di aver già superato il boss nell'altra dimensione.

Per risolvere questo problema e migliorare l'esperienza utente, ho implementato una meccanica di suggerimento:
- **Percezione Dimensionale:** Utilizzando l'azione "Info zona attuale" (tasto 5), se il giocatore si trova nel Mondo Reale e il Demotorzone è presente nella zona speculare del Soprasotto, il sistema genera un output specifico.
- **Messaggio Criptico:** Invece di rivelare apertamente la posizione del nemico, il programma stampa un avviso narrativo (es. *"[!!!] Un brivido ti percorre la schiena: senti una presenza taltamente pericolosa nella versione oscura di questo luogo. Forse dovresti Cambiare Mondo per investigare..."*) che suggerisce strategicamente di cambiare mondo.
- **Obiettivo:** Questa modifica previene situazioni di stallo logico durante il gioco. Senza questo accorgimento, qualora l'utente decidesse di personalizzare la mappa spostando il Demotorzone in una delle zone iniziali o intermedie (anziché lasciarlo nell'ultima cella come da impostazioni predefinite), un giocatore nel Mondo Reale rischierebbe di superare il Boss senza saperlo. Fornendo questo indizio, il completamento del gioco rimane fluido e coerente, evitando che il giocatore debba percorrere l'intera mappa a ritroso senza alcun suggerimento.
