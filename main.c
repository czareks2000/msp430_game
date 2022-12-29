#include <msp430x14x.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"
#include "portyLcd.h"

//---------Stałe---------//
#define B1 BIT4&P4IN // Klawisz B1 - P4.4
#define B2 BIT5&P4IN // Klawisz B2 - P4.5
#define B3 BIT6&P4IN // Klawisz B3 - P4.6
#define B4 BIT7&P4IN // Klawisz B4 - P4.7

// definicje blokow trasy
const int liczbaBlokow = 3;

// 0 - przeszkoda, 32 - puste, 36 - $
const char blok1_0[16] = {32, 32,  0, 32, 36, 32,  0, 32, 32, 32, 32, 32,  0, 32, 32, 32};
const char blok1_1[16] = {32, 32, 32, 32, 32, 32, 32, 32, 32,  0, 32, 32, 32, 32, 36, 32};

const char blok2_0[16] = {32, 32, 32, 32, 32, 32, 32, 32, 32,  0, 32, 32, 32, 32, 36, 32};
const char blok2_1[16] = {32, 32,  0, 32, 36, 32,  0, 32, 32, 32, 32, 32,  0, 32, 32, 32};

const char blok3_0[16] = {32, 32, 32, 32, 32,  0, 32, 32, 32, 32, 32,  0, 32, 32, 32, 32};
const char blok3_1[16] = {32, 32,  0, 36, 32, 32, 32, 32,  0, 36, 32, 32, 32, 36,  0, 32};

//---------Zmienne globalne (warstwa logiki)---------//

int czyTrwaRozgrywka = 0; // 0 lub 1
int wybranaPostac = 1; // 1, 2, 3 lub 4
int aktualnyWynik = 0; // max 99
int aktualnaPozycjaPostaci = 0; // 0 lub 1

int najlepszyWynikPostac1 = 0; // max 99
int najlepszyWynikPostac2 = 0; // max 99
int najlepszyWynikPostac3 = 0; // max 99
int najlepszyWynikPostac4 = 0; // max 99

//---------Zmienne globalne (warstwa silnika)---------//

// bufor na wygenerowaną trasę (3 bloki po 16)
char trasa_0[48];
char trasa_1[48];

int wskaznikTrasy = 0; // od 0 do 47

// bufor na klatki do wyswietlenia
char bufor_0[16] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
char bufor_1[16] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};

// zmienne zwiazane z czasem
unsigned int licznik = 0; // licznik setnych sekundy
unsigned int czestotliwoscPrzesuwania = 200; // 200*10ms = 2 sekundy

//---------Deklaracje funkcji---------//

void inicjalizacja();

void dodanieZnakow();

void ekranWyboruPostaci();

void ekranRozpoczeciaRozgrywki();

void ekranRozgrywki();

void przygotowanieKlatkiRozgrywki();

void aktualizacjaPrzyspieszenia();

void przesuniecieTrasy();

int sprawdzenieKolizji();

void generowanieTrasy();

void ekranPodsumowania();

int ekranTablicyWynikow();

void wyswietlKlatke();

char getDziesiatki(int liczba);

char getJednosci(int liczba);

//---------Program główny---------//

void main( void )
{
  inicjalizacja();

  while(1)
  {
    // menu wyboru postaci
    ekranWyboruPostaci();

    // rozgrywka wybraną postacią, dopóki gracz nie wróci do menu
    do
    {
      ekranRozpoczeciaRozgrywki();
      ekranRozgrywki();
      ekranPodsumowania();
    }
    while(ekranTablicyWynikow()); // 0 - powrót do menu
  }
}

//---------Definicje fukncji---------//

void inicjalizacja()
{
  WDTCTL = WDTPW + WDTHOLD; // zatrzymanie WDT

  // LCD
  InitPortsLcd(); // inicjalizacja portów LCD
  InitLCD(); // inicjalizacja LCD
  clearDisplay(); // wyczyszczenie ekranu
  dodanieZnakow(); // dodanie własnych znaków do pamięci LCD

  // Basic Clock Module ustawiamy na ACLK(zegar 8 MHz ) i dzielimy częstotliwość przez 2 (4 MHz)
  BCSCTL1 |= XTS;  // ACLK = LFXT1 = HF XTAL 8MHz

  BCSCTL1 |= DIVA_1; // ACLK=8 MHz/2=4 MHz
  BCSCTL2 |= SELM0 | SELM1; // zmieniamy Master Clock(MCLK) na zegar LFXT1CLK

  // Timer_A  ustawiamy na 500 kHz
  TACTL = TASSEL_1 + MC_1 + ID_3; // Wybieramy ACLK + tryb Up + ACLK/8=500kHz 
  CCTL0 = CCIE; // włączenie przerwań od CCR0
  CCR0 = 5000; // (1/500k)*5000=0.01, przerwanie co 10 ms

  _EINT(); // włączenie obłsługi przerwań
}

void dodanieZnakow()
{
  // zdefiniowanie znakow
  int przeszkoda[8]={31, 31, 31, 31, 31, 31, 31, 31};
  int postac1[8]={31, 21, 27, 17, 31, 14, 10, 27};
  int postac2[8]={14, 14, 4, 31, 4, 4, 10, 17};
  int postac3[8]={31, 17, 17, 17, 31, 31, 27, 27};
  int postac4[8]={14, 21, 27, 14, 14, 0, 14, 27};

  // zapisanie znakow w pamieci CG_RAM
  SEND_CMD(CG_RAM_ADDR);

  // adres znaku: 0
  for(int i=0; i<8; i++)
    SEND_CHAR(przeszkoda[i]); 

  // adres znaku: 1
  for(int i=0; i<8; i++)
    SEND_CHAR(postac1[i]);

  // adres znaku: 2
  for(int i=0; i<8; i++)
    SEND_CHAR(postac2[i]);

  // adres znaku: 3
  for(int i=0; i<8; i++)
    SEND_CHAR(postac3[i]);

  // adres znaku: 4
  for(int i=0; i<8; i++)
    SEND_CHAR(postac4[i]);
}

// obsługa ekranu wyboru postaci
void ekranWyboruPostaci()
{
  // przygotowanie klatki do wyswietlenia
  strncpy(bufor_0, " WYBIERZ POSTAC ", 16);
  strncpy(bufor_1, " 1_  2_  3_  4_ ", 16);
  bufor_1[2] = 1;
  bufor_1[6] = 2;
  bufor_1[10] = 3;
  bufor_1[14] = 4;

  // wyświetlenie klatki
  wyswietlKlatke();

  // obsługa wyboru za pomocą przycisków
  while(1)
  {
    if (B1 == 0) {
        wybranaPostac = 1;
        return;
    }
    else if (B2 == 0) {
        wybranaPostac = 2;
        return;
    }
    else if (B3 == 0) {
        wybranaPostac = 3;
        return;
    }
    else if (B4 == 0) {
        wybranaPostac = 4;
        return;
    }
  }
}

// obsługa ekranu rozpoczecia rozgrywki
void ekranRozpoczeciaRozgrywki()
{
  // przygotowanie klatki do wyswietlenia
  strncpy(bufor_0, "    POSTAC:_    ", 16);
  strncpy(bufor_1, "  1:ROZPOCZNIJ  ", 16);
  bufor_0[11] = wybranaPostac;
  
  // wyświetlenie klatki
  wyswietlKlatke();

  // obsługa przycisku
  while(1)
  {
    if (B1 == 0)  
      return; 
  } 
}

// obsługa ekranu rozgrywki
void ekranRozgrywki()
{
  // wygenerowanie startowej trasy (3 bloki)
  generowanieTrasy(0);
  generowanieTrasy(16);
  generowanieTrasy(32);
  wskaznikTrasy = 0;
  czestotliwoscPrzesuwania = 200; // 2 sekundy

  // rozpoczecie rozgrywki
  czyTrwaRozgrywka = 1;

  // petla dopoki trwa gra
  while(czyTrwaRozgrywka)
  { 
    // obsluga sterowania postacia
    if (B1 == 0)
      aktualnaPozycjaPostaci ^= 1;

    // jezeli minela odpowiednia ilosc czasu
    if (licznik >= czestotliwoscPrzesuwania)
    {
      // przesuniecie wyswietlanej trasy
      przesuniecieTrasy();

      // zerowanie licznika setnych sekundy
      licznik = 0; 
    } 

    // sprawdzenie kolizji
    if (sprawdzenieKolizji() == 2)// jeżeli zdobyto punkt
      aktualizacjaPrzyspieszenia();

    // obliczenie klatki
    przygotowanieKlatkiRozgrywki();

    // wyswietlenie klatki
    wyswietlKlatke();
  }
}

// przygotowanie klatki rozgrywki 
void przygotowanieKlatkiRozgrywki()
{
  // przygotowanie pustej klatki  
  strncpy(bufor_0, "                ", 16);
  strncpy(bufor_1, "                ", 16);

  // aktualny wynik
  bufor_0[0] = getDziesiatki(aktualnyWynik);
  bufor_0[1] = getJednosci(aktualnyWynik);

  // postac
  if (aktualnaPozycjaPostaci == 0)
    bufor_0[2] = wybranaPostac;
  else if (aktualnaPozycjaPostaci == 1)
    bufor_1[2] = wybranaPostac;

  // jezeli przeszkoda lub punkt na pozycji postaci
  if (trasa_0[wskaznikTrasy] != ' ')
    bufor_0[2] = trasa_0[wskaznikTrasy];
  if (trasa_1[wskaznikTrasy] != ' ')
    bufor_1[2] = trasa_1[wskaznikTrasy];

  // przeszkody i punkty na reszcie trasy
  for (int i=1; i<14; i++)
  {
      bufor_0[i+2] = trasa_0[(wskaznikTrasy+i)%48];
      bufor_1[i+2] = trasa_1[(wskaznikTrasy+i)%48];       
  }
}

// procedura obsługi przerwania od TimerA
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
  if (czyTrwaRozgrywka)
  {
    licznik++; //zwiększenie licznika setnych sekundy
  }
}

// fukncja aktualizująca przyspieszenie
void aktualizacjaPrzyspieszenia()
{
  //przyspieszenie o 50ms;
  czestotliwoscPrzesuwania -= 5;
}

// przesuwa wskaznik trasy i generuje nową trase jezeli jest wolne miejse
void przesuniecieTrasy()
{
  // przesuwaniecie wskaźnika trasy
  wskaznikTrasy++;

  // generowanie trasy jezeli jest miejsce (wskaźnik ma wartość: (16, 32, 48))
  if (wskaznikTrasy == 16 || wskaznikTrasy == 32 || wskaznikTrasy == 48)
    generowanieTrasy(wskaznikTrasy-16);
  
  // cofniecie wskaznika na poczatek
  if (wskaznikTrasy == 48)
    wskaznikTrasy = 0;
}

// obsluga kolizji
// zwraca 0 jezeli nie ma kolizji
// zwraca 1 jezeli nastapilo zderzenie z przeszkoda
// zwraca 2 jezeli zebrano punkt
int sprawdzenieKolizji()
{
  // jeżeli postać w górnej linii
  if (aktualnaPozycjaPostaci == 0)
  {
    // jeżeli na trasie nic nie ma
    if (trasa_0[wskaznikTrasy] == 32)
      return 0;

    // jeżeli na trasie jest przeszkoda
    if (trasa_0[wskaznikTrasy] == 0)
    {
      // zakonczenie rozgrywki
      czyTrwaRozgrywka = 0;
      return 1;
    }

    // jeżeli na trasie jest punkt
    if(trasa_0[wskaznikTrasy] == 36)
    {
      // dodanie punktu
      aktualnyWynik++;
      // usunięcie elementu z trasy
      trasa_0[wskaznikTrasy] = 32;
      return 2;
    }
  }

  // jeżeli postać w dolnej linii
  if (aktualnaPozycjaPostaci == 1)
  {
    // jeżeli na trasie nic nie ma
    if (trasa_1[wskaznikTrasy] == 32)
      return 0;

    // jeżeli na trasie jest przeszkoda
    if (trasa_1[wskaznikTrasy] == 0)
    {
      // zakonczenie rozgrywki
      czyTrwaRozgrywka = 0;
      return 1;
    } 

    // jeżeli na trasie jest punkt
    if(trasa_1[wskaznikTrasy] == 36)
    {
      // dodanie punktu
      aktualnyWynik++;
      // usunięcie elementu z trasy
      trasa_1[wskaznikTrasy] = 32;
      return 2;
    }
  }
  
  return -1;
}

// dodaje losowy blok do trasy
// jako argument przyjmuje wskaznik do miejsca na trasie, w którym ma być wstawiony blok
void generowanieTrasy(int indeks)
{
  // Wygenerowanie liczby z zakresu od 0 do liczbaBlokow - 1
  int r = rand() % liczbaBlokow;

  // Wybranie bloku na postawie wygenerowanej liczby
  switch (r) {
    case 0:
      // Dodanie bloku1 do trasy 
      memcpy(&trasa_0[indeks], blok1_0, sizeof(blok1_0));
      memcpy(&trasa_1[indeks], blok1_1, sizeof(blok1_1));
      break;
    case 1:
      // Dodanie bloku2 do trasy 
      memcpy(&trasa_0[indeks], blok2_0, sizeof(blok2_0));
      memcpy(&trasa_1[indeks], blok2_1, sizeof(blok2_1));
      break;
    case 2:
      // Dodanie bloku3 do trasy 
      memcpy(&trasa_0[indeks], blok3_0, sizeof(blok3_0));
      memcpy(&trasa_1[indeks], blok3_1, sizeof(blok3_1));
      break;
  }
}

// obsługa ekranu podsumowania
void ekranPodsumowania()
{
  // przygotowanie klatki do wyswietlenia
  strncpy(bufor_0, "    WYNIK:__    ", 16);
  strncpy(bufor_1, "     1DALEJ     ", 16);

  bufor_0[10] = getDziesiatki(aktualnyWynik);
  bufor_0[11] = getJednosci(aktualnyWynik);
  
  // wyświetlenie klatki
  wyswietlKlatke();

  // obsługa przycisku
  while(1)
  {
    if (B1 == 0)  
      return; 
  } 
}

// obsługa ekranu podsumowania
// zwraca 1 lub 0 : (graj dalej / powrót do menu)
int ekranTablicyWynikow()
{
  // przygotowanie klatki do wyswietlenia
  strncpy(bufor_0, "___ ___ ___ ___ ", 16);
  strncpy(bufor_1, "1GRAJ DAL. 2MENU", 16);

  bufor_0[0] = 1;
  bufor_0[1] = getDziesiatki(najlepszyWynikPostac1);
  bufor_0[2] = getJednosci(najlepszyWynikPostac1);

  bufor_0[4] = 2;
  bufor_0[5] = getDziesiatki(najlepszyWynikPostac2);
  bufor_0[6] = getJednosci(najlepszyWynikPostac2);

  bufor_0[8] = 3;
  bufor_0[9] = getDziesiatki(najlepszyWynikPostac3);
  bufor_0[10] = getJednosci(najlepszyWynikPostac3);

  bufor_0[12] = 4;
  bufor_0[13] = getDziesiatki(najlepszyWynikPostac4);
  bufor_0[14] = getJednosci(najlepszyWynikPostac4);
  
  // wyświetlenie klatki
  wyswietlKlatke();

  // obsługa przycisków
  while(1)
  {
    if (B1 == 0) {
        return 1;
    }
    else if (B2 == 0) {
        return 0;
    }
  }
}

// wyswietla na ekranie aktualną klatkę z bufora
void wyswietlKlatke()
{
  // górna linia
  SEND_CMD(DD_RAM_ADDR);
  for(int i=0; i<16; i++)
    SEND_CHAR(bufor_0[i]);

  // dolna linia
  SEND_CMD(DD_RAM_ADDR2);
  for(int i=0; i<16; i++)
    SEND_CHAR(bufor_1[i]);
}

//------Funkcje użytkowe------//

// zwraca liczbe dziesiatek danej liczby
char getDziesiatki(int liczba) {
  if (liczba < 10) {
    return '0';
  } else {
    return (liczba / 10) + '0';
  }
}

// zwraca liczbe jednosci danej liczby
char getJednosci(int liczba) {
  return (liczba % 10) + '0';
}