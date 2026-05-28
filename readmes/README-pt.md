[🇬🇧 English](../README.md) | [🇵🇹 Português](README-pt.md) | [🇪🇸 Español](README-es.md) | [🇫🇷 Français](README-fr.md) | [🇩🇪 Deutsch](README-de.md) | [🇵🇱 Polski](README-pl.md)

---

# 🌱 HIDROVIDA
### Sistema Inteligente de Automação Hidropónica para Agricultura Sustentável

<p align="center">
  <img src="https://img.shields.io/badge/Estado-Desenvolvimento-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Plataforma-ESP32--S3-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Licen%C3%A7a-Educacional-lightgrey?style=for-the-badge"/>
</p>

---

## 📖 Visão Geral

**HIDROVIDA** é uma plataforma avançada de automação hidropónica focada na gestão inteligente de nutrientes, monitorização ambiental e análise agrícola em tempo real.

O projeto combina:

- Engenharia de sistemas embarcados
- Aquisição científica de sensores
- Infraestrutura IoT
- Dosagem automatizada de nutrientes
- Controlo ambiental inteligente
- Agricultura baseada em dados

Concetualizado como uma iniciativa educacional e tecnológica, a HIDROVIDA demonstra como a automação moderna e a monitorização científica podem transformar a produção alimentar sustentável.

O sistema foi desenvolvido no contexto da inovação científica e da investigação educacional, com o objetivo de criar um ecossistema hidropónico escalável e inteligente, capaz de operação autónoma e futura integração com IA.

---

# 🌍 Visão

A HIDROVIDA tem como objetivo colmatar a lacuna entre:

- Agricultura sustentável
- Automação industrial
- Experimentação científica
- Internet das Coisas (IoT)
- Tecnologias de agricultura inteligente

A visão de longo prazo é evoluir a plataforma para um sistema de gestão hidropónica totalmente adaptativo, capaz de otimização preditiva de nutrientes, deteção de anomalias e tomada de decisão autónoma.

---

# 🌱 Porquê Hidroponia?

A hidroponia é um método de cultivo sem solo, onde as plantas recebem nutrientes diretamente através de uma solução aquosa rica em minerais.

Comparativamente à agricultura tradicional, a hidroponia oferece:

- Redução significativa de água
- Crescimento mais rápido das plantas
- Maior eficiência nutricional
- Condições ambientais controladas
- Menor dependência de pesticidas
- Capacidade de agricultura urbana

Ao integrar automação na hidroponia, a HIDROVIDA melhora ainda mais:

- Estabilidade da concentração de nutrientes
- Precisão da monitorização
- Eficiência dos recursos
- Escalabilidade operacional

---

# 🎯 Objetivos do Projeto

- Desenvolver uma plataforma inteligente de controlo hidropónico
- Monitorizar parâmetros químicos e ambientais em tempo real
- Automatizar processos de dosagem de nutrientes
- Reduzir a intervenção humana e erros operacionais
- Criar um projeto educacional interdisciplinar de engenharia
- Demonstrar aplicações práticas de IoT na agricultura
- Construir uma arquitetura escalável para futura integração com IA

---

# 🧠 Arquitetura do Sistema

A plataforma baseia-se numa arquitetura modular distribuída centrada no microcontrolador **ESP32-S3**.

```text
Sensores → Aquisição de Dados → Processamento ESP32-S3 → Motor de Decisão → Dashboard & Atuadores
```

O sistema realiza:

- Aquisição contínua de sensores
- Análise de parâmetros em tempo real
- Lógica de correção automática
- Comunicação com dashboard em cloud e local
- Gestão de eventos e controlo de atuadores

---

# ⚙️ Tecnologias Principais

## Hardware Embarcado
- ESP32-S3
- Módulo RTC
- Interface LCD I2C
- Módulos de Relé
- Bombas Peristálticas

## Sensores Científicos
- Sensor de pH
- Sensor de TDS
- Sensor de Temperatura
- Sensor de Nível de Água
- Sensor de Humidade
- Sensor de Luz
- Sensor de Turbidez

## Stack de Software
- C++
- Arduino Framework
- ESP-IDF
- MQTT
- Comunicação Wi-Fi
- Plataforma IoT Blynk

---

# 🔬 Fundamentação Científica

A HIDROVIDA é construída com base em princípios de:

- Química Analítica
- Nutrição Vegetal
- Monitorização Ambiental
- Automação Industrial
- Engenharia Eletrónica
- Sistemas de Aquisição de Dados
- Agricultura Inteligente

---

# 🧪 Monitorização Científica

## Regulação de pH

O nível de pH afeta diretamente a biodisponibilidade de nutrientes e a eficiência da absorção radicular.

Intervalo hidropónico ideal:

```math
5.5 \leq pH \leq 6.5
```

Valores de pH inadequados podem causar:

- Bloqueio de nutrientes
- Toxicidade mineral
- Desenvolvimento reduzido das plantas
- Stress radicular

O sistema monitoriza continuamente o pH e realiza dosagem corretiva automática quando necessário.

---

## Monitorização de TDS (Sólidos Dissolvidos Totais)

Os valores de TDS estimam a concentração de nutrientes minerais dissolvidos na solução.

Este parâmetro é crítico para:
- Estabilidade nutricional
- Análise de condutividade elétrica
- Eficiência da fertilização
- Otimização do crescimento

A plataforma avalia dinamicamente as flutuações de TDS e ativa as bombas de dosagem de nutrientes conforme necessário.

---

## Monitorização Ambiental

O sistema monitoriza:
- Temperatura ambiente
- Humidade
- Intensidade luminosa
- Temperatura da água

Estas variáveis influenciam:
- Eficiência da fotossíntese
- Taxas de evaporação
- Desenvolvimento radicular
- Absorção de nutrientes

---

# 🤖 Sistema de Dosagem Inteligente

A HIDROVIDA inclui um mecanismo de correção de nutrientes semiautónomo.

## Fluxo de Dosagem

```text
Leitura do Sensor
       ↓
Validação do Parâmetro
       ↓
Comparação com Limiar
       ↓
Ativação da Bomba
       ↓
Período de Mistura
       ↓
Novo Ciclo de Validação
```

Este processo ajuda a prevenir:
- Sobredosagem de nutrientes
- Instabilidade química
- Desperdício de recursos
- Erros operacionais humanos

---

# 📡 Conectividade e Acesso Remoto

A plataforma suporta:
- Comunicação Wi-Fi
- Infraestrutura MQTT
- Dashboards remotos
- Monitorização em tempo real
- Integração IoT

Implementações futuras podem incluir:
- Sincronização cloud
- Aplicações móveis
- Alertas remotos
- Análise assistida por IA

---

# 🖥️ Funcionalidades do Dashboard

O dashboard de monitorização fornece:

- Visualização de sensores em tempo real
- Monitorização do estado dos relés
- Controlo do sistema de nutrientes
- Análise de dados históricos
- Gestão remota de atuadores
- Sistemas de alerta e notificação

---

# 🏗️ Arquitetura de Software

O firmware foi concebido com modularidade e escalabilidade em mente.

## Estrutura do Projeto

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

# 🔐 Segurança Operacional

O sistema implementa vários mecanismos de segurança:

- Intervalos de ativação das bombas
- Proteção contra sobredosagem
- Inicialização segura dos relés
- Rotinas de validação de sensores
- Sistemas de deteção de erros
- Proteção de segurança por nível de água

Estas funcionalidades melhoram a estabilidade operacional e reduzem o risco de danos no hardware ou desequilíbrio nutricional.

---

# 🌍 Impacto na Sustentabilidade

A HIDROVIDA contribui para a agricultura sustentável:

- Reduzindo o consumo de água
- Otimizando o uso de nutrientes
- Minimizando o desperdício
- Incentivando a produção alimentar local
- Apoiando o desenvolvimento científico educacional

O projeto demonstra como sistemas embarcados de baixo custo podem apoiar modelos agrícolas mais sustentáveis.

---

# 📈 Desenvolvimento Futuro

As próximas etapas evolutivas da HIDROVIDA incluem:

## Inteligência Artificial
- Ajuste preditivo de nutrientes
- Modelos de otimização de crescimento
- Sistemas de dosagem adaptativa

## Visão Computacional
- Análise do crescimento das plantas
- Deteção de doenças
- Monitorização da coloração foliar

## Análise Avançada de Dados
- Análise de tendências históricas
- Monitorização baseada em cloud
- Manutenção preditiva

## Expansão para Agricultura Inteligente
- Cultivo multizona
- Implementação em estufas escaláveis
- Redes de sensores distribuídos

---

# 🎓 Impacto Educacional

A HIDROVIDA foi também concebida como um projeto educacional interdisciplinar que integra:

- Programação
- Robótica
- Física
- Química
- Biologia
- Eletrónica
- Engenharia IoT

O projeto promove a aprendizagem científica prática e a resolução de problemas reais de engenharia.

---

# 📸 Poster Científico

O poster científico desenvolvido para o projeto apresenta:
- Arquitetura do sistema
- Princípios científicos
- Processo de dosagem inteligente
- Impacto na sustentabilidade
- Fluxo operacional
- Roteiro de desenvolvimento futuro

Representa a visão concetual e científica da plataforma para além do estado atual de implementação.

---

# 🚀 Objetivos do Repositório

Este repositório visa:

- Documentar a arquitetura do projeto
- Fornecer referências de firmware e hardware
- Partilhar investigação científica e técnica
- Apoiar futuros contribuidores
- Demonstrar a evolução da plataforma HIDROVIDA

---

# 👨‍💻 Equipa

Desenvolvido por:

- Francisco Soares
- David Silva

Em colaboração com:
- Agrupamento de Escolas Augusto Cabrita
- National Young Scientists Competition 2026

---

# 📷 Pré-visualização do Projeto

> Automação hidropónica inteligente focada na monitorização científica, sustentabilidade e agricultura preparada para o futuro.

---

# 📜 Licença

Este projeto é atualmente desenvolvido para:
- Fins educacionais
- Investigação científica
- Demonstração tecnológica

Futuros termos de licenciamento poderão ser adicionados à medida que a plataforma evoluir.

---

# ⭐ HIDROVIDA

### "Onde a inovação faz crescer a vida."

---
