[🇬🇧 English](../README.md) | [🇵🇹 Português](README-pt.md) | [🇪🇸 Español](README-es.md) | [🇫🇷 Français](README-fr.md) | [🇩🇪 Deutsch](README-de.md) | [🇵🇱 Polski](README-pl.md)

---

# 🌱 HIDROVIDA
### Intelligentes Hydrokultur-Automationssystem für Nachhaltige Landwirtschaft

<p align="center">
  <img src="https://img.shields.io/badge/Status-Entwicklung-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Plattform-ESP32--S3-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Lizenz-Bildungszwecke-lightgrey?style=for-the-badge"/>
</p>

---

## 📖 Überblick

**HIDROVIDA** ist eine fortschrittliche Hydrokultur-Automatisierungsplattform, die sich auf intelligentes Nährstoffmanagement, Umweltüberwachung und landwirtschaftliche Echtzeitanalyse konzentriert.

Das Projekt kombiniert:

- Embedded Systems Engineering
- Wissenschaftliche Sensorerfassung
- IoT-Infrastruktur
- Automatisierte Nährstoffdosierung
- Intelligente Umweltkontrolle
- Datengetriebene Landwirtschaft

Konzipiert als Bildungs- und Technologieinitiative zeigt HIDROVIDA, wie moderne Automatisierung und wissenschaftliche Überwachung die nachhaltige Lebensmittelproduktion verändern können.

Das System wurde im Kontext wissenschaftlicher Innovation und Bildungsforschung entwickelt, mit dem Ziel, ein skalierbares und intelligentes Hydrokultur-Ökosystem zu schaffen, das zu autonomen Betrieb und zukünftiger KI-Integration fähig ist.

---

# 🌍 Vision

HIDROVIDA zielt darauf ab, die Lücke zu schließen zwischen:

- Nachhaltiger Landwirtschaft
- Industrieller Automatisierung
- Wissenschaftlicher Experimentierung
- Internet der Dinge (IoT)
- Smart Farming Technologien

Die langfristige Vision ist es, die Plattform zu einem vollständig adaptiven Hydrokultur-Managementsystem weiterzuentwickeln, das prädiktive Nährstoffoptimierung, Anomalieerkennung und autonome Entscheidungsfindung ermöglicht.

---

# 🌱 Warum Hydrokultur?

Hydrokultur ist eine erdlose Anbaumethode, bei der Pflanzen Nährstoffe direkt über eine mineralreiche Wasserlösung erhalten.

Im Vergleich zur traditionellen Landwirtschaft bietet Hydrokultur:

- Deutliche Wasserersparnis
- Schnelleres Pflanzenwachstum
- Höhere Nährstoffeffizienz
- Kontrollierte Umweltbedingungen
- Geringere Pestizidabhängigkeit
- Möglichkeit der urbanen Landwirtschaft

Durch die Integration von Automatisierung in die Hydrokultur verbessert HIDROVIDA weiter:

- Stabilität der Nährstoffkonzentration
- Überwachungsgenauigkeit
- Ressourceneffizienz
- Betriebliche Skalierbarkeit

---

# 🎯 Projektziele

- Entwicklung einer intelligenten Hydrokultur-Steuerungsplattform
- Echtzeit-Überwachung chemischer und Umweltparameter
- Automatisierung von Nährstoffdosierungsprozessen
- Reduzierung menschlicher Eingriffe und Betriebsfehler
- Schaffung eines interdisziplinären ingenieurwissenschaftlichen Bildungsprojekts
- Demonstration praktischer IoT-Anwendungen in der Landwirtschaft
- Aufbau einer skalierbaren Architektur für zukünftige KI-Integration

---

# 🧠 Systemarchitektur

Die Plattform basiert auf einer modularen, verteilten Architektur, die auf dem Mikrocontroller **ESP32-S3** zentriert ist.

```text
Sensoren → Datenerfassung → ESP32-S3 Verarbeitung → Entscheidungsmotor → Dashboard & Aktoren
```

Das System führt aus:

- Kontinuierliche Sensorerfassung
- Echtzeit-Parameteranalyse
- Automatische Korrekturlogik
- Cloud- und lokale Dashboard-Kommunikation
- Ereignisbehandlung und Aktorsteuerung

---

# ⚙️ Kerntechnologien

## Eingebettete Hardware
- ESP32-S3
- RTC-Modul
- LCD I2C-Schnittstelle
- Relaismodule
- Peristaltikpumpen

## Wissenschaftliche Sensoren
- pH-Sensor
- TDS-Sensor
- Temperatursensor
- Wasserstandsensor
- Luftfeuchtigkeitssensor
- Lichtsensor
- Trübungssensor

## Software-Stack
- C++
- Arduino Framework
- ESP-IDF
- MQTT
- Wi-Fi-Kommunikation
- Blynk IoT-Plattform

---

# 🔬 Wissenschaftliche Grundlage

HIDROVIDA basiert auf Prinzipien aus:

- Analytischer Chemie
- Pflanzenernährung
- Umweltüberwachung
- Industrieller Automatisierung
- Elektroniktechnik
- Datenerfassungssystemen
- Intelligenter Landwirtschaft

---

# 🧪 Wissenschaftliche Überwachung

## pH-Regulierung

Der pH-Wert beeinflusst direkt die Nährstoffverfügbarkeit und die Wurzelabsorptionseffizienz.

Optimaler Hydrokultur-Bereich:

```math
5.5 \leq pH \leq 6.5
```

Unangemessene pH-Werte können verursachen:

- Nährstoffblockade
- Mineraltoxizität
- Vermindertes Pflanzenwachstum
- Wurzelstress

Das System überwacht kontinuierlich den pH-Wert und führt bei Bedarf eine automatische Korrekturdosierung durch.

---

## TDS-Überwachung (Total Dissolved Solids)

TDS-Werte schätzen die Konzentration gelöster mineralischer Nährstoffe in der Lösung.

Dieser Parameter ist kritisch für:
- Nährstoffstabilität
- Elektrische Leitfähigkeitsanalyse
- Düngungseffizienz
- Wachstumsoptimierung

Die Plattform bewertet dynamisch TDS-Schwankungen und aktiviert entsprechend die Nährstoffdosierpumpen.

---

## Umweltüberwachung

Das System überwacht:
- Umgebungstemperatur
- Luftfeuchtigkeit
- Lichtintensität
- Wassertemperatur

Diese Variablen beeinflussen:
- Photosyntheseeffizienz
- Verdunstungsraten
- Wurzelentwicklung
- Nährstoffaufnahme

---

# 🤖 Intelligentes Dosiersystem

HIDROVIDA enthält einen semi-autonomen Nährstoffkorrekturmechanismus.

## Dosierablauf

```text
Sensorablesung
       ↓
Parametervalidierung
       ↓
Schwellenwertvergleich
       ↓
Pumpenaktivierung
       ↓
Mischphase
       ↓
Neuer Validierungszyklus
```

Dieser Prozess hilft zu verhindern:
- Nährstoffüberdosierung
- Chemische Instabilität
- Ressourcenverschwendung
- Menschliche Bedienfehler

---

# 📡 Konnektivität und Fernzugriff

Die Plattform unterstützt:
- Wi-Fi-Kommunikation
- MQTT-Infrastruktur
- Fern-Dashboards
- Echtzeit-Überwachung
- IoT-Integration

Zukünftige Implementierungen können umfassen:
- Cloud-Synchronisation
- Mobile Anwendungen
- Fernwarnungen
- KI-gestützte Analyse

---

# 🖥️ Dashboard-Funktionen

Das Überwachungs-Dashboard bietet:

- Echtzeit-Sensorvisualisierung
- Relaiszustandsüberwachung
- Nährstoffsystemsteuerung
- Historische Datenanalyse
- Fernverwaltung von Aktoren
- Alarm- und Benachrichtigungssysteme

---

# 🏗️ Softwarearchitektur

Die Firmware wurde mit Blick auf Modularität und Skalierbarkeit entwickelt.

## Projektstruktur

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

# 🔐 Betriebssicherheit

Das System implementiert mehrere Sicherheitsmechanismen:

- Pumpenaktivierungsintervalle
- Überdosierungsschutz
- Sichere Relaisinitialisierung
- Sensorvalidierungsroutinen
- Fehlererkennungssysteme
- Wasserstands-Ausfallsicherung

Diese Funktionen verbessern die Betriebsstabilität und verringern das Risiko von Hardware-Schäden oder Nährstoffungleichgewicht.

---

# 🌍 Nachhaltigkeitsbeitrag

HIDROVIDA trägt zur nachhaltigen Landwirtschaft bei, indem es:

- Den Wasserverbrauch reduziert
- Die Nährstoffnutzung optimiert
- Abfall minimiert
- Die lokale Lebensmittelproduktion fördert
- Die wissenschaftliche Bildungsentwicklung unterstützt

Das Projekt zeigt, wie kostengünstige eingebettete Systeme nachhaltigere landwirtschaftliche Modelle unterstützen können.

---

# 📈 Zukünftige Entwicklung

Die nächsten Evolutionsstufen von HIDROVIDA umfassen:

## Künstliche Intelligenz
- Prädiktive Nährstoffanpassung
- Wachstumsoptimierungsmodelle
- Adaptive Dosiersysteme

## Computer Vision
- Pflanzenwachstumsanalyse
- Krankheitserkennung
- Überwachung der Blattfärbung

## Fortschrittliche Datenanalyse
- Historische Trendanalyse
- Cloud-basierte Überwachung
- Vorausschauende Wartung

## Erweiterung der Smart Agriculture
- Mehrzonenanbau
- Skalierbarer Gewächshauseinsatz
- Verteilte Sensornetzwerke

---

# 🎓 Bildungswirkung

HIDROVIDA wurde auch als interdisziplinäres Bildungsprojekt konzipiert, das integriert:

- Programmierung
- Robotik
- Physik
- Chemie
- Biologie
- Elektronik
- IoT-Ingenieurwesen

Das Projekt fördert praktisches wissenschaftliches Lernen und reale ingenieurwissenschaftliche Problemlösung.

---

# 📸 Wissenschaftliches Poster

Das für das Projekt entwickelte wissenschaftliche Poster präsentiert:
- Systemarchitektur
- Wissenschaftliche Prinzipien
- Intelligenten Dosierprozess
- Nachhaltigkeitsauswirkungen
- Betriebsablauf
- Fahrplan für die zukünftige Entwicklung

Es repräsentiert die konzeptionelle und wissenschaftliche Vision der Plattform über den aktuellen Implementierungsstand hinaus.

---

# 🚀 Repository-Ziele

Dieses Repository hat zum Ziel:

- Die Projektarchitektur zu dokumentieren
- Firmware- und Hardware-Referenzen bereitzustellen
- Wissenschaftliche und technische Forschung zu teilen
- Zukünftige Mitwirkende zu unterstützen
- Die Weiterentwicklung der HIDROVIDA-Plattform zu demonstrieren

---

# 👨‍💻 Team

Entwickelt von:

- Francisco Soares
- David Silva

In Zusammenarbeit mit:
- Agrupamento de Escolas Augusto Cabrita
- National Young Scientists Competition 2026

---

# 📷 Projektvorschau

> Intelligente Hydrokultur-Automation mit Fokus auf wissenschaftliche Überwachung, Nachhaltigkeit und zukunftsfähige Landwirtschaft.

---

# 📜 Lizenz

Dieses Projekt wird derzeit entwickelt für:
- Bildungszwecke
- Wissenschaftliche Forschung
- Technologiedemonstration

Zukünftige Lizenzbedingungen können hinzugefügt werden, wenn sich die Plattform weiterentwickelt.

---

# ⭐ HIDROVIDA

### "Wo Innovation das Leben wachsen lässt."

---
