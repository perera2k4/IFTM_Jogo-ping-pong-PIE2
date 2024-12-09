#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaração de pinos e constantes
const int PIN_BOTAO = 4; // pino digital conectado à saída do botão
const int JOGADOR1 = A1; // pino analógico conectado ao eixo Y do jogador 1
const int JOGADOR2 = A0; // pino analógico conectado ao eixo Y do jogador 2
const int BOTAO_PAUSA = 5; // Botão de pausa conectado à porta digital 5

const unsigned long TAXA_RAQUETE = 45; // Taxa de atualização da raquete (ms)
const unsigned long TAXA_BOLA = 0;    // Taxa de atualização da bola (ms)
const uint8_t ALTURA_RAQUETE = 12;    // Altura da raquete em pixels
int pontuacaoJogador1 = 0;            // Pontuação do jogador 1
int pontuacaoJogador2 = 0;            // Pontuação do jogador 2
int pontuacaoMaxima = 5;              // Pontuação máxima para vencer
int EMISSOR_SOM = 12;                 // Pino do emissor de som
bool resetarBola = false;             // Indicador para reiniciar a posição da bola
bool jogoPausado = false;             // Estado do jogo (pausado ou não)

#define LARGURA_TELA 128              // Largura do display OLED, em pixels
#define ALTURA_TELA 64                // Altura do display OLED, em pixels
#define BOTAO_RESET 3                 // Pino do botão de reset
#define OLED_RESET 4                  // Pino de reset do OLED (ou -1 para compartilhar reset do Arduino)

Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, OLED_RESET);

// Funções auxiliares
void desenharQuadra();
void desenharPontuacao();

// Variáveis para a posição e direção da bola
uint8_t bola_x = 64, bola_y = 32;
uint8_t direcaoBola_x = 1, direcaoBola_y = 1;
unsigned long tempoAtualizacaoBola;

unsigned long tempoAtualizacaoRaquete;
const uint8_t POS_X_JOGADOR2 = 22; // Posição X da raquete do jogador 2
uint8_t posicaoJogador2_y = 26;   // Posição Y da raquete do jogador 2

const uint8_t POS_X_JOGADOR1 = 105; // Posição X da raquete do jogador 1
uint8_t posicaoJogador1_y = 26;    // Posição Y da raquete do jogador 1

// Parâmetros iniciais
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Inicializa o display
  display.clearDisplay(); // Limpa o display

  // Exibe mensagem inicial
  somInicial();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 10); // Define a posição do texto
  display.print("IFTM - 12o A.D.S.");
  display.display(); // Atualiza o display para mostrar a mensagem

  delay(1000); // Pausa de 1 segundos

  display.setCursor(0, 25);
  display.print("Andreza, Arthur,\nBruno Medeiros,\nBruno Pereira,\nDiogenes, Jose Vitor,\nLuiz, Vitor");
  display.display(); // Atualiza para mostrar "12o A.D.S."

  delay(5000); // Pausa adicional de 5 segundos

  // Configuração inicial do jogo
  pinMode(EMISSOR_SOM, OUTPUT);
  pinMode(PIN_BOTAO, INPUT);
  pinMode(BOTAO_RESET, INPUT_PULLUP);
  pinMode(BOTAO_PAUSA, INPUT_PULLUP); // Configura o botão de pausa
  digitalWrite(PIN_BOTAO, HIGH);

  display.clearDisplay(); // Limpa o display para iniciar o jogo
  desenharQuadra();
  desenharPontuacao();
  display.display();

  tempoAtualizacaoBola = millis();
  tempoAtualizacaoRaquete = tempoAtualizacaoBola;
}

void loop() {
  // Verifica se o botão de pausa foi pressionado
  static bool estadoAnteriorPausa = HIGH;
  bool estadoAtualPausa = digitalRead(BOTAO_PAUSA);

  if (estadoAtualPausa == LOW && estadoAnteriorPausa == HIGH) {
    jogoPausado = !jogoPausado; // Alterna entre pausado e não pausado
    delay(200); // Debounce
  }
  estadoAnteriorPausa = estadoAtualPausa;

  // Pausa o jogo
  if (jogoPausado) {
    display.setCursor(29, 30);
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);
    display.print("Jogo Pausado");
    display.display();
    while (jogoPausado) {
      // Mantém o estado de pausa até o botão ser pressionado novamente
      if (digitalRead(BOTAO_PAUSA) == LOW) {
        jogoPausado = false;
        delay(200); // Debounce
      }
    }
    display.clearDisplay(); // Limpa a mensagem de pausa
    desenharQuadra();
    desenharPontuacao();
  }
  bool atualizar = false;
  unsigned long tempoAtual = millis();

  static bool estadoCima = false;
  static bool estadoBaixo = false;

  if (resetarBola) {
    if (pontuacaoJogador1 == pontuacaoMaxima || pontuacaoJogador2 == pontuacaoMaxima) {
      fimDeJogo();
    } else {
      display.fillScreen(BLACK);
      desenharPontuacao();
      desenharQuadra();
      bola_x = random(45, 50);
      bola_y = random(23, 33);
      do {
        direcaoBola_x = random(-1, 2);
      } while (direcaoBola_x == 0);

      do {
        direcaoBola_y = random(-1, 2);
      } while (direcaoBola_y == 0);

      resetarBola = false;
    }
  }

  // Atualização da posição da bola
  if (tempoAtual > tempoAtualizacaoBola) {
    uint8_t nova_x = bola_x + direcaoBola_x;
    uint8_t nova_y = bola_y + direcaoBola_y;

    // Verifica colisão com paredes verticais
    if (nova_x == 0 || nova_x == 127) {
      if (nova_x == 0) {
        pontuacaoJogador1 += 1;
        display.fillScreen(BLACK);
        somPontuacao();
        resetarBola = true;
      } else if (nova_x == 127) {
        pontuacaoJogador2 += 1;
        display.fillScreen(BLACK);
        somPontuacao();
        resetarBola = true;
      }
      direcaoBola_x = -direcaoBola_x;
      nova_x += direcaoBola_x + direcaoBola_x;
    }

    // Verifica colisão com paredes horizontais
    if (nova_y == 0 || nova_y == 63) {
      somRebote();
      direcaoBola_y = -direcaoBola_y;
      nova_y += direcaoBola_y + direcaoBola_y;
    }

    // Verifica colisão com raquete do jogador 2
    if (nova_x == POS_X_JOGADOR2 && nova_y >= posicaoJogador2_y && nova_y <= posicaoJogador2_y + ALTURA_RAQUETE) {
      somRebote();
      direcaoBola_x = -direcaoBola_x;
      nova_x += direcaoBola_x + direcaoBola_x;
    }

    // Verifica colisão com raquete do jogador 1
    if (nova_x == POS_X_JOGADOR1 && nova_y >= posicaoJogador1_y && nova_y <= posicaoJogador1_y + ALTURA_RAQUETE) {
      somRebote();
      direcaoBola_x = -direcaoBola_x;
      nova_x += direcaoBola_x + direcaoBola_x;
    }

    display.drawPixel(bola_x, bola_y, BLACK);
    display.drawPixel(nova_x, nova_y, WHITE);
    bola_x = nova_x;
    bola_y = nova_y;

    tempoAtualizacaoBola += TAXA_BOLA;
    atualizar = true;
  }

  // Atualização das posições das raquetes
  if (tempoAtual > tempoAtualizacaoRaquete) {
    tempoAtualizacaoRaquete += TAXA_RAQUETE;

    // Raquete do jogador 2
    display.drawFastVLine(POS_X_JOGADOR2, posicaoJogador2_y, ALTURA_RAQUETE, BLACK);
    if (analogRead(JOGADOR2) < 475) {
      posicaoJogador2_y -= 1;
    }
    if (analogRead(JOGADOR2) > 550) {
      posicaoJogador2_y += 1;
    }
    if (posicaoJogador2_y < 1) posicaoJogador2_y = 1;
    if (posicaoJogador2_y + ALTURA_RAQUETE > 63) posicaoJogador2_y = 63 - ALTURA_RAQUETE;
    display.drawFastVLine(POS_X_JOGADOR2, posicaoJogador2_y, ALTURA_RAQUETE, WHITE);

    // Raquete do jogador 1
    display.drawFastVLine(POS_X_JOGADOR1, posicaoJogador1_y, ALTURA_RAQUETE, BLACK);
    if (analogRead(JOGADOR1) < 475) {
      posicaoJogador1_y -= 1;
    }
    if (analogRead(JOGADOR1) > 550) {
      posicaoJogador1_y += 1;
    }
    if (posicaoJogador1_y < 1) posicaoJogador1_y = 1;
    if (posicaoJogador1_y + ALTURA_RAQUETE > 63) posicaoJogador1_y = 63 - ALTURA_RAQUETE;
    display.drawFastVLine(POS_X_JOGADOR1, posicaoJogador1_y, ALTURA_RAQUETE, WHITE);
  }

  if (atualizar) {
    desenharPontuacao();
    display.display();
    if (digitalRead(PIN_BOTAO) == 0) {
      // Reseta a pontuação e reinicia o jogo
      pontuacaoJogador1 = pontuacaoJogador2 = 0;

      unsigned long inicio = millis();
      while (millis() - inicio < 2000); // Pausa adicional antes de reiniciar

      tempoAtualizacaoBola = millis();
      tempoAtualizacaoRaquete = tempoAtualizacaoBola;
      resetarBola = true; // Sinaliza para reposicionar a bola
      setup(); // Volta para a tela inicial
    }
  }
}

// Desenhar a quadra
void desenharQuadra() {
  display.drawRect(0, 0, 128, 64, WHITE);
}

// Desenhar a pontuação
void desenharPontuacao() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(45, 0);
  display.println(pontuacaoJogador2);
  display.setCursor(75, 0);
  display.println(pontuacaoJogador1);
}

// Finalizar o jogo
void fimDeJogo() {
  display.fillScreen(BLACK);
  if (pontuacaoJogador1 > pontuacaoJogador2) {
    display.setCursor(20, 15);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("Jogador 1");
    display.setCursor(40, 35);
    display.print("venceu!");
  } else {
    display.setCursor(20, 15);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("Jogador 2");
    display.setCursor(40, 35);
    display.print("venceu!");
  }
  
  delay(100); // Pausa para exibir a mensagem
  display.display();
  delay(2000); // Mantém a mensagem na tela por 2 segundos
  somFinal();
  
  // Reseta a pontuação e reinicia o jogo
  pontuacaoJogador1 = pontuacaoJogador2 = 0;

  unsigned long inicio = millis();
  while (millis() - inicio < 2000); // Pausa adicional antes de reiniciar

  tempoAtualizacaoBola = millis();
  tempoAtualizacaoRaquete = tempoAtualizacaoBola;
  resetarBola = true; // Sinaliza para reposicionar a bola
}

// Som ao bater na parede ou na raquete
void somRebote() {
  tone(EMISSOR_SOM, 500, 50); // Emite um som de 500 Hz por 50 ms
}

// Som ao marcar ponto
void somPontuacao() {
  tone(EMISSOR_SOM, 100, 50); // Emite um som de 100 Hz por 50 ms
}

void somInicial() {
  int notas[] = {659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0};  // Frequências em Hz (0 é pausa)
  int duracoes[] = {150, 150, 75, 150, 75, 150, 150, 75, 200, 75, 150, 75};  // Durações em ms
  int totalNotas = sizeof(notas) / sizeof(notas[0]);  // Quantidade de notas
  
  for (int i = 0; i < totalNotas; i++) {
    if (notas[i] == 0) {
      delay(duracoes[i]);  // Pausa
    } else {
      tone(EMISSOR_SOM, notas[i], duracoes[i]);  // Toca a nota
      delay(duracoes[i]);  // Aguarda a duração
    }
    delay(25);  // Pequena pausa entre notas
  }
  noTone(EMISSOR_SOM);  // Para o som ao final
}

void somFinal() {
  int notas[] = {523, 659, 784, 880, 1047};  // Frequências das notas (em Hz)
  int duracoes[] = {300, 300, 300, 300, 500};  // Duração das notas (em ms)
  
  int totalNotas = sizeof(notas) / sizeof(notas[0]);  // Quantidade de notas
  
  for (int i = 0; i < totalNotas; i++) {
    tone(EMISSOR_SOM, notas[i], duracoes[i]);  // Toca a nota
    delay(duracoes[i]);  // Aguarda a duração da nota
    delay(50);  // Pequena pausa entre as notas
  }
  noTone(EMISSOR_SOM);  // Para o som ao final
}