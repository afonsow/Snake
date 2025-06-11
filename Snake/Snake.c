#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// Defines
#define TIMEOUT 100
#define DELAY 50000

// Variáveis Globais
typedef struct ponto {
    int x;
    int y;
} ponto;

typedef struct node {
    int x;
    int y;
    struct node* link;
    int data;
} node;

typedef enum {
    ESQ,
    DIR,
    CIMA,
    BAIXO
} direcao;

int Xmax = 0, Ymax = 0;
ponto comida;
node* head = NULL;
bool gameover;
direcao dirAtual = DIR;

// Funções
void iniciaNCurses();
void menu();
void jogo();
void criaComida();
void desenhaEcra();
void desenhaparte(ponto drawPoint);
void movimentoSnake(int* pontos);
bool colisaoComida();
void adicionaSegmento();
bool colisaoParede();
bool colisaoCauda();
void fimDoJogo(int pontos);
int pontuacao(node* head);
void libertarMemoria();

// Main
int main()
{
   
    iniciaNCurses();                // iniciar janela e programa
    menu();                         // lança o menu de jogo

    endwin(); // fecha a janela
    return 0;
}

void iniciaNCurses()  // inicia todas as configurações da janela do jogo
{
    initscr();              // Iniciar a janela
    noecho();               // input nao ecoar
    keypad(stdscr, TRUE);   // permitir input de teclas especiais
    cbreak();               // Ativar buffer de linha para input
    timeout(TIMEOUT);       // tempo de espera pelo input
    curs_set(FALSE);        // não mostrar o cursor

    // variável stdscr criada por init()
    getmaxyx(stdscr, Ymax, Xmax);
}

void menu()  // lança o menu do jogo
{
    clear();
    char opcaoMenu;
    do {                     
        // imprime o menu                                                   
        mvprintw(Ymax / 2 - 2, Xmax / 2 - 15, "=== Menu de jogo ===\n");
        mvprintw(Ymax / 2 - 1, Xmax / 2 - 15, "1. Jogar\n");
        mvprintw(Ymax / 2 , Xmax / 2 - 15, "3. Sair\n");
        mvprintw(Ymax / 2 + 1, Xmax / 2 - 15, "Seleciona uma opcao: \n");
        refresh();

        opcaoMenu = getchar();  // input do utilizador

        switch (opcaoMenu) {
            case '1':  // começa o jogo
                clear();
                mvprintw(Ymax / 2, Xmax / 2 - 15, "A começar o jogo...\n");
                refresh();
                sleep(2);  // pausar execução
                jogo();  // loop do jogo
                break;


            default:
                clear();
                mvprintw(Ymax / 2 - 2, Xmax / 2 - 15, "=== ERRO ===\n");
                mvprintw(Ymax / 2 - 1, Xmax / 2 - 15, "opcao invalida\n");
                refresh();
                break;
        }
    } while (opcaoMenu == 3);
}

void jogo()                         // lança o jogo
{
    int pontos = 0;
    criaComida();

    head = malloc(sizeof(node));    // cria a cabeça da cobra
    head->x = Xmax / 2;
    head->y = Ymax / 2;
    head->link = NULL;

    do {                            // loop do jogo
        desenhaEcra();
        movimentoSnake(&pontos);
    } while (gameover == false);
}

void criaComida()  // cria coordenadas aleatórias para a comida
{
    srand(time(NULL));
    comida.x = (rand() % Xmax);
    comida.y = (rand() % Ymax);
}

void desenhaEcra()
{
    clear();                                    // limpa o ecrã
    desenhaparte(comida);                       // desenha a comida
    node* segmento = head;
    while (segmento != NULL) {                  // desenha a cobra
        desenhaparte((ponto){ segmento->x, segmento->y });
        segmento = segmento->link;
    }
    refresh();                                  // refresh da ncurses
    usleep(DELAY);                              // delay entre frames
}

void desenhaparte(ponto drawPoint)  // desenha um segmento da cobra ou a comida
{
    mvprintw(drawPoint.y, drawPoint.x, "o");
}

void movimentoSnake(int* pontos)
{
    int ch = getch();

    switch (ch) {                   // faz o input alterar a direção ou acabar o jogo
        case 'x':
            gameover = true;
            break;
        case 'a':
        case KEY_LEFT:
            if (dirAtual != DIR)    // if para não andar na direção oposta à atual
                dirAtual = ESQ;
            break;
        case 'd':
        case KEY_RIGHT:
            if (dirAtual != ESQ)
                dirAtual = DIR;
            break;
        case 'w':
        case KEY_UP:
            if (dirAtual != BAIXO)
                dirAtual = CIMA;
            break;
        case 's':
        case KEY_DOWN:
            if (dirAtual != CIMA)
                dirAtual = BAIXO;
            break;
    }

    node* novoNo = malloc(sizeof(node));  // adiciona um nó na cabeça da cobra e iguala a cabeça a esse nó
    novoNo->x = head->x;
    novoNo->y = head->y;
    novoNo->link = head;
    head = novoNo;

    switch (dirAtual) {  // faz a cobra mudar de direção
        case DIR:
            head->x++;
            break;
        case ESQ:
            head->x--;
            break;
        case CIMA:
            head->y--;
            break;
        case BAIXO:
            head->y++;
            break;
    }

    if (colisaoComida()==false) {   
            node* tail = head;
            while (tail->link->link != NULL) {
                tail = tail->link;
            }
            free(tail->link);   // remove o ultimo nó
            tail->link = NULL;  // o penúlltimo fica a cauda para o tamanho da cobra ficar igual depois dela se mexer
    } else {
        node* tail = head;                  //  o mesmo acontece, apenas altera a pontuação
        while (tail->link->link != NULL) {
            tail = tail->link;
        }
        free(tail->link);
        tail->link = NULL;
        *pontos = pontuacao(head);
    }

    if (colisaoParede() || colisaoCauda()) {  // se perder, acaba o loop
        fimDoJogo(*pontos);
    }
}

bool colisaoComida()  // verifica se as coordenadas da comida sao iguais às da cabeça
{
    if (head->x == comida.x && head->y == comida.y) {
        criaComida();         // cria a comida outra vez
        adicionaSegmento();  // faz crescer a cobra
        return true;
    }
    return false;
}

void adicionaSegmento()  // adiciona um segmento á cobra
{
    node* segmento = head;
    while (segmento->link != NULL) {
        segmento = segmento->link;    // vai até ao último nó
    }

    node* novoSegmento = malloc(sizeof(node));  // cria um novo nó
    novoSegmento->x = segmento->x;  
    novoSegmento->y = segmento->y;
    novoSegmento->link = NULL;                  // define-o como o último nó

    segmento->link = novoSegmento;
}

bool colisaoParede()  // verifica se o segmento da cabeça partilha coordenadas com a parede
{
    if (head->x >= Xmax || head->x < 0 || head->y >= Ymax || head->y < 0) {
        return true;
    }
    return false;
}

bool colisaoCauda()  // verifica se o segmento da cabeça partilha coordenadas com qualquer outro segmento da cobra
{
    node* segmento = head->link;
    while (segmento != NULL) {
        if (head->x == segmento->x && head->y == segmento->y) {
            return true;
        }
        segmento = segmento->link;
    }
    return false;
}

void fimDoJogo(int pontos)  // imprime mensagem, espera por input e liberta a memória
{
    clear();
    mvprintw(Ymax / 2, Xmax / 2 - 15, "=== Fim do jogo ===\n");
    mvprintw(Ymax / 2 + 1, Xmax / 2 - 15, "Pontuacao: %d\n", pontos);
    mvprintw(Ymax / 2 + 2, Xmax / 2 - 15, "Pressione qualquer tecla para sair\n");
    refresh();

    nodelay(stdscr, false);  // ativar o bloqueio de input

    while (getch() == ERR)
    {
        // esperar por input
    }

    gameover = true; // sair do loop do jogo
    libertarMemoria();
}

int pontuacao(node* head)  // conta os nós da lista e itera a pontuação a cada nó
{
    int pontos = 0;
    node* segmento = head;
    while (segmento != NULL) {
        pontos++;
        segmento = segmento->link;
    }
    return pontos;
}

void libertarMemoria()  // itera pela cobra a libertar a memória
{
    node* segmento = head;
    while (segmento != NULL) {
        node* temp = segmento;
        segmento = segmento->link;
        free(temp);
    }
}

