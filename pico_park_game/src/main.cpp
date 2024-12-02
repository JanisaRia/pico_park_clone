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

// Utility function to set console text color
void setColor(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Render the grid with all game elements and colors
void renderGrid(const std::vector<std::vector<char>> &grid, int timeLeft, int moves, int score, int level)
{
    system("cls"); // Clear the screen
    std::cout << "Level: " << level << " | Time Remaining: " << timeLeft << " seconds | Moves: " << moves
              << " | Score: " << score << std::endl;

    for (const auto &row : grid)
    {
        for (const auto &cell : row)
        {
            switch (cell)
            {
            case 'P':
                setColor(COLOR_GREEN);
                break; // Player
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

// Timer thread
void timerThread(std::atomic<int> &timeLeft, std::atomic<bool> &gameRunning)
{
    while (timeLeft > 0 && gameRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timeLeft--; // Decrement timer
    }
    gameRunning = false; // End game if timer runs out
}

// Chasing obstacle thread
void chasingObstacleThread(std::vector<std::pair<int, int>> &obstacles, std::vector<std::vector<char>> &grid,
                           std::atomic<bool> &gameRunning, int &playerX, int &playerY, int goalX, int goalY)
{
    while (gameRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (auto &obstacle : obstacles)
        {
            int oldX = obstacle.first;
            int oldY = obstacle.second;
            int newX = oldX, newY = oldY;

            if (playerX > oldX)
                newX++;
            else if (playerX < oldX)
                newX--;

            if (playerY > oldY)
                newY++;
            else if (playerY < oldY)
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

// Patrolling obstacle thread
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

// Setup level
void setupLevel(int level, std::vector<std::vector<char>> &grid,
                std::vector<std::pair<int, int>> &chasingObstacles,
                std::vector<std::pair<int, int>> &patrollingObstacles,
                std::vector<std::pair<int, int>> &collectibles,
                std::vector<std::pair<int, int>> &traps,
                std::atomic<int> &timeLeft, int &playerX, int &playerY, int &goalX, int &goalY)
{
    grid = std::vector<std::vector<char>>(GRID_SIZE, std::vector<char>(GRID_SIZE, '.'));
    playerX = 1;
    playerY = 1;
    goalX = GRID_SIZE - 2;
    goalY = GRID_SIZE - 2;
    grid[playerY][playerX] = 'P';
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

// Handle input
bool handleInput(char &direction)
{
    if (_kbhit())
    {
        char key = _getch();
        switch (key)
        {
        case 'w':
            direction = 'U';
            return true;
        case 's':
            direction = 'D';
            return true;
        case 'a':
            direction = 'L';
            return true;
        case 'd':
            direction = 'R';
            return true;
        case 'q':
            direction = 'Q';
            return true;
        }
    }
    return false;
}

// Update player position
bool updatePlayerPosition(char direction, int &playerX, int &playerY, std::vector<std::vector<char>> &grid,
                          std::vector<std::pair<int, int>> &collectibles, std::vector<std::pair<int, int>> &traps,
                          int &score, std::atomic<bool> &gameRunning)
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
        grid[newY][newX] != 'X')
    {
        playerX = newX;
        playerY = newY;

        for (auto it = collectibles.begin(); it != collectibles.end(); ++it)
        {
            if (it->first == newX && it->second == newY)
            {
                score += 50;
                collectibles.erase(it);
                break;
            }
        }

        for (auto it = traps.begin(); it != traps.end(); ++it)
        {
            if (it->first == newX && it->second == newY)
            {
                score -= 50;
                if (score < 0)
                    gameRunning = false;
                traps.erase(it);
                break;
            }
        }
        return true;
    }
    return false;
}

// Main function
int main()
{
    std::atomic<int> timeLeft(30);
    std::atomic<bool> gameRunning(true);
    int playerX = 1, playerY = 1, goalX = GRID_SIZE - 2, goalY = GRID_SIZE - 2;
    int moves = 0, score = 0, level = 1;
    std::vector<std::vector<char>> grid(GRID_SIZE, std::vector<char>(GRID_SIZE, '.'));
    std::vector<std::pair<int, int>> chasingObstacles, patrollingObstacles, collectibles, traps;

    setupLevel(level, grid, chasingObstacles, patrollingObstacles, collectibles, traps, timeLeft, playerX, playerY,
               goalX, goalY);

    std::thread timer(timerThread, std::ref(timeLeft), std::ref(gameRunning));
    std::thread chasingThread(chasingObstacleThread, std::ref(chasingObstacles), std::ref(grid),
                              std::ref(gameRunning), std::ref(playerX), std::ref(playerY), goalX, goalY);
    std::thread patrollingThread(patrollingObstacleThread, std::ref(patrollingObstacles), std::ref(grid),
                                 std::ref(gameRunning));

    renderGrid(grid, timeLeft, moves, score, level);

    while (gameRunning)
    {
        char direction = ' ';
        if (handleInput(direction))
        {
            grid[playerY][playerX] = '.';
            if (updatePlayerPosition(direction, playerX, playerY, grid, collectibles, traps, score, gameRunning))
            {
                moves++;
            }
            if (playerX == goalX && playerY == goalY)
            {
                score += timeLeft * 10 - moves;
                level++;
                setupLevel(level, grid, chasingObstacles, patrollingObstacles, collectibles, traps, timeLeft,
                           playerX, playerY, goalX, goalY);
                moves = 0;
            }
            grid[playerY][playerX] = 'P';
        }
        renderGrid(grid, timeLeft, moves, score, level);
    }

    timer.join();
    chasingThread.join();
    patrollingThread.join();

    std::cout << "Game Over! Final Score: " << score << std::endl;
    return 0;
}
