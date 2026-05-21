# Sistema Hidropónico Automatizado: Monitorização, Controlo e Interface

## 1. Enquadramento do projeto

Este projeto tem como objetivo o desenvolvimento de um sistema hidropónico automatizado capaz de monitorizar, interpretar e corrigir parâmetros críticos da solução nutritiva e das condições de operação. A proposta combina princípios de produção hidropónica com eletrónica embarcada, sensores analógicos e digitais, atuadores de dosagem e uma interface local de operação.

Num sistema hidropónico, as plantas desenvolvem-se sem solo, recebendo água, oxigénio e nutrientes minerais diretamente por meio de uma solução nutritiva. Como não existe o efeito tampão natural do substrato, o controlo dos parâmetros físico-químicos torna-se essencial para garantir estabilidade, crescimento equilibrado e eficiência no uso de recursos.

## 2. Funcionamento do processo hidropónico

O funcionamento do sistema baseia-se em três processos principais:

### 2.1. Alimentação mineral das plantas

As plantas absorvem nutrientes dissolvidos em água sob a forma de iões. No presente sistema, a solução nutritiva é preparada a partir de quatro componentes principais:

- micronutrientes
- nitrato de cálcio
- nitrato de potássio
- sulfato de magnésio

A concentração global da solução é acompanhada por meio da medição de TDS e EC. Estes valores não identificam qual nutriente está em falta, mas permitem estimar a quantidade total de sais dissolvidos na solução. Assim, o controlo é feito com base em receitas de dosagem por fase de crescimento, em vez de correções isoladas por nutriente.

### 2.2. Controlo do pH

O pH influencia diretamente a disponibilidade dos nutrientes para absorção radicular. Mesmo que a solução tenha nutrientes suficientes, um pH fora da janela ideal pode reduzir a eficiência de absorção e induzir sintomas de carência. Por esse motivo, o sistema mede continuamente o pH e utiliza esta informação como condição de segurança antes de permitir a dosagem automática.

### 2.3. Gestão do volume e recirculação

O nível de água nos tanques principais e no reservatório de reposição é monitorizado para evitar operação em seco, erros de leitura e instabilidade na solução. Em paralelo, o sistema inclui uma bomba de circulação controlada por relay, permitindo integrar ciclos de mistura e homogeneização da solução nutritiva.

## 3. Automatização desenvolvida

O subsistema de automatização foi concebido para executar três funções:

- leitura periódica dos sensores
- decisão lógica com base em thresholds e perfis de crescimento
- atuação automática sobre bombas e relays

### 3.1. Lógica de dosagem

O sistema compara o valor atual de TDS com o valor alvo definido para a fase de crescimento ativa. Quando o TDS desce abaixo do limiar de acionamento e o pH se encontra dentro da janela aceitável, o controlador aciona as bombas doseadoras durante um tempo calculado a partir do caudal de cada motor.

Cada fase de crescimento possui:

- TDS alvo
- TDS de acionamento
- janela mínima e máxima de pH
- volume de dosagem por nutriente

Desta forma, a dosagem deixa de ser fixa e passa a responder ao estado da solução nutritiva e ao estádio de desenvolvimento da cultura.

### 3.2. Temporização e segurança

Para evitar sobredosagem, o sistema implementa:

- intervalo de cooldown entre dosagens
- tempo de mistura após cada correção
- bloqueio de dosagem quando o pH está fora da gama definida
- bloqueio de atuação quando já existe uma bomba ativa

Além disso, as saídas digitais são inicializadas em estado seguro no arranque da placa, reduzindo o risco de ativação indevida dos relays e das bombas durante o boot.

### 3.3. Gestão temporal

O projeto inclui uma camada de agendamento para controlo de eventos temporizados, como circulação e iluminação. A referência temporal é fornecida pelo módulo RTC DS3231, que alimenta o scheduler e a interface mesmo sem ligação à rede.

## 4. Sensores e atuadores do sistema

### 4.1. Sensores ativos

Os sensores atualmente integrados ou preparados no sistema são:

- sensor de temperatura DS18B20
- sensor de pH DFRobot Gravity pH V2
- sensor de TDS DFRobot Gravity TDS V2
- sensor de nível de água Grove Water Level 10 cm para o tanque principal 1
- sensor de nível de água Grove Water Level 10 cm para o tanque principal 2
- sensor de limite de água no tanque de reposição
- sensor de turbidez
- sensor de humidade DHT22

O sensor de CO2 encontra-se previsto na arquitetura, mas não faz parte do conjunto principal de sensores ativos nesta fase.

### 4.2. Atuadores ativos

O sistema inclui os seguintes atuadores:

- bomba doseadora de micronutrientes
- bomba doseadora de nitrato de cálcio
- bomba doseadora de nitrato de potássio
- bomba doseadora de sulfato de magnésio
- relay da bomba de circulação 12 V
- relay do sistema de iluminação

## 5. Materiais e componentes utilizados

Os principais materiais e módulos eletrónicos utilizados no desenvolvimento incluem:

- placa controladora ESP32-S3
- sensor DS18B20
- sensor DFRobot Gravity pH V2
- sensor DFRobot Gravity TDS V2
- sensor Grove Water Level 10 cm
- sensor de turbidez analógico
- sensor DHT22
- display LCD 20x4 com interface I2C
- botões de navegação da interface local
- módulo RTC DS3231
- relays para circulação e iluminação
- quatro bombas doseadoras para nutrientes
- reservatórios de solução nutritiva
- tubagem e linhas de dosagem
- estrutura dos tanques de cultivo e reservatório de reposição

## 6. Desenvolvimento da interface

Uma parte importante do projeto foi a evolução da interface homem-máquina. Numa fase inicial foram realizados testes com display OLED, mas o sistema foi posteriormente consolidado sobre um LCD 20x4, por oferecer melhor integração com menus locais, navegação por botões e organização modular do código.

A interface atual foi desenvolvida com três objetivos:

- visualização clara dos parâmetros principais
- navegação simples por menus locais
- preparação para expansão futura do sistema

### 6.1. Informação apresentada

O LCD apresenta informação organizada em menus, incluindo:

- data e hora do sistema
- qualidade da solução nutritiva
- níveis dos tanques
- estado de reposição
- turbidez
- fase de crescimento ativa
- estado da dosagem

### 6.2. Estrutura modular

A interface foi separada do núcleo de monitorização e controlo. Esta abordagem permitiu:

- reduzir conflitos entre subsistemas
- manter o código mais legível
- simplificar a manutenção
- permitir evolução futura para interface remota ou dashboard web

## 7. Desafios técnicos identificados

Durante o desenvolvimento foram identificados vários desafios relevantes:

- leituras instáveis de TDS com a sonda fora de água
- necessidade de garantir estados seguros no arranque das saídas
- conflitos de pinos entre sensores, interface e atuadores
- adaptação da medição de nível a tanques com alturas diferentes
- integração gradual entre controlo em tempo real e agendamento temporal

Estes desafios motivaram a criação de uma arquitetura mais organizada, com mapa de pinos centralizado, separação modular por responsabilidade e validação dos sinais antes da tomada de decisão.

## 8. Conclusão

O sistema desenvolvido representa uma plataforma funcional de hidroponia automatizada com capacidade de monitorização contínua, dosagem assistida por sensores, controlo local por interface LCD e preparação para expansão futura. A integração de sensores, atuadores e lógica de decisão permite transformar um processo manual num sistema mais consistente, escalável e tecnicamente documentado.

Numa fase seguinte, o projeto pode evoluir para:

- calibração mais robusta dos sensores
- integração efetiva do sensor de CO2
- interface remota em rede
- modelos de apoio à decisão com visão computacional
- registo histórico e análise de dados de cultivo
