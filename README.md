# Space-Kombat

# Descrição do Algoritmo

O algoritmo do jogo funciona como um ciclo que roda o tempo todo enquanto o jogo está acontecendo. Basicamente, o programa fica repetindo várias etapas rapidamente para atualizar tudo que está acontecendo na tela.

Primeiro o jogo inicializa todas as variáveis, como posição do jogador, inimigos, tiros, pontuação e o fundo do mapa. Também cria as estrelas no fundo para dar a sensação de movimento no espaço.

_Depois disso o jogo entra no menu inicial, onde o jogador pode escolher começar o jogo, ver as instruções ou sair._

# **Quando o jogo começa, o programa passa a repetir um loop principal que faz três coisas principais:**

**Desenha tudo na tela, o jogo monta um mapa usando uma matriz de caracteres. Nessa matriz ele desenha:**

bordas do mapa

fundo com estrelas

nave do jogador

inimigos

tiros

chefes

power-ups

drone aliado

_Depois ele imprime tudo no terminal de uma vez._

**Logica. Nessa parte o jogo atualiza tudo que está acontecendo:**

move o fundo do mapa

move inimigos

move tiros

verifica colisões

gera power-ups

cria chefes

aumenta a dificuldade com o tempo

_Também verifica se algum tiro acertou um inimigo ou chefe e diminui a vida deles._

**Lê os comandos do jogador. O programa verifica se o jogador apertou alguma tecla:**

W A S D para mover a nave

Espaço para atirar

Q para sair

Quando o jogador atira, um novo tiro é criado e começa a subir na tela.

**Durante o jogo, quando o jogador destrói inimigos, ele ganha kills. A cada 10 inimigos destruídos aparece um boss com mais vida e comportamentos diferentes.
Quando um boss é derrotado, o jogador ganha ajuda de um drone automático, que voa pelo mapa e também atira nos inimigos por um tempo.
Alguns inimigos também podem deixar power-ups, que melhoram a nave do jogador, aumentando velocidade, dano ou cadência de tiro.**

**A dificuldade vai aumentando com o tempo porque:**

os inimigos ficam mais rápidos

aparecem mais inimigos

os bosses ficam mais difíceis


O jogo termina quando o jogador colide com um inimigo, com um tiro inimigo ou com um boss. Quando isso acontece, aparece a tela de Game Over com a pontuação final e o jogo reinicia.

