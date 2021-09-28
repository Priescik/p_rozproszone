#include "main.h"
#include "watek_komunikacyjny.h"
#include "queue.h"


/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    packet_t* ans = malloc(sizeof(packet_t));
    int maxLamportSeen = 0;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    if (typWatku == 'C') {
        while (1) {
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);

            switch (pakiet.typ) {
                case ZLECENIE:
                    if (stan == cChceZlecenie) {
                        // sprawdz kolejke
                        ans->data = 1;  // akceptuj
                        ans->typ = ZADANIE_PRZYJETE;
                        pairId = pakiet.src;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                        answerCount = 0;
			zmienStan(cWaitStroj);

                        ans->typ = REQslipki;
                        sendPacketToAllConans(ans, MSG_TAG);
                    }
                    else {
                        // jestem zajety czyms innym                    
                        ans->data = 0;  // odrzuc
                        ans->typ = ZADANIE_ODRZUCONE;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                    }
                    break;

                case REQzlecenie:
                    //dodaj nadawce tego komunikatu do kolejki zlecen
                    break;

                case ACKzlecenie:
                    //usuń nadawce tego komunikatu z kolejki zlecen
                    break;

                case REQslipki:
                    //dodaj do lokalnej kolejki sQueue // .push_back(status.MPI_SOURCE);
                    insertToQ(WaitQueueS, newNode(pakiet.src, 1, pakiet.ts));
                    ans->typ = ACKslipki;
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case ACKslipki:
                    if (stan == cWaitStroj)
                    {
                        sTimes[pakiet.src - B] = pakiet.ts;
			answerCount++;
			    //zwiekszLamporta();
                        //TODO: aktualizuje lamporta pod identyfikatorem nadawcy tej wiadomosci
                        //sTimes[pakiet.src] = pakiet.ts;
                    }
                    else {/*ignore*/ }
                    break;

                case REQpralnia:
                    //dodaj do lokalnej kolejki pQueue // .push_back(status.MPI_SOURCE);
                    //ans->ts = lamportValue; <- robione w funckji sendPacket
                    
		            insertToQ(WaitQueueP, newNode(pakiet.src, 1, pakiet.ts));
		            ans->typ = ACKpralnia;
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case ACKpralnia:
                    if (stan == cWaitPranie || stan == cInSecPranie)
                    {
			pTimes[pakiet.src - B] = pakiet.ts;
			answerCount++;
                        //zwiekszLamporta();
                        //TODO: aktualizuje lamporta pod identyfikatorem nadawcy tej wiadomosci
                        //pTimes[pakiet.src] = pakiet.ts;
                    }
                    else {/*ignore*/ }
                    break;

                case RELEASE:
                    delFromQueue(WaitQueueS, pakiet.src);
                    delFromQueue(WaitQueueP, pakiet.src);
                    break;
            }
        }
    }
    else if (typWatku == 'B')
    {
        while(1) {
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);

            switch (pakiet.typ) {
            case ZADANIE_PRZYJETE:
                if (stan == bTworzyZlecenie) {
                    pairId = pakiet.src;
                    zmienStan(bCzeka);
                } else {
                    // ignoruje
                }
                break;

            case ZADANIE_ODRZUCONE:
                if (stan == bTworzyZlecenie) {
                    // odrzuconeNum++
                    // jesli odrzuconeNum == ilosc odbiorcow, to powroc do odpoczywania
                    zmienStan(bOdpoczywa);
                } else {
                    // ignoruje
                }
                break;

            case ZADANIE_ZAKONCZONE:
                if (stan == bCzeka) {
                    if (pakiet.src == pairId) {
                        pairId = -1;
                        zmienStan(bOdpoczywa);
                    }
                } else {
                    // ignoruje
                }
                break;
            }
        }
    }      
}
