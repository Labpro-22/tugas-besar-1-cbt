#include "models/GameManager/GameState.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"
#include <iostream>
#include <sstream>

using namespace std;

GameState::GameState(int currentTurn, int maxTurn, int activePlayerIndex,
    vector<Player> players, vector<int> turnOrder,
    vector<Property *> properties,
    vector<SkillCard *> skillDeckCard, vector<LogEntry> log)
    : currentTurn(currentTurn), maxTurn(maxTurn),
        activePlayerIndex(activePlayerIndex), players(players),
        turnOrder(turnOrder), properties(properties),
        skillDeckCards(skillDeckCard), log(log) {}

string GameState::serialize() const {
    stringstream ss;

    ss << currentTurn << " " << maxTurn << " " << activePlayerIndex << "\n";

    ss << players.size() << "\n";
    for (Player p : players) {
        // Data dasar pemain
        ss << p.getUsername() << " " << p.getCash() << " " << p.getPosition()
            << " ";

        if (p.getStatus() == ACTIVE) {
            ss << "ACTIVE ";
        } else if (p.getStatus() == BANKRUPT) {
            ss << "BANKRUPT ";
        } else if (p.getStatus() == JAILED) {
            ss << "JAILED ";
        }
        int cardCount = p.getCardCount();
        ss << cardCount << " ";

        if (cardCount > 0) {
            ss << " ";
            for (Card *card : p.getHand()) {
                string jenis = card->getType();
                ss << jenis;
                if (jenis == "MoveCard") {
                    ss << " " << card->getValue();
                } else if (jenis == "DiscountCard") {
                    ss << " " << card->getValue() << " " << card->getDuration();
                }

                ss << " ";
            }
        }
        ss << "\n";
    }

    ss << turnOrder.size() << "\n";
    for (int t : turnOrder) {
        ss << t << "\n";
    }

    ss << properties.size() << "\n";
    for (Property *prop : properties) {
        ss << prop->getCode() << " " << prop->getType() << " " << prop->getName()
            << " " << prop->getStatusString() << " " << prop->getFMult() << " "
            << prop->getFDur() << " " << prop->getBuildingCount() << "\n";
    }

    ss << skillDeckCards.size() << "\n";
    for (SkillCard *card : skillDeckCards) {
        ss << card->getType() << "\n";
    }

    ss << log.size() << "\n";
    for (LogEntry entry : log) {
        ss << entry.turn << " " << entry.username << " " << entry.actionType << " "
            << entry.detail << "\n";
    }

    return ss.str();
}

void GameState::deserialize(const string &data) {
    stringstream ss(data);

    players.clear();
    turnOrder.clear();
    properties.clear();
    skillDeckCards.clear();
    log.clear();
    skillCardFactory.clear();

    ss >> currentTurn >> maxTurn >> activePlayerIndex;

    int playersSize;
    if (ss >> playersSize) {
        for (int i = 0; i < playersSize; i++) {
            string username, statusStr;
            int cash, position, cardCount;

            ss >> username >> cash >> position >> statusStr >> cardCount;
            PlayerStatus status = ACTIVE;
            if (statusStr == "BANKRUPT")
                status = BANKRUPT;
            else if (statusStr == "JAILED")
                status = JAILED;
            vector<Card *> hand;

            for (int c = 0; c < cardCount; c++) {
                string jenisKartu;
                ss >> jenisKartu;
                int nilaiKartu = 0;
                int durasiKartu = 0;
                if (jenisKartu == "MoveCard") {
                    ss >> nilaiKartu;
                } else if (jenisKartu == "DiscountCard") {
                    ss >> nilaiKartu >> durasiKartu;
                }
                SkillCard *parsed =
                    skillCardFactory.create(jenisKartu, nilaiKartu, durasiKartu);
                if (parsed != nullptr) {
                    hand.push_back(parsed);
                }
            }
            vector<Property *> emptyProperties;
            Player p(username, cash, status, position, hand, emptyProperties, 0,
                false, 0, 0);
            players.push_back(p);
        }
    }

    int turnOrderSize;
    if (ss >> turnOrderSize) {
        for (int i = 0; i < turnOrderSize; i++) {
            int t;
            ss >> t;
            turnOrder.push_back(t);
        }
    }

    int propertiesSize;
    if (ss >> propertiesSize) {
        for (int i = 0; i < propertiesSize; i++) {
            string kode, jenis, pemilik, status;
            int fmult, fdur, bangunan;

            ss >> kode >> jenis >> pemilik >> status >> fmult >> fdur >> bangunan;

            Property *prop = nullptr;

            // ALOKASI BERDASARKAN TIPE
            if (jenis == "Street") {
                prop = new Street();
            } else if (jenis == "Railroad") {
                prop = new Railroad();
            } else if (jenis == "Utility") {
                prop = new Utility();
            }

            // JIKA VALID, SET VALUENYA
            if (prop != nullptr) {
                prop->setCode(kode);

                if (pemilik != "BANK") {
                    // prop->setOwner(player_pointer_dari_list);
                }

                prop->setStatusStr(status);
                prop->setFestival(fmult, fdur);
                prop->setBuildingCount(bangunan);

                properties.push_back(prop);
            }
        }
    }

    int deckSize;
    if (ss >> deckSize) {
        for (int i = 0; i < deckSize; i++) {
            string jenisKartu;
            ss >> jenisKartu;

            SkillCard *parsed = skillCardFactory.create(jenisKartu);
            if (parsed != nullptr) {
                skillDeckCards.push_back(parsed);
            }
        }
    }
    int logSize;
    if (ss >> logSize) {
        for (int i = 0; i < logSize; i++) {
            int turn;
            string uname, act, detail;

            ss >> turn >> uname >> act;

            getline(ss >> ws, detail);

            log.push_back(LogEntry(turn, uname, act, detail));
        }
    }
}
