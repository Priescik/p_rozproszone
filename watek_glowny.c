#include "main.h"
#include "watek_glowny.h"
#include "queue.h"

void bibliLoop() {
    // MPI_Status status;
    // packet_t rec;
    srand(rank);
    packet_t *pkt = malloc(sizeof(packet_t));

    while (1) {
        sleep(SEC_IN_STATE);
        if (stan == bOdpoczywa) {
            // bibliotekarz odpoczywa, losowa szansa na przejscie do kolejnego stanu
            int perc = random() % 100;
            if (perc < 75) {
                printf("Watek-[%c] id-[%d]          - bede tworzyl zlecenie\n", typWatku, rank);
                zmienStan(bTworzyZlecenie);
            }
            else {
                printf("Watek-[%c] id-[%d]          - pozostaje w stanie odpoczynku\n", typWatku, rank);
            }
        }
        else if (stan == bTworzyZlecenie) {
            // bibliotekarz tworzy zlecenie, losujac conana
            // w zależności od otrzymanych odpowiedzi albo wraca do poprzedniego stanu, albo przechodzi do nastepnego
            int con = random() % C + B;
            pkt->src = rank;
            pkt->typ = ZLECENIE;
            pkt->data = con;
            printf("Watek-[%c] id-[%d]          - wysylam zlecenie do Conana[%d]\n", typWatku, rank, con);
            sendPacket(pkt, con, MSG_TAG);
        }
        else if (stan == bCzeka) {
            // bibliotekarz czeka na zakonczenie zlecenia
            // moze przejsc do stanu Odpoczywa gdy dostanie odpowiednia wiadomosc
            printf("Watek-[%c] id-[%d]          - czekam na ukonczenie zlecenia\n", typWatku, rank);
        }
    }
}

void conanLoop()
{
    srand(rank);
    packet_t *pkt = malloc(sizeof(packet_t));

    while (1) {
        if (stan == cOdpoczywa) {
            // conan odpoczywa po zrealizowanym zadaniu, losowa szansa na przejscie do nastepnego stanu
            int perc = random() % 100;
            if (perc < 75) {
                zmienStan(cChceZlecenie);
                printf("Watek-[%c] id-[%d] lamp-{%d} - bede ubiegal sie o zlecenie\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - pozostaje w stanie odpoczynku\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cChceZlecenie) {
			// wyslij do wszystkich conanow prosbe o dodanie do kolejki zlecen
            // przejdz do nastepnego stanu jesli dostales ZLECENIE a twoj priorytet jest najwiekszy sposrod adresatow
            printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o zlecenie\n", typWatku, rank, lamportValue);
        }
        else if (stan == cWaitStroj) {
            // wejscie do nastepnego stanu jesli liczba obcych REQslipki nie przekracza liczby strojow Snum
            //!if (getQueueNum(WaitQueueS, rank) < Snum && answerCount == C && canGetToSlipkiSec()) {
            int myQueuePosition = canGetToSlipkiSec();
            if (myQueuePosition < Snum) {
                zmienStan(cInSecStroj);
                setInactive(WaitQueueS, rank);  // req pozostaje w kolejce, ale jest żądaniem "nieaktywnym"
                //!answerCount = 0;
                //!myReqTs = 0;
                printf("Watek-[%c] id-[%d] lamp-{%d} - wchodze do sekcji kryt. ze Strojami\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o Stroj\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cInSecStroj) {
            // conan ubiera sie, losowa szansa na przejscie do nastepnego stanu
            int perc = random() % 100;
            if (perc < 75) {
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
            if (perc < 75) {
                // conan ukonczyl zadanie
                // wyslij informacje o zakonczeniu do sparowanego bibliotekarza
                //!myReqTs = zwiekszLamporta();
                pkt->data = 1;
                pkt->typ = ZADANIE_ZAKONCZONE;
                pkt->ts = zwiekszLamporta();
                sendPacket(pkt, pairId, MSG_TAG);
                printf("Watek-[%c] id-[%d] lamp-{%d} - zlecenie skonczone, ide do pralni\n", typWatku, rank, lamportValue);
                pairId = -1;
                //!answerCount = 0;
		        zmienStan(cWaitPranie);
                //!pkt->data = myReqTs;
                pkt->typ = REQpralnia;
                sendPacketToAllConans(pkt, MSG_TAG);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - zlecenie w trakcie realizacji\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cWaitPranie) {
            // wyslij REQpralnia do wszystkich conanow
            // kazda przychodzaca wiadomosc REQpralnia zapisywana jest w lokalnej tablicy/kolejce pQueue
            // wejscie do nastepnego stanu jesli liczba obcych REQpralnia nie przekracza liczby strojow Pnum
            //!if (getQueueNum(WaitQueueP, rank) < Pnum && answerCount == C && canGetToPralkiSec()) {
            int myQueuePosition = canGetToPralkiSec();
            if (myQueuePosition < Pnum) {
                zmienStan(cInSecPranie);
                setInactive(WaitQueueP, rank);  // req pozostaje w kolejce, ale jest żądaniem "nieaktywnym"
                //!answerCount = 0;
                //!myReqTs = 0;
                printf("Watek-[%c] id-[%d] lamp-{%d} - wchodze do sekcji kryt. z Pralkami\n", typWatku, rank, lamportValue);
            }
            else {
                printf("Watek-[%c] id-[%d] lamp-{%d} - ubiegam sie o Pralke\n", typWatku, rank, lamportValue);
            }
        }
        else if (stan == cInSecPranie) {
            printf("Watek-[%c] id-[%d] lamp-{%d} - robie pranie\n", typWatku, rank, lamportValue);
            // conan robi pranie
            // tworzenie nowego watku, ktory odczeka sleep(x) czasu, a potem wysle do conana informacje o zakonczeniu prania
            // po stworzeniu dodatkowego watku mozna przejsc do stanu odpoczynku
            
            pkt->data = 1;  // PRZENIESC TO POTEM DO WATKU POMOCNICZEGO
            pkt->typ = RELEASE;  
            pkt->ts = zwiekszLamporta();
            sendPacketToAllConans(pkt, MSG_TAG);

            zmienStan(cOdpoczywa);
        }
        sleep(SEC_IN_STATE);
    }
}
