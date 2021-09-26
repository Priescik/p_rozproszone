#include "main.h"
#include "watek_glowny.h"

void bibliLoop() {
    MPI_Status status;
    packet_t rec;

    while (stan != InFinish) {
        sleep(SEC_IN_STATE);
        if (stan == KATALOGOWANIE) {
            int perc = random() % 100;
            if (perc < 80)
                zmienStan(CHCE_ZLECENIE);

        }
        if (stan == TWORZENIE_ZLECENIA) {
            // losowe wybranie konana
            packet_t *pkt = malloc(sizeof(packet_t));
            pkt->src = rank;
            pkt->typ = ZLECENIE;
            int con = random() % C + B;
            printf("[%c] id-[%d] - Wysyłam zlecenie do Conana[%d]\n", watek_type, rank, con);
            sendPacket(pkt, con, MPI_MSG);
            printf("[%c] id-[%d] - Czekam na przyjęcie\n", watek_type, rank, con);
            MPI_Recv(rec, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);
            if (rec.data == 1)
                zmienStan(OCZEKIWANIE_NA_WYKONANIE)
        }
        if (stan == OCZEKIWANIE_NA_WYKONANIE) {
            printf("[%c] id-[%d] - Czekam na ukończenie zlecenia\n", watek_type, rank, con);
            MPI_Recv(rec, 1, MPI_PACKET_T, rec.src, MSG_TAG, MPI_COMM_WORLD, &status);
        }
    }
}

void conanLoop()
{
    srandom(rank);
    while (stan != InFinish) {
        if (stan == OCZEKUJE) {
            int perc = random() % 100;
            if (perc < 80)
                zmienStan(CHCE_ZLECENIE);

        }
        if (stan == CHCE_ZLECENIE) {
			//zmianaLamporta(-1);
            printf("[%c] id-[%d] lamp-{%d} - Oczekuje na zlecenia\n", watek_type, rank, lamportValue);
			//for (int i = 0; i < size; i++)
			//	zAckTab[i] = 0;
			////TODO: nie wysyłać reqZ jesli wyslalem ackz na to BIBid ?
			//for (int i = B; i < size; i++) {
			//	if (i != rank) {
			//		MPI_Send(BIBid, 1, MPI_INT, i, REQzlecenie, MPI_COMM_WORLD);
			//		sendPacket();
			//	}
			//}
        }
        if (stan == CHCE_STROJ) {

            printf("[%c] id-[%d] lamp-{%d} - Oczekuje na strój\n", watek_type, rank, lamportValue);
            zmienStan(BIERZE_STROJ);

        }
        if (stan == BIERZE_STROJ) {

            printf("[%c] id-[%d] lamp-{%d} - bierze strój\n", watek_type, rank, lamportValue);
            int perc = random() % 100;
            if (perc < 80)
                zmienStan(PRACA);

        }
        if (stan == PRACA) {

            printf("[%c] id-[%d] lamp-{%d} - Wykonuję zlecenie\n", watek_type, rank, lamportValue);
            int perc = random() % 100;
            if (perc < 80) 
                zmienStan(CHCE_PRANIE);
            

        }
        if (stan == CHCE_PRANIE) {

            printf("[%c] id-[%d] lamp-{%d} - oczekuje na pranie\n", watek_type, rank, lamportValue);
            int perc = random() % 100;
            if (perc < 80)
                zmienStan(ROBI_PRANIE);

        }
        if (stan == ROBI_PRANIE) {

            printf("[%c] id-[%d] lamp-{%d} - Robi pranie\n", watek_type, rank, lamportValue);
            int perc = random() % 100;
            if (perc < 80)
                zmienStan(ODPOCZYNEK);

        }
        sleep(SEC_IN_STATE);
    }
}
