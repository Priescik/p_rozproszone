#include "main.h"
#include "watek_komunikacyjny.h"
#include "queue.h"


/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    packet_t* ans = malloc(sizeof(packet_t));
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    if (typWatku == 'C') {
        while (1) {
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);
            if (C <= 5)
	    	printf("\033[0;37mTYP -[%c] id-[%d] lamp-{%d} - Odebralem wiadomosc od {%d}, czas %d, typ %d.\n", typWatku, rank, lamportValue, pakiet.src, pakiet.ts, pakiet.typ);

            int currentLamport = lamportValue;
            if (pakiet.src >= B) {
                updateTimes(pakiet.src, pakiet.ts);
                if (pakiet.ts > currentLamport) {
                    zmianaLamporta(pakiet.ts);
                }
                if (pakiet.src != rank)
                    zwiekszLamporta();
            }

            switch (pakiet.typ) {
                case ZLECENIE:
                    if (stan == cChceZlecenie && zId == -1) {
                        int highestZid = readZid(pakiet.src);
                        if (pakiet.zid <= highestZid) {
                            // widziałem już zlecenie o takim id
                            // prawdopodobnie wyslalem juz komus ACK na to zadanie
                            ans->typ = ZADANIE_ODRZUCONE;
                            ans->zid = pakiet.zid;
                            ans->bibid = pakiet.bibid;
                            sendPacket(ans, pakiet.src, MSG_TAG);
                        }
                        else {
                            // świeże zlecenie, zapisuję indeks bibliotekarza
                            // wysyłam do wszystkich innych prośbę o potwierdzenie
                            updateZIds(pakiet.src, pakiet.zid);
                            ans->typ = CHCE_ZLECENIE;
                            ans->ts = zwiekszLamporta();
                            ans->zid = pakiet.zid;
                            ans->bibid = pakiet.src;
                            pairId = pakiet.src;
                            zId = pakiet.zid;
                            zMessagesCount = 0;
                            sendPacketToOtherConans(ans, MSG_TAG);
                        }

                    }
                    else {
                        // jestem zajety czyms innym
                        updateZIds(pakiet.src, pakiet.zid);
                        ans->typ = ZADANIE_ODRZUCONE;
                        ans->zid = pakiet.zid;
                        ans->bibid = pakiet.bibid;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                    }
                    break;
                
                case CHCE_ZLECENIE:
                    if (stan == cChceZlecenie) {
                        if (pakiet.zid == zId && pakiet.bibid == pairId) {
                            //sprawdz czy masz pierwszenstwo
                            int otherIsBetter = checkActive(WaitQueueZ, pakiet.src);
                            if (otherIsBetter == 1) {
                                // rezygnuje z tego zlecenia
                                zId = -1;
                                pairId = -1;
                                zMessagesCount = 0;
                                // wysyłam potwierdzenie do konkurenta
                                updateZIds(pakiet.bibid, pakiet.zid);
                                ans->typ = MOZESZ_ZLECENIE;
                                ans->ts = zwiekszLamporta();
                                ans->zid = pakiet.zid;
                                ans->bibid = pakiet.bibid;
                                sendPacket(ans, pakiet.src, MSG_TAG);
                            }
                        }
                        else {
                            // nie ubiegam sie o to konkretne zlecenie
                            updateZIds(pakiet.bibid, pakiet.zid);
                            ans->typ = MOZESZ_ZLECENIE;
                            ans->ts = zwiekszLamporta();
                            ans->zid = pakiet.zid;
                            ans->bibid = pakiet.bibid;
                            sendPacket(ans, pakiet.src, MSG_TAG);
                        }
                    }
                    else {
                        // nie ubiegam sie o żadne zlecenie
                        updateZIds(pakiet.bibid, pakiet.zid);
                        ans->typ = MOZESZ_ZLECENIE;
                        ans->ts = zwiekszLamporta();
                        ans->zid = pakiet.zid;
                        ans->bibid = pakiet.bibid;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                    }
                    break;
                
                case MOZESZ_ZLECENIE:
                    if (pakiet.zid == zId && pakiet.bibid == pairId) {
                        zMessagesCount++;
                        if (zMessagesCount == C-1) {
                            // zebraliśmy pozwolenia od wszystkich innych conanów
                            // wyślij potwierdzenie przyjęcia do bibliotekarza
                            ans->typ = ZADANIE_PRZYJETE;
                            int currentLamport = zwiekszLamporta();
                            ans->ts = currentLamport;
                            ans->zid = zId;
                            ans->bibid = pairId;
                            sendPacket(ans, pairId, MSG_TAG);
                            // popros innych conanow o dodanie do kolejki do sekcji krytycznej strojow
                            zmienStan(cWaitStroj);
                            ans->typ = REQslipki;
                            sendPacketToAllConans(ans, MSG_TAG);
                        }
                    }
                    break;

                case REQzlecenie:
                    // dodaj do lokalnej kolejki zQueue
                    insertToQ(WaitQueueZ, newNode(pakiet.src, 1, pakiet.ts));
                    // odeslij informacje, ze dowiedziales sie o Req
		            ans->typ = ACK;
                    ans->ts = zwiekszLamporta();
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case REQslipki:
                    // usun z kolejki zQueue oraz dodaj do kolejki sQueue
                    delFromQueue(WaitQueueZ, pakiet.src);
                    insertToQ(WaitQueueS, newNode(pakiet.src, 1, pakiet.ts));
                    // odeslij informacje, ze dowiedziales sie o Req
                    ans->typ = ACK;
                    ans->ts = zwiekszLamporta();
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case REQpralnia:
                    // dodaj do lokalnej kolejki pQueue
		            insertToQ(WaitQueueP, newNode(pakiet.src, 1, pakiet.ts));
                    // odeslij informacje, ze dowiedziales sie o Req
		            ans->typ = ACK;
                    ans->ts = zwiekszLamporta();
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case ACK:
                    // zegar zostal zaktualizowany, nie trzeba robic nic wiecej
                    break;

                case RELEASE:
                    // usun z kolejek sQueue i pQueue
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

            int currentLamport = lamportValue;
            if (pakiet.ts > currentLamport) {
                zmianaLamporta(pakiet.ts);
            }

            switch (pakiet.typ) {
            case ZADANIE_PRZYJETE:
                if (stan == bTworzyZlecenie) {
                    if (pakiet.zid == zId) {
                        pairId = pakiet.src;
                        zmienStan(bCzeka);
                        zMessagesCount = 0;
                    }
                } else {
                    // w innych stanach ignoruje
                }
                break;

            case ZADANIE_ODRZUCONE:
                if (stan == bTworzyZlecenie) {
                    if (pakiet.zid == zId) {
                        zMessagesCount++;
                        if (zMessagesCount == CONAN_GROUP_SIZE) {
                            zmienStan(bOdpoczywa);
                            zId++;
                            zMessagesCount = 0;
                        }
                    } 
                } else {
                    // w innych stanach ignoruje
                }
                break;

            case ZADANIE_ZAKONCZONE:
                if (stan == bCzeka) {
                    if (pakiet.src == pairId) {
                        pairId = -1;
                        zId++;
                        zmienStan(bOdpoczywa);
                    }
                } else {
                    // w innych stanach ignoruje
                }
                break;
            }
        }
    }      
}
