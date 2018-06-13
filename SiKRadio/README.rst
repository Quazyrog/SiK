SiK Radio
#########

Co jest, a czego nie ma w rozwiązaniu:

- Nadajnik:

    - Obsługa protokołu kontrolnego
    - Wysyłanie danych
    - Ale nie ma ładnej obsługi argumentów (komunikatów błędowych w przypadku, gdy są niepoprawne)
    
- Odbiornik

    - Jest obsługa protokołu kontrolnego
    - Też nie ma ładnej obsługi argumentów
    - Ani UI po Telnecie
    
Rozwiązanie wykorzystuje bibliotekę boost (program options i log), więc kompilacja trochę trwa...


Ogólny opis programów
=====================

Ogólnie to głównym elementem obu programów jest reaktor (``Utility::Reactor::Reactor``), który wykorzystując mechanizm 
`epoll` zapewnia podstawową obsługę zdarzeń, na których bazuje program. Zdarzenia są przez reaktor rozsyłane do 
zarejestrowanych odbiorców (``Utility::Reactor::EventListener``). 

Na każdym obiekcie odbiorcy uruchomiony jest jeden wątek (reaktor działa w głównym wątku), który śpi na zmiennej 
warunkowej do czasu otrzymania powiadomienia o wydarzeniu. Reaktor najpierw umieszcza w kolejce niebsłużonych zdarzeń
danego odbiorcy informacje o nim, a następnie bódzi wątak śpiący na zmiennej warunkowej. Gdy reaktor dostarczy 
powiadomienia do wszystkich zarejestrowanych odbiorców, czeka na kolejne wydarzenie.

Wątek odbiorcy w pętli śpi, jeżeli kolejka nieobsłużonych zdarzeń jest pusta. Po obudzeniu pobiera pierwsze wydarzenie
z kolejki (kolejka to FIFO) i obsługuje je (lub ignoruje — do podstawowego filtrowania zdarzeń służą ich nazwy).
Po zakończeniu obsługi sprawdza, czy kolejka jest pusta i jeżeli tak, to idzie spać na zmiennej warunkowej, 
a jeżeli nie, zabiera się za obsługę kolejnego zdarzenia.

Zasadniczo zdarzenia dzielą się na dwa rodzaje. Pierwszym z nich są zdarzenia z *zasobów* — w skrócie dotyczące
tego co dzieje się na deskryptorach:

    - Zdarzenia z najzwyklejszych deskryptorów, takich jak gniazda, ``stdin`` czy ``stdout``, z których czytamy dane.
    - Zdarzenia z timerów; generowane przez timery utworzone za pomocą ``timer_fd``
    
Kolejnym są zdarzenia powodowane przez wywołanie ``Utility::Reactor::Reactor::broadcast_event(event)``, które powoduje
rozesłanie w systemie podanego jako argument zdarzenia. To powyższe jest zaimplementowane przy użyciu łącza pipe 
z ``O_DIRECT``. Reaktor w epollu trzyma jeszcze jeden deskryptor — koniec do czytania łącza pipe. Rozgłoszenie takiego 
zdarzenia polega zatem na umieszczeniu odpowiedniego obiektu w mapie ``nazwa wydarzenia -> obiekt``, a następnie wpisaniu 
do tego łącza nazwy wydarzenia. Spowoduje to obudzenie wątku reaktora, odczytanie przez niego nazwy wydarzenia, 
wyciągnięcie odpowiedniego obiektu z mapy i następnie rozgłoszenie zdarzenia.

To łącze może służyć także do powiadomienia o czymś reaktora (obecnie tylko do powiadomienia o jego zatrzymaniu) — 
wówczas ma ono specjalną nazwę zaczynającą się od ``/Reactor/`` i nie ma odpowiadającego mu obiektu w mapie.

Jako, że obiekty zdarzeń... są obiektami, to mogą przekazywać informacje między wątkami takie, jak lista paczek, 
o których retransmisję prosimy.


Zdarzenia w odbiorniku
======================

::
    /Lookup  
        /Internal
            /LookupTime  -- zdarzenie timera, gdy nadszedł czas na wysłanie zapytania lookup [zasób]
            /StationsGCTime  -- zdarzenie timera, gdy nadszedł czas na odśmiecenie listy stacji 
                                (usunięcie nieodpowiadających) [zasób]
            /Socket  -- zdarzenie gniazda, dane do przeczytania dostępne na gnieździe protokołu kontrolnego [zasób]
        /Station
            /New -- rozgłaszane, gdy na lookup odpowiedziała stacja, której wcześniej nie było na liście
            /AddressChanged -- rozgłaszane, gdy stacja będąca już na liście zgłosiła się z innym adresem
            /TimedOut -- rozgłaszane gdy stacja jest usunięta z listy przez odśmiecanie
            /WombatHereFriend -- odpowiedź na /Player/WombatLooksForFriends
        
    /Player
        /Internal
            /DataAvailable -- zdarzenie gniazda z danymi [zasób]
            /StdoutReady -- zdarzenie standardowego wyjścia [zasób]
            /Retransmit -- zdarzenie timera odpowiadającego za retransmisje [zasób]
        /WombatLooksForFriends -- wysyłane, gdy z listy stacji zostanie usunięta właśnie odtwarzana
        /Retransmission -- wysyłane, gdy odtwarzacz chce poprosić o retransmisję danych
        
        
Zdarzenia w nadajniku
=====================

::
    /Control
        /Internal
            /Socket -- gdy na gniazdo protokołu kontrolnego przyjdą dane [zasób]
            /Retransmission -- gdy wybije timer retransmisji [zasób]
        /Control/Retransmission -- żądanie do Playera wysłania retransmisji podanych w obiekcie numerów paczek
    /Wizard
        /Internal
            /Input -- gdy możliwe jest odczytanie danych ze standardowego wejścia
            
