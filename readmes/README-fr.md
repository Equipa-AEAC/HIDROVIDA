[🇬🇧 English](../README.md) | [🇵🇹 Português](README-pt.md) | [🇪🇸 Español](README-es.md) | [🇫🇷 Français](README-fr.md) | [🇩🇪 Deutsch](README-de.md) | [🇵🇱 Polski](README-pl.md)

---

# 🌱 HIDROVIDA
### Système d'Automatisation Hydrogonique Intelligent pour une Agriculture Durable

<p align="center">
  <img src="https://img.shields.io/badge/Statut-D%C3%A9veloppement-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Plateforme-ESP32--S3-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Licence-%C3%89ducative-lightgrey?style=for-the-badge"/>
</p>

---

## 📖 Aperçu

**HIDROVIDA** est une plateforme avancée d'automatisation hydroponique axée sur la gestion intelligente des nutriments, la surveillance environnementale et l'analyse agricole en temps réel.

Le projet combine :

- Ingénierie des systèmes embarqués
- Acquisition scientifique de capteurs
- Infrastructure IoT
- Dosage automatisé des nutriments
- Contrôle environnemental intelligent
- Agriculture pilotée par les données

Conçu comme une initiative éducative et technologique, HIDROVIDA démontre comment l'automatisation moderne et la surveillance scientifique peuvent transformer la production alimentaire durable.

Le système a été développé dans le contexte de l'innovation scientifique et de la recherche éducative, avec l'objectif de créer un écosystème hydroponique scalable et intelligent capable de fonctionnement autonome et d'intégration future avec l'IA.

---

# 🌍 Vision

HIDROVIDA vise à combler le fossé entre :

- L'agriculture durable
- L'automatisation industrielle
- L'expérimentation scientifique
- L'Internet des Objets (IoT)
- Les technologies agricoles intelligentes

La vision à long terme est de faire évoluer la plateforme vers un système de gestion hydroponique entièrement adaptatif, capable d'optimisation prédictive des nutriments, de détection d'anomalies et de prise de décision autonome.

---

# 🌱 Pourquoi l'Hydroponie ?

L'hydroponie est une méthode de culture sans sol où les plantes reçoivent les nutriments directement par une solution aqueuse riche en minéraux.

Comparée à l'agriculture traditionnelle, l'hydroponie offre :

- Une réduction significative de l'eau
- Une croissance plus rapide des plantes
- Une meilleure efficacité nutritionnelle
- Des conditions environnementales contrôlées
- Une dépendance réduite aux pesticides
- Une capacité d'agriculture urbaine

En intégrant l'automatisation à l'hydroponie, HIDROVIDA améliore encore :

- La stabilité de la concentration en nutriments
- La précision de la surveillance
- L'efficacité des ressources
- L'évolutivité opérationnelle

---

# 🎯 Objectifs du Projet

- Développer une plateforme de contrôle hydroponique intelligente
- Surveiller les paramètres chimiques et environnementaux en temps réel
- Automatiser les processus de dosage des nutriments
- Réduire l'intervention humaine et les erreurs opérationnelles
- Créer un projet éducatif interdisciplinaire d'ingénierie
- Démontrer les applications pratiques de l'IoT dans l'agriculture
- Construire une architecture évolutive pour une future intégration avec l'IA

---

# 🧠 Architecture du Système

La plateforme repose sur une architecture modulaire distribuée centrée sur le microcontrôleur **ESP32-S3**.

```text
Capteurs → Acquisition de Données → Traitement ESP32-S3 → Moteur de Décision → Tableau de Bord & Actionneurs
```

Le système effectue :

- L'acquisition continue de capteurs
- L'analyse des paramètres en temps réel
- La logique de correction automatique
- La communication avec le tableau de bord cloud et local
- La gestion des événements et le contrôle des actionneurs

---

# ⚙️ Technologies Principales

## Matériel Embarqué
- ESP32-S3
- Module RTC
- Interface LCD I2C
- Modules Relais
- Pompes Péristaltiques

## Capteurs Scientifiques
- Capteur de pH
- Capteur TDS
- Capteur de Température
- Capteur de Niveau d'Eau
- Capteur d'Humidité
- Capteur de Lumière
- Capteur de Turbidité

## Stack Logiciel
- C++
- Arduino Framework
- ESP-IDF
- MQTT
- Communication Wi-Fi
- Plateforme IoT Blynk

---

# 🔬 Fondement Scientifique

HIDROVIDA est construite sur des principes provenant :

- De la Chimie Analytique
- De la Nutrition Végétale
- De la Surveillance Environnementale
- De l'Automatisation Industrielle
- Du Génie Électronique
- Des Systèmes d'Acquisition de Données
- De l'Agriculture Intelligente

---

# 🧪 Surveillance Scientifique

## Régulation du pH

Le niveau de pH affecte directement la biodisponibilité des nutriments et l'efficacité d'absorption racinaire.

Plage hydroponique optimale :

```math
5.5 \leq pH \leq 6.5
```

Des valeurs de pH inappropriées peuvent causer :

- Le blocage des nutriments
- La toxicité minérale
- Un développement réduit des plantes
- Le stress racinaire

Le système surveille en continu le pH et effectue un dosage correctif automatique si nécessaire.

---

## Surveillance TDS (Solides Dissous Totaux)

Les valeurs TDS estiment la concentration des nutriments minéraux dissous dans la solution.

Ce paramètre est essentiel pour :
- La stabilité nutritionnelle
- L'analyse de conductivité électrique
- L'efficacité de la fertilisation
- L'optimisation de la croissance

La plateforme évalue dynamiquement les fluctuations TDS et active les pompes de dosage des nutriments en conséquence.

---

## Surveillance Environnementale

Le système surveille :
- La température ambiante
- L'humidité
- L'intensité lumineuse
- La température de l'eau

Ces variables influencent :
- L'efficacité de la photosynthèse
- Les taux d'évaporation
- Le développement racinaire
- L'absorption des nutriments

---

# 🤖 Système de Dosage Intelligent

HIDROVIDA inclut un mécanisme de correction des nutriments semi-autonome.

## Flux de Dosage

```text
Lecture du Capteur
       ↓
Validation du Paramètre
       ↓
Comparaison de Seuil
       ↓
Activation de la Pompe
       ↓
Période de Mélange
       ↓
Nouveau Cycle de Validation
```

Ce processus aide à prévenir :
- Le surdosage de nutriments
- L'instabilité chimique
- Le gaspillage de ressources
- Les erreurs opérationnelles humaines

---

# 📡 Connectivité et Accès à Distance

La plateforme prend en charge :
- Communication Wi-Fi
- Infrastructure MQTT
- Tableaux de bord à distance
- Surveillance en temps réel
- Intégration IoT

Les implémentations futures pourraient inclure :
- Synchronisation cloud
- Applications mobiles
- Alertes à distance
- Analyse assistée par IA

---

# 🖥️ Fonctionnalités du Tableau de Bord

Le tableau de bord de surveillance fournit :

- Visualisation des capteurs en temps réel
- Surveillance de l'état des relais
- Contrôle du système nutritif
- Analyse des données historiques
- Gestion à distance des actionneurs
- Systèmes d'alerte et de notification

---

# 🏗️ Architecture Logicielle

Le firmware a été conçu avec la modularité et l'évolutivité à l'esprit.

## Structure du Projet

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

# 🔐 Sécurité Opérationnelle

Le système implémente plusieurs mécanismes de sécurité :

- Intervalles d'activation des pompes
- Protection anti-surdosage
- Initialisation sécurisée des relais
- Routines de validation des capteurs
- Systèmes de détection d'erreurs
- Protection de sécurité par niveau d'eau

Ces fonctionnalités améliorent la stabilité opérationnelle et réduisent les risques de dommages matériels ou de déséquilibre nutritif.

---

# 🌍 Impact sur la Durabilité

HIDROVIDA contribue à l'agriculture durable en :

- Réduisant la consommation d'eau
- Optimisant l'utilisation des nutriments
- Minimisant les déchets
- Encourageant la production alimentaire locale
- Soutenant le développement scientifique éducatif

Le projet démontre comment les systèmes embarqués à faible coût peuvent soutenir des modèles agricoles plus durables.

---

# 📈 Développement Futur

Les prochaines étapes évolutives d'HIDROVIDA incluent :

## Intelligence Artificielle
- Ajustement prédictif des nutriments
- Modèles d'optimisation de la croissance
- Systèmes de dosage adaptatif

## Vision par Ordinateur
- Analyse de la croissance des plantes
- Détection des maladies
- Surveillance de la coloration foliaire

## Analyse Avancée des Données
- Analyse des tendances historiques
- Surveillance basée sur le cloud
- Maintenance prédictive

## Expansion de l'Agriculture Intelligente
- Culture multizone
- Déploiement en serres évolutives
- Réseaux de capteurs distribués

---

# 🎓 Impact Éducatif

HIDROVIDA a également été conçue comme un projet éducatif interdisciplinaire intégrant :

- La programmation
- La robotique
- La physique
- La chimie
- La biologie
- L'électronique
- L'ingénierie IoT

Le projet favorise l'apprentissage scientifique pratique et la résolution de problèmes d'ingénierie concrets.

---

# 📸 Affiche Scientifique

L'affiche scientifique développée pour le projet présente :
- L'architecture du système
- Les principes scientifiques
- Le processus de dosage intelligent
- L'impact sur la durabilité
- Le flux opérationnel
- La feuille de route de développement futur

Elle représente la vision conceptuelle et scientifique de la plateforme au-delà de l'état actuel de mise en œuvre.

---

# 🚀 Objectifs du Dépôt

Ce dépôt vise à :

- Documenter l'architecture du projet
- Fournir des références de firmware et de matériel
- Partager la recherche scientifique et technique
- Soutenir les futurs contributeurs
- Démontrer l'évolution de la plateforme HIDROVIDA

---

# 👨‍💻 Équipe

Développé par :

- Francisco Soares
- David Silva

En collaboration avec :
- Agrupamento de Escolas Augusto Cabrita
- National Young Scientists Competition 2026

---

# 📷 Aperçu du Projet

> Automatisation hydroponique intelligente axée sur la surveillance scientifique, la durabilité et une agriculture prête pour l'avenir.

---

# 📜 Licence

Ce projet est actuellement développé pour :
- Des fins éducatives
- La recherche scientifique
- La démonstration technologique

De futurs termes de licence pourront être ajoutés à mesure que la plateforme évoluera.

---

# ⭐ HIDROVIDA

### "Là où l'innovation fait grandir la vie."

---
