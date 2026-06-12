#include "StartScreen.h"
#include <iostream>

StartScreen::StartScreen(sf::RenderWindow& window)
    : window(window), selectedDifficulty(NORMAL) {
    loadFont();
}

void StartScreen::loadFont() {
    if (!font.openFromFile("C:\\Windows\\Fonts\\simhei.ttf")) {
        if (!font.openFromFile("C:\\Windows\\Fonts\\msyh.ttc")) {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
                std::cerr << "Warning: Could not load font" << std::endl;
            }
        }
    }
}

void StartScreen::draw() {
    // Draw title
    sf::Text title(font, "POKER GAME", 60);
    title.setPosition(sf::Vector2f(window.getSize().x / 2.f - 250, 100.f));
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);
    
    // Draw description
    sf::Text description(font, "Player vs 3 AI Opponents\nEach player gets 5 cards", 24);
    description.setPosition(sf::Vector2f(window.getSize().x / 2.f - 200, 250.f));
    description.setFillColor(sf::Color::Cyan);
    window.draw(description);
    
    drawDifficultyButtons();
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    bool startHovered = startButtonRect.contains(mousePos);
    bool quitHovered = quitButtonRect.contains(mousePos);
    
    drawButton("START GAME", window.getSize().x / 2.f - 100, 450, 200, 60, startHovered, startButtonRect);
    drawButton("QUIT", window.getSize().x / 2.f - 100, 550, 200, 60, quitHovered, quitButtonRect);
}

void StartScreen::drawDifficultyButtons() {
    float centerX = window.getSize().x / 2.f;
    float baseY = 360.f;
    
    bool easySelected = selectedDifficulty == EASY;
    bool normalSelected = selectedDifficulty == NORMAL;
    bool hardSelected = selectedDifficulty == HARD;
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool easyHovered = easyButtonRect.contains(mousePos);
    bool normalHovered = normalButtonRect.contains(mousePos);
    bool hardHovered = hardButtonRect.contains(mousePos);
    
    drawButton("EASY", centerX - 260, baseY, 120, 50, easyHovered, easyButtonRect, easySelected);
    drawButton("NORMAL", centerX - 60, baseY, 120, 50, normalHovered, normalButtonRect, normalSelected);
    drawButton("HARD", centerX + 140, baseY, 120, 50, hardHovered, hardButtonRect, hardSelected);
    
    sf::Text label(font, "Select Difficulty:", 18);
    label.setPosition(sf::Vector2f(centerX - 240, baseY - 30));
    label.setFillColor(sf::Color::White);
    window.draw(label);
}

void StartScreen::drawButton(const std::string& text, float x, float y, float width, float height,
                            bool isHovered, sf::FloatRect& outRect, bool selected) {
    sf::RectangleShape button(sf::Vector2f(width, height));
    button.setPosition(sf::Vector2f(x, y));
    
    if (selected) {
        button.setFillColor(sf::Color(100, 200, 100));
    } else if (isHovered) {
        button.setFillColor(sf::Color::Green);
    } else {
        button.setFillColor(sf::Color::Blue);
    }
    
    button.setOutlineColor(sf::Color::White);
    button.setOutlineThickness(selected ? 4.f : 2.f);
    window.draw(button);
    
    sf::Text buttonText(font, text, 22);
    buttonText.setPosition(sf::Vector2f(x + width / 2.f - text.size() * 6.f, y + height / 2.f - 14));
    buttonText.setFillColor(sf::Color::White);
    window.draw(buttonText);
    
    outRect = sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(width, height));
}

void StartScreen::handleMouseClick(sf::Vector2f mousePos) {
    if (easyButtonRect.contains(mousePos)) {
        selectedDifficulty = EASY;
    } else if (normalButtonRect.contains(mousePos)) {
        selectedDifficulty = NORMAL;
    } else if (hardButtonRect.contains(mousePos)) {
        selectedDifficulty = HARD;
    }
}

bool StartScreen::isStartButtonClicked(sf::Vector2f mousePos) const {
    return startButtonRect.contains(mousePos);
}

bool StartScreen::isQuitButtonClicked(sf::Vector2f mousePos) const {
    return quitButtonRect.contains(mousePos);
}

StartScreen::Difficulty StartScreen::getSelectedDifficulty() const {
    return selectedDifficulty;
}
