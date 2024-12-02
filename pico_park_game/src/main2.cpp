#include <iostream>
#include <conio.h> // For _kbhit() and _getch() on Windows
#include <vector>
#include <thread>
#include <chrono>    // For timer
#include <atomic>    // For atomic variables
#include <cstdlib>   // For rand()
#include <windows.h> // For colored text in the console

const int GRID_SIZE = 10;

// Color constants
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_RED 12
#define COLOR_MAGENTA 13
#define COLOR_CYAN 11
#define COLOR_DEFAULT 7

void setColor(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void renderGrid(const std::vector<std::vector<char>> &grid, int timeLeft, int movesP1, int movesP2, int score, int level)
{
    system("cls"); // Clear the screen
    std::cout << "Level: " << level << " | Time Remaining: " << timeLeft << " seconds"
              << " | P1 Moves: " << movesP1 << " | P2 Moves: " << movesP2 << " | Score: " << score << std::endl;

    for (const auto &row : grid)
    {
        for (const auto &cell : row)
        {
            switch (cell)
            {
            case 'P':
                setColor(COLOR_GREEN);
                break; // Player 1
            case '2':
                setColor(COLOR_CYAN);
                break; // Player 2
            case 'G':
                setColor(COLOR_YELLOW);
                break; // Goal
            case 'X':
                setColor(COLOR_RED);
                break; // Obstacles
            case 'C':
                setColor(COLOR_CYAN);
                break; // Collectibles
            case 'T':
                setColor(COLOR_MAGENTA);
                break; // Traps
            default:
                setColor(COLOR_DEFAULT);
                break; // Default (empty spaces)
            }
            std::cout << cell << " ";
        }
        setColor(COLOR_DEFAULT); // Reset color after each row
        std::cout << std::endl;
    }
}

void timerThread(std::atomic<int> &timeLeft, std::atomic<bool> &gameRunning)
{
    while (timeLeft > 0 && gameRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timeLeft--; // Decrement timer
    }
    gameRunning = false; // End game if timer runs out
}

void chasingObstacleThread(std::vector<std::pair<int, int>> &obstacles, std::vector<std::vector<char>> &grid,
                           std::atomic<bool> &gameRunning, int &player1X, int &player1Y, int &player2X, int &player2Y)
{
    while (gameRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (auto &obstacle : obstacles)
        {
            int oldX = obstacle.first;
            int oldY = obstacle.second;
            int newX = oldX, newY = oldY;

            if (player1X > oldX)
                newX++;
            else if (player1X < oldX)
                newX--;

            if (player1Y > oldY)
                newY++;
            else if (player1Y < oldY)
                newY--;

            if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE &&
                grid[newY][newX] == '.')
            {
                grid[oldY][oldX] = '.';
                grid[newY][newX] = 'X';
                obstacle = {newX, newY};
            }
        }
    }
}

void patrollingObstacleThread(std::vector<std::pair<int, int>> &obstacles, std::vector<std::vector<char>> &grid,
                              std::atomic<bool> &gameRunning)
{
    std::vector<int> directions(obstacles.size(), 0);

    while (gameRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (size_t i = 0; i < obstacles.size(); i++)
        {
            int oldX = obstacles[i].first;
            int oldY = obstacles[i].second;
            int newX = oldX, newY = oldY;

            if (directions[i] == 0)
            {
                newX += (rand() % 2 == 0) ? -1 : 1;
            }
            else
            {
                newY += (rand() % 2 == 0) ? -1 : 1;
            }

            if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE &&
                grid[newY][newX] == '.')
            {
                grid[oldY][oldX] = '.';
                grid[newY][newX] = 'X';
                obstacles[i] = {newX, newY};
            }
            else
            {
                directions[i] = 1 - directions[i];
            }
        }
    }
}

void setupLevel(int level, std::vector<std::vector<char>> &grid,
                std::vector<std::pair<int, int>> &chasingObstacles,
                std::vector<std::pair<int, int>> &patrollingObstacles,
                std::vector<std::pair<int, int>> &collectibles,
                std::vector<std::pair<int, int>> &traps,
                std::atomic<int> &timeLeft, int &player1X, int &player1Y, int &player2X, int &player2Y, int &goalX, int &goalY)
{
    grid = std::vector<std::vector<char>>(GRID_SIZE, std::vector<char>(GRID_SIZE, '.'));
    player1X = 1;
    player1Y = 1;
    player2X = GRID_SIZE - 2;
    player2Y = 1;
    goalX = GRID_SIZE - 2;
    goalY = GRID_SIZE - 2;
    grid[player1Y][player1X] = 'P';
    grid[player2Y][player2X] = '2';
    grid[goalY][goalX] = 'G';

    chasingObstacles.clear();
    patrollingObstacles.clear();
    collectibles.clear();
    traps.clear();

    int numObstacles = 3 + level;
    int numCollectibles = level;
    int numTraps = level;

    for (int i = 0; i < numObstacles; i++)
    {
        int obsX, obsY;
        do
        {
            obsX = rand() % GRID_SIZE;
            obsY = rand() % GRID_SIZE;
        } while (grid[obsY][obsX] != '.');
        if (i % 2 == 0)
            chasingObstacles.push_back({obsX, obsY});
        else
            patrollingObstacles.push_back({obsX, obsY});
        grid[obsY][obsX] = 'X';
    }

    for (int i = 0; i < numCollectibles; i++)
    {
        int colX, colY;
        do
        {
            colX = rand() % GRID_SIZE;
            colY = rand() % GRID_SIZE;
        } while (grid[colY][colX] != '.');
        collectibles.push_back({colX, colY});
        grid[colY][colX] = 'C';
    }

    for (int i = 0; i < numTraps; i++)
    {
        int trapX, trapY;
        do
        {
            trapX = rand() % GRID_SIZE;
            trapY = rand() % GRID_SIZE;
        } while (grid[trapY][trapX] != '.');
        traps.push_back({trapX, trapY});
        grid[trapY][trapX] = 'T';
    }

    int newTime = 30 - (level * 5);
    timeLeft = (newTime < 10) ? 10 : newTime;
}

bool handleInput(char &directionP1, char &directionP2)
{
    if (_kbhit())
    {
        char key = _getch();
        switch (key)
        {
        case 'w':
            directionP1 = 'U';
            return true;
        case 's':
            directionP1 = 'D';
            return true;
        case 'a':
            directionP1 = 'L';
            return true;
        case 'd':
            directionP1 = 'R';
            return true;
        case 72:
            directionP2 = 'U';
            return true; // Up arrow
        case 80:
            directionP2 = 'D';
            return true; // Down arrow
        case 75:
            directionP2 = 'L';
            return true; // Left arrow
        case 77:
            directionP2 = 'R';
            return true; // Right arrow
        case 'q':
            return false; // Quit game
        }
    }
    return false;
}

bool updatePlayerPosition(char direction, int &playerX, int &playerY, std::vector<std::vector<char>> &grid)
{
    int newX = playerX, newY = playerY;

    switch (direction)
    {
    case 'U':
        newY--;
        break;
    case 'D':
        newY++;
        break;
    case 'L':
        newX--;
        break;
    case 'R':
        newX++;
        break;
    }

    if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE &&
        grid[newY][newX] == '.')
    {
        playerX = newX;
        playerY = newY;
        return true;
    }
    return false;
}

int main()
{
    std::atomic<int> timeLeft(30);
    std::atomic<bool> gameRunning(true);
    int player1X = 1, player1Y = 1, player2X = GRID_SIZE - 2, player2Y = 1;
    int goalX = GRID_SIZE - 2, goalY = GRID_SIZE - 2;
    int movesP1 = 0, movesP2 = 0, score = 0, level = 1;
    std::vector<std::vector<char>> grid(GRID_SIZE, std::vector<char>(GRID_SIZE, '.'));
    std::vector<std::pair<int, int>> chasingObstacles, patrollingObstacles, collectibles, traps;

    setupLevel(level, grid, chasingObstacles, patrollingObstacles, collectibles, traps, timeLeft,
               player1X, player1Y, player2X, player2Y, goalX, goalY);

    std::thread timer(timerThread, std::ref(timeLeft), std::ref(gameRunning));
    std::thread chasingThread(chasingObstacleThread, std::ref(chasingObstacles), std::ref(grid),
                              std::ref(gameRunning), std::ref(player1X), std::ref(player1Y), std::ref(player2X), std::ref(player2Y));
    std::thread patrollingThread(patrollingObstacleThread, std::ref(patrollingObstacles), std::ref(grid),
                                 std::ref(gameRunning));

    renderGrid(grid, timeLeft, movesP1, movesP2, score, level);

    while (gameRunning)
    {
        char directionP1 = ' ', directionP2 = ' ';
        if (handleInput(directionP1, directionP2))
        {
            grid[player1Y][player1X] = '.';
            if (updatePlayerPosition(directionP1, player1X, player1Y, grid))
            {
                movesP1++;
            }
            grid[player1Y][player1X] = 'P';

            grid[player2Y][player2X] = '.';
            if (updatePlayerPosition(directionP2, player2X, player2Y, grid))
            {
                movesP2++;
            }
            grid[player2Y][player2X] = '2';

            if ((player1X == goalX && player1Y == goalY) || (player2X == goalX && player2Y == goalY))
            {
                score += timeLeft * 10 - (movesP1 + movesP2);
                level++;
                setupLevel(level, grid, chasingObstacles, patrollingObstacles, collectibles, traps, timeLeft,
                           player1X, player1Y, player2X, player2Y, goalX, goalY);
                movesP1 = movesP2 = 0;
            }
            renderGrid(grid, timeLeft, movesP1, movesP2, score, level);
        }
    }

    timer.join();
    chasingThread.join();
    patrollingThread.join();

    std::cout << "Game Over! Final Score: " << score << std::endl;
    return 0;
}
