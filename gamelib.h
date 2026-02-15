void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

typedef enum Tipo_zona { bosco, scuola, laboratorio, caverna, strada, giardino, supermercato, centrale_elettrica, deposito_abbandonato, stazione_polizia } Tipo_zona;
typedef enum Tipo_nemico { nessun_nemico, billi, democane, demotorzone } Tipo_nemico;
typedef enum Tipo_oggetto { nessun_oggetto, bicicletta, maglietta_fuocoinferno, bussola, schitarrata_metallica } Tipo_oggetto;

struct Zona_mondoreale;
struct Zona_soprasotto;

typedef struct Giocatore {
    char nome[100];
    int mondo; // 0 per Mondo Reale, 1 per Soprasotto
    struct Zona_mondoreale* pos_mondoreale;
    struct Zona_soprasotto* pos_soprasotto;
    unsigned char attacco_psichico;
    unsigned char difesa_psichica;
    unsigned char fortuna;
    Tipo_oggetto zaino[3];
} Giocatore;

typedef struct Zona_mondoreale {
    Tipo_zona zona;
    Tipo_nemico nemico;
    Tipo_oggetto oggetto;
    struct Zona_mondoreale* avanti;
    struct Zona_mondoreale* indietro;
    struct Zona_soprasotto* link_soprasotto;
} Zona_mondoreale;

typedef struct Zona_soprasotto {
    Tipo_zona zona;
    Tipo_nemico nemico;
    struct Zona_soprasotto* avanti;
    struct Zona_soprasotto* indietro;
    struct Zona_mondoreale* link_mondoreale;
} Zona_soprasotto;
