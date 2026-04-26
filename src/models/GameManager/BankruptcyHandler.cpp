#include "models/GameManager/BankruptcyHandler.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "models/Property/Street.hpp"
#include "views/InputHandler.hpp"
#include <algorithm>
#include <sstream>

BankruptcyHandler::BankruptcyHandler(Player *debtor, Player *creditor, int debt)
    : debtor(debtor), creditor(creditor), debtAmount(debt) {
  buildAssetLists();
}

void BankruptcyHandler::buildAssetLists() {
  sellableProperties.clear();
  mortgageableProperties.clear();
  for (Property *prop : debtor->getProperties()) {
    if (prop == nullptr) {
      continue;
    }
    if (prop->getBuildingCount() > 0) {
      sellableProperties.push_back(prop);
    }
    if (prop->getBuildingCount() == 0 &&
        prop->getStatusString() != "MORTGAGED") {
      mortgageableProperties.push_back(prop);
    }
  }
}

bool BankruptcyHandler::sellToBank(Property *prop) {
  if (prop == nullptr || prop->getBuildingCount() <= 0) {
    return false;
  }
  int sellValue = prop->getBuildingSellValue();
  debtor->addCash(sellValue);

  prop->setBuildingCount(prop->getBuildingCount() - 1);

  if (prop->getType() == "Railroad" || prop->getType() == "Utility") {
    sellValue = prop->getMortgageValue();
  } else {
    sellValue = prop->getBuyPrice() + prop->getBuildingSellValue();
  }
  debtor->addCash(sellValue);
  debtor->removeProperty(prop);
  prop->setOwner(nullptr);
  prop->setStatusStr("BANK");
  prop->setFestival(1, 0);
  prop->setBuildingCount(0);
  return true;
}

bool BankruptcyHandler::mortgageProperty(Property *prop) {
  if (prop == nullptr || prop->getBuildingCount() > 0 ||
      prop->getStatusString() == "MORTGAGED") {
    return false;
  }

  int mortValue = prop->getMortgageValue();
  debtor->addCash(mortValue);
  prop->setStatusStr("MORTGAGED");

  return true;
}

bool BankruptcyHandler::isDebtSatisfied() const {
  return debtor->getCash() >= debtAmount;
}

int BankruptcyHandler::calculateMaxLiquidation() {
  int total = debtor->getCash();
  for (Property *prop : sellableProperties) {
    if (prop == nullptr)
      continue;
    if (prop->getType() == "Railroad" || prop->getType() == "Utility") {
      total += prop->getMortgageValue();
    } else {
      total += prop->getBuyPrice() + prop->getBuildingSellValue();
    }
  }
  return total;
}
bool BankruptcyHandler::canCoverDebt() {
  return calculateMaxLiquidation() >= debtAmount;
}

bool BankruptcyHandler::initiateLiquidation() {
  if (!canCoverDebt())
    return false;
  InputHandler ui;

  while (!isDebtSatisfied()) {
    if (sellableProperties.empty() && mortgageableProperties.empty()) {
      cout << "[!] Anda sudah menjual seluruh bangunan dan menggadaikan "
              "seluruh properti, tetapi dana tetap tidak mencukupi.\n";
      break;
    }
    string info = "UTANG: M" + to_string(debtAmount) + " | SALDO: M" +
                  to_string(debtor->getCash()) + " | ";
    info += "Kurang: M" + to_string(debtAmount - debtor->getCash()) + "\n\n";
    info += "Daftar Aset Anda:\n";

    for (Property *p : debtor->getProperties()) {
      if (p == nullptr)
        continue;
      info += "- [" + p->getCode() + "] " + p->getName() +
              " (Bangunan: " + to_string(p->getBuildingCount()) +
              ", Status: " + p->getStatusString() + ")\n";
    }
    info += "\nKetik: JUAL <kode> atau GADAI <kode>";

    string line = ui.readPromptLine(info, "Menu Likuidasi");
    if (line.empty())
      continue;

    stringstream ss(line);
    string cmd, code;
    ss >> cmd >> code;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    transform(code.begin(), code.end(), code.begin(), ::toupper);

    Property *target = nullptr;
    for (Property *p : debtor->getProperties()) {
      if (p != nullptr && p->getCode() == code) {
        target = p;
        break;
      }
    }

    if (!target)
      continue;

    if (cmd == "JUAL") {
      try {
        if (!sellToBank(target))
          throw PropertyBuildException(
              target->getCode(),
              "Gagal menjual. Pastikan ada bangunan di properti ini.");
        cout << "[INFO] Berhasil menjual 1 bangunan di " << target->getCode()
             << ".\n";
      } catch (const NimonspoliException &e) {
        cout << "Error: " << e.what() << "\n";
      }
    } else if (cmd == "GADAI") {
      try {
        if (!mortgageProperty(target))
          throw PropertyMortgageException(
              target->getCode(), "Gagal gadai. Pastikan bangunan kosong dan "
                                 "properti belum tergadai.");
        cout << "[INFO] Berhasil menggadaikan " << target->getCode() << ".\n";
      } catch (const NimonspoliException &e) {
        cout << "Error: " << e.what() << "\n";
      }
    }
    buildAssetLists();
  }
  if (!isDebtSatisfied()) {
    return false;
  }
  return true;
}
vector<Property *> BankruptcyHandler::declareBankrupt() {
  debtor->setStatus(BANKRUPT);
  vector<Property *> repossessedList;
  if (creditor != nullptr) {
    transferAssets();
  } else {
    repossessedList = repossessProperties();
  }
  debtor->setBankrupt();
  return repossessedList;
}

void BankruptcyHandler::transferAssets() {
  vector<Property *> props = debtor->getProperties();
  for (size_t i = 0; i < props.size(); i++) {
    if (props[i] == nullptr)
      continue;
    creditor->addProperty(props[i]);
    props[i]->setOwner(creditor);
  }
  const int cashToTransfer = max(0, debtor->getCash());
  creditor->addCash(cashToTransfer);
  debtor->reduceCash(debtor->getCash());
  for (Property *prop : props) {
    if (prop != nullptr) {
      debtor->removeProperty(prop);
    }
  }
}

vector<Property *> BankruptcyHandler::repossessProperties() {
  vector<Property *> repossessedList;
  vector<Property *> props = debtor->getProperties();

  for (size_t i = 0; i < props.size(); i++) {
    if (props[i] == nullptr)
      continue;
    repossessedList.push_back(props[i]);

    props[i]->setOwner(nullptr);
    props[i]->setStatusStr("BANK");
    props[i]->setFestival(1, 0);
    props[i]->setBuildingCount(0);
  }

  for (size_t i = 0; i < props.size(); i++) {
    if (props[i] != nullptr) {
      debtor->removeProperty(props[i]);
    }
  }
  debtor->reduceCash(debtor->getCash());
  return repossessedList;
}
