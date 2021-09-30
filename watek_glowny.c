#include "main.h"
#include "watek_glowny.h"
#include "queue.h"
#include <time.h>


void zajmijPralke() {
    for (int i=0; i<Pnum; i++) {
        if (pralniaTimes[i] == -1) { 
            pralniaTimes[i] = SEC_IN_PRALKA;
            return;
        }
    }
}

void aktualizujPralki() {
    for (int i=0; i<Pnum; i++) {
        if (pralniaTimes[i] > 0) { pralniaTimes[i] -= 1; }
        else if (pralniaTimes[i] == 0) { 
            packet_t *pkt = malloc(sizeof(packet_t));
            pkt->src = rank;
            pkt->typ = RELEASE;
            pkt->ts = zwiekszLamporta();
	        printf("Watek-[%c] id-[%d] lamp-{%d} - Pranie skonczone, wysylam RELEASE\n", typWatku, rank, lamportValue);
            sendPacketToAllConans(pkt, MSG_TAG);
            free(pkt);
            pralniaTimes[i] = -1;
        }
    }
}

void losujConanow() {
    int ic, icgs;
    icgs = 0;

    for (ic = 0; ic < C && icgs < CONAN_GROUP_SIZE; ++ic) {
    int rc = C - ic;
    int rcgs = CONAN_GROUP_SIZE - icgs;
    if (rand() % rc < rcgs)    
        updateChosenConans(icgs, ic+B);  /* +B bo indeksy conanow zaczynaja sie od B */
        icgs++; 
    }

    assert(icgs == CONAN_GROUP_SIZE);
}


void bibliLoop()
{
    //srand(time(NULL) + rank);
    srand(rank);
    packet_t *pkt = malloc(sizeof(packet_t));
    int notSent = 1;
    zId = rank;

    while (1) {
        sleep(SEC_IN_STATE);
        if (stan == bOdpoczywa) {
            // bibliotekarz odpoczywa, losowa szansa na przejscie do kolejnego stanu
            int perc = random() % 100;
            if (perc < STATE_CHANGE_PROB) {
                printf("Watek-[%c] id-[%d]          - bede tworzyl zlecenie\n", typWatku, rank);
                notSent == 1;
                zmienStan(bTworzyZlecenie);
            }
            else {
                printf("Watek-[%c] id-[%d]          - pozostaje w stanie odpoczynku\n", typWatku, rank);
            }
        }
        else if (stan == bTworzyZlecenie) {
            // bibliotekarz tworzy zlecenie, losujac conanow
            // w zależności od otrzymanych odpowiedzi albo wraca do poprzedniego stanu, albo przechodzi do nastepnego
            if (notSent == 1) {
                zMessagesCount = 0;

                losujConanow();

                pkt->typ = ZLECENIE;
                pkt->zid = zId;
                pkt->bibid = rank;
                for (int i=0; i<CONAN_GROUP_SIZE; i++) {
                    int Cid = readChosenConan[i];
                    printf("Watek-[%c] id-[%d]          - wysylam zlecenia do Conana[%d]\n", typWatku, rank, Cid);
                    sendPacket(pkt, Cid, MSG_TAG);
                }
                notSent == 0;
            }
        }
        else if (stan == bCzeka) {
            // bibliotekarz czeka na zakonczenie zlecenia
            // moze przejsc do stanu Odpoczywa gdy dostanie odpowiednia wiadomosc w watku kom.
            printf("Watek-[%c] id-[%d]          - czekam na ukonczenie zlecenia\n", typWatku, rank);
        }
    }
}

void conanLoop()
{
    srand(rank);
    packet_t *pkt = malloc(sizeof(packet_t));

    while (1) {
        aktualizujPralki();

        if (stan == cOdpoczywa) {
            // conan odpoczywa po zrealizowanym zadaniu, losowa szansa na przejscie do nastepnego stanu
            int perc = random() % 100;
            if (perc < STATE_CHANGE_PROB) {
                zmienStan(cChceZlecenie);
                printf("Watek-[%c] id-[%d] lamp-{%d} - bede ubiegal sie o zlecenie\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - pozostaje w stanie odpoczynku\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cChceZlecenie) {
			// wyslij do wszystkich conanow prosbe o dodanie do kolejki zlecen
            // przejdz do nastepnego stanu jesli dostales ZLECENIE oraz ACK od pozostałych conanów
            printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o zlecenie\n", typWatku, rank, lamportValue);
        }
        else if (stan == cWaitStroj) {
            // wejscie do nastepnego stanu jesli liczba obcych REQslipki nie przekracza liczby strojow Snum
            int myQueuePosition = canGetToSlipkiSec();
            if (myQueuePosition < Snum) {
                zmienStan(cInSecStroj);
                setInactive(WaitQueueS, rank);  // req pozostaje w kolejce, ale jest żądaniem "nieaktywnym"
                printf("Watek-[%c] id-[%d] lamp-{%d} - wchodze do sekcji kryt. ze Strojami\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o Stroj\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cInSecStroj) {
            // conan ubiera sie, losowa szansa na przejscie do nastepnego stanu
            int perc = random() % 100;
            if (perc < STATE_CHANGE_PROB) {
                printf("Watek-[%c] id-[%d] lamp-{%d} - ubralem sie, ide pracowac\n", typWatku, rank, lamportValue);
                zmienStan(cPraca);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - w sekcji kryt., ubieram sie\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cPraca) {
            // conan wykonuje zlecenie, losowa szansa na przejscie do nastepnego stanu
            int perc = random() % 100;
            if (perc < STATE_CHANGE_PROB) {
                // conan ukonczyl zadanie
                // wyslij informacje o zakonczeniu zlecenia do sparowanego bibliotekarza
                pkt->typ = ZADANIE_ZAKONCZONE;
                pkt->ts = zwiekszLamporta();
                sendPacket(pkt, pairId, MSG_TAG);
                printf("Watek-[%c] id-[%d] lamp-{%d} - zlecenie skonczone, ide do pralni\n", typWatku, rank, lamportValue);
                pairId = -1;
                zId = -1;
		        // popros innych conanow o dodanie do kolejki do sekcji krytycznej pralni
                zmienStan(cWaitPranie);
                pkt->typ = REQpralnia;
                sendPacketToAllConans(pkt, MSG_TAG);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - zlecenie w trakcie realizacji\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cWaitPranie) {
            // wejscie do nastepnego stanu jesli liczba obcych REQpralnia nie przekracza liczby strojow Pnum
            int myQueuePosition = canGetToPralkiSec();
            if (myQueuePosition < Pnum) {
                zmienStan(cInSecPranie);
                setInactive(WaitQueueP, rank);  // req pozostaje w kolejce, ale jest żądaniem "nieaktywnym"
                printf("Watek-[%c] id-[%d] lamp-{%d} - wchodze do sekcji kryt. z Pralkami\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o Pralke\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cInSecPranie) {
            printf("Watek-[%c] id-[%d] lamp-{%d} - robie pranie\n", typWatku, rank, lamportValue);
            // conan robi pranie
            // po aktualizacji tablicy pralniaTimes przechodzi do stanu odpoczynku
            zajmijPralke();
            zmienStan(cOdpoczywa);
        }
        sleep(SEC_IN_STATE);
    }
}
