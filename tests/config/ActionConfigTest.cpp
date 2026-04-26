#include "data/Configuration.hpp"
#include "data/config/ActionConfig.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void writesParsedActionFields() {
    ActionConfig action("31 PPJ Petak_Pergi_ke_Penjara SPESIAL DEFAULT");

    require(action.id == 31, "id should be parsed");
    require(action.code == "PPJ", "code should be uppercased");
    require(action.name == "Petak Pergi ke Penjara", "name should decode underscores");
    require(action.tileType == "SPESIAL", "tile type should be uppercased");
    require(action.color == "DEFAULT", "color should be uppercased");
}

void loadsActionConfigsFromAksiFile() {
    const std::filesystem::path dir = "build/test-action-config";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "aksi.txt";

    std::ofstream file(path);
    file << "ID KODE NAMA JENIS_PETAK WARNA\n";
    file << "1 GO Petak_Mulai SPESIAL DEFAULT\n";
    file << "3 DNU Dana_Umum KARTU DEFAULT\n";
    file.close();

    Configuration configuration(dir.string());
    configuration.loadActionConfigs(path.string());

    const ActionConfig* go = configuration.getActionConfig(1);
    const ActionConfig* danaUmum = configuration.getActionConfig(3);

    require(go != nullptr, "GO action config should be loaded by id");
    require(danaUmum != nullptr, "Dana Umum action config should be loaded by id");
    require(go->code == "GO", "GO code should match");
    require(go->tileType == "SPESIAL", "GO tile type should match");
    require(danaUmum->name == "Dana Umum", "Dana Umum name should decode underscores");
    require(configuration.getActionConfigs().size() == 2,
            "action config map should contain two rows");
}

void copyConfigFile(const std::filesystem::path& targetDir,
                    const std::string& filename) {
    std::filesystem::copy_file(std::filesystem::path("config") / filename,
                               targetDir / filename,
                               std::filesystem::copy_options::overwrite_existing);
}

void loadAllConfigsIncludesAksiFile() {
    Configuration configuration("config");
    configuration.loadAllConfigs();

    const ActionConfig* pbm = configuration.getActionConfig(39);
    require(pbm != nullptr, "PBM action config should be loaded by loadAllConfigs");
    require(pbm->code == "PBM", "PBM code should match");
    require(pbm->tileType == "PAJAK", "PBM tile type should match");
}

void boardTileConfigLoadsFromActionAndPropertyConfigsAtOnce() {
    Configuration configuration("config");
    configuration.loadActionConfigs("config/aksi.txt");
    configuration.loadPropertyConfigs("config/property.txt");

    const std::vector<BoardTileConfig> layout =
        BoardTileConfig::loadFromConfigs(configuration.getActionConfigs(),
                                         configuration.getAllPropertyConfigs());

    require(layout.size() == 40,
            "BoardTileConfig should load complete board from action and property configs");
    require(layout[0].code == "GO", "first generated board tile should be GO");
    require(layout[1].code == "GRT", "second generated board tile should be GRT");
    require(layout[2].code == "DNU", "third generated board tile should be DNU");
    require(layout[39].code == "IKN", "last generated board tile should be IKN");
}

void loadAllConfigsGeneratesBoardLayoutWithoutBoardFile() {
    const std::filesystem::path dir = "build/test-generated-board-config";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    copyConfigFile(dir, "aksi.txt");
    copyConfigFile(dir, "property.txt");
    copyConfigFile(dir, "railroad.txt");
    copyConfigFile(dir, "utility.txt");
    copyConfigFile(dir, "tax.txt");
    copyConfigFile(dir, "special.txt");
    copyConfigFile(dir, "misc.txt");

    Configuration configuration(dir.string());
    configuration.loadAllConfigs();

    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    require(layout.size() == 40, "generated board layout should contain 40 tiles");
    require(layout[0].code == "GO", "position 1 should come from GO action");
    require(layout[0].type == "GO", "GO action should generate GO tile type");
    require(layout[1].code == "GRT", "position 2 should come from Garut property");
    require(layout[1].type == "PROPERTY",
            "property config should generate PROPERTY tile type");
    require(layout[1].propertyCode == "GRT",
            "property layout entry should reference its property code");
    require(layout[30].code == "PPJ",
            "position 31 should come from Go To Jail action");
    require(layout[30].type == "GO_TO_JAIL",
            "PPJ action should generate GO_TO_JAIL tile type");
}

}  // namespace

int main() {
    try {
        writesParsedActionFields();
        loadsActionConfigsFromAksiFile();
        loadAllConfigsIncludesAksiFile();
        boardTileConfigLoadsFromActionAndPropertyConfigsAtOnce();
        loadAllConfigsGeneratesBoardLayoutWithoutBoardFile();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
