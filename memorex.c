#include "raylib.h"
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
  Rectangle bounds;
  char *texName;
  Texture2D *tex;
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

void loadTextures(Texture2D *texArr, char **texNames) {
  DIR *dirp;
  struct dirent *file;
  char *path = "./textures/";

  dirp = opendir(path);

  if (dirp == NULL) {
    fprintf(stderr, "Error: failed to open target directory\n");
  }

  size_t index = 0;
  while ((file = readdir(dirp))) {
    if (DT_REG == file->d_type) {
      printf("Loading texture %s\n", file->d_name);
      char buffer[100];
      snprintf(buffer, 100, "%s%s", path, file->d_name);
      texArr[index] = LoadTexture(buffer);
      texNames[index] = file->d_name;
      index++;
    }
  }

  closedir(dirp);
}

void initCards(Card *cards, Texture2D *texArr, char **texNames, int texSize,
               int size, int n, int startX, int startY, int gapX, int gapY) {
  size_t i;
  for (i = 0; i < n; i++) {

    int col = i % GRID_WIDTH_EASY;
    int row = i / GRID_WIDTH_EASY;

    cards[i].matched = false;
    cards[i].revealed = false;
    cards[i].col = SKYBLUE;
    cards[i].bounds = (Rectangle){startX + col * (size + gapX),
                                  startY + row * (size + gapY), size, size};

    cards[i].texName = texNames[i % texSize];
    cards[i].tex = &texArr[i % texSize];
  }
}

void drawGrid(Card *cards, int n) {
  size_t i;
  for (i = 0; i < n; i++) {
    if (cards[i].revealed || cards[i].matched) {
      Rectangle src = {0, 0, cards[i].tex->width, cards[i].tex->height};
      Rectangle dst = cards[i].bounds;
      DrawTexturePro(*cards[i].tex, src, dst, (Vector2){0, 0}, 0, WHITE);
    } else {
      DrawRectangleRec(cards[i].bounds, cards[i].col);
    }
  }
}

void updateGrid(Card *cards, int n, int *score, int *firstCard, int *secondCard,
                float *timer, bool *waiting) {

  float dt = GetFrameTime();
  if (*waiting) {
    *timer += dt;
    if (*timer >= 1.0f) {
      cards[*firstCard].revealed = false;
      cards[*secondCard].revealed = false;
      *firstCard = -1;
      *secondCard = -1;
      *waiting = false;
      *timer = 0.0f;
    }
    return;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse = GetMousePosition();
    size_t i;
    int revealedIndex = -1;

    for (i = 0; i < n; i++) {
      if (cards[i].revealed) {
        revealedIndex = i;
        break;
      }
    }

    for (i = 0; i < n; i++) {
      if (!cards[i].matched && !cards[i].revealed &&
          CheckCollisionPointRec(mouse, cards[i].bounds)) {

        cards[i].revealed = true;

        if (*firstCard == -1) {
          *firstCard = i;
        } else if (*secondCard == -1) {
          *secondCard = i;
          if (strcmp(cards[*firstCard].texName, cards[*secondCard].texName) ==
              0) {
            cards[*firstCard].matched = true;
            cards[*secondCard].matched = true;

            *firstCard = -1;
            *secondCard = -1;

            if (++(*score) >= n / 2) {
              printf("You won\n");
            }
          } else {
            *waiting = true;
            *timer = 0.0f;
          }
        }
        break;
      }
    }
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Memorex");
  SetTargetFPS(60);

  Card *cards = (Card *)malloc(sizeof(Card) * MEM_EASY_SIZE);
  int firstCard = -1;
  int secondCard = -1;
  float revealTimer = 0.0f;
  bool waiting = false;
  Texture2D texArr[100];
  char *texNames[100];
  int score = 0;

  loadTextures(texArr, texNames);

  Difficulty currDiff = EASY;
  int gapX = 10;
  int gapY = 10;
  int startX = 200;
  int startY = 50;

  initCards(cards, texArr, texNames, 2, CARD_SIZE, MEM_EASY_SIZE, startX,
            startY, gapX, gapY);

  while (!WindowShouldClose()) {
    updateGrid(cards, MEM_EASY_SIZE, &score, &firstCard, &secondCard,
               &revealTimer, &waiting);
    BeginDrawing();
    ClearBackground(DARKGRAY);
    drawGrid(cards, MEM_EASY_SIZE);
    EndDrawing();
  }

  CloseWindow();
  free(cards);
  return 0;
}
