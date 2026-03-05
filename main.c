#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

// Variaveis Constantes
#define ALTURA 24
#define LARGURA 55
#define MAX_TIROS 80 
#define MAX_TIROS_BOSS 25 
#define MAX_INIMIGOS 10
#define MAX_DROPS 5

struct Entidade {
    int x, y, ativo, tipo, hp, dir;
};

// Variaveis Globais
struct Entidade jogador;
struct Entidade drone;
struct Entidade tiros[MAX_TIROS];
struct Entidade tirosBoss[MAX_TIROS_BOSS];
struct Entidade inimigos[MAX_INIMIGOS];
struct Entidade drops[MAX_DROPS];
struct Entidade boss;

int kills = 0, frameCount = 0, gameRunning = 1;
int speedMult = 1, fireRate = 12, tiroDano = 1, tiroTimer = 0, droneTimer = 0;
int fireRateDisplay = 1; // Apenas para mostrar no painel
float aceleracao = 1.0; 

char tela[ALTURA][LARGURA], fundo[ALTURA][LARGURA];

// --- PRINCIPAIS FUNCOES ---


//Inicializar = garante que os contadores comecem no valor correto e que o jogador tenha o spawn no local correto e que os inimigos não tenham spawn antes de iniciar o jogo
void inicializar() {
    srand(time(NULL));
    kills = 0; frameCount = 0; gameRunning = 1;
    speedMult = 1; fireRate = 12; fireRateDisplay = 1; tiroDano = 1; tiroTimer = 0; droneTimer = 0; aceleracao = 1.0;
    
    jogador.x = LARGURA / 2; jogador.y = ALTURA - 5;
    boss.ativo = 0; boss.hp = 0; boss.dir = 1;
    drone.ativo = 0; drone.x = LARGURA / 2; drone.y = ALTURA - 8; drone.dir = 1;

    for(int i = 0; i < ALTURA; i++)
        for(int j = 0; j < LARGURA; j++)
            fundo[i][j] = (rand() % 50 == 0) ? '.' : ' ';

    for(int i = 0; i < MAX_TIROS; i++) tiros[i].ativo = 0;
    for(int i = 0; i < MAX_TIROS_BOSS; i++) tirosBoss[i].ativo = 0;
    for(int i = 0; i < MAX_DROPS; i++) drops[i].ativo = 0;
    for(int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].ativo = 1; 
        inimigos[i].x = (rand() % (LARGURA - 9)) + 4; 
        inimigos[i].y = (rand() % 10) - 10; 
        inimigos[i].hp = 1;
    }
}

//mostrarHelp = essa funcao habilita o tutorial e ajuda para o usuario, apenas apaga tudo e mostra as instruções com vários printLn

void mostrarHelp() {
    system("cls");
    printf("\n\n   ==============================================\n");
    printf("               MANUAL DO PILOTO ESTELAR          \n");
    printf("   ==============================================\n\n");
    printf("   MOVIMENTACAO: [ W ] [ A ] [ S ] [ D ]\n");
    printf("   DISPARAR:     [ ESPACO ]\n");
    printf("   SAIR:         [ Q ]\n\n");
    printf("   MECANICAS ESPECIAIS:\n");
    printf("   - TIRO PERFURANTE: Se o dano for maior que a vida,\n");
    printf("     o tiro atravessa e continua a destruir.\n");
    printf("   - CHEFOES: Surgem a cada 10 abates.\n");
    printf("     Temos o Esmagador, o Deslizante e a Nave Mae!\n");
    printf("   - DRONE AUTONOMO: Derrotar um chefe ativa um\n");
    printf("     drone aliado que voa sozinho, atira e destroi\n");
    printf("     asteroides no impacto fisico!\n\n");
    printf("   ITENS DE DROP:\n");
    printf("   [ S ] SPEED:   Voa mais casas (Max Nivel 4).\n");
    printf("   [ F ] FIRE:    Cadencia infinita!\n");
    printf("   [ D ] DAMAGE:  Dano infinito!\n\n");
    printf("   Pressione qualquer tecla para voltar...");
    getch();
}

//desenhar = responsável por fazer o escopo inicial do jogo, desenha os inimigos, jogador, mapa... 

void desenhar() {
    //tamanho mapa e utiliza o buffer, melhor do que vários printf
    static char buffer[(ALTURA + 15) * (LARGURA + 5)];
    int k = 0; 
    //move o cursor do terminal para o topo da tela permitindo redesenhar o jogo
    printf("\033[H");

    //desenha o mapa base, logica if else de: se estiver na borda "#" então usa o fundo
    for(int i = 0; i < ALTURA; i++) {
        for(int j = 0; j < LARGURA; j++) {
            if(i == 0 || i == ALTURA - 1 || j == 0 || j == LARGURA - 1) tela[i][j] = '#';
            else tela[i][j] = fundo[i][j];
        }
    }

    //desenha os drops de inimigos, S = speed, F = fire rate, D = damage. 
    //for loop para limitar o máximo de drops
    for(int i = 0; i < MAX_DROPS; i++)
        if(drops[i].ativo && drops[i].y < ALTURA-1) {
            char simb[] = {'S', 'F', 'D'}; tela[drops[i].y][drops[i].x] = simb[drops[i].tipo];
        }
    //Desenha e limita os inimigos para nao ultrapassarem o limite imposto nas constantes, se inimigos ativos e estiverem com o y < 0 ou ALTURA - 1, ele cria novos inimigos. Ou seja, se estiverem fora dos limites do mapa ele gera um novo inimigo
    for(int i = 0; i < MAX_INIMIGOS; i++)
        if(inimigos[i].ativo && inimigos[i].y > 0 && inimigos[i].y < ALTURA-1) {
            tela[inimigos[i].y][inimigos[i].x] = 'W';
            tela[inimigos[i].y][inimigos[i].x + 1] = 'W';
        }
    //Desenha os tiros do Boss representado por "*" para confundir o jogador com as estrelas atrás, garante que o tiro saia diretamente do boss
    for(int i = 0; i < MAX_TIROS_BOSS; i++)
        if(tirosBoss[i].ativo && tirosBoss[i].y > 0 && tirosBoss[i].y < ALTURA-1) 
            tela[tirosBoss[i].y][tirosBoss[i].x] = '*';
    //Desenha um boss diferente para cada tipo, verifica se o boss está dentro do mapa, entao ele desenha cada um respectivamente de acordo com o numero do tipo.
    if(boss.ativo && boss.y > 0 && boss.y < ALTURA-1) {
        int bx = boss.x, by = boss.y;
        if(boss.tipo == 0) { 
            tela[by][bx-1] = '['; tela[by][bx] = 'M'; tela[by][bx+1] = ']';
            if(by+1 < ALTURA-1) { tela[by+1][bx-2] = '/'; tela[by+1][bx] = 'V'; tela[by+1][bx+2] = '\\'; }
        } else if (boss.tipo == 1) { 
            tela[by][bx-5] = '<'; tela[by][bx-4] = '='; tela[by][bx-3] = '='; tela[by][bx-2] = '='; tela[by][bx-1] = '='; tela[by][bx] = 'H'; tela[by][bx+1] = '='; tela[by][bx+2] = '='; tela[by][bx+3] = '='; tela[by][bx+4] = '='; tela[by][bx+5] = '>';
            if(by+1 < ALTURA-1) { tela[by+1][bx-5] = '['; tela[by+1][bx-4] = '\\'; tela[by+1][bx-3] = '\\'; tela[by+1][bx-2] = 'O'; tela[by+1][bx-1] = '='; tela[by+1][bx] = 'X'; tela[by+1][bx+1] = '='; tela[by+1][bx+2] = 'O'; tela[by+1][bx+3] = '/'; tela[by+1][bx+4] = '/'; tela[by+1][bx+5] = ']'; }
            if(by+2 < ALTURA-1) { tela[by+2][bx-5] = 'v'; tela[by+2][bx-2] = 'v'; tela[by+2][bx] = 'V'; tela[by+2][bx+2] = 'v'; tela[by+2][bx+5] = 'v'; }
        } else if (boss.tipo == 2) { 
            tela[by][bx-2] = '{'; tela[by][bx-1] = 'O'; tela[by][bx] = 'W'; tela[by][bx+1] = 'O'; tela[by][bx+2] = '}';
            if(by+1 < ALTURA-1) { tela[by+1][bx-1] = '\\'; tela[by+1][bx] = 'V'; tela[by+1][bx+1] = '/'; }
        }
    }

    //Desenha os tiros do jogador e garante que os tiros comecem exatamente da ponta da espaçonave do jogador e como de costume verifica se pode fazer o desenho.
    for(int i = 0; i < MAX_TIROS; i++)
        if(tiros[i].ativo && tiros[i].y > 0) tela[tiros[i].y][tiros[i].x] = '|';

    int px = jogador.x, py = jogador.y;
    //se o drone esta ativo, está dentro da tela e nao colado nas bordas ele desenha o drone aliado
    if(drone.ativo && drone.y > 0 && drone.y < ALTURA-1 && drone.x > 1 && drone.x < LARGURA-2) {
        tela[drone.y][drone.x] = 'd'; 
        tela[drone.y-1][drone.x-1] = '\\'; 
        tela[drone.y-1][drone.x+1] = '/';
    }
    //garante que não aconteça o spawn próximo as bordas e depois desenha a nave.
    if(py > 2 && py < ALTURA-1 && px > 2 && px < LARGURA-3) {
        tela[py-2][px] = '^';
        tela[py-1][px-1] = '/'; tela[py-1][px] = 'O'; tela[py-1][px+1] = '\\';
        tela[py][px-2] = '<'; tela[py][px-1] = '['; tela[py][px] = 'A'; tela[py][px+1] = ']'; tela[py][px+2] = '>';
    }
    //Aqui ele pega cada caracter da char tela[] e coloca no char buffer[]. Isso transforma a matriz em texto e depois uma quebra de linha para garantir que os itens empilhem, e nao fique apenas na horizontal.
    for(int i = 0; i < ALTURA; i++) {
        for(int j = 0; j < LARGURA; j++) buffer[k++] = tela[i][j];
        buffer[k++] = '\n';
    }
    
    // mostra as informações do jogo
    k += sprintf(&buffer[k], " KILLS: %d | DANO: %d | SPEED: %d | CADENCIA: %d\n", kills, tiroDano, speedMult, fireRateDisplay);
    //garante que o boss esteja ativo para desenhar o alerta do chefe.
    if(boss.ativo) k += sprintf(&buffer[k], " !!! ALERTA: CHEFE HP [%d] APROXIMANDO-SE !!!\n", boss.hp);
    //se tiver um drone ativo no mapa, ele dá o aviso de quantos ciclos restantes ainda tem.
    else if(drone.ativo) k += sprintf(&buffer[k], " >>> DRONE AUTONOMO: %d CICLOS RESTANTES <<<\n", droneTimer);
    //se nao tiver nada.
    else k += sprintf(&buffer[k], " STATUS: SETOR LIMPO\n");
    //finaliza a String e imprime todo o conteudo de uma vez.
    buffer[k] = '\0'; printf("%s", buffer);
}


//A função logica() atualiza tudo que acontece no jogo a cada frame: move o fundo, controla tiros, inimigos, boss, drone, drops, colisões, aumento de dificuldade e verifica se o jogador morreu
void logica() {
    //contador de frames do jogo e diminui o tempo de recarga do tiro
    frameCount++; if(tiroTimer > 0) tiroTimer--;
    //a cada 5 frames desce "estrelas" no fundo do mapa, recebe linhas aleatorias de estrelas (da o efeito de estar se movendo).
    if(frameCount % 5 == 0) {
        for (int i = ALTURA-2; i > 1; i--) for (int j = 1; j < LARGURA-1; j++) fundo[i][j] = fundo[i-1][j];
        for (int j = 1; j < LARGURA-1; j++) fundo[1][j] = (rand() % 50 == 0) ? '.' : ' ';
    }

    //aumenta a dificuldade do jogo conforme os frames
    if(frameCount % 500 == 0 && aceleracao < 3.8) aceleracao += 0.15;

    //aparece um boss a cada 10 kills do jogador, o boss recebe posicao, vida e tipo aleatorio. A vida utilizamos 15 como fixo mais as kills do atuais do jogador.
    if(kills > 0 && kills % 10 == 0 && !boss.ativo) {
        boss.x = LARGURA / 2; boss.y = 1; boss.hp = 15 + (kills); 
        boss.tipo = rand() % 3; 
        boss.dir = 1; 
        boss.ativo = 1;
    }
    //se o drone estiver ativo ele move lateralmente, atira automaticamente e verifica a colisao com inimigos para mata-los ao encostar.
    if(drone.ativo) {
        droneTimer--;
        if(droneTimer <= 0) drone.ativo = 0;
        
        if(frameCount % 3 == 0) { 
            drone.x += drone.dir;
            if(drone.x <= 4) drone.dir = 1;
            if(drone.x >= LARGURA - 5) drone.dir = -1;
        }

        if(frameCount % 12 == 0) { 
            for(int i=0; i<MAX_TIROS; i++) if(!tiros[i].ativo) {
                tiros[i].x = drone.x; tiros[i].y = drone.y - 2; tiros[i].ativo = 1; break;
            }
        }

        for(int j = 0; j < MAX_INIMIGOS; j++) {
            if(inimigos[j].ativo && abs(inimigos[j].y - drone.y) <= 1 && inimigos[j].x + 1 >= drone.x - 1 && inimigos[j].x <= drone.x + 1) {
                inimigos[j].ativo = 0; kills++;
            }
        }
    }
    //percorre todos os tiros
    for(int i = 0; i < MAX_TIROS; i++) {
        if(tiros[i].ativo) {
            //se os tiros estiverem fora do mapa eles tem que sumir.
            tiros[i].y--;
            if(tiros[i].y <= 0) { tiros[i].ativo = 0; continue; }

            //verifica a colisao do inimigo com a do tiro do player, entao, se inimigo morre (colidido) kills++ e faz 30% de chance de dropar power-up
            for(int j = 0; j < MAX_INIMIGOS; j++) {
                if(inimigos[j].ativo && tiros[i].x >= inimigos[j].x && tiros[i].x <= inimigos[j].x + 1 && tiros[i].y == inimigos[j].y) {
                    int hpAnt = inimigos[j].hp;
                    inimigos[j].hp -= tiroDano;
                    
                    if(inimigos[j].hp <= 0) {
                        if(rand() % 100 < 30) {
                            for(int d=0; d<MAX_DROPS; d++) {
                                if(!drops[d].ativo) {
                                    drops[d].x=inimigos[j].x; drops[d].y=inimigos[j].y; 
                                    drops[d].tipo=rand()%3; drops[d].ativo=1; break;
                                }
                            }
                        }
                        inimigos[j].ativo = 0; kills++;
                        if(tiroDano <= hpAnt) tiros[i].ativo = 0; 
                    } else {
                        tiros[i].ativo = 0;
                    }
                }
            }

            //define altura e largura do boss, isso existe pois existe variacao de tamanho entre os bosses e influenciando nas funcionalidades
            
            int bossAlt = (boss.tipo == 1) ? 3 : 2;
            int bossLarg = (boss.tipo == 1) ? 5 : 2; 
            
            //verifica colisao entre os tiros com o boss, isso se o boss estiver vivo e se o tiro está perto na horizontal. 
            //exemplo: boss.x = 40 e tiro.x = 43 abs(43,40) = 3. Entao se 3 <= bossLarg ele acertou lateralmente
            //mesma coisa com altura

            if(boss.ativo && abs(tiros[i].x - boss.x) <= bossLarg && tiros[i].y >= boss.y && tiros[i].y <= boss.y + bossAlt) {
                //reduz vida do boss se acertar e faz os tiros sumirem na colisao
                boss.hp -= tiroDano; tiros[i].ativo = 0;
                if(boss.hp <= 0) { 
                    //se o boss morrer desativa ele e ativa o drone por 500 frames e define onde vai ser o spawn do drone
                    boss.ativo = 0; kills += 5; 
                    drone.ativo = 1; droneTimer = 500; 
                    drone.y = jogador.y - 6; 
                }
            }
        }
    }


    //velocidade dos tiros depende da quantia de kills, e se um tiro acertar o player é game over
    int velTiroBoss = 5 - (kills / 25);
    if(velTiroBoss < 1) velTiroBoss = 1; 
    
    if(frameCount % velTiroBoss == 0) {
        for(int i = 0; i < MAX_TIROS_BOSS; i++) {
            if(tirosBoss[i].ativo) {
                tirosBoss[i].y++;
                if(tirosBoss[i].y >= jogador.y-2 && tirosBoss[i].y <= jogador.y && abs(tirosBoss[i].x - jogador.x) <= 2) {
                    gameRunning = 0; 
                }
                if(tirosBoss[i].y >= ALTURA-1) tirosBoss[i].ativo = 0;
            }
        }
    }
    //define a velocidade conforme a dificuldade atual. Vao descnedo aos poucos e se colidirem com o jogador ficam desativados, ou seja mortos.
    int delayAsteroides = (int)(32 / aceleracao);
    if(frameCount % (delayAsteroides < 2 ? 2 : delayAsteroides) == 0) {
        for(int i = 0; i < MAX_INIMIGOS; i++) {
            if(inimigos[i].ativo) {
                inimigos[i].y++;
                if(inimigos[i].y >= jogador.y-2 && inimigos[i].y <= jogador.y && inimigos[i].x + 1 >= jogador.x - 2 && inimigos[i].x <= jogador.x + 2) {
                    gameRunning = 0; 
                }
                if(inimigos[i].y >= ALTURA-1) inimigos[i].ativo = 0;
            } else { inimigos[i].x = (rand() % (LARGURA-9))+4; inimigos[i].y = 1; inimigos[i].hp = 1+(kills/20); inimigos[i].ativo = 1; }
        }
    }
    //garante colisao entre jogador e boss e altera valores do boss
    if(boss.ativo) {
        int delayChefe = 25 - (kills / 3);
        if (delayChefe < 8) delayChefe = 8; 
        int velocidadeFinal = delayChefe;
        if(boss.tipo == 2) velocidadeFinal += 8;
        
        if (frameCount % velocidadeFinal == 0) {
            boss.y++;
            int bLarg = (boss.tipo == 1) ? 5 : 2; 
            int bAlt = (boss.tipo == 1) ? 2 : 1; 
            if(boss.y + bAlt >= jogador.y-2 && abs(boss.x - jogador.x) <= bLarg + 1) {
                gameRunning = 0;
            }
            if(boss.y >= ALTURA-1) {
                gameRunning = 0; 
            }
        }
        //mecanica para que o boss atire em várias posicoes 
        if(boss.tipo == 1 && frameCount % 7 == 0 && rand() % 2 == 0) {
            for(int i=0; i < MAX_TIROS_BOSS; i++) {
                if(!tirosBoss[i].ativo) {
                    tirosBoss[i].ativo = 1; tirosBoss[i].y = boss.y + 3;
                    int pos = rand() % 5; 
                    if(pos == 0) tirosBoss[i].x = boss.x - 5; 
                    else if(pos == 1) tirosBoss[i].x = boss.x - 2; 
                    else if(pos == 2) tirosBoss[i].x = boss.x; 
                    else if(pos == 3) tirosBoss[i].x = boss.x + 2; 
                    else tirosBoss[i].x = boss.x + 5; 
                    break;
                }
            }
        }
        
        //ajuste de velocidade e direção de movimento do boss
        //as kills definem a velocidade atual
        int velLateral = 6 - (kills / 15);
        if(velLateral < 2) velLateral = 2; 
        //garante que vá na horizontal e rebata nas bordas
        if(boss.tipo == 2 && frameCount % velLateral == 0) {
            boss.x += boss.dir;
            if(boss.x <= 4) boss.dir = 1;
            if(boss.x >= LARGURA - 5) boss.dir = -1; 
        }
    }


    //Logica de drops
    for(int i = 0; i < MAX_DROPS; i++) {
        //garante que esteja no mapa
        if(drops[i].ativo) {
            //garante que o jogador pegue o powerup numa area de 3
            if(abs(drops[i].y - jogador.y) <= 1 && abs(drops[i].x - jogador.x) <= 3) {
                
                // POWER UPS (SPEED COM LIMITE, O RESTO INFINITO)
                if(drops[i].tipo == 0 && speedMult < 4) speedMult++; 
                if(drops[i].tipo == 1) { 
                    fireRate -= 2; 
                    if(fireRate < 0) fireRate = 0; // Limite fisico do jogo (1 tiro por quadro)
                    fireRateDisplay++; // Apenas soma para o jogador ver o nivel subindo na interface
                }
                if(drops[i].tipo == 2) tiroDano++;
                
                drops[i].ativo = 0;
            }
            if(drops[i].ativo && frameCount % 6 == 0) {
                drops[i].y++;
                if(drops[i].y >= ALTURA-1) drops[i].ativo = 0;
            }
        }
    }
}
//funcao responsavel por chamar todas as funcoes e garantir o funcionamento do jogo
int main() {
    //desativa o buffer de saida do printf, isso que faz parecer com que tudo apareça instantaneamente na tela e esconde o cursor.
    //ou seja, limpa o buffer, limpa a tela e garante que o cursor nao apareça.
    setvbuf(stdout, NULL, _IONBF, 0); system("cls"); printf("\033[?25l");

    //loop infinito para iniciarlizar o jogo e começa a contar os frames
    while(1) {
        inicializar();
        frameCount = 0;


        //este segundo loop controla a tela inicial do jogo e atualiza o fundo animado com estrelas e move o fundo a cada 5 frames
        while(1) {
            frameCount++; if(frameCount % 5 == 0) {
                for (int i = ALTURA-2; i > 1; i--) for (int j = 1; j < LARGURA-1; j++) fundo[i][j] = fundo[i-1][j];
                for (int j = 1; j < LARGURA-1; j++) fundo[1][j] = (rand() % 50 == 0) ? '.' : ' ';
            }
            //desenha tudo na tela. (fundo, interface, bordas)
            desenhar();

            //desenha o menu nas posições especificadas
            printf("\033[9;15H###########################");
            printf("\033[10;15H#     GALACTIC ROGUE      #");
            printf("\033[11;15H###########################");
            printf("\033[13;15H  [ ENTER  ]  COMECAR      ");
            printf("\033[14;15H  [   H    ]  INSTRUCOES   ");
            printf("\033[15;15H  [   Q    ]  SAIR         ");
            
            //se alguma tecla foi pressionada ele verifica se é uma das opções correspondentes e chama as devidas funcoes
            if(kbhit()) { 
                char op = getch(); 
                if(op == 13) break; 
                if(op == 'h' || op == 'H') mostrarHelp(); 
                if(op == 'q' || op == 'Q') exit(0); 
            }
            //animar um pouco o menu
            Sleep(20);
        }

        //tempo cooldown para o player se mover novamente e define um maximo para o player nao se mover rapido demais
        int playerCooldown = 0;
        int maxCooldown = 3; 
        //enquanto o jogo estiver rodando (1), ele desenha e aplica a logica
        while(gameRunning) {
            desenhar(); logica();

            //Se o jogador ainda estiver em tempo de espera, diminui o contador, uma hora ele chega em 0 novamente e possibilita mover-se novamente
            if (playerCooldown > 0) playerCooldown--;

            //quando o cooldown chegar a 0 ele pode se mover. Se nao se mover o cooldown nao renicia, ja se ele se mover o cooldown renicia e volta o loop
            if (playerCooldown <= 0) {
                int moved = 0;
                //verifica se a tecla A, D, W, ou S estão seguradas e nao somente pressionadas. Com base em cada uma ele altera lateralmente ou verticalmente
                if (GetAsyncKeyState('A') & 0x8000) { jogador.x -= speedMult; if(jogador.x < 3) jogador.x = 3; moved = 1; }
                if (GetAsyncKeyState('D') & 0x8000) { jogador.x += speedMult; if(jogador.x > LARGURA - 4) jogador.x = LARGURA - 4; moved = 1; }
                if (GetAsyncKeyState('W') & 0x8000) { jogador.y -= speedMult; if(jogador.y < 3) jogador.y = 3; moved = 1; }
                if (GetAsyncKeyState('S') & 0x8000) { jogador.y += speedMult; if(jogador.y > ALTURA - 2) jogador.y = ALTURA - 2; moved = 1; }
                
                if (moved) playerCooldown = maxCooldown; 
            }

            //verifica se a tecla "SPACE" está segurada e se nao está em recarga, após isso procura um espaço no vetor livre, 
            if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && tiroTimer <= 0) {
                for(int i=0; i<MAX_TIROS; i++) if(!tiros[i].ativo) {
                    //se os tiros nao estiverem ativos, ativa ele e garante a posicao correta do tiro.
                    tiros[i].x = jogador.x; tiros[i].y = jogador.y-3; tiros[i].ativo = 1; tiroTimer = fireRate; break;
                }
            }

            //Garante que se pressionar a telca 'Q' finaliza o jogo.

            if (GetAsyncKeyState('Q') & 0x8000) { gameRunning = 0; break; }
            //velocidade do jogo, ou seja aqui usamos uma pausa de 15 milisegundos
            Sleep(15);
        }
        
        //se o jogador morrer ele limpa a tela e aparece a tela de Game Over, espera 2 segundos e volta o loop.
        system("cls");
        printf("\n\n\n\n       G A M E   O V E R\n       SCORE: %d\n\n       REINICIANDO...", kills);
        Sleep(2000);
    }
    return 0;
}
