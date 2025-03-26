# Pico Park Game Clone 🎮

A fully functional **console-based clone** of the popular multiplayer puzzle-platformer **Pico Park**, developed entirely in **C++**. This version features real-time gameplay with AI-controlled obstacles, collectibles, traps, and level progression — all rendered in the Windows console.

---

## 🧩 Features

- 🔲 **Grid-based Gameplay**: 10x10 dynamic grid with live updates
- 👤 **Player Controls**: `W`, `A`, `S`, `D` to move; `Q` to quit
- 🎯 **Goal System**: Reach the goal while avoiding obstacles and traps
- ❌ **Chasing & Patrolling Obstacles**: Multithreaded AI enemies move in real time
- 💥 **Traps & Collectibles**: Increase or decrease score by interacting with elements
- ⏱️ **Timer**: Each level has a countdown, encouraging quick thinking
- ⬆️ **Level Progression**: Game gets harder as you advance
- 🎨 **Colored Console Output**: Enhanced visuals with color-coded elements

---

## 🛠️ Tech Stack

- **Language**: C++
- **Libraries**:
  - `<thread>`, `<chrono>` — for real-time mechanics
  - `<atomic>` — for thread-safe flags and countdown
  - `<windows.h>` — for colored output in the console
  - `<conio.h>` — for non-blocking keypress handling (Windows only)

---

## 🚀 Getting Started

### Prerequisites

- **Windows OS**
- A C++ compiler (e.g., g++, MSVC)
- Command-line interface (CMD, PowerShell, or Terminal)

### Compile and Run

g++ -std=c++11 pico_park_clone.cpp -o pico_park_clone
./pico_park_clone

## 🧠 Game Mechanics

🟢 Player (P) – You!

🟡 Goal (G) – Reach this to clear the level

🔴 Obstacles (X) – Enemies that patrol or chase

🔵 Collectibles (C) – Add +50 to your score

🟣 Traps (T) – Subtract -50 from your score (game over if score < 0)

