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
#define DIFF_CARD_HEIGHT 400
#define DIFF_CARD_WIDTH 200
#define DIFF_CARD_GAP 20
#define DIFF_CARD_POS                                                          \
  ((WINDOW_WIDTH / 2) - ((DIFF_CARD_WIDTH * 3 + DIFF_CARD_GAP * 2) / 2))
#define H1_SIZE 30
#define H2_SIZE 20

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
      texNames[index] = strdup(file->d_name);
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
            printf("Waiting\n");
            *waiting = true;
            *timer = 0.0f;
          }
        }
        break;
      }
    }
  }
}

void updateDiffPage(Rectangle *rec1, Rectangle *rec2, Rectangle *rec3,
                    Screen *page, Difficulty *diff) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, *rec1)) {
      *diff = EASY;
      *page = GAMEPAGE;
    } else if (CheckCollisionPointRec(mouse, *rec2)) {
      *diff = MEDIUM;
      *page = GAMEPAGE;
    } else if (CheckCollisionPointRec(mouse, *rec3)) {
      *diff = HARD;
      *page = GAMEPAGE;
    }
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Memorex");
  SetTargetFPS(60);

  Card *cards = (Card *)malloc(sizeof(Card) * MEM_HARD_SIZE);
  int firstCard = -1;
  int secondCard = -1;
  float revealTimer = 0.0f;
  bool waiting = false;
  Texture2D texArr[100];
  char *texNames[100];
  int score = 0;

  loadTextures(texArr, texNames);

  Difficulty currDiff = EASY;
  Screen currScreen = DIFFPAGE;
  int gapX = 10;
  int gapY = 10;
  int startX = 200;
  int startY = 50;

  Rectangle rec1 = {.x =
                        DIFF_CARD_GAP * 1 + DIFF_CARD_WIDTH * 0 + DIFF_CARD_POS,
                    .y = WINDOW_HEIGHT / 2.0 - DIFF_CARD_HEIGHT / 2.0,
                    .width = DIFF_CARD_WIDTH,
                    .height = DIFF_CARD_HEIGHT};

  Rectangle rec2 = {.x =
                        DIFF_CARD_GAP * 2 + DIFF_CARD_WIDTH * 1 + DIFF_CARD_POS,
                    .y = WINDOW_HEIGHT / 2.0 - DIFF_CARD_HEIGHT / 2.0,
                    .width = DIFF_CARD_WIDTH,
                    .height = DIFF_CARD_HEIGHT};

  Rectangle rec3 = {.x =
                        DIFF_CARD_GAP * 3 + DIFF_CARD_WIDTH * 2 + DIFF_CARD_POS,
                    .y = WINDOW_HEIGHT / 2.0 - DIFF_CARD_HEIGHT / 2.0,
                    .width = DIFF_CARD_WIDTH,
                    .height = DIFF_CARD_HEIGHT};

  initCards(cards, texArr, texNames, 12, CARD_SIZE, MEM_EASY_SIZE, startX,
            startY, gapX, gapY);

  const char *easy = "Easy";
  const char *medium = "Medium";
  const char *hard = "Hard";

  int easyTextWidth = MeasureText(easy, H1_SIZE);
  int mediumTextWidth = MeasureText(medium, H1_SIZE);
  int hardTextWidth = MeasureText(hard, H1_SIZE);

  while (!WindowShouldClose()) {

    switch (currScreen) {
    case DIFFPAGE:
      updateDiffPage(&rec1, &rec2, &rec3, &currScreen, &currDiff);
      BeginDrawing();
      DrawRectangleRec(rec1, GREEN);
      DrawRectangleRec(rec2, GREEN);
      DrawRectangleRec(rec3, GREEN);

      DrawRectangle(rec1.x + 5, rec1.y + 5, DIFF_CARD_WIDTH - 10,
                    DIFF_CARD_HEIGHT - 10, BLACK);
      DrawRectangle(rec2.x + 5, rec2.y + 5, DIFF_CARD_WIDTH - 10,
                    DIFF_CARD_HEIGHT - 10, BLACK);
      DrawRectangle(rec3.x + 5, rec3.y + 5, DIFF_CARD_WIDTH - 10,
                    DIFF_CARD_HEIGHT - 10, BLACK);

      DrawText(easy, (rec1.x + DIFF_CARD_WIDTH / 2.0) - (easyTextWidth / 2.0),
               (rec1.y + DIFF_CARD_HEIGHT / 2.0) - H1_SIZE, H1_SIZE, RAYWHITE);

      DrawText(medium,
               (rec2.x + DIFF_CARD_WIDTH / 2.0) - (mediumTextWidth / 2.0),
               (rec2.y + DIFF_CARD_HEIGHT / 2.0) - H1_SIZE, H1_SIZE, RAYWHITE);

      DrawText(hard, (rec3.x + DIFF_CARD_WIDTH / 2.0) - (hardTextWidth / 2.0),
               (rec3.y + DIFF_CARD_HEIGHT / 2.0) - H1_SIZE, H1_SIZE, RAYWHITE);
      EndDrawing();
      break;
    case GAMEPAGE:
      updateGrid(cards, MEM_EASY_SIZE, &score, &firstCard, &secondCard,
                 &revealTimer, &waiting);
      BeginDrawing();
      ClearBackground(DARKGRAY);
      drawGrid(cards, MEM_EASY_SIZE);
      EndDrawing();
      break;
    }
  }

  CloseWindow();
  free(cards);
  return 0;
}
