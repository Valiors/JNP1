#include <iostream>
#include <map>
#include <unordered_map>
#include <regex>

// Type aliases for easier modification and improved readability.
namespace {
    // Mileage is internally represented as an unsigned 64-bit integer
    // (which should be enough for any real use case) obtained by multiplying
    // original mileage by 10.
    using Mileage = uint_fast64_t;

    using LicensePlate = std::string;

    using RoadNumber = unsigned;
    using RoadCategory = char;

    using Road = std::pair<RoadNumber, RoadCategory>;

    using MileageByRoadCategory = std::map<RoadCategory, Mileage>;

    using CarStatistics = std::map<LicensePlate, MileageByRoadCategory>;
    using RoadStatistics = std::map<Road, Mileage>;

    using RoadEntranceData = std::tuple<LicensePlate, Road, Mileage>;

    using LineNumber = size_t;
    using Line = std::string;

    using UnpairedEntrance = std::map<LicensePlate, std::tuple<Road, Mileage, LineNumber>>;
}

// Conversions between raw and internal data representations.
namespace {
    // Converts internal Mileage representation to decimal mileage / 10.
    inline std::string internalMileageToMileage(const Mileage mileage) {
        return std::to_string(mileage / 10) + ',' + std::to_string(mileage % 10);
    }

    // Converts rawRoadNumber string to internal RoadNumber representation.
    inline RoadNumber rawRoadNumberToInternal(const std::string &rawRoadNumber) {
        return static_cast<RoadNumber>(stoul(rawRoadNumber));
    }

    // Converts rawRoadCategory string to internal RoadCategory representation.
    inline RoadCategory rawRoadCategoryToInternal(const std::string &rawRoadCategory) {
        return rawRoadCategory[0];
    }

    // Converts raw road data to internal Road representation.
    inline Road rawRoadDataToInternal(const std::string &rawRoadNumber,
                                      const std::string &rawRoadCategory) {

        const RoadNumber roadNumber = rawRoadNumberToInternal(rawRoadNumber);
        const RoadCategory roadCategory = rawRoadCategoryToInternal(rawRoadCategory);

        return {roadNumber, roadCategory};
    }

    // Converts raw mileage data to internal Mileage representation.
    inline Mileage rawMileageDataToInternal(const std::string &rawMileageIntegerPart,
                                            const std::string &rawMileageDecimalPart) {

        return static_cast<Mileage>(stoull(rawMileageIntegerPart) * 10
                                    + stoull(rawMileageDecimalPart));
    }
}

// Parsing line events.
namespace {
    // Parses line of the form - Car A1 13,4.
    RoadEntranceData parseRoadEntrance(std::smatch &matches) {
        const std::string licensePlate = matches.str(1);

        const std::string rawRoadCategory = matches.str(2);
        const std::string rawRoadNumber = matches.str(3);

        const Road road = rawRoadDataToInternal(rawRoadNumber, rawRoadCategory);

        const std::string rawMileageIntegerPart = matches.str(4);
        const std::string rawMileageDecimalPart = matches.str(5);

        const Mileage mileage = rawMileageDataToInternal(rawMileageIntegerPart,
                                                         rawMileageDecimalPart);

        return {licensePlate, road, mileage};
    }

    // Parses query concerning distance driven on all road categories by a given car.
    inline LicensePlate parseCarMileageQuery(std::smatch &matches) {
        return matches.str(1);
    }

    // Parses query concerning total distance driven by all cars on a given road.
    Road parseRoadMileageQuery(std::smatch &matches) {
        const std::string rawRoadCategory = matches.str(1);
        const std::string rawRoadNumber = matches.str(2);

        const Road road = rawRoadDataToInternal(rawRoadNumber, rawRoadCategory);

        return road;
    }
}

// Answer output.
namespace {
    // Outputs error concerning detected erroneous line.
    inline void outputErroneousLine(const Line &erroneousLine, const LineNumber lineNumber) {
        std::cerr << "Error in line " << lineNumber << ": " << erroneousLine << std::endl;
    }

    // Outputs mileage statistics, grouped by road categories, for a car with a
    // given licensePlate. Example: Car A 1,3 S 4,5.
    void outputCarMileageByRoadCategories(const MileageByRoadCategory &carMileage,
                                          const LicensePlate &licensePlate) {

        std::cout << licensePlate;

        for (const auto [roadCategory, mileage] : carMileage) {
            std::cout << " " << roadCategory << " " << internalMileageToMileage(mileage);
        }

        std::cout << std::endl;
    }

    // Outputs mileages, grouped by road categories, for all cars.
    void outputMileagesOfCarsByRoadCategories(const CarStatistics &mileagesOfCarsByRoadCategories) {
        for (const auto &[licensePlate, carMileage] : mileagesOfCarsByRoadCategories) {
            outputCarMileageByRoadCategories(carMileage, licensePlate);
        }
    }

    // Outputs mileage statistics, grouped by road categories, for a car
    // with given licensePlate from statistics structure.
    void outputCarMileageByRoadCategories(const CarStatistics &mileagesOfCarsByRoadCategories,
                                          const LicensePlate &licensePlate) {

        auto iterator = mileagesOfCarsByRoadCategories.find(licensePlate);

        if (iterator != mileagesOfCarsByRoadCategories.end()) {
            const auto &carMileage = iterator->second;

            outputCarMileageByRoadCategories(carMileage, licensePlate);
        }
    }

    // Outputs total distance driven by all cars on a given road.
    inline void outputRoadMileage(const Road &road, Mileage mileage) {
        const auto [roadNumber, roadCategory] = road;

        std::cout << roadCategory << roadNumber
                  << " " << internalMileageToMileage(mileage) << std::endl;
    }

    // Outputs total distance driven by all cars for every road.
    void outputMileagesOfRoads(const RoadStatistics &mileagesOfRoads) {
        for (const auto &[road, mileage] : mileagesOfRoads) {
            outputRoadMileage(road, mileage);
        }
    }

    // Outputs total distance driven by all cars on a given road from statistics structure.
    void outputRoadMileage(const RoadStatistics &mileagesOfRoads, const Road &road) {
        auto iterator = mileagesOfRoads.find(road);

        if (iterator != mileagesOfRoads.end()) {
            Mileage mileage = iterator->second;

            outputRoadMileage(road, mileage);
        }
    }
}

// Processing line events.
namespace {
    // Processes road entrance by updating information stored in statistics structures.
    void processRoadEntrance(std::unordered_map<LineNumber, Line> &inputLineByNumber,
                             UnpairedEntrance &unpairedCarEntrances,
                             CarStatistics &mileagesOfCarsByRoadCategories,
                             RoadStatistics &mileagesOfRoads,
                             const LicensePlate &licensePlate,
                             const Road &road,
                             const Mileage mileage,
                             const Line &line,
                             const LineNumber lineNumber) {

        auto iterator = unpairedCarEntrances.find(licensePlate);

        if (iterator != unpairedCarEntrances.end()) {
            const auto [previousRoad,
            previousMileage,
            previousLineNumber] = iterator->second;

            // Pair of information found, update structures.
            if (road == previousRoad) {
                Mileage distance = std::max(mileage, previousMileage) -
                                   std::min(mileage, previousMileage);

                RoadCategory roadCategory = road.second;

                mileagesOfCarsByRoadCategories[licensePlate][roadCategory] += distance;
                mileagesOfRoads[road] += distance;

                unpairedCarEntrances.erase(iterator);
            } else {
                // Previous information turns out to be wrong.
                outputErroneousLine(inputLineByNumber[previousLineNumber], previousLineNumber);
                iterator->second = {road, mileage, lineNumber};
                inputLineByNumber[lineNumber] = line;
            }

            inputLineByNumber.erase(previousLineNumber);
        } else {
            // If there was no previous information, simply insert current one.
            unpairedCarEntrances[licensePlate] = {road, mileage, lineNumber};
            inputLineByNumber[lineNumber] = line;
        }
    }

    // Processes car mileage query.
    void processCarMileageQuery(const CarStatistics &mileagesOfCarsByRoadCategories,
                                const LicensePlate &licensePlate) {

        outputCarMileageByRoadCategories(mileagesOfCarsByRoadCategories, licensePlate);
    }

    // Processes road mileage query.
    void processRoadMileageQuery(const RoadStatistics &mileagesOfRoads, const Road &road) {
        outputRoadMileage(mileagesOfRoads, road);
    }
}

int main() {
    // Map lineNumber -> line.
    std::unordered_map<LineNumber, Line> inputLineByNumber;

    // Map licensePlate -> ((roadNumber, roadCategory), mileage, lineNumber).
    UnpairedEntrance unpairedCarEntrances;

    // Map licensePlate -> (roadCategory -> mileage).
    CarStatistics mileagesOfCarsByRoadCategories;

    // Map (roadNumber, roadCategory) -> mileage.
    RoadStatistics mileagesOfRoads;

    const std::string carRegexGroup = R"(([A-Za-z0-9]{3,11}))";
    const std::string roadCategoriesRegexGroup = R"(([AS]))";
    const std::string roadRegexGroup = roadCategoriesRegexGroup + R"(([1-9]\d{0,2}))";
    const std::string mileageRegexGroup = R"((0|[1-9]\d*),(\d))";

    // Regexes for checking line correctness and parsing.
    std::regex roadEntranceRegex(R"(^\s*)" + carRegexGroup +
                                 R"(\s+)" + roadRegexGroup +
                                 R"(\s+)" + mileageRegexGroup + R"(\s*$)");

    std::regex carStatisticsQueryRegex(R"(^\s*\?\s*)" + carRegexGroup + R"(\s*$)");
    std::regex roadStatisticsQueryRegex(R"(^\s*\?\s*)" + roadRegexGroup + R"(\s*$)");
    std::regex allStatisticsQueryRegex(R"(^\s*\?\s*$)");

    LineNumber lineNumber = 0;

    Line line;
    while (getline(std::cin, line)) {
        lineNumber++;

        // Ignore empty lines.
        if (line.empty()) {
            continue;
        }

        // Match line against regexes and perform requested operations.
        if (std::smatch matches; regex_match(line, matches, roadEntranceRegex)) {
            const auto [licensePlate, road, mileage] = parseRoadEntrance(matches);

            processRoadEntrance(inputLineByNumber, unpairedCarEntrances,
                                mileagesOfCarsByRoadCategories, mileagesOfRoads,
                                licensePlate, road, mileage, line, lineNumber);

        } else if (regex_match(line, allStatisticsQueryRegex)) {
            outputMileagesOfCarsByRoadCategories(mileagesOfCarsByRoadCategories);
            outputMileagesOfRoads(mileagesOfRoads);
        } else {
            std::smatch matchesOfCarQuery;
            std::smatch matchesOfRoadQuery;

            if (regex_match(line, matchesOfCarQuery, carStatisticsQueryRegex)) {
                const LicensePlate licensePlate = parseCarMileageQuery(matchesOfCarQuery);

                processCarMileageQuery(mileagesOfCarsByRoadCategories, licensePlate);
            }

            if (regex_match(line, matchesOfRoadQuery, roadStatisticsQueryRegex)) {
                const Road road = parseRoadMileageQuery(matchesOfRoadQuery);

                processRoadMileageQuery(mileagesOfRoads, road);
            }

            if (matchesOfCarQuery.empty() && matchesOfRoadQuery.empty()) {
                outputErroneousLine(line, lineNumber);
            }
        }
    }
}