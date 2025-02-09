# Contador com Display e Exibição de LED - Comunicação Serial
## Descrição do Projeto
Este projeto foi desenvolvido para demonstrar o uso de **comunicação serial**, **matriz de LEDs WS2812**, **display OLED SSD1306** e **botões** no Raspberry Pi Pico. O sistema permite que o usuário envie um caractere via USB (dígitos de `'0'` a `'9'`) e exiba esse dígito tanto na matriz de LEDs quanto no display OLED. Além disso, dois botões permitem alternar o estado de LEDs externos (verde e azul), com feedback visual no display.

O código utiliza o **PIO (Programmable Input/Output)** para controlar os LEDs WS2812 com precisão temporal, enquanto o display OLED é usado para fornecer informações adicionais ao usuário. A implementação inclui debounce para evitar múltiplas detecções de pressionamento dos botões.

---
## Componentes Utilizados
- **Matriz de LEDs WS2812 (5x5):** Conectada ao pino 7 do Raspberry Pi Pico.
- **Display OLED SSD1306:** Conectado via I2C nos pinos 14 (SDA) e 15 (SCL).
- **Botão A (Pino 5):** Controla o LED verde.
- **Botão B (Pino 6):** Controla o LED azul.
- **LED Verde (Pino 11):** Acende ou apaga ao pressionar o Botão A.
- **LED Azul (Pino 12):** Acende ou apaga ao pressionar o Botão B.
- **Raspberry Pi Pico:** Placa utilizada para executar o código.
- **Resistores:** Para limitar a corrente nos LEDs (recomenda-se 220Ω ou 330Ω).
---
## Funcionalidades do Projeto
- **Exibição de Dígitos:**
  - O usuário pode enviar um dígito (`'0'` a `'9'`) via comunicação serial (USB).
  - O dígito é exibido na matriz de LEDs WS2812 e no display OLED.
- **Controle de LEDs Externos:**
  - Botão A liga/desliga o LED verde.
  - Botão B liga/desliga o LED azul.
- **Feedback Visual:**
  - O estado dos LEDs externos (verde e azul) é exibido no display OLED.
- **Debounce:**
  - Implementação de debounce para evitar múltiplas detecções de pressionamento dos botões.
---
## Guia de Funcionamento na Sua Máquina
Para executar este projeto localmente, siga as instruções abaixo:
### 1. **Clone o repositório:**
   - Abra o **VS Code** e clone este repositório para sua máquina.
### 2. **Importe o projeto:**
   - Certifique-se de ter as extensões do **Raspberry Pi Pico** instaladas no VS Code.
   - Importe o projeto para poder compilá-lo e executá-lo na placa RP2040.
### 3. **Conecte a placa:**
   - Conecte a placa ao computador via USB e coloque-a no modo **BOOTSEL**.
### 4. **Compile o código:**
   - Compile o código diretamente no VS Code.
### 5. **Simulação no Wokwi:**
   - Para simular o projeto, abra o arquivo `diagram.json` disponível nos arquivos do projeto e execute-o no [Wokwi](https://wokwi.com).
### 6. **Execute na placa:**
   - Após a compilação e com a placa no modo **BOOTSEL**, clique em **Executar** ou **Run** para carregar o programa na placa.
### 7. **Envio de Caracteres via USB:**
   - Use um terminal serial (como `minicom`, `PuTTY` ou o monitor serial do VS Code) para enviar caracteres ao Raspberry Pi Pico.
   - Envie um dígito (`'0'` a `'9'`) para exibi-lo na matriz de LEDs e no display OLED.
---
## Funcionamento do Sistema
O sistema funciona da seguinte forma:
1. **Estado Inicial:**
   - Todos os LEDs estão apagados.
   - O display OLED exibe uma mensagem inicial indicando que o sistema está pronto para receber comandos.
2. **Envio de Dígitos via USB:**
   - Quando o usuário envia um dígito (`'0'` a `'9'`) via USB:
     - O dígito é exibido na matriz de LEDs WS2812.
     - O mesmo dígito é exibido no display OLED.
3. **Pressionamento dos Botões:**
   - **Botão A:**
     - Alterna o estado do LED verde (liga/desliga).
     - O estado do LED verde é atualizado no display OLED.
   - **Botão B:**
     - Alterna o estado do LED azul (liga/desliga).
     - O estado do LED azul é atualizado no display OLED.
4. **Debounce:**
   - O código implementa debounce com um intervalo de 200 ms para evitar múltiplas detecções de pressionamento dos botões.
---
## Código Fonte
O código fonte está organizado da seguinte forma:
- **Função `initialize_sets`:** Configura os periféricos (I2C, GPIO, PIO, etc.) e inicializa o sistema.
- **Função `setup`:** Processa entradas do usuário via USB e atualiza a matriz de LEDs e o display OLED.
- **Função `buttonInterruptHandler`:** Lida com interrupções dos botões e alterna o estado dos LEDs externos.
- **Função `displayDigitMatrix`:** Exibe um dígito na matriz de LEDs WS2812.
- **Função `drawFrame`:** Desenha um frame específico na matriz de LEDs.
- **Função `sendLedBuffer`:** Envia os dados de cor para os LEDs WS2812 via PIO.
- **Loop Principal (`main`):** Executa continuamente a função `setup` para processar entradas do usuário.
---
## Observações Finais
Este projeto foi desenvolvido com foco em boas práticas de programação, organização e documentação. Ele é ideal para estudantes que desejam praticar o uso de comunicação serial, controle de LEDs WS2812, displays OLED e interrupções no Raspberry Pi Pico.

Caso tenha dúvidas ou sugestões, sinta-se à vontade para abrir uma **issue** ou entrar em contato.

---
### Créditos
- **Autor:** Ângelo Miguel Ribeiro Cerqueira Lima
- **Data:** 09/02/2024
---