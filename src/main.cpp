#include "core/Board-Tiles/FreeParkingTile.hpp"
#include "core/Board-Tiles/GoTile.hpp"
#include "core/Board-Tiles/JailTile.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "models/GameManager/Auction.hpp"
#include "models/GameManager/Dice.hpp"
#include "models/GameManager/FestivalEffect.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/JailManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/GameManager/TransactionLogger.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void testPlayer() {
    cout << "=== TEST PLAYER ===" << endl;
    Player p1("Alice", 1500);
    Player p2("Bob", 1500);

    cout << "Player 1: " << p1.getUsername() << " | Cash: " << p1.getCash()
        << endl;
    cout << "Player 2: " << p2.getUsername() << " | Cash: " << p2.getCash()
        << endl;

    p1.addCash(500);
    cout << "Alice +500: " << p1.getCash() << endl;

    p1.reduceCash(200);
    cout << "Alice -200: " << p1.getCash() << endl;

    cout << "Alice canPay(2000)? " << (p1.canPay(2000) ? "Ya" : "Tidak") << endl;
    cout << "Alice canPay(1000)? " << (p1.canPay(1000) ? "Ya" : "Tidak") << endl;

    cout << "Alice status: " << p1.getStatus() << " (0=ACTIVE)" << endl;
    cout << "Alice position: " << p1.getPosition() << endl;
    p1.setPosition(10);
    cout << "Alice position setelah setPosition(10): " << p1.getPosition()
        << endl;

    cout << endl;
}

void testDice() {
    cout << "=== TEST DICE ===" << endl;
    Dice dice;

    dice.roll();
    cout << "Roll: " << dice.getDie1() << " + " << dice.getDie2() << " = "
        << dice.getTotal() << endl;
    cout << "Double? " << (dice.checkDouble() ? "Ya" : "Tidak") << endl;

    dice.setValues(3, 3);
    cout << "Set (3,3): " << dice.getDie1() << " + " << dice.getDie2() << " = "
        << dice.getTotal() << endl;
    cout << "Double? " << (dice.checkDouble() ? "Ya" : "Tidak") << endl;
    cout << "Consecutive doubles: " << dice.getConsecutiveDoubles() << endl;

    dice.setValues(3, 3);
    cout << "Set (3,3) lagi: Consecutive doubles: "
        << dice.getConsecutiveDoubles() << endl;

    dice.setValues(3, 3);
    cout << "Set (3,3) lagi: Consecutive doubles: "
        << dice.getConsecutiveDoubles() << endl;
    cout << "Has three consecutive doubles? "
        << (dice.hasThreeConsecutiveDoubles() ? "Ya" : "Tidak") << endl;

    dice.resetConsecutiveDoubles();
    cout << "Reset: Consecutive doubles: " << dice.getConsecutiveDoubles()
        << endl;

    cout << endl;
}

void testProperty() {
    cout << "=== TEST PROPERTY ===" << endl;

    // Street
    vector<int> rentLevels = {2, 10, 30, 90, 160, 250};
    Street *street = new Street(60, ColorGroup::COKLAT, rentLevels, 50, 50,
        BuildingLevel::EMPTY, 1);
    street->setCode("GRT");
    street->setName("Gerut");
    street->setOwner(nullptr);
    cout << "Street: " << street->getCode() << " | Type: " << street->getType()
        << " | BuyPrice: " << street->getBuyPrice()
        << " | Mortgage: " << street->getMortgageValue() << endl;
    street->printTitle();

    // Railroad
    map<int, int> rentTable = {{1, 25}, {2, 50}, {3, 100}, {4, 200}};
    Railroad *railroad = new Railroad(200, rentTable);
    railroad->setCode("GBR");
    railroad->setName("Gambir");
    cout << "Railroad: " << railroad->getCode()
        << " | Type: " << railroad->getType()
        << " | BuyPrice: " << railroad->getBuyPrice() << endl;
    railroad->printTitle();

    // Utility
    map<int, int> multTable = {{1, 4}, {2, 10}};
    Utility *utility = new Utility(150, multTable);
    utility->setCode("PLN");
    utility->setName("PLN Listrik");
    cout << "Utility: " << utility->getCode() << " | Type: " << utility->getType()
        << " | BuyPrice: " << utility->getBuyPrice() << endl;
    utility->printTitle();

    delete street;
    delete railroad;
    delete utility;

    cout << endl;
}

void testPlayerProperty() {
    cout << "=== TEST PLAYER + PROPERTY ===" << endl;

    Player p("Charlie", 2000);

    vector<int> rent = {10, 50, 150, 450, 625, 750};
    Street *s1 = new Street(140, ColorGroup::MERAH_MUDA, rent, 100, 100,
        BuildingLevel::EMPTY, 1);
    s1->setCode("MGL");
    s1->setOwner(&p);
    s1->setStatusStr("OWNED");

    Railroad *r1 = new Railroad();
    r1->setCode("STB");
    r1->setOwner(&p);
    r1->setStatusStr("OWNED");

    p.addProperty(s1);
    p.addProperty(r1);

    cout << "Charlie properti: " << p.getPropertyCount() << endl;
    cout << "Railroad count: " << p.getRailroadCount() << endl;
    cout << "Total wealth: " << p.getTotalWealth() << endl;

    for (Property *prop : p.getProperties()) {
        cout << "  - " << prop->getCode() << " (" << prop->getType() << ")" << endl;
    }

    // Cleanup
    delete s1;
    delete r1;

    cout << endl;
}

void testAuction() {
    cout << "=== TEST AUCTION ===" << endl;

    vector<int> rent = {50, 200, 600, 1400, 1700, 2000};
    Street *prop = new Street(400, ColorGroup::BIRU_TUA, rent, 200, 200,
        BuildingLevel::EMPTY, 1);
    prop->setCode("JKT");

    Player p1("Alice", 1500);
    Player p2("Bob", 1500);
    Player p3("Charlie", 1500);

    vector<Player *> participants = {&p1, &p2, &p3};
    Auction auction(prop, participants);
    auction.start();

    auction.submitBid(&p1, 100);
    auction.submitBid(&p2, 150);
    auction.pass(&p3);
    auction.pass(&p1);

    Player *winner = auction.getWinner();
    if (winner) {
        cout << "Pemenang: " << winner->getUsername() << " dengan harga "
            << auction.getWinningBid() << endl;
    }

    delete prop;

    cout << endl;
}

void testTransactionLogger() {
    cout << "=== TEST TRANSACTION LOGGER ===" << endl;
    TransactionLogger logger;
    logger.log(1, "Alice", "DADU", "Lempar: 3+4=7 → mendarat di Bandung (BDG)");
    logger.log(1, "Alice", "BELI", "Beli Bandung (BDG) seharga M300");
    logger.log(2, "Bob", "DADU", "Lempar: 6+6=12 (double) → mendarat di Parkir");

    cout << "Semua log:" << endl;
    logger.printAll();

    cout << "\n2 log terakhir:" << endl;
    logger.printLast(2);

    cout << endl;
}

void testFestivalEffect() {
    cout << "=== TEST FESTIVAL EFFECT ===" << endl;
    FestivalEffect fe;

    int mult = fe.activateEffect("JKT", 1);
    cout << "JKT festival activated: multiplier=" << mult
        << " duration=" << fe.getDuration("JKT") << endl;

    mult = fe.activateEffect("JKT", mult);
    cout << "JKT festival diperkuat: multiplier=" << mult
        << " duration=" << fe.getDuration("JKT") << endl;

    fe.tickDuration("JKT");
    cout << "Setelah 1 tick: duration=" << fe.getDuration("JKT") << endl;

    cout << "Max? " << (fe.isMaxMultiplier("JKT") ? "Ya" : "Tidak") << endl;

    cout << endl;
}

void testJailManager() {
    cout << "=== TEST JAIL MANAGER ===" << endl;
    JailManager jm(3, 50);

    Player p("Dave", 1000);
    cout << "Dave in jail? " << (jm.isInJail(p) ? "Ya" : "Tidak") << endl;

    jm.sendToJail(p);
    cout << "Setelah sendToJail: " << (jm.isInJail(p) ? "Ya" : "Tidak") << endl;
    cout << "Dave status: " << p.getStatus() << " (2=JAILED)" << endl;

    cout << "canReleaseByFine? " << (jm.canReleaseByFine(p) ? "Ya" : "Tidak")
        << endl;

    jm.payFine(p);
    cout << "Setelah payFine: in jail? " << (jm.isInJail(p) ? "Ya" : "Tidak")
        << endl;
    cout << "Dave cash: " << p.getCash() << endl;
    cout << "Dave status: " << p.getStatus() << " (0=ACTIVE)" << endl;

    cout << endl;
}

void testTiles() {
    cout << "=== TEST TILES ===" << endl;
    Player p1("Eve", 1500);

    GoTile goTile("GO", "Petak Mulai", 0, 200);
    cout << "GoTile: code=" << goTile.getCode() << " name=" << goTile.getName()
        << " salary=" << goTile.getSalary() << endl;

    FreeParkingTile fpTile("BBP", "Bebas Parkir", 20);
    cout << "FreeParkingTile: code=" << fpTile.getCode()
        << " name=" << fpTile.getName() << endl;

    JailTile jailTile("PEN", "Penjara", 10);
    cout << "JailTile: code=" << jailTile.getCode()
        << " name=" << jailTile.getName() << endl;

    goTile.addPlayer(p1);
    cout << "Players on GO: " << goTile.getPlayerList().size() << endl;
    goTile.removePlayer(p1);
    cout << "Players on GO after remove: " << goTile.getPlayerList().size()
        << endl;

    cout << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "      NIMONSPOLI - TEST PROGRAM         " << endl;
    cout << "========================================" << endl;
    cout << endl;

    testPlayer();
    testDice();
    testProperty();
    testPlayerProperty();
    testAuction();
    testTransactionLogger();
    testFestivalEffect();
    testJailManager();
    testTiles();

    cout << "========================================" << endl;
    cout << "     SEMUA TEST BERHASIL DIJALANKAN     " << endl;
    cout << "========================================" << endl;

    return 0;
}
