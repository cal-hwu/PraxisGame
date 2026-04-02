#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

//Written by Cal Dempster 2026
//All code is written by me, but I used the raylib library for the graphics and input handling
//Examples from raylib were used for reference and guidance, but the code is my own


//Pre processing
#define MAX_OBSTACLES 90
#define OBSTACLES_WIDTH 50
#define OBSTACLES_HEIGHT 50

//ALL OF THE STRUCTS AND VARIABLES NEEDED FOR THE GAME

typedef struct Player {
    Vector2 position;
    Color color;
    Rectangle rec;
} Player;

typedef struct Floor {
    Vector2 position;
    Color color;
    Rectangle rec;
} Floor;

typedef struct Obstacles {
    Rectangle rec;
    Color color;
    bool active;
} Obstacles;

//Screen dimensions
static const int screenWidth = 1000;
static const int screenHeight = 800;

//Textures / Images (Images needed for per-pixel alpha checks)
static Texture2D playerTexture;
static Image playerImage;
static Texture2D backgroundTexture;
static Texture2D obstacleOneTexture;
static Image obstacleOneImage;
static Texture2D floorTexture;

//Widths
static unsigned int frameWidth = 200;
static unsigned int frameHeight = 300;

//Animation
static int currentFrame = 0;
static float frameTime = 0.0f;
static float frameSpeed = 0.2f; // seconds per frame

//Menu
static bool gameOver = false;
static bool pause = false;
//make sure menu is true when actually playing
//For gameplay testing this is easier
static bool menu = true;
static bool helpScreen = false;
static unsigned int score = 0;
static unsigned int hiScore = 0;

//Text sizes 
const unsigned int mainTextSize = 40;
const unsigned int subTextSize = 20;
const unsigned int titleFontSize = 70;
const unsigned int menuFontSize = 20;

//Positioning text (initialized at runtime after window created)
static int centerX = 0;
static int centerY = 0;
static int titleY = 0;

//Game play
//FLOOR
static int floorHeight = 300;
static int floorWidth = 1000;

//Creating structs for player, floor and obstacles
static Player player = { 0 };
static Floor floor = { 0 };
static Obstacles obstacles[MAX_OBSTACLES] = { 0 };
static Vector2 obstaclesPos[MAX_OBSTACLES] = { 0 };
static int obstaclesSpeedX = 0;

//Scaling
static float scale = 2.0f;

//PLAYER
static unsigned int playerWidth;
static unsigned int playerHeight;


//Physics 
static const float gravity = 0.5f;
static bool isJumping = true;
static const float jumpForce = 20;

//game functions
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload 
static void UpdateDrawFrame(void);  // Update and Draw (one frame)




int main(void)
{
    //initating the window and putting the title
    InitWindow(screenWidth, screenHeight, "Jumper Game");

    // initialize centered positions now that window exists
    centerX = GetScreenWidth() / 2;
    centerY = GetScreenHeight() / 2;
    titleY = GetScreenHeight() / 4;

	//Sets the target FPS to 60 and starts the game loop
    InitGame();
    SetTargetFPS(60);
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
	//Unload all loaded resources and close the window
    UnloadGame();
    CloseWindow();
    return 0;
}

void InitGame(void)
{
	//Sets the player to the correct size based on the frame dimensions and scale
    playerWidth = frameWidth * scale;
    playerHeight = frameHeight * scale;

    //Initialise player
    player.position = (Vector2){ 80 + 30, screenHeight / 2 };
    player.rec.height = playerHeight;
    player.rec.width = playerWidth;
    isJumping = true;

    //Initialise floor
    floor.position = (Vector2){ 0, screenHeight / 1.5 };
    floor.rec.height = floorHeight;
    floor.rec.width = floorWidth;

    //Initialise obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
		//random values to have the obstacles appear at different heights and distances, but still offscreen at the start
        obstaclesPos[i].x = GetRandomValue(400, 500) * i;
        obstaclesPos[i].y = floor.position.y - GetRandomValue(50, 100);
    }

    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
		//Set the obstacle rectangles based on the positions and defined width/height
        obstacles[i].rec.x = obstaclesPos[i].x;
        obstacles[i].rec.y = obstaclesPos[i].y;
        obstacles[i].rec.width = OBSTACLES_WIDTH;
        obstacles[i].rec.height = 50 ;

        obstacles[i].active = true;
    }

	//Setting score to 0 at the start of the game
    score = 0;

	//these should already be true but just to be sure they are set correctly at the start of the game
    gameOver = false;
    pause = false;

    //LOADING ASSETS AND ANIMTION
    backgroundTexture = LoadTexture("resources/background.png");

    // Load image + texture for player (we need Image for pixel checks)
    playerImage = LoadImage("resources/main_sprite.png");
    playerTexture = LoadTextureFromImage(playerImage);

    // Load obstacle image + texture and keep image for pixel checks
    obstacleOneImage = LoadImage("resources/obstacleOne.png");
    obstacleOneTexture = LoadTextureFromImage(obstacleOneImage);

    floorTexture = LoadTexture("resources/floor.png");

    // frameWidth/Height: frames are stacked vertically (2 frames)
    frameWidth = playerTexture.width;
    frameHeight = playerTexture.height / 2;
}

void UpdateGame(void)
{
	// Handle input for pausing and jumping, update player animation, move obstacles, apply gravity, and check collisions.
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            //ANIMATING THE PLAYER
            frameTime += GetFrameTime();

            if (frameTime >= frameSpeed)
            {
                frameTime = 0.0f;
                currentFrame++;

                if (currentFrame >= 2)
                    currentFrame = 0;
            }

            //brings the obstacles closer
            for (int i = 0; i < MAX_OBSTACLES; i++) obstaclesPos[i].x -= obstaclesSpeedX;

            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                obstacles[i].rec.x = obstaclesPos[i].x;
            }

            static float speedToGoDown = 0;

            //gravity
            speedToGoDown += gravity;
            player.position.y += speedToGoDown;


            player.rec.x = player.position.x;
            player.rec.y = player.position.y;

            floor.rec.x = floor.position.x;
            floor.rec.y = floor.position.y;

            // Check collision with floor
            if (CheckCollisionRecs(player.rec, floor.rec))
            {
                // Snap player to top of floor using the rectangle height (already scaled)
                player.position.y = floor.position.y - player.rec.height;
                speedToGoDown = 0;
                isJumping = false;
            }
            else
            {
                isJumping = true;
            }

            // Check collisions using rectangle overlap only (end game when rects overlap)
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                if (CheckCollisionRecs(player.rec, obstacles[i].rec))
                {
                    gameOver = true;
                    pause = false;
                    break; // stop checking further obstacles
                }

                // scoring when obstacle passed (keep this behavior)
                if ((obstaclesPos[i].x < player.position.x) && obstacles[i].active && !gameOver)
                {
                    score += 100;
                    obstacles[i].active = false;

                    if (score > hiScore) hiScore = score;
                }
            }
			//using space to jump, only if not already jumping (prevents double jumps)
            if (IsKeyPressed(KEY_SPACE) && !isJumping)
            {
                speedToGoDown -= jumpForce; // jump impulse
                isJumping = true;
            }
        }
    }
}

void MainMenu(void)
{
    // Unified menu drawing and input handling with proper vertical spacing.
    ClearBackground(RAYWHITE);

    if (!helpScreen)
    {
        // Title (centered)
        DrawText("JUMPER", centerX - MeasureText("JUMPER", titleFontSize) / 2, titleY - titleFontSize / 2, titleFontSize, BLACK);

        // Menu items with consistent spacing (centered)
        int y = titleY + 40;
        const int spacing = 36;

		//spacing is a bit tight but it looks better than the alternatives, and the menu isn't too long
        DrawText("PRESS [1] TO PLAY EASY MODE", centerX - MeasureText("PRESS [1] TO PLAY EASY MODE", menuFontSize) / 2, y, menuFontSize, GRAY);
        y += spacing;
        DrawText("PRESS [2] TO PLAY MEDIUM MODE", centerX - MeasureText("PRESS [2] TO PLAY MEDIUM MODE", menuFontSize) / 2, y, menuFontSize, GRAY);
        y += spacing;
        DrawText("PRESS [3] TO PLAY HARD MODE", centerX - MeasureText("PRESS [3] TO PLAY HARD MODE", menuFontSize) / 2, y, menuFontSize, GRAY);
        y += spacing;
        DrawText("PRESS [H] FOR HELP", centerX - MeasureText("PRESS [H] FOR HELP", menuFontSize) / 2, y, menuFontSize, GRAY);
        y += spacing;

   
        // Input handling
        if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_THREE))
        {
            if (IsKeyPressed(KEY_ONE))
            {
                obstaclesSpeedX = 2;
            }
            else if (IsKeyPressed(KEY_TWO))
            {
                obstaclesSpeedX = 5;
            }
            else if (IsKeyPressed(KEY_THREE))
            {
                obstaclesSpeedX = 10;
            }

            menu = false;
            gameOver = false;
            InitGame();
        }
		// Help screen toggle
        if (IsKeyPressed(KEY_H))
        {
            helpScreen = true;
        }
    }
    else
    {
        // Help screen layout 
        const int helpTitleSize = 40;
        const int helpFontSize = 20;
        DrawText("HELP", centerX - MeasureText("HELP", helpTitleSize) / 2, titleY - helpTitleSize / 2, helpTitleSize, BLUE);

        int y = titleY + 20;
        const int helpSpacing = 30;

        DrawText("PRESS SPACE TO JUMP", centerX - MeasureText("PRESS SPACE TO JUMP", helpFontSize) / 2, y, helpFontSize, GRAY);
        y += helpSpacing;
        DrawText("JUMP OVER ALL OBSTACLES", centerX - MeasureText("JUMP OVER ALL OBSTACLES", helpFontSize) / 2, y, helpFontSize, GRAY);
        y += helpSpacing;
        DrawText("THE LONGER YOU STAY THE HIGHER YOUR SCORE", centerX - MeasureText("THE LONGER YOU STAY THE HIGHER YOUR SCORE", helpFontSize) / 2, y, helpFontSize, GRAY);
        y += helpSpacing;
        DrawText("PRESS [M] TO RETURN TO THE MENU", centerX - MeasureText("PRESS [M] TO RETURN TO THE MENU", helpFontSize) / 2, y, helpFontSize, GRAY);

        if (IsKeyPressed(KEY_M))
        {
            helpScreen = false;
        }
    }
}

void DrawGame(void)
{
    //Begins drawing the game frames, clears everything beforehand
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawTexture(backgroundTexture, 0, 0, WHITE);

    if (menu)
    {
        //If the menu is needed the main menu is drawn
        MainMenu();
    }
    else
    {
        if (!gameOver && !menu)
        {
            //Draw obstacles with DrawTexturePro to match obstacles[i].rec size
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                Rectangle obsSrc = { 0.0f, 0.0f, (float)obstacleOneTexture.width, (float)obstacleOneTexture.height };
                Rectangle obsDest = { obstacles[i].rec.x, obstacles[i].rec.y, obstacles[i].rec.width, obstacles[i].rec.height };
                DrawTexturePro(obstacleOneTexture, obsSrc, obsDest, (Vector2) { 0, 0 }, 0.0f, WHITE);
            }

            // Correct source rectangle for player's current frame (frames are vertical)
            Rectangle source = { 0.0f, (float)(currentFrame * frameHeight), (float)frameWidth, (float)frameHeight };

            Rectangle dest = {
                player.position.x,
                player.position.y,
                frameWidth * scale,
                frameHeight * scale
            };

            DrawTexturePro(playerTexture, source, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            //Draw floor
            DrawTexture(floorTexture, floor.position.x, floor.position.y, WHITE);

            DrawText(TextFormat("%04i", score), 20, 20, 40, BLACK);
            DrawText(TextFormat("HI-SCORE: %04i", hiScore), 20, 70, 20, BLACK);

            // Centered pause text
            if (pause) DrawText("GAME PAUSED", centerX - MeasureText("GAME PAUSED", mainTextSize) / 2, centerY - 40, mainTextSize, BLACK);
        }
        else
        {
            // Centered game over text and prompts
            DrawText("GAME OVER", centerX - MeasureText("GAME OVER", mainTextSize) / 2, centerY - 70, mainTextSize, RED);
            DrawText("PRESS [ENTER] TO PLAY AGAIN", centerX - MeasureText("PRESS [ENTER] TO PLAY AGAIN", subTextSize) / 2, centerY - 10, subTextSize, BLACK);
            DrawText("PRESS [M] TO RETURN TO MENU", centerX - MeasureText("PRESS [M] TO RETURN TO MENU", subTextSize) / 2, centerY + 20, subTextSize, BLACK);

        }
        if (IsKeyPressed(KEY_ENTER) && gameOver)
        {
            InitGame();
        }
        if (IsKeyPressed(KEY_M) && gameOver)
        {
            menu = true;
            gameOver = false;
        }

    }

    EndDrawing();
}

void UnloadGame(void)
{
    //Unloading everything to prevent leaks
    UnloadTexture(playerTexture);
    UnloadImage(playerImage);

    UnloadTexture(backgroundTexture);

    UnloadTexture(obstacleOneTexture);
    UnloadImage(obstacleOneImage);

    UnloadTexture(floorTexture);
}
// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}