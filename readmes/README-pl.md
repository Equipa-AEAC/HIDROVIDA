[🇬🇧 English](../README.md) | [🇵🇹 Português](README-pt.md) | [🇪🇸 Español](README-es.md) | [🇫🇷 Français](README-fr.md) | [🇩🇪 Deutsch](README-de.md) | [🇵🇱 Polski](README-pl.md)

---

# 🌱 HIDROVIDA
### Inteligentny System Automatyki Hydroponicznej dla Zrównoważonego Rolnictwa

<p align="center">
  <img src="https://img.shields.io/badge/Status-Rozw%C3%B3j-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Platforma-ESP32--S3-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Licencja-Edukacyjna-lightgrey?style=for-the-badge"/>
</p>

---

## 📖 Przegląd

**HIDROVIDA** to zaawansowana platforma automatyki hydroponicznej skoncentrowana na inteligentnym zarządzaniu składnikami odżywczymi, monitorowaniu środowiska i analizie rolniczej w czasie rzeczywistym.

Projekt łączy:

- Inżynierię systemów wbudowanych
- Naukową akwizycję danych z czujników
- Infrastrukturę IoT
- Zautomatyzowane dozowanie składników odżywczych
- Inteligentną kontrolę środowiska
- Rolnictwo oparte na danych

Zaprojektowany jako inicjatywa edukacyjna i technologiczna, HIDROVIDA pokazuje, jak nowoczesna automatyka i naukowe monitorowanie mogą przekształcić zrównoważoną produkcję żywności.

System został opracowany w kontekście innowacji naukowych i badań edukacyjnych, z celem stworzenia skalowalnego i inteligentnego ekosystemu hydroponicznego zdolnego do autonomicznej pracy i przyszłej integracji z AI.

---

# 🌍 Wizja

HIDROVIDA ma na celu zniwelowanie przepaści pomiędzy:

- Zrównoważonym rolnictwem
- Automatyką przemysłową
- Eksperymentami naukowymi
- Internetem Rzeczy (IoT)
- Technologiami inteligentnego rolnictwa

Długoterminową wizją jest przekształcenie platformy w w pełni adaptacyjny system zarządzania hydroponiką zdolny do predykcyjnej optymalizacji składników odżywczych, wykrywania anomalii i autonomicznego podejmowania decyzji.

---

# 🌱 Dlaczego Hydroponika?

Hydroponika to metoda uprawy bezglebowej, w której rośliny otrzymują składniki odżywcze bezpośrednio przez bogaty w minerały roztwór wodny.

W porównaniu z tradycyjnym rolnictwem, hydroponika oferuje:

- Znaczną redukcję zużycia wody
- Szybszy wzrost roślin
- Większą efektywność wykorzystania składników odżywczych
- Kontrolowane warunki środowiskowe
- Mniejszą zależność od pestycydów
- Możliwość rolnictwa miejskiego

Integrując automatykę z hydroponiką, HIDROVIDA dodatkowo poprawia:

- Stabilność stężenia składników odżywczych
- Precyzję monitorowania
- Efektywność wykorzystania zasobów
- Skalowalność operacyjną

---

# 🎯 Cele Projektu

- Opracowanie inteligentnej platformy kontroli hydroponicznej
- Monitorowanie parametrów chemicznych i środowiskowych w czasie rzeczywistym
- Automatyzacja procesów dozowania składników odżywczych
- Redukcja interwencji człowieka i błędów operacyjnych
- Stworzenie interdyscyplinarnego projektu inżynieryjno-edukacyjnego
- Demonstracja praktycznych zastosowań IoT w rolnictwie
- Zbudowanie skalowalnej architektury dla przyszłej integracji z AI

---

# 🧠 Architektura Systemu

Platforma opiera się na modułowej, rozproszonej architekturze skoncentrowanej na mikrokontrolerze **ESP32-S3**.

```text
Czujniki → Akwizycja Danych → Przetwarzanie ESP32-S3 → Silnik Decyzyjny → Panel Sterowania & Aktuatory
```

System realizuje:

- Ciągłą akwizycję danych z czujników
- Analizę parametrów w czasie rzeczywistym
- Logikę automatycznej korekty
- Komunikację z panelem sterowania w chmurze i lokalnie
- Obsługę zdarzeń i sterowanie aktuatorami

---

# ⚙️ Kluczowe Technologie

## Sprzęt Wbudowany
- ESP32-S3
- Moduł RTC
- Interfejs LCD I2C
- Moduły Przekaźników
- Pompy Perystaltyczne

## Czujniki Naukowe
- Czujnik pH
- Czujnik TDS
- Czujnik Temperatury
- Czujnik Poziomu Wody
- Czujnik Wilgotności
- Czujnik Światła
- Czujnik Mętności

## Stos Oprogramowania
- C++
- Arduino Framework
- ESP-IDF
- MQTT
- Komunikacja Wi-Fi
- Platforma IoT Blynk

---

# 🔬 Podstawa Naukowa

HIDROVIDA jest zbudowana na zasadach z:

- Chemii Analitycznej
- Żywienia Roślin
- Monitorowania Środowiska
- Automatyki Przemysłowej
- Inżynierii Elektronicznej
- Systemów Akwizycji Danych
- Inteligentnego Rolnictwa

---

# 🧪 Monitorowanie Naukowe

## Regulacja pH

Poziom pH bezpośrednio wpływa na biodostępność składników odżywczych i efektywność wchłaniania korzeniowego.

Optymalny zakres hydroponiczny:

```math
5.5 \leq pH \leq 6.5
```

Niewłaściwe wartości pH mogą powodować:

- Blokadę składników odżywczych
- Toksyczność mineralną
- Ograniczony rozwój roślin
- Stres korzeniowy

System stale monitoruje pH i w razie potrzeby przeprowadza automatyczne dozowanie korygujące.

---

## Monitorowanie TDS (Całkowita Ilość Rozpuszczonych Ciał Stałych)

Wartości TDS szacują stężenie rozpuszczonych mineralnych składników odżywczych w roztworze.

Ten parametr jest krytyczny dla:
- Stabilności składników odżywczych
- Analizy przewodności elektrycznej
- Efektywności nawożenia
- Optymalizacji wzrostu

Platforma dynamicznie ocenia wahania TDS i odpowiednio aktywuje pompy dozujące składniki odżywcze.

---

## Monitorowanie Środowiska

System monitoruje:
- Temperaturę otoczenia
- Wilgotność
- Natężenie światła
- Temperaturę wody

Te zmienne wpływają na:
- Efektywność fotosyntezy
- Szybkość parowania
- Rozwój korzeni
- Pobieranie składników odżywczych

---

# 🤖 Inteligentny System Dozowania

HIDROVIDA zawiera półautonomiczny mechanizm korekty składników odżywczych.

## Przebieg Dozowania

```text
Odczyt Czujnika
       ↓
Walidacja Parametru
       ↓
Porównanie z Progiem
       ↓
Aktywacja Pompy
       ↓
Okres Mieszania
       ↓
Nowy Cykl Walidacji
```

Ten proces pomaga zapobiegać:
- Przedawkowaniu składników odżywczych
- Niestabilności chemicznej
- Marnowaniu zasobów
- Ludzkim błędom operacyjnym

---

# 📡 Łączność i Zdalny Dostęp

Platforma obsługuje:
- Komunikację Wi-Fi
- Infrastrukturę MQTT
- Zdalne panele sterowania
- Monitorowanie w czasie rzeczywistym
- Integrację IoT

Przyszłe implementacje mogą obejmować:
- Synchronizację w chmurze
- Aplikacje mobilne
- Zdalne alerty
- Analitykę wspomaganą AI

---

# 🖥️ Funkcje Panelu Sterowania

Panel monitorowania zapewnia:

- Wizualizację czujników w czasie rzeczywistym
- Monitorowanie stanu przekaźników
- Sterowanie systemem odżywczym
- Analizę danych historycznych
- Zdalne zarządzanie aktuatorami
- Systemy alertów i powiadomień

---

# 🏗️ Architektura Oprogramowania

Oprogramowanie zostało zaprojektowane z myślą o modułowości i skalowalności.

## Struktura Projektu

```bash
/src
 ├── sensors/
 ├── actuators/
 ├── networking/
 ├── dashboard/
 ├── automation/
 ├── utilities/
 └── main.cpp
```

---

# 🔐 Bezpieczeństwo Operacyjne

System implementuje kilka mechanizmów bezpieczeństwa:

- Interwały aktywacji pomp
- Ochrona przed przedawkowaniem
- Bezpieczna inicjalizacja przekaźników
- Procedury walidacji czujników
- Systemy wykrywania błędów
- Zabezpieczenie przed niskim poziomem wody

Te funkcje poprawiają stabilność operacyjną i zmniejszają ryzyko uszkodzenia sprzętu lub braku równowagi składników odżywczych.

---

# 🌍 Wpływ na Zrównoważony Rozwój

HIDROVIDA przyczynia się do zrównoważonego rolnictwa poprzez:

- Zmniejszenie zużycia wody
- Optymalizację wykorzystania składników odżywczych
- Minimalizację odpadów
- Wspieranie lokalnej produkcji żywności
- Wspomaganie edukacyjnego rozwoju naukowego

Projekt pokazuje, jak niedrogie systemy wbudowane mogą wspierać bardziej zrównoważone modele rolnictwa.

---

# 📈 Przyszły Rozwój

Kolejne etapy ewolucji HIDROVIDA obejmują:

## Sztuczna Inteligencja
- Predykcyjne dostosowywanie składników odżywczych
- Modele optymalizacji wzrostu
- Adaptacyjne systemy dozowania

## Wizja Komputerowa
- Analiza wzrostu roślin
- Wykrywanie chorób
- Monitorowanie zabarwienia liści

## Zaawansowana Analityka Danych
- Analiza trendów historycznych
- Monitorowanie w chmurze
- Konserwacja predykcyjna

## Ekspansja Inteligentnego Rolnictwa
- Uprawa wielostrefowa
- Skalowalne wdrażanie w szklarniach
- Rozproszone sieci czujników

---

# 🎓 Wpływ Edukacyjny

HIDROVIDA została również zaprojektowana jako interdyscyplinarny projekt edukacyjny integrujący:

- Programowanie
- Robotykę
- Fizykę
- Chemię
- Biologię
- Elektronikę
- Inżynierię IoT

Projekt promuje praktyczne uczenie się nauki i rozwiązywanie rzeczywistych problemów inżynieryjnych.

---

# 📸 Plakat Naukowy

Plakat naukowy opracowany dla projektu przedstawia:
- Architekturę systemu
- Zasady naukowe
- Proces inteligentnego dozowania
- Wpływ na zrównoważony rozwój
- Przebieg operacyjny
- Mapę drogową przyszłego rozwoju

Reprezentuje koncepcyjną i naukową wizję platformy wykraczającą poza obecny stan implementacji.

---

# 🚀 Cele Repozytorium

To repozytorium ma na celu:

- Dokumentację architektury projektu
- Dostarczenie referencji oprogramowania i sprzętu
- Udostępnienie badań naukowych i technicznych
- Wsparcie przyszłych współtwórców
- Zademonstrowanie ewolucji platformy HIDROVIDA

---

# 👨‍💻 Zespół

Opracowane przez:

- Francisco Soares
- David Silva

We współpracy z:
- Agrupamento de Escolas Augusto Cabrita
- National Young Scientists Competition 2026

---

# 📷 Podgląd Projektu

> Inteligentna automatyka hydroponiczna skoncentrowana na monitorowaniu naukowym, zrównoważonym rozwoju i rolnictwie gotowym na przyszłość.

---

# 📜 Licencja

Projekt jest obecnie rozwijany dla:
- Celów edukacyjnych
- Badań naukowych
- Demonstracji technologicznej

Przyszłe warunki licencyjne mogą zostać dodane w miarę rozwoju platformy.

---

# ⭐ HIDROVIDA

### "Gdzie innowacja sprawia, że życie rośnie."

---
