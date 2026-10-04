/// @file AiAssistedDoc.cpp
/// @brief Educational game demonstrating reviewed AI-assisted Doxygen documentation.
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

using namespace std;

 
/// @brief Stores a player's name, score and level in memory.
/// @details Names are not validated. No file persistence or synchronization is provided.
/// @code{.cpp}
/// Player alice("Alice");
/// alice.addScore(200);
/// alice.checkLevelUp(200); // Level 2, score 0.
/// @endcode
class Player {
private:
    /// @brief Display name; empty names are accepted.
    string name;    
    /// @brief Accumulated positive points, initially zero.
    int score;      
    /// @brief Progression level, initially one.
    int level;      

public:
    
    /// @brief Creates a player with zero points and level one.
    /// @param playerName Name copied without validation; may be empty.
    Player(const string& playerName) {
        
        name = playerName;
        score = 0;
        level = 1;
    }

    
    /// @brief Adds positive points; zero and negative values are ignored.
    /// @param points Score increment.
    /// @pre For positive points, the resulting score must be representable as int.
    void addScore(int points) {
        if (points > 0) {  
            score += points;
        }
    }

    
    /// @brief Reads the current score.
    /// @return Accumulated points.
    int getScore() const {
        return score;
    }

    
    /// @brief Clears the score while preserving name and level.
    void resetScore() {
        score = 0;
    }

    
    /// @brief Increments the level by one without changing the score.
    /// @pre The incremented level must be representable as int.
    void levelUp() {
        level++;
    }

    
    /// @brief Reads the current level.
    /// @return Progression level, initially one.
    int getLevel() const {
        return level;
    }

    
    /// @brief Advances once and clears all points when a positive threshold is met.
    /// @param scoreThreshold Required score; nonpositive values do nothing.
    /// @details Excess points are discarded. Multiple level advances are not performed.
    /// @pre If advancement occurs, the next level must be representable as int.
    void checkLevelUp(int scoreThreshold) {
        
        if (score >= scoreThreshold && scoreThreshold > 0) {
            levelUp();
            resetScore();  
        }
    }

    
    /// @brief Reads the player's name.
    /// @return Reference to the stored name, valid while this player exists.
    const string& getName() const {
        return name;
    }

    
    /// @brief Formats the player's current statistics.
    /// @return Text in the form Player: name | Level: level | Score: score.
    string getDisplayInfo() const {
        
        return "Player: " + name + " | Level: " + to_string(level) +
            " | Score: " + to_string(score);
    }

    
    /// @brief Restores score zero and level one while preserving the name.
    void resetPlayer() {
        score = 0;
        level = 1;
    }
};

/// @brief Manages players and console-based number-guessing rounds.
/// @details Targets range from 1 to 100 inclusive. Only exact guesses earn points
/// through makeGuess(). Call endRound() explicitly to finish a round.
/// @note State is in memory only and is not thread-safe.
class SimpleGame {
private:
    /// @brief Registered players in insertion order, with case-sensitive unique names.
    vector<Player> players;    
    /// @brief Current target; zero before the first round and after reset.
    int targetNumber;         
    /// @brief Number of rounds started since construction or reset.
    int roundNumber;          
    /// @brief Whether guesses are currently accepted.
    bool gameActive;          

public:
    
    /// @brief Creates an empty game with zero counters and no active round.
    SimpleGame() {
        
        targetNumber = 0;
        roundNumber = 0;
        gameActive = false;
    }

    
    /// @brief Registers a player or prints a message for a duplicate name.
    /// @param playerName Case-sensitive name; an empty name is accepted if unique.
    void addPlayer(const string& playerName) {
        
        for (const auto& player : players) {
            if (player.getName() == playerName) {
                cout << "Player " << playerName << " already exists!" << endl;
                return;
            }
        }
        players.emplace_back(playerName);
        cout << "Added player: " << playerName << endl;
    }

    
    /// @brief Generates a target in [1, 100], increments the round and activates play.
    /// @details Replaces any active round without ending it; retains players and scores.
    /// A fresh mt19937 engine is seeded with random_device for each call.
    /// This generator is not cryptographically secure.
    /// @pre The incremented round counter must be representable as int.
    void startNewRound() {
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);

        targetNumber = dis(gen);
        roundNumber++;
        gameActive = true;

        cout << "Round " << roundNumber << " started! Guess a number between 1-100." << endl;
    }

    
    /// @brief Checks a guess and prints feedback; exact matches add 100 points.
    /// @param playerName Registered player's case-sensitive name.
    /// @param guess Guessed integer; the range is not validated.
    /// @return True for an exact match by a registered player in an active round;
    /// false for an incorrect guess, unknown player or inactive game.
    /// @details An exact match leaves the round active. Repeated exact guesses can
    /// earn points until endRound() is called. Incorrect guesses earn no points.
    /// @pre On an exact match, adding 100 to the player's score must fit in int.
    bool makeGuess(const string& playerName, int guess) {
        if (!gameActive) {
            cout << "No active game round!" << endl;
            return false;
        }

        
        auto playerIt = find_if(players.begin(), players.end(),
            [&playerName](const Player& p) { return p.getName() == playerName; });

        if (playerIt == players.end()) {
            cout << "Player not found!" << endl;
            return false;
        }

        
        if (guess == targetNumber) {
            int score = calculateScore(guess);
            playerIt->addScore(score);
            cout << playerName << " guessed correctly! Score: " << score << endl;
            return true;
        }
        else if (guess < targetNumber) {
            cout << "Too low! Try again." << endl;
        }
        else {
            cout << "Too high! Try again." << endl;
        }

        return false;
    }

    
    /// @brief Computes a proximity score without awarding points or checking round state.
    /// @param guess Number to compare with the current target.
    /// @return 100 for difference 0; 50 for 1-5; 25 for 6-15; 10 for 16-30; 5 otherwise.
    /// @pre Both guess - targetNumber and its absolute value must fit in int.
    /// @note makeGuess() calls this function only for exact matches.
    int calculateScore(int guess) {
        
        int difference = abs(guess - targetNumber);

        if (difference == 0) return 100;      
        else if (difference <= 5) return 50;  
        else if (difference <= 15) return 25; 
        else if (difference <= 30) return 10; 
        else return 5;                        
    }

    
    /// @brief Ends an active round, reveals its target and prints player standings.
    /// @details Does nothing if inactive. Each standing is printed before checking
    /// a 200-point threshold: qualifying players advance once and lose all points.
    /// No separate level-up notification is printed.
    /// @pre For qualifying players, the next level must be representable as int.
    void endRound() {
        if (!gameActive) return;

        gameActive = false;

        cout << "\nRound " << roundNumber << " ended!" << endl;
        cout << "The target number was: " << targetNumber << endl;

        
        cout << "\nPlayer standings:" << endl;
        for (const auto& player : players) {
            cout << player.getDisplayInfo() << endl;

            
            Player& mutablePlayer = const_cast<Player&>(player);
            mutablePlayer.checkLevelUp(200);
        }
    }

    
    /// @brief Prints players in registration order, or a message for an empty roster.
    void displayPlayers() const {
        if (players.empty()) {
            cout << "No players in the game." << endl;
            return;
        }

        cout << "\nCurrent Players:" << endl;
        for (const auto& player : players) {
            cout << "- " << player.getDisplayInfo() << endl;
        }
    }

    
    /// @brief Reads the roster size.
    /// @return Number of registered players.
    size_t getPlayerCount() const {
        return players.size();
    }

    
    /// @brief Reads round activity.
    /// @return True while guesses are accepted, including after a correct guess.
    bool isGameActive() const {
        return gameActive;
    }

    
    /// @brief Removes all players, zeros target and round counters, and deactivates play.
    void resetGame() {
        players.clear();
        targetNumber = 0;
        roundNumber = 0;
        gameActive = false;
        cout << "Game reset complete!" << endl;
    }
};

/// @brief Demonstrates three rounds with Alice, Bob and Charlie.
/// @details Uses six fixed guesses per round with random targets, so results vary.
/// Ends each round after the first correct guess or after all six attempts.
/// Waits for Enter after every round, including the final round.
/// @return Zero on normal completion.
int main() {
    cout << "AI-Assisted Game Library Documentation Demo" << endl;
    cout << "===========================================" << endl;

    
    SimpleGame game;

    cout << "\nSetting up players..." << endl;
    game.addPlayer("Alice");
    game.addPlayer("Bob");
    game.addPlayer("Charlie");

    game.displayPlayers();

    
    cout << "\nStarting game demonstration..." << endl;

    for (int round = 1; round <= 3; round++) {
        cout << "\n" << string(40, '=') << endl;
        game.startNewRound();

        
        vector<pair<string, int>> guesses = {
            {"Alice", 50}, {"Bob", 25}, {"Charlie", 75},
            {"Alice", 30}, {"Bob", 60}, {"Charlie", 45}
        };

        for (const auto& guess : guesses) {
            cout << "\n" << guess.first << " guesses: " << guess.second << endl;
            bool correct = game.makeGuess(guess.first, guess.second);
            if (correct) {
                game.endRound();
                break;
            }
        }

        
        if (game.isGameActive()) {
            game.endRound();
        }

        cout << "\nPress Enter to continue to next round...";
        cin.get();
    }

    
    cout << "\n" << string(50, '=') << endl;
    cout << "Final Game Results:" << endl;
    game.displayPlayers();

    cout << "\nAI-assisted documentation demo completed!" << endl;
    cout << "Review the AI-generated documentation above!" << endl;

    return 0;
}
