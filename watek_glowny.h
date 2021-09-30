#ifndef WATEK_GLOWNY_H
#define WATEK_GLOWNY_H

/* pętla główna dla procesu bibliotekarza */
void bibliLoop();
/* pętla główna dla procesu conana */
void conanLoop();

/* conan ustawia minutnik na SEC_IN_PRALKA aby wiedziec, kiedy pranie sie skonczy */
void zajmijPralke();
/* aktualizacja czasow do zakonczenia prania
   jezeli pozostaly czas ma wartosc 0, wyslij RELEASE do wszystkich conanow */
void aktualizujPralki();
/* algorytm Knutha do losowania CGS liczb z zakresu <0;C) */
void losujConanow();

#endif