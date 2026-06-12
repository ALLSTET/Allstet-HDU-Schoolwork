#ifndef CARD_H
#define CARD_H

#include <string>
#include <vector>

enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
enum Rank { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

class Card {
public:
    Card(Suit suit, Rank rank);
    
    Suit getSuit() const;
    Rank getRank() const;
    int getValue() const;
    std::string toString() const;
    std::string getSuitSymbol() const;
    std::string getRankSymbol() const;
    
    // 花色英文名（Club/Diamond/Heart/Spade），用于图片文件名
    std::string getSuitName() const;
    // 返回 PNG 文件名，如 "SpadeA.png"
    std::string getImageFilename() const;

private:
    Suit suit;
    Rank rank;
};

#endif // CARD_H
