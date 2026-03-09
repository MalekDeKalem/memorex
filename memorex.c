#include "raylib.h"
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define MEM_EASY_SIZE 24
#define MEM_MEDIUM_SIZE 48
#define MEM_HARD_SIZE 64
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_WIDTH_EASY 6
#define GRID_HEIGHT_EASY 4
#define CARD_SIZE 50
#define FNV_PRIME 1099511628211UL
#define FNV_OFFSET 14695981039346656037UL

typedef enum { DIFFPAGE, GAMEPAGE } Screen;
typedef enum { EASY = 1, MEDIUM, HARD } Difficulty;

typedef struct {
  bool matched;
  bool revealed;
  int size;
  uint64_t key;
  Rectangle bounds;
  Color col;
} Card;

static uint64_t hashFNV(const char *key) {
  uint64_t hash = FNV_OFFSET;
  for (const char *p; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

void initCards(Card *cards, int size, int n, int startX, int startY, int gapX,
               int gapY) {
  size_t i;
  for (i = 0; i < n; i++) {

    int col = i % GRID_WIDTH_EASY;
    int row = i / GRID_WIDTH_EASY;

    cards[i].matched = false;
    cards[i].revealed = false;
    cards[i].key = 0;
    cards[i].col = SKYBLUE;
    cards[i].size = size;
    cards[i].bounds = (Rectangle){startX + col * (size + gapX),
                                  startY + row * (size + gapY), size, size};
  }
}

void drawGrid(Card *cards, int n) {
  size_t i;
  for (i = 0; i < n; i++) {
    if (cards[i].revealed) {
      DrawRectangleRec(cards[i].bounds, BLACK);
    } else {
      DrawRectangleRec(cards[i].bounds, cards[i].col);
    }
  }
}

void updateGrid(Card *cards, int n) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse = GetMousePosition();
    size_t i;
    for (i = 0; i < n; i++) {
      if (!cards[i].matched && !cards[i].revealed &&
          CheckCollisionPointRec(mouse, cards[i].bounds)) {

        cards[i].revealed = true;
        cards[i].col = BLACK;
      }
    }
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Memorex");
  SetTargetFPS(60);

  Card cards[MEM_HARD_SIZE];

  Difficulty currDiff = EASY;
  int gapX = 10;
  int gapY = 10;
  int startX = 200;
  int startY = 50;

  initCards(cards, CARD_SIZE, MEM_HARD_SIZE, startX, startY, gapX, gapY);

  while (!WindowShouldClose()) {
    updateGrid(cards, MEM_EASY_SIZE);
    BeginDrawing();
    ClearBackground(DARKGRAY);
    drawGrid(cards, MEM_EASY_SIZE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
