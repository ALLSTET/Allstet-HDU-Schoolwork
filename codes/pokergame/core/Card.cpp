#include "Card.h"

Card::Card(Suit suit, Rank rank) : suit(suit), rank(rank) {}

Suit Card::getSuit() const {
    return suit;
}

Rank Card::getRank() const {
    return rank;
}

int Card::getValue() const {
    switch (rank) {
        case ACE:
            return 1;
        case JACK:
        case QUEEN:
        case KING:
            return 10;
        default:
            return static_cast<int>(rank);
    }
}

std::string Card::getSuitSymbol() const {
    switch (suit) {
        case HEARTS: return "H";
        case DIAMONDS: return "D";
        case CLUBS: return "C";
        case SPADES: return "S";
        default: return "";
    }
}

std::string Card::getRankSymbol() const {
    switch (rank) {
        case TWO: return "2";
        case THREE: return "3";
        case FOUR: return "4";
        case FIVE: return "5";
        case SIX: return "6";
        case SEVEN: return "7";
        case EIGHT: return "8";
        case NINE: return "9";
        case TEN: return "10";
        case JACK: return "J";
        case QUEEN: return "Q";
        case KING: return "K";
        case ACE: return "A";
        default: return "";
    }
}

std::string Card::getSuitName() const {
    switch (suit) {
        case CLUBS:    return "Club";
        case DIAMONDS: return "Diamond";
        case HEARTS:   return "Heart";
        case SPADES:   return "Spade";
        default:       return "";
    }
}

std::string Card::getImageFilename() const {
    return getSuitName() + getRankSymbol() + ".png";
}

std::string Card::toString() const {
    std::string rank = getRankSymbol();
    char suitLetter = '?';
    switch (suit) {
        case HEARTS: suitLetter = 'A'; break; // A = Hearts
        case DIAMONDS: suitLetter = 'B'; break; // B = Diamonds
        case CLUBS: suitLetter = 'C'; break; // C = Clubs
        case SPADES: suitLetter = 'D'; break; // D = Spades
        default: suitLetter = '?'; break;
    }
    return rank + std::string("_") + suitLetter;
}
