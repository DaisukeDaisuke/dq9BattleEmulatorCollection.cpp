# Function Overview

## BattleEmulator::Main

Return Type: `bool`

Arguments that are modified during processing:

- `int *position`
- `Player[2] players`
- `std::optional<BattleResult> &result`
- `uint64_t *nowState`

Function Attributes:

- [ ] Can be modified function
- [x] static
- [ ] constexpr
- [ ] Arguments can be modified

Function Signature:

```cpp
bool BattleEmulator::Main(int *position, int RunCount, const int32_t Gene[350], Player *players,
                          std::optional<BattleResult> &result,
                          uint64_t seed, const int eActions[350], const int damages[350], int mode,
                          uint64_t *NowState) {
```

Arguments:

- `position`: Current position pointer in the battle sequence, Do not edit with search algorithms
- `RunCount`: Number of turns to simulate, The total number of turns is calculated automatically, and if you perform
  another turn after performing two turns, it will automatically restart from the third turn.
- `Gene`: Array of action genes, Specify the action ID here, nullptr is NOT allowed
- `players`: Array of two Player objects (ally and enemy), Do not edit with search algorithms
- `result`: Optional reference to store battle results, If mode is -2, result can be abbreviated as: (std::
  optional<BattleResult> &) std::nullopt
- `seed`: Random seed for battle, -1 is Allowed
- `eActions`: Array of constant values, For modes -1 and -2, nullptr is allowed
- `damages`: Array to store damage values, For modes -1 and -2, nullptr is allowed
- `mode`: Maximum number of elements, -1: Recording Mode, -2: not recording mode and not brute force mode, i.e. the
  fastest mode.
- `nowState`: Current state pointer, Do not edit with search algorithms

## ActionOptimizer::RunAlgorithm

Return Type: `Genome`

Arguments that are modified during processing:

- None (all inputs are const)

Function Attributes:

- [ ] Can be modified Function
- [x] static
- [ ] constexpr
- [ ] Arguments can be modified

Function Signature:

```cpp
Genome ActionOptimizer::RunAlgorithm(const Player players[2], 
    uint64_t seed, int turns, int maxGenerations,
    int actions[350], int seedOffset)
```

Arguments:

- `players`: Const array of two Player objects
- `seed`: Random seed
- `turns`: Number of turns
- `maxGenerations`: Max generations for optimization
- `actions`: Array of initial actions
- `seedOffset`: Offset for the random seed

## Player

Main structure for storing player state.

Note: Copying a Player is expensive

Attributes:

- `hp`: `int` - Current health points
- `maxHp`: `double` - Maximum health points
- `atk`: `int` - Attack power
- `def`: `int` - Defense power
- `speed`: `int` - Speed stat
- `mp`: `int` - Magic points
- `maxMp`: `int` - Maximum magic points
- `specialCharge`: `bool` - Special charge status
- `sleeping`: `bool` - Sleep status
- `paralysis`: `bool` - Paralysis status
- `BuffLevel`: `int` - Current buff level
- `BuffTurns`: `int` - Remaining buff turns
- `TensionLevel`: `int` - Current tension level
- `PoisonEnable`: `bool` - Poison status
- `acrobaticStar`: `bool` - Acrobatic star status

Methods:

```cpp
static bool isPlayerAlive(const Player &obj)
static void reduceHp(Player &obj, int amount)
static void heal(Player &obj, int amount)
```

## Genome

Structure for genetic algorithm optimization.

Function Attributes:

- [x] Can be modified
- [ ] static
- [ ] constexpr
- [x] Arguments can be modified

Note: Copying a Genome is expensive

Attributes:

- `AllyPlayer`: `Player` - Ally player state
- `EnemyPlayer`: `Player` - Enemy player state
- `state`: `uint64_t` - Current state
- `turn`: `int` - Current turn
- `position`: `int` - Position in sequence
- `fitness`: `int` - Fitness score
- `actions`: `int[350]` - Action sequence
- `EActions`: `int[2]` - Enemy actions
- `Aactions`: `int` - Ally action
- `compromiseScore`: `int` - Compromise penalty
- `Initialized`: `bool` - Initialization flag
- `processed`: `int` - Processed turns

Operators:

```cpp
bool operator<(const Genome& other) const
bool operator>(const Genome& other) const
```

## BruteForceRequest

Return Type: `uint64_t`

Function Attributes:

- [ ] Can be modified
- [ ] static
- [ ] constexpr
- [ ] Arguments can be modified

Function Signature:

```cpp
uint64_t BruteForceRequest(const Player copiedPlayers[2],
    int hours, int minutes, int seconds, int turns,
    int aActions[350], int damages[350])
```

Arguments:

- `copiedPlayers`: Array of copied player states
- `hours`: Target hours
- `minutes`: Target minutes
- `seconds`: Target seconds
- `turns`: Number of turns
- `aActions`: Array of actions
- `damages`: Array of damages

## HeapQueue

A priority queue that is faster than std for Genome  
TYPE: class

```c++
explicit HeapQueue(size_t maxSize)
void push(const Genome &genome)
void pop()
[[nodiscard]] Genome top()
[[nodiscard]] bool empty()
[[nodiscard]] size_t size()
```

Use it like this:

```c++
HeapQueue que(300);
que.push(genome);

while (!que.empty()) {
    currentGenome = que.top();
    que.pop();
}
```

## Cheat Sheet for AI

Build arguments to pass to BattleEmu

```c++
Genome ActionOptimizer::RunAlgorithm(const Player players[2], 
    uint64_t seed, int turns, int maxGenerations,
    int actions[350], int seedOffset){
    std::unique_ptr<int> position = std::make_unique<int>(1);
    std::unique_ptr<uint64_t> nowState = std::make_unique<uint64_t>(0);

    genome = {};

    genome.EnemyPlayer = players[1];
    genome.AllyPlayer = players[0];
    
    std::optional<BattleResult> result;
    
    for (int i = 0; i < 350; ++i) {
        if (actions[i] == -1 || actions[i] == 0) {
            genome.actions[i] = BattleEmulator::ATTACK_ALLY;
            break;
        } else {
            genome.actions[i] = actions[i];
        }
    }
}
```

Battle with Battle Emulator:

```c++
        Player CopedPlayers[2] = {currentGenome.AllyPlayer, currentGenome.EnemyPlayer};
        *position = currentGenome.position;
        *nowState = currentGenome.state;

        (*position) = tmpgenomu.position;
        (*nowState) = tmpgenomu.state;

        BattleEmulator::Main(position.get(), tmpgenomu.turn - tmpgenomu.processed, currentGenome.actions,
                             CopedPlayers,
                             (std::optional<BattleResult> &) std::nullopt, seed,
                             nullptr, nullptr, -2, nowState.get());
         if (CopedPlayers[0].hp <= 0) {
            continue;
        }
```

Get the battle results

```c++
        Player CopedPlayers[2] = {currentGenome.AllyPlayer, currentGenome.EnemyPlayer};
        *position = currentGenome.position;
        *nowState = currentGenome.state;

        currentGenome.actions[turns - 1] = action;

        const auto tmpgenomu = currentGenome;
        result->clear();//Creating a result is **very** expensive, so reuse pointers

        currentGenome.actions[turns - 1] = action;//int[350]

        BattleEmulator::Main(position.get(), currentGenome.turn - currentGenome.processed, currentGenome.actions,
                             CopedPlayers,
                             result, seed,
                             nullptr, nullptr, -1, nowState.get());

        //Since the plyer from this point onwards has been moved to the next turn, you can restore it from the copy if necessary.

        for (int i = result->position - 3; i < result->position; ++i) {
            if (result->isEnemy[i]) {
                //Eactions[counter1] = result->actions[i];
                //Edamage[counter1] = result->damages[i];
                currentGenome.EActions[counter1++] = result->actions[i];
            } else {
                Aactions = result->actions[i];
                //Adamage = result->damages[i];
                currentGenome.Aactions = result->actions[i];
            }
        }
```