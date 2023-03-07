# msp430_game

<h2>1. Opis gry</h2>
<ul>
<li>Gracz kieruje postacią. 
<li>Postać może poruszać się po dwóch ścieżkach.
<li>Na ścieżkach pojawiają się przeszkody do ominięcia i punkty do zebrania.
<li>Z czasem prędkość zbliżających obiektów zwiększa się.
<li>Gra kończy się, gdy postać uderzy w przeszkodę. 
<li>Najlepsze wyniki zapisywane są dla każdej postaci.
</ul>

<h2>2. Silnik gry</h2>

<h3>Zmienne globalne</h3>

Bufor na klatki do wyświetlenia, przechowuje konkretne znaki, które mają być wyświetlone na konkretnej pozycji na ekranie:

- **char bufor\_0[16]** - górna linia
- **char bufor\_1[16]** - dolna linia

Licznik setnych sekundy, inkrementowany w przerwaniu timera co 10ms:

- **unsigned int licznik** 

Definicje preprocesora dla poszczególnych klawiszy:

- **#define B1 BIT4&P4IN** // Klawisz B1 - P4.4
- **#define B2 BIT5&P4IN** // Klawisz B2 - P4.5
- **#define B3 BIT6&P4IN** // Klawisz B3 - P4.6
- **#define B4 BIT7&P4IN** // Klawisz B4 - P4.7

<h3>Inicjalizacja urządzenia</h3>

Funkcja: **void inicjalizacja()**

- zatrzymuje działanie watchdoga
- inicjalizuje wyświetlacz LCD:
  - **void InitPortsLcd()**
  - **void InitLCD()**
  - **void clearDisplay()**
- dodaje własne znaki:
  - **void dodanieZnakow()**
- ustawienie zegara:
  - ACLK na 4 MHz (źródło LFXT1)
  - MCLK (źródło LFXT1)
- ustawienie Timer\_A:
  - ACLK
  - tryb UP
  - 500 kHz
  - przerwanie co 10ms

Funkcja: **void InitPortsLcd()**

- ustawienie wszystkich pinów portu 2 jako wejścia/wyjścia cyfrowe oraz zmiana ich stanu na 0

Funkcja: **void InitLCD()**

- Wykonywana jest seria opóźnień o długości 100ms.
- Ustawiany jest stan wysoki na wybranych pinach portu 2.
- Wykonywana jest sekwencja operacji zmieniających stan pinu E.
- Ustawiany jest stan niski na wybranym pinie portu 2.
- Wykonywana jest sekwencja operacji zmieniających stan pinu E.
- Wysyłane są polecenia do wyświetlacza, umożliwiające włączenie go oraz wyczyszczenie ekranu.
- Wykonywana jest seria opóźnień o długości 100ms.

Funkcja: **void clearDisplay()**

- wyczyszczenie ekranu

Funkcja: **void dodanieZnakow()**

- dodanie własnych znaków:
  - przeszoda (adres 0):

**int przeszkoda[8]={31, 31, 31, 31, 31, 31, 31, 31};**

- postać 1 (adres 1):

**int postac1[8]={31, 21, 27, 17, 31, 14, 10, 27};**

- postać 2 (adres 2):

**int postac2[8]={14, 14, 4, 31, 4, 4, 10, 17};**

- postać 3 (adres 3):

**int postac3[8]={31, 17, 17, 17, 31, 31, 27, 27};**

- postać 4 (adres 4):

**int postac4[8]={14, 21, 27, 14, 14, 0, 14, 27};**



<h3>Obsługa ekranu</h3>

Funkcja: **void wyswietlKlatke()**

- Wyświetla pojedynczą klatkę gry, na podstawie bufora. Wysyła odpowiednie znaki na pierwszą i drugą linie ekranu.

<h2>3. Warstwa logiczna</h2>

<h3>Zmienne globalne</h3>

Zmienna informująca o stanie rozgrywki:

- **int czyTrwaRozgrywka** - przyjmuje wartość 0 lub 1

Zmienna przechowująca numer wybranej postaci:

- **int wybranaPostac** - przyjmuje wartości od 1 do 4

Zmienna przechowująca wynik aktualnej rozgrywki:

- **int aktualnyWynik** - przyjmuje wartości od 0 do 99

Zmienna przechowująca aktualną pozycję postaci:

- **int aktualnaPozycjaPostaci** - przyjmuje wartość 0 lub 1

Zmienne przechowujące najlepsze wyniki danych postaci:

- **int najlepszyWynikPostac1** - przyjmuje wartości od 0 do 99
- **int najlepszyWynikPostac2** - przyjmuje wartości od 0 do 99
- **int najlepszyWynikPostac3** - przyjmuje wartości od 0 do 99
- **int najlepszyWynikPostac4** - przyjmuje wartości od 0 do 99

Zmienna przechowująca co jaki okres mają być przesuwane elementy na trasie

- **unsigned int okresPrzesuwania** - przyjmowana wartość jest wielokrotnością 10ms, np dla 2 sekund będzie to 200 (200 x 10ms = 2000ms = 2s)

Bufor na wygenerowaną trasę (3 bloki po 16) 

- **char trasa\_0[48]** - górna linia
- **char trasa\_1[48]** - dolna linia

Bloki tras definiowane są w następujący sposób:

**0** - przeszkoda, **32** - puste, **36** - znak punktu ($) np:

- **const char blokX\_0[16]** = {32, 32,  0, 32, 36, 32,  0, 32, 32, 32, 32, 32,  0, 32, 32, 32};
- **const char blokX\_1[16]** = {32, 32, 32, 32, 32, 32, 32, 32, 32,  0, 32, 32, 32, 32, 36, 32};

   **X** - numer bloku

Wskaźnik na bufor trasy, wskazuje od którego miejsca należy wyświetlić trasę na ekranie:

- **int wskaznikTrasy** - przyjmuje wartości od 0 do 47

<h3>Generowanie interfejsu</h3>

Każdemu ekranowi odpowiada funkcja, która na podstawie konkretnych zmiennych wysyła do bufora gotową klatkę do wyświetlenia.

Ekrany statyczne (wystarczy raz wyświetlić):

Funkcja: **void** **ekranWyboruPostaci()**

- przygotowuję klatkę do wyświetlenia w buforze
- wyświetla klatkę: **wyswietlKlatke()**
- obsługuje wybór postaci za pomocą przycisków (wybór postaci powoduje zakończenie funkcji)

Funkcja: **void ekranRozpoczeciaRozgrywki()**

- przygotowuję klatkę do wyświetlenia w buforze
- wyświetla klatkę: **wyswietlKlatke()**
- obsługuje przycisk (naciśnięcie przycisku nr 1 powoduje zakończenie funkcji)

Funkcja: **void ekranPodsumowania()**

- przygotowuję klatkę do wyświetlenia w buforze
- wyświetla klatkę: **wyswietlKlatke()**
- obsługuje przycisk (naciśnięcie przycisku nr 1 powoduje zakończenie funkcji)

Funkcja: **int ekranTablicyWynikow()**

- przygotowuję klatkę do wyświetlenia w buforze
- wyświetla klatkę: **wyswietlKlatke()**
- obsługuje przyciski 
  - przycisk 1 - funkcja zwraca **1** (graj dalej)
  - przycisk 2 - funkcja zwraca **0** (powrót do menu)

Ekran dynamiczny (odświeżanie poprzez pętlę rozgrywki):

Funkcja: **void ekranRozgrywki()**

- generuje startową trasę (3 bloki) 
  - 3x **generowanieTrasy(wskaznik)**
- ustawia zmienne
  - **wskaznikTrasy** **= 0**
  - **aktulanyWynik** **= 0**
  - **okresPrzesuwania = 200**
  - **czyTrwaRozgrywka** **= 1**
- pętla rozgrywki (dopóki **czyTrwaRozgrywka** **= 1**):
  - sterowanie postacią: **aktualnaPozycjaPostaci ^= 1**
  - jeżeli minęła odpowiednia ilość czasu:             (**licznik >= okresPrzesuwania**)
    - przesuwa trasę: **przesuniecieTrasy()**
    - zeruje licznik: **licznik = 0**
  - sprawdza kolizje: **sprawdzenieKolizji()**
    - jeżeli zdobyto punkt:             
      - **aktualizacjaPrzyspieszenia()**
  - oblicza klatkę: **przygotowanieKlatkiRozgrywki()**
  - wyświetla klatkę: **wyswietlKlatke()**

Funkcja: **void przygotowanieKlatkiRozgrywki()**

- przygotowuje pustą klatkę w buforze
- dodanie aktualnego wyniku 
- dodanie postaci na odpowiedniej pozycji 
- dodanie przeszkód i punktów na trasie 

<h3>Logika rozgrywki</h3>

Generowanie toru przeszkód i punktów:

Zdefiniowane zostały całe bloki tras, które mogą się łączyć dowolnie każda z każdą. Proces generowania odbywa się poprzez losowanie numeru bloku, a następnie dodanie go do trasy.

Funkcja: **void generowanieTrasy(int indeks)**

- jako argument przyjmuje wskaznik do miejsca na trasie, w którym ma być wstawiony blok
- dodaje losowy blok do trasy 

Mechanika przesuwania trasy:

Funkcja: **void przesuniecieTrasy()**

- przesuwa wskaźnik trasy o 1
- jeżeli jest wolne miejsce (wskaźnik ma wartość: (16, 32, 48)):
  - **generowanieTrasy(wskaznikTrasy-16)**
- jeżeli wskaźnik ma wartość 48:
  - to nastepuje cofnięcie wskaźnika na początek: **wskaznikTrasy = 0**

Mechanika przyspieszenia elementów:

Funkcja: **void aktualizacjaPrzyspieszenia()**

- zmniejszenie interwału przesuwania obiektów o 50ms: 

Mechanika kolizji:

Sprawdzenie czy na w miejscu postaci jest przeszkoda albo punkt, następnie dodanie punktu lub koniec rozgrywki.

Funkcja: **int sprawdzenieKolizji()**

- jeżeli nie ma kolizji
  - zwrócenie **0**
- jeżeli nastąpiło zderzenie z przeszkodą
  - zakończenie rozgrywki
  - aktualizacja najlepszego wyniku
  - zwrócenie **1**
- jeżeli zebrano punkt
  - dodanie punktu
  - usunięcie elementu z trasy
  - zwrócenie **2**

<h3>Funkcje użytkowe</h3>

Funkcja: **char getDziesiatki(int liczba)**

- zwraca liczbę dziesiątek podanej liczby jako znak

Funkcja: **char getJednosci(int liczba)**

- zwraca liczbę jedności podanej liczby jako znak
