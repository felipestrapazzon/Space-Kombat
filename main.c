#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define ALTURA 20
#define LARGURA 40
#define MAX_ESTRELAS 25

struct Entidade {
    int x, y;
    int ativo;
};

// Variáveis Globais
char tela[ALTURA][LARGURA];
char fundo[ALTURA][LARGURA]; // Matriz para as estrelas fixas
struct Entidade jogador;
struct Entidade tiros[10];
struct Entidade inimigos[5];

int kills = 0;
int frameCount = 0; // Para controlar a velocidade de descida
int gameRunning = 1;

void inicializar() {
    srand(time(NULL));
    
    jogador.x = LARGURA / 2;
    jogador.y = ALTURA - 3;

    // Inicializa Estrelas no Fundo
    for(int i = 1; i < ALTURA - 1; i++) {
        for(int j = 1; j < LARGURA - 1; j++) {
            if(rand() % 50 == 0) fundo[i][j] = '.'; // 2% de chance de ter estrela
            else fundo[i][j] = ' ';
        }
    }

    for(int i = 0; i < 10; i++) tiros[i].ativo = 0;

    for(int i = 0; i < 5; i++) {
        inimigos[i].x = (rand() % (LARGURA - 6)) + 3;
        inimigos[i].y = 1;
        inimigos[i].ativo = 1;
    }
}

void desenhar() {
    static char buffer[(ALTURA + 5) * (LARGURA + 2)];
    int k = 0;

    printf("\033[H");

    for(int i = 0; i < 3; i++)
    buffer[k++] = '\n';

    // Limpa tela com o Fundo (Estrelas) e Bordas
    for(int i = 0; i < ALTURA; i++) {
        for(int j = 0; j < LARGURA; j++) {
            if(i == 0 || i == ALTURA - 1 || j == 0 || j == LARGURA - 1)
                tela[i][j] = '#';
            else
                tela[i][j] = fundo[i][j];
        }
    }

    // Renderiza Inimigos
    for(int i = 0; i < 5; i++)
        if(inimigos[i].ativo)
            tela[inimigos[i].y][inimigos[i].x] = 'W';

    // Renderiza Tiros
    for(int i = 0; i < 10; i++)
        if(tiros[i].ativo)
            tela[tiros[i].y][tiros[i].x] = '|';

    // Renderiza Nave
    int px = jogador.x;
    int py = jogador.y;
    tela[py - 2][px] = '^';
    tela[py - 1][px - 1] = '/';
    tela[py - 1][px]     = 'O';
    tela[py - 1][px + 1] = '\\';
    tela[py][px - 2] = '<';
    tela[py][px - 1] = '[';
    tela[py][px]     = 'A';
    tela[py][px + 1] = ']';
    tela[py][px + 2] = '>';

    // Monta buffer
    for(int i = 0; i < ALTURA; i++) {
        for(int j = 0; j < LARGURA; j++)
            buffer[k++] = tela[i][j];
        buffer[k++] = '\n';
    }

    // Interface
    k += sprintf(&buffer[k], "Kills: %d | Comandos: [A][D] Mover | [W] Atirar | [Q] Sair\n", kills);
    buffer[k] = '\0';

    printf("%s", buffer);
}

void atualizarEstrelas() {
    // Move todas as linhas uma posição para baixo
    for (int i = ALTURA - 2; i > 1; i--) {
        for (int j = 1; j < LARGURA - 1; j++) {
            fundo[i][j] = fundo[i - 1][j];
        }
    }

    // Gera novas estrelas apenas na primeira linha (topo)
    for (int j = 1; j < LARGURA - 1; j++) {
        if (rand() % 50 == 0) // Mesma probabilidade de antes
            fundo[1][j] = '.';
        else
            fundo[1][j] = ' ';
    }
}

void logica() {
    frameCount++;

    // Dificuldade: Velocidade aumenta conforme as kills (desce a cada X frames)
    // Começa em 30 frames e vai diminuindo até o mínimo de 5

    if (frameCount % 3 == 0) {
        atualizarEstrelas();
    }

    int velocidadeInimigo = 30 - (kills / 2);
    if(velocidadeInimigo < 5) velocidadeInimigo = 5;

    // Movimentação dos tiros
    for(int i = 0; i < 10; i++) {
        if(tiros[i].ativo) {
            tiros[i].y--;
            if(tiros[i].y <= 0) {
                tiros[i].ativo = 0;
                continue;
            }
            // Colisão Tiro vs Inimigo
            for(int j = 0; j < 5; j++) {
                if(inimigos[j].ativo && tiros[i].x == inimigos[j].x && tiros[i].y == inimigos[j].y) {
                    inimigos[j].ativo = 0;
                    tiros[i].ativo = 0;
                    kills++;
                }
            }
        }
    }

    // Movimentação dos Inimigos
    if(frameCount % velocidadeInimigo == 0) {
        for(int i = 0; i < 5; i++) {
            if(inimigos[i].ativo) {
                inimigos[i].y++;
                
                // Colisão Inimigo vs Player (Verifica se encostou em qualquer parte da nave)
                if(inimigos[i].y >= jogador.y - 2 && 
                   inimigos[i].x >= jogador.x - 2 && 
                   inimigos[i].x <= jogador.x + 2) {
                    gameRunning = 0;
                }

                // Inimigo passou da tela ou morreu: Respawn no topo
                if(inimigos[i].y >= ALTURA - 1) {
                    inimigos[i].y = 1;
                    inimigos[i].x = (rand() % (LARGURA - 6)) + 3;
                }
            } else {
                // Se o inimigo foi morto, respawn imediato no topo
                inimigos[i].y = 1;
                inimigos[i].x = (rand() % (LARGURA - 6)) + 3;
                inimigos[i].ativo = 1;
            }
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\033[2J"); // limpa tela inteira
    printf("\033[H");  // cursor topo
    printf("\033[2J\033[?25l");
    inicializar();

    while(gameRunning) {
        desenhar();
        logica();

        if(kbhit()) {
            char tecla = getch();
            if(tecla == 'a' && jogador.x > 3) jogador.x--;
            if(tecla == 'd' && jogador.x < LARGURA - 4) jogador.x++;
            if(tecla == 'w') {
                for(int i = 0; i < 10; i++) {
                    if(!tiros[i].ativo) {
                        tiros[i].x = jogador.x;
                        tiros[i].y = jogador.y - 3;
                        tiros[i].ativo = 1;
                        break;
                    }
                }
            }
            if(tecla == 'q') break;
        }
        Sleep(16);
    }

    // Tela de Game Over
    printf("\033[H\033[2J");
    printf("\n\n   GAME OVER!\n");
    printf("   Total de Kills: %d\n\n", kills);
    
    printf("\033[?25h");
    return 0;
}