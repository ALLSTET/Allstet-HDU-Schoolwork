#include "Deck.h"
#include <algorithm>

Deck::Deck() {
    reset();
}

void Deck::reset() {
    cards.clear();
    Suit suits[] = {HEARTS, DIAMONDS, CLUBS, SPADES};
    Rank ranks[] = {TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE};
    
    for (Suit suit : suits) {
        for (Rank rank : ranks) {
            cards.push_back(Card(suit, rank));
        }
    }
    shuffle();
}

void Deck::shuffle() {
    for (int i = cards.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(cards[i], cards[j]);
    }
}

Card Deck::drawCard() {
    Card card = cards.back();
    cards.pop_back();
    if (cards.empty()) {
        reset();
    }
    return card;
}

int Deck::getCardCount() const {
    return cards.size();
}
