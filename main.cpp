#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>  // Added this for sin function
using namespace std;

const int TILE_WIDTH = 60;
const int TILE_HEIGHT = 60;
const int TILE_SPACING = 10;
const int BOARD_X = 50;
const int BOARD_Y = 400;
const int NUM_TILES = 12;

enum GameState {
    MENU,
    INTRODUCTION,
    RULES,
    PLAYING,
    GAME_OVER,
    ROUND_END
};

struct Tile {
    int number;
    sf::RectangleShape shape;
    sf::Text text;
    bool active = true;
    bool selected = false;
    sf::Clock animationClock;
    bool animating = false;
    sf::Color originalColor;
};

struct Player {
    string name;
    vector<bool> closedTiles;
    int roundScore = 0;
    int totalScore = 0;
    
    Player(const string& n) : name(n), closedTiles(NUM_TILES, false) {}
    
    void resetRound() {
        fill(closedTiles.begin(), closedTiles.end(), false);
        roundScore = 0;
    }
    
    int calculateRoundScore() {
        roundScore = 0;
        for (int i = 0; i < NUM_TILES; i++) {
            if (!closedTiles[i]) {
                roundScore += (i + 1);
            }
        }
        return roundScore;
    }
};

class ShutTheBoxGame {
private:
    sf::RenderWindow window;
    sf::Font font;
    GameState state;
    
    // Game variables
    vector<Tile> tiles;
    vector<Player> players;
    int currentPlayer;
    int dice1, dice2;
    bool diceRolled;
    vector<int> selectedNumbers;
    int currentRound;
    int maxRounds;
    
    // UI elements
    sf::Text titleText, instructionText, playerText, diceText, scoreText;
    sf::RectangleShape dice1Shape, dice2Shape;
    sf::Text dice1Text, dice2Text;
    sf::Clock animationClock;
    bool diceAnimating;
    
    // Menu buttons
    sf::RectangleShape startButton, rulesButton, exitButton, backButton, continueButton;
    sf::Text startButtonText, rulesButtonText, exitButtonText, backButtonText, continueButtonText;
    
    // Rules and introduction text
    vector<sf::Text> rulesTextLines;
    vector<sf::Text> introTextLines;

public:
    ShutTheBoxGame() : window(sf::VideoMode(1200, 800), "Shut the Box - Enhanced"), 
                       state(MENU), currentPlayer(0), diceRolled(false), 
                       currentRound(1), maxRounds(5), diceAnimating(false) {
        srand(static_cast<unsigned>(time(0)));
        
        // Load font
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            cout << "Warning: Could not load Arial font, using default font." << endl;
        }
        
        initializeUI();
        initializePlayers();
        initializeTiles();
    }
    
    void initializeUI() {
        // Title
        titleText.setFont(font);
        titleText.setString("SHUT THE BOX");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color::Blue);
        titleText.setPosition(400, 30);
        
        // Instructions
        instructionText.setFont(font);
        instructionText.setCharacterSize(20);
        instructionText.setFillColor(sf::Color::Black);
        instructionText.setPosition(50, 150);
        
        // Player text
        playerText.setFont(font);
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::Red);
        playerText.setPosition(50, 200);
        
        // Dice display
        dice1Shape.setSize(sf::Vector2f(80, 80));
        dice1Shape.setFillColor(sf::Color::White);
        dice1Shape.setOutlineThickness(3);
        dice1Shape.setOutlineColor(sf::Color::Black);
        dice1Shape.setPosition(500, 250);
        
        dice2Shape.setSize(sf::Vector2f(80, 80));
        dice2Shape.setFillColor(sf::Color::White);
        dice2Shape.setOutlineThickness(3);
        dice2Shape.setOutlineColor(sf::Color::Black);
        dice2Shape.setPosition(600, 250);
        
        dice1Text.setFont(font);
        dice1Text.setCharacterSize(36);
        dice1Text.setFillColor(sf::Color::Black);
        dice1Text.setPosition(525, 270);
        
        dice2Text.setFont(font);
        dice2Text.setCharacterSize(36);
        dice2Text.setFillColor(sf::Color::Black);
        dice2Text.setPosition(625, 270);
        
        // Score text
        scoreText.setFont(font);
        scoreText.setCharacterSize(18);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(750, 200);
        
        initializeMenuButtons();
        initializeIntroductionText();
        initializeRulesText();
    }
    
    void initializeMenuButtons() {
        // Start button
        startButton.setSize(sf::Vector2f(200, 60));
        startButton.setFillColor(sf::Color::Green);
        startButton.setPosition(500, 300);
        
        startButtonText.setFont(font);
        startButtonText.setString("START GAME");
        startButtonText.setCharacterSize(24);
        startButtonText.setFillColor(sf::Color::Black);
        startButtonText.setPosition(520, 320);
    
        
        // Rules button
        rulesButton.setSize(sf::Vector2f(200, 60));
        rulesButton.setFillColor(sf::Color::Yellow);
        rulesButton.setPosition(500, 380);
        
        rulesButtonText.setFont(font);
        rulesButtonText.setString("VIEW RULES");
        rulesButtonText.setCharacterSize(24);
        rulesButtonText.setFillColor(sf::Color::Black);
        rulesButtonText.setPosition(535, 400);
        
        // Exit button
        exitButton.setSize(sf::Vector2f(200, 60));
        exitButton.setFillColor(sf::Color::Red);
        exitButton.setPosition(500, 460);
        
        exitButtonText.setFont(font);
        exitButtonText.setString("EXIT");
        exitButtonText.setCharacterSize(24);
        exitButtonText.setFillColor(sf::Color::White);
        exitButtonText.setPosition(565, 480);
        
        // Back button - Fixed: Changed Gray to RGB values
        backButton.setSize(sf::Vector2f(100, 40));
        backButton.setFillColor(sf::Color(128, 128, 128)); // Fixed: Gray color
        backButton.setPosition(50, 720);
        
        backButtonText.setFont(font);
        backButtonText.setString("BACK");
        backButtonText.setCharacterSize(18);
        backButtonText.setFillColor(sf::Color::White);
        backButtonText.setPosition(70, 730);
        backButtonText.setStyle(sf::Text::Bold);
        backButtonText.setStyle(sf::Text::Underlined);

        // Continue button
        continueButton.setSize(sf::Vector2f(200, 50));
        continueButton.setFillColor(sf::Color::Green);
        continueButton.setPosition(500, 720);
        
        continueButtonText.setFont(font);
        continueButtonText.setString("CONTINUE");
        continueButtonText.setCharacterSize(20);
        continueButtonText.setFillColor(sf::Color::White);
        continueButtonText.setPosition(545, 735);
    }
    
    void initializeIntroductionText() {
        vector<string> intro = {
            "WELCOME TO SHUT THE BOX!",
            "",
               "",
            "  *GAME BASICS:",
               "",
            "   A 2-player dice game of strategy and luck",
            "   Goal: Get lowest total score after 5 rounds",
            "",
            "  *HOW TO PLAY:",
               "",
            "   Roll two dice on your turn",
            "   Close numbered tiles that match either:",
            "   Sum of dice (roll 3+4=7: close 7)",
            "   Individual dice (roll 3,4: close 3 or 4)",
            "   Combinations (roll 7: close 2+5 or 3+4)",
            "   Keep rolling until no moves possible",
            "   Score = sum of remaining open tiles",
            "",
            "   *END OF TURN:",
               "",
            "   If no moves possible, turn ends",
            "   Current player score is recorded",
            "   Next player takes their turn",
            "   Game continues until 5 rounds complete",
            "",
               "",
            "   *CONTROLS:",
               "",
            "   SPACE: Roll dice",
            "   Click: Select tiles", 
            "   ENTER: Confirm move",
            "   ESC: Exit game",
            "",
        
            "Click CONTINUE to start playing!"
        };
        
        introTextLines.clear();
        for (size_t i = 0; i < intro.size(); i++) {
            sf::Text text;
            text.setFont(font);
            text.setString(intro[i]);
            
            // Different formatting for different types of text
            if (intro[i] == "WELCOME TO SHUT THE BOX!") {
                text.setCharacterSize(28);
                text.setFillColor(sf::Color::Blue);
                text.setStyle(sf::Text::Bold);
            } else if (intro[i].find(":") != string::npos && intro[i].length() < 30) {
                text.setCharacterSize(18);
                text.setFillColor(sf::Color::Red);
                text.setStyle(sf::Text::Bold);
            } else if (intro[i].find("•") != string::npos) {
                text.setCharacterSize(16);
                text.setFillColor(sf::Color::Black);
            } else {
                text.setCharacterSize(16);
                text.setFillColor(sf::Color::Black);
            }
            
            text.setPosition(50, 100 + i * 18);
            introTextLines.push_back(text);
        }
    }
    
    void initializeRulesText() {
        vector<string> rules = {
            "SHUT THE BOX - QUICK RULES",
            "",
            "GOAL: Get lowest score after 5 rounds",
            "",
            "BASICS:",
             "",
            " 2 players, alternating turns",
            " Each player has tiles 1-12",
            " Roll dice and shut (close) tiles",
            "",
            "ON YOUR TURN:",
             "",
            "1. Roll two dice",
            "2. Shut tiles that equal either:",
            "    Sum of dice (roll 3+4=7: shut 7)",
            "    Individual dice (roll 3,4: shut 3 or 4)",
            "    Combinations (roll 7: shut 2+5 or 3+4)",
            "3. Repeat until no moves possible",
            "",
            "SCORING:",
            "",
            " Add up remaining open tiles",
            " Lower score is better!",
            "",
            "TIP: Focus on closing high numbers",
            "",
            "Press BACK for menu or START to play!"
        };
        
        rulesTextLines.clear();
        for (size_t i = 0; i < rules.size(); i++) {
            sf::Text text;
            text.setFont(font);
            text.setString(rules[i]);
            if (rules[i] == "SHUT THE BOX - QUICK RULES") {
                text.setCharacterSize(28);
                text.setFillColor(sf::Color::Blue);
                text.setStyle(sf::Text::Bold);
            } else if (rules[i] == "DETAILED GAME RULES") {
                text.setCharacterSize(24);
                text.setFillColor(sf::Color::Blue);
                text.setStyle(sf::Text::Bold);
            } else if (rules[i].find(":") != string::npos && rules[i].length() < 25) {
                text.setCharacterSize(18);
                text.setFillColor(sf::Color::Red);
                text.setStyle(sf::Text::Bold);
            } else if (rules[i].find("•") != string::npos) {
                text.setCharacterSize(15);
                text.setFillColor(sf::Color::Black);
            } else {
                text.setCharacterSize(15);
                text.setFillColor(sf::Color::Black);
            }
            
            text.setPosition(50, 90 + i * 16);
            rulesTextLines.push_back(text);
        }
    }
    
    void initializePlayers() {
        players.clear();
        players.push_back(Player("Player 1"));
        players.push_back(Player("Player 2"));
    }
    
    void initializeTiles() {
        tiles.clear();
        for (int i = 0; i < NUM_TILES; ++i) {
            Tile tile;
            tile.number = i + 1;
            tile.shape.setSize(sf::Vector2f(TILE_WIDTH, TILE_HEIGHT));
            tile.originalColor = sf::Color::Green;
            tile.shape.setFillColor(tile.originalColor);
            tile.shape.setPosition(BOARD_X + i * (TILE_WIDTH + TILE_SPACING), BOARD_Y);
            
            tile.text.setFont(font);
            tile.text.setString(to_string(i + 1));
            tile.text.setCharacterSize(20);
            tile.text.setFillColor(sf::Color::Black);
            tile.text.setPosition(tile.shape.getPosition().x + 20, tile.shape.getPosition().y + 20);
            
            tiles.push_back(tile);
        }
        updateTilesFromPlayer();
    }
    
    void updateTilesFromPlayer() {
        for (int i = 0; i < NUM_TILES; i++) {
            tiles[i].active = !players[currentPlayer].closedTiles[i];
            if (!tiles[i].active) {
                tiles[i].shape.setFillColor(sf::Color(100, 100, 100));
            } else {
                tiles[i].shape.setFillColor(tiles[i].originalColor);
            }
            tiles[i].selected = false;
        }
    }
    
    void rollDice() {
        dice1 = rand() % 6 + 1;
        dice2 = rand() % 6 + 1;
        diceRolled = true;
        diceAnimating = true;
        animationClock.restart();
        selectedNumbers.clear();
        
        for (auto& tile : tiles) {
            tile.selected = false;
            if (tile.active) {
                tile.shape.setFillColor(tile.originalColor);
            }
        }
    }
    
    void updateDiceAnimation() {
        if (diceAnimating) {
            float elapsed = animationClock.getElapsedTime().asSeconds();
            if (elapsed < 1.0f) {
                // Animate dice rolling
                int tempDice1 = rand() % 6 + 1;
                int tempDice2 = rand() % 6 + 1;
                dice1Text.setString(to_string(tempDice1));
                dice2Text.setString(to_string(tempDice2));
                
                // Shake effect - Fixed: sin function now available
                float shake = sin(elapsed * 50) * 5;
                dice1Shape.setPosition(500 + shake, 250);
                dice2Shape.setPosition(600 - shake, 250);
            } else {
                // Animation finished
                diceAnimating = false;
                dice1Text.setString(to_string(dice1));
                dice2Text.setString(to_string(dice2));
                dice1Shape.setPosition(500, 250);
                dice2Shape.setPosition(600, 250);
            }
        }
    }
    
    bool canMakeMove() {
        int total = dice1 + dice2;
        
        // Check if we can shut the total
        if (total <= NUM_TILES && tiles[total - 1].active) return true;
        
        // Check if we can shut individual dice values
        if (tiles[dice1 - 1].active || tiles[dice2 - 1].active) return true;
        
        // Check for combinations that sum to total
        for (int i = 0; i < NUM_TILES; i++) {
            if (!tiles[i].active) continue;
            for (int j = i + 1; j < NUM_TILES; j++) {
                if (!tiles[j].active) continue;
                if (tiles[i].number + tiles[j].number == total) return true;
            }
        }
        
        return false;
    }
    
    void handleTileSelection(sf::Vector2f mousePos) {
        if (!diceRolled || diceAnimating) return;
        
        for (auto& tile : tiles) {
            if (tile.active && tile.shape.getGlobalBounds().contains(mousePos)) {
                tile.selected = !tile.selected;
                if (tile.selected) {
                    tile.shape.setFillColor(sf::Color::Yellow);
                    selectedNumbers.push_back(tile.number);
                    // Start tile animation
                    tile.animating = true;
                    tile.animationClock.restart();
                } else {
                    tile.shape.setFillColor(tile.originalColor);
                    selectedNumbers.erase(remove(selectedNumbers.begin(), selectedNumbers.end(), tile.number), selectedNumbers.end());
                    tile.animating = false;
                }
                break;
            }
        }
    }
    
    void updateTileAnimations() {
        for (auto& tile : tiles) {
            if (tile.animating) {
                float elapsed = tile.animationClock.getElapsedTime().asSeconds();
                if (elapsed < 0.3f) {
                    // Pulsing effect - Fixed: sin function now available
                    float scale = 1.0f + sin(elapsed * 20) * 0.1f;
                    sf::Vector2f originalSize(TILE_WIDTH, TILE_HEIGHT);
                    sf::Vector2f newSize = originalSize * scale;
                    sf::Vector2f originalPos = sf::Vector2f(BOARD_X + (tile.number - 1) * (TILE_WIDTH + TILE_SPACING), BOARD_Y);
                    sf::Vector2f offset = (originalSize - newSize) * 0.5f;
                    
                    tile.shape.setSize(newSize);
                    tile.shape.setPosition(originalPos + offset);
                } else {
                    tile.animating = false;
                    tile.shape.setSize(sf::Vector2f(TILE_WIDTH, TILE_HEIGHT));
                    tile.shape.setPosition(BOARD_X + (tile.number - 1) * (TILE_WIDTH + TILE_SPACING), BOARD_Y);
                }
            }
        }
    }
    
    void confirmMove() {
        if (!diceRolled || selectedNumbers.empty()) return;
        
        int sum = accumulate(selectedNumbers.begin(), selectedNumbers.end(), 0);
        int total = dice1 + dice2;
        
        bool validMove = (sum == total) || (sum == dice1) || (sum == dice2);
        
        if (validMove) {
            // Close selected tiles
            for (auto& tile : tiles) {
                if (tile.selected) {
                    tile.active = false;
                    tile.shape.setFillColor(sf::Color(100, 100, 100));
                    tile.selected = false;
                    players[currentPlayer].closedTiles[tile.number - 1] = true;
                }
            }
            selectedNumbers.clear();
            diceRolled = false;
            
            // Check if player can continue
            if (!canMakeMove()) {
                endPlayerTurn();
            }
        } else {
            // Invalid move
            instructionText.setString("Invalid move! Sum must equal dice total or individual die value.");
            // Reset selections
            for (auto& tile : tiles) {
                if (tile.selected) {
                    tile.selected = false;
                    tile.shape.setFillColor(tile.originalColor);
                }
            }
            selectedNumbers.clear();
        }
    }
    
    void endPlayerTurn() {
        players[currentPlayer].calculateRoundScore();
        
        if (currentPlayer == 1) {
            // Both players finished, end round
            endRound();
        } else {
            // Switch to next player
            currentPlayer = 1;
            updateTilesFromPlayer();
            diceRolled = false;
            selectedNumbers.clear();
        }
    }
    
    void endRound() {
        for (auto& player : players) {
            player.totalScore += player.roundScore;
        }
        
        if (currentRound >= maxRounds) {
            state = GAME_OVER;
        } else {
            state = ROUND_END;
        }
    }
    
    void startNewRound() {
        currentRound++;
        currentPlayer = 0;
        for (auto& player : players) {
            player.resetRound();
        }
        updateTilesFromPlayer();
        diceRolled = false;
        selectedNumbers.clear();
        state = PLAYING;
    }
    
    void updateUI() {
        switch (state) {
            case PLAYING: {  // Added braces to fix variable initialization error
                playerText.setString(players[currentPlayer].name + "'s Turn - Round " + to_string(currentRound));
                
                if (!diceRolled) {
                    instructionText.setString("Press SPACE to roll dice");
                } else if (selectedNumbers.empty()) {
                    instructionText.setString("Select tiles that sum to " + to_string(dice1 + dice2) + 
                                            " or individual dice (" + to_string(dice1) + " or " + to_string(dice2) + ")");
                } else {
                    int sum = accumulate(selectedNumbers.begin(), selectedNumbers.end(), 0);
                    instructionText.setString("Selected sum: " + to_string(sum) + " - Press ENTER to confirm");
                }
                
                // Update score display
                string scoreStr = "SCORES:\n";
                for (size_t i = 0; i < players.size(); i++) {
                    scoreStr += players[i].name + ": " + to_string(players[i].totalScore) + 
                               " (This round: " + to_string(players[i].calculateRoundScore()) + ")\n";
                }
                scoreText.setString(scoreStr);
                break;
            }
                
            case ROUND_END: {  // Added braces
                instructionText.setString("Round " + to_string(currentRound) + " complete! Press SPACE for next round");
                break;
            }
                
            case GAME_OVER: {  // Added braces
                auto winner = min_element(players.begin(), players.end(), 
                    [](const Player& a, const Player& b) { return a.totalScore < b.totalScore; });
                instructionText.setString("Game Over! " + winner->name + " wins with " + 
                                        to_string(winner->totalScore) + " points! Press SPACE to restart");
                break;
            }
        }
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    if (state == RULES || state == INTRODUCTION) {
                        state = MENU;
                    } else {
                        window.close();
                    }
                }
                
                if (state == PLAYING) {
                    if (event.key.code == sf::Keyboard::Space && !diceRolled) {
                        rollDice();
                    }
                    
                    if (event.key.code == sf::Keyboard::Enter && diceRolled) {
                        confirmMove();
                    }
                } else if (state == ROUND_END) {
                    if (event.key.code == sf::Keyboard::Space) {
                        startNewRound();
                    }
                } else if (state == GAME_OVER) {
                    if (event.key.code == sf::Keyboard::Space) {
                        // Restart game
                        currentRound = 1;
                        currentPlayer = 0;
                        initializePlayers();
                        initializeTiles();
                        state = PLAYING;
                    }
                }
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                    
                    if (state == MENU) {
                        if (startButton.getGlobalBounds().contains(mousePos)) {
                            state = INTRODUCTION;  // Go to introduction first
                        } else if (rulesButton.getGlobalBounds().contains(mousePos)) {
                            state = RULES;
                        } else if (exitButton.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    } else if (state == INTRODUCTION) {
                        if (continueButton.getGlobalBounds().contains(mousePos)) {
                            state = PLAYING;
                            initializePlayers();
                            initializeTiles();
                        } else if (backButton.getGlobalBounds().contains(mousePos)) {
                            state = MENU;
                        }
                    } else if (state == RULES) {
                        if (backButton.getGlobalBounds().contains(mousePos)) {
                            state = MENU;
                        }
                    } else if (state == PLAYING) {
                        handleTileSelection(mousePos);
                    }
                }
            }
        }
    }
    
    void render() {
        window.clear(sf::Color::White);
        
        switch (state) {
            case MENU:
                window.draw(titleText);
                window.draw(startButton);
                window.draw(startButtonText);
                window.draw(rulesButton);
                window.draw(rulesButtonText);
                window.draw(exitButton);
                window.draw(exitButtonText);
                break;
                
            case INTRODUCTION:
                for (const auto& text : introTextLines) {
                    window.draw(text);
                }
                window.draw(continueButton);
                window.draw(continueButtonText);
                window.draw(backButton);
                window.draw(backButtonText);
                break;
                
            case RULES:
                for (const auto& text : rulesTextLines) {
                    window.draw(text);
                }
                window.draw(backButton);
                window.draw(backButtonText);
                break;
                
            case PLAYING:
            case ROUND_END:
            case GAME_OVER:
                window.draw(instructionText);
                window.draw(playerText);
                window.draw(scoreText);
                
                if (diceRolled) {
                    window.draw(dice1Shape);
                    window.draw(dice2Shape);
                    window.draw(dice1Text);
                    window.draw(dice2Text);
                }
                
                for (const auto& tile : tiles) {
                    window.draw(tile.shape);
                    window.draw(tile.text);
                }
                break;
        }
        
        window.display();
    }
    
    void run() {
        while (window.isOpen()) {
            handleEvents();
            
            if (state == PLAYING) {
                updateDiceAnimation();
                updateTileAnimations();
                
                // Check if current player can make any moves
                if (diceRolled && !diceAnimating && canMakeMove() == false && selectedNumbers.empty()) {
                    endPlayerTurn();
                }
            }
            
            updateUI();
            render();
        }
    }
};

int main() {
    ShutTheBoxGame game;
    game.run();
    return 0;
}
