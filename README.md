# Game AI Programming Project

**Author:** Wannes De Tollenaere  
**Course:** Game AI Programming  
**Academic Year:** 2025-2026 (DAE - Game Development)  
**Engine:** Unreal Engine 5.6.3

## About This Project

This project explores and implements core Game AI movement algorithms within Unreal Engine using C++. It focuses on autonomous agent movement, group dynamics, and performance optimization. The project provides a modular framework for assigning and combining various steering behaviors to AI agents, alongside a robust debug rendering system using ImGui to visualize AI decision-making in real-time.

## Key Features

### 1. Steering Behaviors
A library of individual movement algorithms allowing agents to navigate their environment autonomously:
* **Seek & Flee:** Moving towards or away from a target.
* **Wander:** Realistic, smooth random movement.
* **Evade:** Predicting and avoiding a moving target.

### 2. Combined Steering
Complex agent movement achieved by combining multiple basic behaviors:
* **Blended Steering:** Calculating a weighted average of multiple steering outputs.
* **Priority Steering:** Evaluating behaviors in a strict order of importance (e.g., Evade takes priority over Wander).

### 3. Flocking & Group Dynamics
Emergent group behavior mimicking flocks of birds or schools of fish, utilizing Boids principles:
* **Separation:** Steering to avoid crowding local flockmates.
* **Alignment (Velocity Match):** Steering towards the average heading of local flockmates.
* **Cohesion:** Steering to move toward the average position of local flockmates.

### 4. Spatial Partitioning Optimization
To ensure the Flocking algorithm remains performant with a high number of agents, a CellSpace partitioning system is implemented.
