#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cmath>

#define SCREEN_WIDTH 800 // Increased screen width
#define SCREEN_HEIGHT 600 // Increased screen height
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 10
#define BALL_SIZE 10
#define BRICK_WIDTH 50
#define BRICK_HEIGHT 20
#define NUM_BRICKS_ROW 16 // Adjusted number of bricks to fit screen width
#define NUM_BRICKS_COL 6

using namespace std;

struct Ball {
    int x, y;
    int dirX, dirY;
    int color;
};

int paddleX, paddleY;
int paddleAngle = 0;
vector<Ball> balls;
int bricks[NUM_BRICKS_ROW][NUM_BRICKS_COL];
int gameOver = 0;
int paddleWidth = PADDLE_WIDTH;

void drawBricks() {
    for (int i = 0; i < NUM_BRICKS_ROW; i++) {
        for (int j = 0; j < NUM_BRICKS_COL; j++) {
            if (bricks[i][j]) {
                setfillstyle(SOLID_FILL, LIGHTBLUE);
                bar(i * BRICK_WIDTH, j * BRICK_HEIGHT, (i + 1) * BRICK_WIDTH - 1, (j + 1) * BRICK_HEIGHT - 1);
            }
        }
    }
}

void drawPaddle() {
    int paddleCenterX = paddleX + paddleWidth / 2;
    int paddleCenterY = paddleY + PADDLE_HEIGHT / 2;
    double angleRad = paddleAngle * M_PI / 180.0;

    int x1 = paddleCenterX - (paddleWidth / 2) * cos(angleRad) + (PADDLE_HEIGHT / 2) * sin(angleRad);
    int y1 = paddleCenterY - (paddleWidth / 2) * sin(angleRad) - (PADDLE_HEIGHT / 2) * cos(angleRad);
    int x2 = paddleCenterX + (paddleWidth / 2) * cos(angleRad) + (PADDLE_HEIGHT / 2) * sin(angleRad);
    int y2 = paddleCenterY + (paddleWidth / 2) * sin(angleRad) - (PADDLE_HEIGHT / 2) * cos(angleRad);
    int x3 = paddleCenterX + (paddleWidth / 2) * cos(angleRad) - (PADDLE_HEIGHT / 2) * sin(angleRad);
    int y3 = paddleCenterY + (paddleWidth / 2) * sin(angleRad) + (PADDLE_HEIGHT / 2) * cos(angleRad);
    int x4 = paddleCenterX - (paddleWidth / 2) * cos(angleRad) - (PADDLE_HEIGHT / 2) * sin(angleRad);
    int y4 = paddleCenterY - (paddleWidth / 2) * sin(angleRad) + (PADDLE_HEIGHT / 2) * cos(angleRad);

    setfillstyle(SOLID_FILL, WHITE);
    fillpoly(4, new int[8]{x1, y1, x2, y2, x3, y3, x4, y4});
}

void drawBalls() {
    for (auto &ball : balls) {
        setfillstyle(SOLID_FILL, ball.color);
        fillellipse(ball.x, ball.y, BALL_SIZE, BALL_SIZE);
    }
}

void moveBalls() {
    for (auto &ball : balls) {
        ball.x += (2 * ball.dirX);
        ball.y += (4 * ball.dirY);

        if (ball.x <= BALL_SIZE || ball.x >= SCREEN_WIDTH - BALL_SIZE) ball.dirX = -ball.dirX;
        if (ball.y <= BALL_SIZE) ball.dirY = -ball.dirY;

        int relativeHitX = ball.x - paddleX;
        if (ball.y >= paddleY - BALL_SIZE && relativeHitX >= 0 && relativeHitX <= paddleWidth) {
            ball.dirY = -ball.dirY;
            ball.dirX += static_cast<int>(paddleAngle / 10);
        }

        int brickX = ball.x / BRICK_WIDTH;
        int brickY = ball.y / BRICK_HEIGHT;

        if (brickX >= 0 && brickX < NUM_BRICKS_ROW && brickY >= 0 && brickY < NUM_BRICKS_COL && bricks[brickX][brickY]) {
            bricks[brickX][brickY] = 0;
            ball.dirY = -ball.dirY;
        }

        if (ball.y > SCREEN_HEIGHT) {
            gameOver = (balls.size() == 1);
            if (balls.size() > 1) balls.erase(balls.begin() + (&ball - &balls[0]));
        }
    }
}

void initGame() {
    srand(time(NULL));
    paddleX = SCREEN_WIDTH / 2 - paddleWidth / 2;
    paddleY = SCREEN_HEIGHT - 30;

    Ball initialBall = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 2, 3, RED};
    balls.push_back(initialBall);

    for (int i = 0; i < NUM_BRICKS_ROW; i++) {
        for (int j = 0; j < NUM_BRICKS_COL; j++) {
            bricks[i][j] = 1;
        }
    }
}

void displayStartScreen() {
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);

    const char* line1 = "PRESS ANY KEY TO START";
    const char* line2 = "USE LEFT/RIGHT ARROW KEYS TO MOVE PADDLE";
    const char* line3 = "PRESS A/D TO ROTATE PADDLE";

    int textWidth1 = textwidth((char*)line1);
    int textWidth2 = textwidth((char*)line2);
    int textWidth3 = textwidth((char*)line3);

    int screenCenterX = SCREEN_WIDTH / 2;
    int screenCenterY = SCREEN_HEIGHT / 2;

    outtextxy(screenCenterX - textWidth1 / 2, screenCenterY - 50, (char*)line1);
    outtextxy(screenCenterX - textWidth2 / 2, screenCenterY, (char*)line2);
    outtextxy(screenCenterX - textWidth3 / 2, screenCenterY + 30, (char*)line3);
}

void displayGameOver() {
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, (char*)"GAME OVER!");
}

int main() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");
    initwindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    displayStartScreen();
    getch();

    initGame();

    while (!gameOver) {
        cleardevice();
        drawBricks();
        drawPaddle();
        drawBalls();
        moveBalls();

        if (GetAsyncKeyState(VK_LEFT) && paddleX > 0) paddleX -= 10;
        if (GetAsyncKeyState(VK_RIGHT) && paddleX < SCREEN_WIDTH - paddleWidth) paddleX += 10;

        if (GetAsyncKeyState('D')) {
            paddleAngle += 5;
            if (paddleAngle > 15) paddleAngle = 15;
        }
        if (GetAsyncKeyState('A')) {
            paddleAngle -= 5;
            if (paddleAngle < -15) paddleAngle = -15;
        }

        delay(33);
    }

    cleardevice();
    displayGameOver();
    delay(1000);
    closegraph();
    return 0;
}
