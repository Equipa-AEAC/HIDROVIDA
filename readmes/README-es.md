[🇬🇧 English](../README.md) | [🇵🇹 Português](README-pt.md) | [🇪🇸 Español](README-es.md) | [🇫🇷 Français](README-fr.md) | [🇩🇪 Deutsch](README-de.md) | [🇵🇱 Polski](README-pl.md)

---

# 🌱 HIDROVIDA
### Sistema Inteligente de Automatización Hidropónica para Agricultura Sostenible

<p align="center">
  <img src="https://img.shields.io/badge/Estado-Desarrollo-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Plataforma-ESP32--S3-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Licencia-Educacional-lightgrey?style=for-the-badge"/>
</p>

---

## 📖 Visión General

**HIDROVIDA** es una plataforma avanzada de automatización hidropónica centrada en la gestión inteligente de nutrientes, monitorización ambiental y análisis agrícola en tiempo real.

El proyecto combina:

- Ingeniería de sistemas embebidos
- Adquisición científica de sensores
- Infraestructura IoT
- Dosificación automatizada de nutrientes
- Control ambiental inteligente
- Agricultura basada en datos

Diseñado como una iniciativa educativa y tecnológica, HIDROVIDA demuestra cómo la automatización moderna y la monitorización científica pueden transformar la producción alimentaria sostenible.

El sistema fue desarrollado en el contexto de la innovación científica y la investigación educativa, con el objetivo de crear un ecosistema hidropónico escalable e inteligente capaz de operación autónoma y futura integración con IA.

---

# 🌍 Visión

HIDROVIDA busca cerrar la brecha entre:

- Agricultura sostenible
- Automatización industrial
- Experimentación científica
- Internet de las Cosas (IoT)
- Tecnologías de agricultura inteligente

La visión a largo plazo es evolucionar la plataforma hacia un sistema de gestión hidropónica totalmente adaptativo, capaz de optimización predictiva de nutrientes, detección de anomalías y toma de decisiones autónoma.

---

# 🌱 ¿Por Qué Hidroponía?

La hidroponía es un método de cultivo sin suelo donde las plantas reciben nutrientes directamente a través de una solución acuosa rica en minerales.

En comparación con la agricultura tradicional, la hidroponía ofrece:

- Reducción significativa de agua
- Crecimiento más rápido de las plantas
- Mayor eficiencia nutricional
- Condiciones ambientales controladas
- Menor dependencia de pesticidas
- Capacidad de agricultura urbana

Al integrar automatización en la hidroponía, HIDROVIDA mejora aún más:

- Estabilidad de la concentración de nutrientes
- Precisión de la monitorización
- Eficiencia de recursos
- Escalabilidad operativa

---

# 🎯 Objetivos del Proyecto

- Desarrollar una plataforma inteligente de control hidropónico
- Monitorizar parámetros químicos y ambientales en tiempo real
- Automatizar procesos de dosificación de nutrientes
- Reducir la intervención humana y errores operativos
- Crear un proyecto educativo interdisciplinario de ingeniería
- Demostrar aplicaciones prácticas de IoT en la agricultura
- Construir una arquitectura escalable para futura integración con IA

---

# 🧠 Arquitectura del Sistema

La plataforma se basa en una arquitectura modular distribuida centrada en el microcontrolador **ESP32-S3**.

```text
Sensores → Adquisición de Datos → Procesamiento ESP32-S3 → Motor de Decisión → Dashboard & Actuadores
```

El sistema realiza:

- Adquisición continua de sensores
- Análisis de parámetros en tiempo real
- Lógica de corrección automática
- Comunicación con dashboard en la nube y local
- Gestión de eventos y control de actuadores

---

# ⚙️ Tecnologías Principales

## Hardware Embebido
- ESP32-S3
- Módulo RTC
- Interfaz LCD I2C
- Módulos de Relé
- Bombas Peristálticas

## Sensores Científicos
- Sensor de pH
- Sensor de TDS
- Sensor de Temperatura
- Sensor de Nivel de Agua
- Sensor de Humedad
- Sensor de Luz
- Sensor de Turbidez

## Stack de Software
- C++
- Arduino Framework
- ESP-IDF
- MQTT
- Comunicación Wi-Fi
- Plataforma IoT Blynk

---

# 🔬 Fundamento Científico

HIDROVIDA se construye sobre principios de:

- Química Analítica
- Nutrición Vegetal
- Monitorización Ambiental
- Automatización Industrial
- Ingeniería Electrónica
- Sistemas de Adquisición de Datos
- Agricultura Inteligente

---

# 🧪 Monitorización Científica

## Regulación de pH

El nivel de pH afecta directamente la biodisponibilidad de nutrientes y la eficiencia de absorción radicular.

Rango hidropónico óptimo:

```math
5.5 \leq pH \leq 6.5
```

Valores de pH inadecuados pueden causar:

- Bloqueo de nutrientes
- Toxicidad mineral
- Desarrollo reducido de las plantas
- Estrés radicular

El sistema monitoriza continuamente el pH y realiza dosificación correctiva automática cuando es necesario.

---

## Monitorización de TDS (Sólidos Disueltos Totales)

Los valores de TDS estiman la concentración de nutrientes minerales disueltos en la solución.

Este parámetro es crítico para:
- Estabilidad nutricional
- Análisis de conductividad eléctrica
- Eficiencia de fertilización
- Optimización del crecimiento

La plataforma evalúa dinámicamente las fluctuaciones de TDS y activa las bombas de dosificación de nutrientes según sea necesario.

---

## Monitorización Ambiental

El sistema monitoriza:
- Temperatura ambiente
- Humedad
- Intensidad lumínica
- Temperatura del agua

Estas variables influyen en:
- Eficiencia de la fotosíntesis
- Tasas de evaporación
- Desarrollo radicular
- Absorción de nutrientes

---

# 🤖 Sistema de Dosificación Inteligente

HIDROVIDA incluye un mecanismo de corrección de nutrientes semiautónomo.

## Flujo de Dosificación

```text
Lectura del Sensor
       ↓
Validación del Parámetro
       ↓
Comparación con Umbral
       ↓
Activación de la Bomba
       ↓
Período de Mezcla
       ↓
Nuevo Ciclo de Validación
```

Este proceso ayuda a prevenir:
- Sobredosificación de nutrientes
- Inestabilidad química
- Desperdicio de recursos
- Errores operativos humanos

---

# 📡 Conectividad y Acceso Remoto

La plataforma soporta:
- Comunicación Wi-Fi
- Infraestructura MQTT
- Dashboards remotos
- Monitorización en tiempo real
- Integración IoT

Implementaciones futuras pueden incluir:
- Sincronización en la nube
- Aplicaciones móviles
- Alertas remotas
- Análisis asistido por IA

---

# 🖥️ Funcionalidades del Dashboard

El dashboard de monitorización proporciona:

- Visualización de sensores en tiempo real
- Monitorización del estado de los relés
- Control del sistema de nutrientes
- Análisis de datos históricos
- Gestión remota de actuadores
- Sistemas de alerta y notificación

---

# 🏗️ Arquitectura de Software

El firmware fue diseñado con modularidad y escalabilidad en mente.

## Estructura del Proyecto

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

# 🔐 Seguridad Operacional

El sistema implementa varios mecanismos de seguridad:

- Intervalos de activación de bombas
- Protección contra sobredosificación
- Inicialización segura de relés
- Rutinas de validación de sensores
- Sistemas de detección de errores
- Protección de seguridad por nivel de agua

Estas características mejoran la estabilidad operativa y reducen el riesgo de daños al hardware o desequilibrio nutricional.

---

# 🌍 Impacto en la Sostenibilidad

HIDROVIDA contribuye a la agricultura sostenible:

- Reduciendo el consumo de agua
- Optimizando el uso de nutrientes
- Minimizando el desperdicio
- Fomentando la producción local de alimentos
- Apoyando el desarrollo científico educativo

El proyecto demuestra cómo los sistemas embebidos de bajo costo pueden respaldar modelos agrícolas más sostenibles.

---

# 📈 Desarrollo Futuro

Las próximas etapas evolutivas de HIDROVIDA incluyen:

## Inteligencia Artificial
- Ajuste predictivo de nutrientes
- Modelos de optimización del crecimiento
- Sistemas de dosificación adaptativa

## Visión por Computadora
- Análisis del crecimiento de las plantas
- Detección de enfermedades
- Monitorización de la coloración foliar

## Análisis Avanzado de Datos
- Análisis de tendencias históricas
- Monitorización basada en la nube
- Mantenimiento predictivo

## Expansión en Agricultura Inteligente
- Cultivo multizona
- Implementación en invernaderos escalables
- Redes de sensores distribuidas

---

# 🎓 Impacto Educativo

HIDROVIDA también fue diseñada como un proyecto educativo interdisciplinario que integra:

- Programación
- Robótica
- Física
- Química
- Biología
- Electrónica
- Ingeniería IoT

El proyecto promueve el aprendizaje científico práctico y la resolución de problemas reales de ingeniería.

---

# 📸 Póster Científico

El póster científico desarrollado para el proyecto presenta:
- Arquitectura del sistema
- Principios científicos
- Proceso de dosificación inteligente
- Impacto en la sostenibilidad
- Flujo operativo
- Hoja de ruta de desarrollo futuro

Representa la visión conceptual y científica de la plataforma más allá del estado actual de implementación.

---

# 🚀 Objetivos del Repositorio

Este repositorio tiene como objetivo:

- Documentar la arquitectura del proyecto
- Proporcionar referencias de firmware y hardware
- Compartir investigación científica y técnica
- Apoyar a futuros contribuyentes
- Demostrar la evolución de la plataforma HIDROVIDA

---

# 👨‍💻 Equipo

Desarrollado por:

- Francisco Soares
- David Silva

En colaboración con:
- Agrupamento de Escolas Augusto Cabrita
- National Young Scientists Competition 2026

---

# 📷 Vista Previa del Proyecto

> Automatización hidropónica inteligente centrada en la monitorización científica, sostenibilidad y agricultura preparada para el futuro.

---

# 📜 Licencia

Este proyecto se desarrolla actualmente para:
- Fines educativos
- Investigación científica
- Demostración tecnológica

Se podrán añadir términos de licencia futuros a medida que la plataforma evolucione.

---

# ⭐ HIDROVIDA

### "Donde la innovación hace crecer la vida."

---
