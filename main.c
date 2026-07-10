#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef struct{
    // [0] - white, [1] - black
    // [0]-pawn, [1]-horse, [2]-bishop, [3]-rook, [4]-queen, [5]-king
    uint64_t pieces[2][6];

    uint64_t allWhite;
    uint64_t allBlack;

    uint64_t all;

    bool isWhiteTurn;
    uint8_t castleRights;
    int enPassant;
    int halfmoveClock;
} Board;


// function which working with uci interface
void uci_loop(Board *board) {
    char line[2000];
    
    // Отключаем буферизацию printf, чтобы GUI мгновенно получал ответы движка
    setbuf(stdout, NULL); 
    setbuf(stdin, NULL);

    while (1) {
        // Читаем строку из стандартного ввода (от GUI)
        if (!fgets(line, sizeof(line), stdin)) {
            continue;
        }

        // Убираем символ переноса строки \n в конце, если он есть
        line[strcspn(line, "\n")] = 0;

        // Если команда пустая — пропускаем
        if (line[0] == '\0') continue;

        // Берем первое слово из строки
        char *command = strtok(line, " ");

        if (strcmp(command, "uci") == 0) {
            // GUI asking engine 
            printf("id name C.O.S.S.A.C.K.\n");
            printf("id author T.Mikhaliev\n");
            // options
            printf("uciok\n");
        } 
        else if (strcmp(command, "isready") == 0) {
            // GUI проверяет, не завис ли движок
            printf("readyok\n");
        } 
        else if (strcmp(command, "ucinewgame") == 0) {
            // Начинается новая партия (очищаем доску, сбрасываем историю)
            // Тут будет вызов функции обнуления/инициализации доски
        } 
        else if (strcmp(command, "position") == 0) {
            char *argument = strtok(NULL, " ");
            if(strcmp(argument,"startpos") == 0){
                printf("making starting pos");
            }
            else if(strcmp(argument,"fen") == 0){
                printf("making fen pos");
            }
            // GUI передает позицию. Например: "position startpos moves e2e4 e7e5"
            // Сюда мы позже допишем парсер ходов
        } 
        else if (strcmp(command, "go") == 0) {
            // GUI говорит: "Думай!". Например: "go depth 6" или "go wtime 300000"
            // Тут мы будем запускать наш Alpha-Beta поиск. 
            // Пока движок ничего не умеет, просто вернем заглушку (первый попавшийся ход)
            printf("bestmove e2e4\n"); 
        } 
        else if (strcmp(command, "quit") == 0) {
            // GUI закрывает программу
            break;
        }
    }
}

int main(){
    printf("C.O.S.S.A.C.K. Structure initialized successfully!\n");
    Board board;

    memset(&board,0,sizeof(Board));

    uci_loop(&board);

    return 0;
}