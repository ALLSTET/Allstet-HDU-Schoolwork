#ifndef DECK_H
#define DECK_H

#include "Card.h"
#include <vector>
#include <cstdlib>
#include <ctime>

class Deck {
public:
    Deck();
    
    void shuffle();
    Card drawCard();
    int getCardCount() const;
    void reset();

private:
    std::vector<Card> cards;
};

#endif // DECK_H
