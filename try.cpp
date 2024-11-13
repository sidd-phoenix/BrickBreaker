#include <graphics.h>
#include<algorithm>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cmath>

#define _USE_MATH_DEFINES
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BRICK_WIDTH 50
#define BRICK_HEIGHT 20
#define NUM_BRICKS_ROW 16
#define NUM_BRICKS_COL 6
#define STAR_SIZE 20

#define MIN_PADDLE_WIDTH 50
#define MAX_PADDLE_WIDTH 150
#define MIN_BALL_SIZE 5
#define MAX_BALL_SIZE 20
#define MIN_PADDLE_ANGLE -15
#define MAX_PADDLE_ANGLE 15

#define CYAN COLOR(0, 255, 255)
#define VIOLET COLOR(148, 0, 211)
#define CREAM COLOR(255, 253, 208)

// Define colors for each power-up type
#define BALL_COLOR CYAN
#define ENLARGE_BALL_COLOR COLOR(0, 255, 0)      // Green
#define SHRINK_BALL_COLOR COLOR(255, 0, 0)       // Red
#define ENLARGE_PADDLE_COLOR COLOR(0, 0, 255)    // Blue
#define SHRINK_PADDLE_COLOR COLOR(255, 165, 0)   // Orange
#define MULTIPLY_BALLS_COLOR COLOR(128, 0, 128)  // Purple

using namespace std;

struct Ball {
    int x, y;
    int dirX, dirY;
    int color;
    int size;
};

struct PowerUp {
    int x, y;
    int size;
    int type; // 0: Enlarge Ball, 1: Shrink Ball, 2: Enlarge Paddle, 3: Shrink Paddle, 4: Multiply Balls
};


void drawBricks();
void drawPaddle();
void drawBalls();
void moveBalls();
void rotatePaddle(int angleChange);
void resizePaddle(int widthChange);
void resizeBall(int sizeChange);
void spawnPowerUp();
void movePowerUps();
void drawPowerUps();
void drawScore();
void displayStartScreen();
void drawShearedTextWithBox(int x, int y, const char* text, int shearFactor);
void drawShearedRectangle(int x, int y, int width, int height, double shear);
void displayGameOver();
void displayVictory();
bool checkVictory();
void initGame();
void drawStar(int centerX, int centerY, int size, int color);
int computeOutCode(int x, int y);
bool cohenSutherlandClip(int& x0, int& y0, int& x1, int& y1);


int clipXMin = 0, clipYMin = 200;  // Top-left corner of clipping window
int clipXMax = 800, clipYMax = 600;  // Bottom-right corner of clipping window
int activePage = 0; // Page for double buffering

int paddleWidth = 100;
int paddleHeight = 10;
int ballBaseSize = 10;
int powerupSize = 10;
int numPowerUps = 5;
int powerupFallSpeed=5;
int paddleX, paddleY;
int paddleAngle = 0;
int paddleSpeed = 10;           // Paddle movement speed
int angleChange = 5;            // Rotation angle change
int paddleSizeChange = 15;      // Paddle size change increment
int ballSizeChange = 3;         // Ball size change increment
int ballSpeedX = 5;             // Horizontal speed for balls
int ballSpeedY = 8;             // Vertical speed for balls
int powerUpSpawnInterval = 100 ; // Frames between power-up spawns
int powerUpFrameCounter = 0;
int shearFrames = 10;            // Controls the duration of the shear effect
int gameOver = 0;           //flag to see game over
int score=0;



const int INSIDE = 0;  // 0000
const int LEFT = 1;    // 0001
const int RIGHT = 2;   // 0010
const int BOTTOM = 4;  // 0100
const int TOP = 8;     // 1000



vector<Ball> balls;
vector<PowerUp> powerUps;
int bricks[NUM_BRICKS_ROW][NUM_BRICKS_COL];



int main() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");
    initwindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    displayStartScreen();
    getch();

    initGame();


    while (!gameOver) {
        setactivepage(activePage);
        setvisualpage(1 - activePage); // Show the other page
        cleardevice();

        drawBricks();
        drawBalls();
        drawPaddle();
        drawScore();
        drawPowerUps();
        movePowerUps();
        moveBalls();

        if(checkVictory()){
            break;
        }

        // Spawn power-ups periodically
        powerUpFrameCounter++;
        if (powerUpFrameCounter >= powerUpSpawnInterval) {
            spawnPowerUp();
            powerUpFrameCounter = 0;
        }


        // Paddle movement
        if (GetAsyncKeyState(VK_LEFT) && paddleX > 0) paddleX -= paddleSpeed;
        if (GetAsyncKeyState(VK_RIGHT) && paddleX < SCREEN_WIDTH - paddleWidth) paddleX += paddleSpeed;

        // Paddle rotation
        if (GetAsyncKeyState('D')) rotatePaddle(angleChange);
        if (GetAsyncKeyState('A')) rotatePaddle(-angleChange);

        // // Paddle size adjustment
        // if (GetAsyncKeyState('I')) resizePaddle(paddleSizeChange);
        // if (GetAsyncKeyState('K')) resizePaddle(-paddleSizeChange);

        // // Ball size adjustment
        // if (GetAsyncKeyState('O')) resizeBall(ballSizeChange);
        // if (GetAsyncKeyState('L')) resizeBall(-ballSizeChange);

        activePage = 1 - activePage; // Switch the active page
        delay(30);
    }

    if(!gameOver){
        setactivepage(1);
        displayVictory();
    }
    else{
        setvisualpage(1);
        displayGameOver();
    }
    delay(1000);

    closegraph();
    return 0;
}


void drawBricks() {
    for (int i = 0; i < NUM_BRICKS_ROW; i++) {
        for (int j = 0; j < NUM_BRICKS_COL; j++) {
            if (bricks[i][j]) {
                int color = COLOR(255 - (i * 15), 100 + (j * 20), 100 + (i * 10)); // Vibrant color variation
                setfillstyle(SOLID_FILL, color);
                bar(i * BRICK_WIDTH, j * BRICK_HEIGHT, (i + 1) * BRICK_WIDTH - 1, (j + 1) * BRICK_HEIGHT - 1);
            }
        }
    }
}

void drawPaddle() {
    int paddleCenterX = paddleX + paddleWidth / 2;
    int paddleCenterY = paddleY + paddleHeight / 2;
    double angleRad = paddleAngle * M_PI / 180.0;

    int x1 = paddleCenterX - (paddleWidth / 2) * cos(angleRad) + (paddleHeight / 2) * sin(angleRad);
    int y1 = paddleCenterY - (paddleWidth / 2) * sin(angleRad) - (paddleHeight / 2) * cos(angleRad);
    int x2 = paddleCenterX + (paddleWidth / 2) * cos(angleRad) + (paddleHeight / 2) * sin(angleRad);
    int y2 = paddleCenterY + (paddleWidth / 2) * sin(angleRad) - (paddleHeight / 2) * cos(angleRad);
    int x3 = paddleCenterX + (paddleWidth / 2) * cos(angleRad) - (paddleHeight / 2) * sin(angleRad);
    int y3 = paddleCenterY + (paddleWidth / 2) * sin(angleRad) + (paddleHeight / 2) * cos(angleRad);
    int x4 = paddleCenterX - (paddleWidth / 2) * cos(angleRad) - (paddleHeight / 2) * sin(angleRad);
    int y4 = paddleCenterY - (paddleWidth / 2) * sin(angleRad) + (paddleHeight / 2) * cos(angleRad);

    setfillstyle(SOLID_FILL, CREAM); // Set paddle color to cream
    fillpoly(4, new int[8]{x1, y1, x2, y2, x3, y3, x4, y4});
}

void drawBalls() {
    for (auto &ball : balls) {
        setfillstyle(SOLID_FILL, BALL_COLOR); // Center dark purple
        fillellipse(ball.x, ball.y, ball.size, ball.size);

        setcolor(VIOLET); // Outer violet ring
        circle(ball.x, ball.y, ball.size);
    }
}

void moveBalls() {
    for (auto &ball : balls) {
        ball.x += ballSpeedX * ball.dirX;
        ball.y += ballSpeedY * ball.dirY;

        if (ball.x <= ball.size || ball.x >= SCREEN_WIDTH - ball.size) ball.dirX = -ball.dirX;
        if (ball.y <= ball.size) ball.dirY = -ball.dirY;

        int relativeHitX = ball.x - paddleX;
        if (ball.y >= paddleY - ball.size && relativeHitX >= 0 && relativeHitX <= paddleWidth) {
            ball.dirY = -ball.dirY;
            ball.dirX += static_cast<int>(paddleAngle / 10);
        }

        int brickX = ball.x / BRICK_WIDTH;
        int brickY = ball.y / BRICK_HEIGHT;

        if (brickX >= 0 && brickX < NUM_BRICKS_ROW && brickY >= 0 && brickY < NUM_BRICKS_COL && bricks[brickX][brickY]) {
            bricks[brickX][brickY] = 0;
            ball.dirY = -ball.dirY;
            score+=10;
        }

        if (ball.y > SCREEN_HEIGHT) {
            if (balls.size() > 1) {
                balls.erase(balls.begin() + (&ball - &balls[0]));
            } else {
                gameOver = 1; // Only set game over if no balls are left
            }
        }
    }
}


void rotatePaddle(int angleChange) {
    paddleAngle = max(MIN_PADDLE_ANGLE, min(MAX_PADDLE_ANGLE, paddleAngle + angleChange));
}

void resizePaddle(int widthChange) {
    paddleWidth = max(MIN_PADDLE_WIDTH, min(MAX_PADDLE_WIDTH, paddleWidth + widthChange));
}

void resizeBall(int sizeChange) {
    ballBaseSize = max(MIN_BALL_SIZE, min(MAX_BALL_SIZE, ballBaseSize + sizeChange));
    for (auto &ball : balls) {
        ball.size = ballBaseSize;
    }
}


void spawnPowerUp() {
    PowerUp newPowerUp;
    newPowerUp.x = rand() % SCREEN_WIDTH;
    newPowerUp.y = 0; // Start from the top
    newPowerUp.size = powerupSize;
    newPowerUp.type = rand() % numPowerUps; // Random power-up type

    powerUps.push_back(newPowerUp);
}

void movePowerUps() {
    for (auto& powerUp : powerUps) {
        powerUp.y += powerupFallSpeed; // Move the power-up down

        // Check if the power-up hits the paddle
        if (powerUp.y >= paddleY - powerUp.size && powerUp.x >= paddleX && powerUp.x <= paddleX + paddleWidth) {
            if (powerUp.type == 0) { // Enlarge Ball
                resizeBall(ballSizeChange);
            } else if (powerUp.type == 1) { // Shrink Ball
                resizeBall(-ballSizeChange);
            } else if (powerUp.type == 2) { // Enlarge Paddle
                resizePaddle(paddleSizeChange);
            } else if (powerUp.type == 3) { // Shrink Paddle
                resizePaddle(-paddleSizeChange);
            } else if (powerUp.type == 4) { // Multiply Balls
                if (balls.size() > 0) {
                    Ball newBall = balls[0]; // Clone the first ball
                    newBall.x = rand() % SCREEN_WIDTH;
                    newBall.y = SCREEN_HEIGHT / 2;
                    balls.push_back(newBall);
                }
            }
            powerUp.y = SCREEN_HEIGHT + 10; // Move out of view
        }
    }

    // Remove power-ups that are off the screen
    powerUps.erase(
        remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& powerUp) {
            return powerUp.y > SCREEN_HEIGHT;  // Remove if the power-up has moved off the screen
        }),
        powerUps.end()
    );
}

void drawPowerUps() {
    for (const auto &powerUp : powerUps) {
        int x0 = powerUp.x - powerUp.size;
        int y0 = powerUp.y - powerUp.size;
        int x1 = powerUp.x + powerUp.size;
        int y1 = powerUp.y + powerUp.size;

        // Apply Cohen-Sutherland clipping
        if (cohenSutherlandClip(x0, y0, x1, y1)) {
            int color;
            switch (powerUp.type) {
                case 0: color = ENLARGE_BALL_COLOR; break;
                case 1: color = SHRINK_BALL_COLOR; break;
                case 2: color = ENLARGE_PADDLE_COLOR; break;
                case 3: color = SHRINK_PADDLE_COLOR; break;
                case 4: color = MULTIPLY_BALLS_COLOR; break;
            }

            setfillstyle(SOLID_FILL, color);
            fillellipse((x0 + x1) / 2, (y0 + y1) / 2, (x1 - x0) / 2, (y1 - y0) / 2);
        }
    }
}



void drawScore() {
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2); // Set font style and size
    setcolor(WHITE); // Set text color
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    outtextxy(10, 10, scoreText); // Display the score at the top-left corner
}

void displayStartScreen() {
    setcolor(YELLOW);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);

    const char* line1 = "PRESS ANY KEY TO START";
    const char* line2 = "USE LEFT/RIGHT ARROW KEYS TO MOVE PADDLE";
    const char* line3 = "PRESS A/D TO ROTATE PADDLE";
    // const char* line4 = "PRESS I/K TO INCREASE/DECREASE PADDLE SIZE";
    // const char* line5 = "PRESS O/L TO INCREASE/DECREASE BALL SIZE";

    int screenCenterX = SCREEN_WIDTH / 2;
    int screenCenterY = SCREEN_HEIGHT / 2;

    outtextxy(screenCenterX - textwidth((char*)line1) / 2, screenCenterY - 80, (char*)line1);
    outtextxy(screenCenterX - textwidth((char*)line2) / 2, screenCenterY - 40, (char*)line2);
    outtextxy(screenCenterX - textwidth((char*)line3) / 2, screenCenterY, (char*)line3);
    // outtextxy(screenCenterX - textwidth((char*)line4) / 2, screenCenterY + 40, (char*)line4);
    // outtextxy(screenCenterX - textwidth((char*)line5) / 2, screenCenterY + 80, (char*)line5);

    // Display Legend
    setcolor(ENLARGE_BALL_COLOR);
    outtextxy(screenCenterX - 200, screenCenterY + 100, (char*)"Green - Enlarge Ball");
    setcolor(SHRINK_BALL_COLOR);
    outtextxy(screenCenterX - 200, screenCenterY + 120, (char*)"Red - Shrink Ball");
    setcolor(ENLARGE_PADDLE_COLOR);
    outtextxy(screenCenterX - 200, screenCenterY + 140, (char*)"Blue - Enlarge Paddle");
    setcolor(SHRINK_PADDLE_COLOR);
    outtextxy(screenCenterX - 200, screenCenterY + 160, (char*)"Orange - Shrink Paddle");
    setcolor(MULTIPLY_BALLS_COLOR);
    outtextxy(screenCenterX - 200, screenCenterY + 180, (char*)"Purple - Double Balls");
}

void drawShearedTextWithBox(int x, int y, const char* text, int shearFactor) {
    int textWidth = ::textwidth((char*)text);
    int textHeight = ::textheight((char*)text);

    int x1 = x - 10, y1 = y - 10;
    int x2 = x + textWidth + 10, y2 = y + textHeight + 10;

    line(x1, y1, x2 + shearFactor * textHeight, y1);
    line(x2 + shearFactor * textHeight, y1, x2, y2);
    line(x2, y2, x1 - shearFactor * textHeight, y2);
    line(x1 - shearFactor * textHeight, y2, x1, y1);

    // Draw sheared text character by character
    int currentX = x;
    for (int i = 0; text[i] != '\0'; i++) {
        char singleChar[2] = {text[i], '\0'};
        int xOffset = shearFactor * (i * textHeight) / 10;
        outtextxy(currentX + xOffset, y, singleChar);
        currentX += textwidth(singleChar);
    }
}

void drawShearedRectangle(int x, int y, int width, int height, double shear) {
    int x1 = x, y1 = y;
    int x2 = x + width, y2 = y;
    int x3 = x + width + shear * height, y3 = y + height;
    int x4 = x + shear * height, y4 = y + height;

    line(x1, y1, x2, y2);
    line(x2, y2, x3, y3);
    line(x3, y3, x4, y4);
    line(x4, y4, x1, y1);
}

void displayGameOver() {
    setcolor(RED);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);

    const char* text = "GAME OVER!";
    int textWidth = textwidth((char*)text);
    int textHeight = textheight((char*)text);
    
    int centerX = SCREEN_WIDTH / 2 - textWidth / 2;
    int centerY = SCREEN_HEIGHT / 2 - textHeight / 2;

    // Increase shear effect frame by frame
    for (shearFrames = 0; shearFrames < 20; shearFrames++) {
        cleardevice();
        // Calculate shear effect, gradually increase over time
        double shear = 0.02 * shearFrames;
        
        // Draw sheared rectangle and text
        drawShearedRectangle(centerX - 20, centerY - 10, textWidth + 40, textHeight + 20, shear);
        outtextxy(centerX, centerY, (char*)text);
        delay(30); // Add a small delay to create animation effect
    }

    // int starX=centerX-STAR_SIZE*2;
    // int starY=centerY-STAR_SIZE*2;
    
    // drawStar(starX,starY,STAR_SIZE,WHITE);
    // drawStar(starX+centerX,starY,STAR_SIZE,WHITE);

}

void displayVictory() {
    setcolor(GREEN);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);

    const char* text = "YOU WIN!";
    int textWidth = textwidth((char*)text);
    int textHeight = textheight((char*)text);

    int centerX = SCREEN_WIDTH / 2 - textWidth / 2;
    int centerY = SCREEN_HEIGHT / 2 - textHeight / 2;

    // Increase shear effect frame by frame
    for (shearFrames = 0; shearFrames < 20; shearFrames++) {
        cleardevice();
        double shear = 0.02 * shearFrames;
        
        drawShearedRectangle(centerX - 20, centerY - 10, textWidth + 40, textHeight + 20, shear);
        outtextxy(centerX, centerY, (char*)text);
        delay(30); // Add a small delay to create animation effect
    } 
    
    // int starX=centerX-STAR_SIZE*2;
    // int starY=centerY-STAR_SIZE*2;

    // drawStar(starX,starY,STAR_SIZE,WHITE);
    // drawStar(starX+centerX,starY,STAR_SIZE,WHITE);
}

bool checkVictory() {
    for (int i = 0; i < NUM_BRICKS_ROW; i++) {
        for (int j = 0; j < NUM_BRICKS_COL; j++) {
            if (bricks[i][j] != 0) {
                return false;
            }
        }
    }
    return true;
}

void initGame() {
    srand(time(NULL));
    paddleX = SCREEN_WIDTH / 2 - paddleWidth / 2;
    paddleY = SCREEN_HEIGHT - 30;

    Ball initialBall = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1, 1, RED, ballBaseSize};
    balls.push_back(initialBall);

    for (int i = 0; i < NUM_BRICKS_ROW; i++) {
        for (int j = 0; j < NUM_BRICKS_COL; j++) {
            bricks[i][j] = 1;
        }
    }
}

void drawStar(int centerX, int centerY, int size, int color) {
    double angle = 72 * M_PI / 180; // 72 degrees in radians
    int points[10];

    for (int i = 0; i < 5; i++) {
        points[i * 2] = centerX + size * cos(i * 2 * angle);
        points[i * 2 + 1] = centerY - size * sin(i * 2 * angle);
    }

    setcolor(color);
    setfillstyle(SOLID_FILL, color);
    fillpoly(5, points); // Draw star
}

int computeOutCode(int x, int y) {
    int code = INSIDE;

    if (x < clipXMin) code |= LEFT;
    else if (x > clipXMax) code |= RIGHT;
    if (y < clipYMin) code |= TOP;
    else if (y > clipYMax) code |= BOTTOM;

    return code;
}

bool cohenSutherlandClip(int& x0, int& y0, int& x1, int& y1) {
    int outcode0 = computeOutCode(x0, y0);
    int outcode1 = computeOutCode(x1, y1);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) { 
            accept = true;
            break;
        } else if (outcode0 & outcode1) { 
            break;
        } else {
            int x, y;
            int outcodeOut = outcode0 ? outcode0 : outcode1;

            if (outcodeOut & TOP) {
                x = x0 + (x1 - x0) * (clipYMin - y0) / (y1 - y0);
                y = clipYMin;
            } else if (outcodeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (clipYMax - y0) / (y1 - y0);
                y = clipYMax;
            } else if (outcodeOut & RIGHT) {
                y = y0 + (y1 - y0) * (clipXMax - x0) / (x1 - x0);
                x = clipXMax;
            } else if (outcodeOut & LEFT) {
                y = y0 + (y1 - y0) * (clipXMin - x0) / (x1 - x0);
                x = clipXMin;
            }

            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = computeOutCode(x0, y0);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = computeOutCode(x1, y1);
            }
        }
    }
    return accept;
}