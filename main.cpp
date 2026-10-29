
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <stdexcept>

using namespace std;

// Abstract class
class Advisory {
public:
    virtual void giveAdvice() const = 0;
    virtual ~Advisory() = default;
};

// Base class
class Crop {
protected:
    string name, season;
    double area, yieldPerAcre;
    static int count;

public:
    Crop() : name("Unknown"), season("Unknown"),
             area(0), yieldPerAcre(0) {
        ++count;
    }

    Crop(string n, double a, string s, double y)
        : name(n), season(s), area(a), yieldPerAcre(y) {
        if (n.empty() || a <= 0 || y < 0)
            throw invalid_argument("Invalid crop details");
        ++count;
    }

    virtual ~Crop() { --count; }

    double totalYield() const {
        return area * yieldPerAcre;
    }

    virtual void display() const = 0;
    virtual void advice() const = 0;
    virtual void save(ofstream& file) const = 0;

    bool operator>(const Crop& other) const {
        return totalYield() > other.totalYield();
    }

    static int getCount() { return count; }
};

int Crop::count = 0;

// Derived class 1
class CerealCrop : public Crop, public Advisory {
    string fertilizer;

public:
    CerealCrop() : Crop(), fertilizer("None") {}

    CerealCrop(string n, double a, string s, double y,
               string f)
        : Crop(n, a, s, y), fertilizer(f) {}

    void display() const override {
        cout << fixed << setprecision(2)
             << "\nType: Cereal"
             << "\nName: " << name
             << "\nArea: " << area << " acres"
             << "\nSeason: " << season
             << "\nYield per acre: " << yieldPerAcre
             << "\nTotal yield: " << totalYield()
             << "\nFertilizer: " << fertilizer << "\n";
    }

    void advice() const override { giveAdvice(); }

    void giveAdvice() const override {
        cout << "Advice: Test soil before choosing fertilizer "
             << "and monitor pests regularly.\n";
    }

    void save(ofstream& file) const override {
        file << "C " << quoted(name) << " " << area << " "
             << quoted(season) << " " << yieldPerAcre << " "
             << quoted(fertilizer) << "\n";
    }
};

// Derived class 2
class VegetableCrop : public Crop, public Advisory {
    int irrigation;

public:
    VegetableCrop() : Crop(), irrigation(1) {}

    VegetableCrop(string n, double a, string s, double y,
                  int i)
        : Crop(n, a, s, y), irrigation(i) {
        if (i < 1)
            throw invalid_argument("Irrigation must be positive");
    }

    void display() const override {
        cout << fixed << setprecision(2)
             << "\nType: Vegetable"
             << "\nName: " << name
             << "\nArea: " << area << " acres"
             << "\nSeason: " << season
             << "\nYield per acre: " << yieldPerAcre
             << "\nTotal yield: " << totalYield()
             << "\nIrrigation per week: " << irrigation << "\n";
    }

    void advice() const override { giveAdvice(); }

    void giveAdvice() const override {
        cout << "Advice: Check soil moisture and avoid "
             << "overwatering vegetables.\n";
    }

    void save(ofstream& file) const override {
        file << "V " << quoted(name) << " " << area << " "
             << quoted(season) << " " << yieldPerAcre << " "
             << irrigation << "\n";
    }
};

// Input helpers
string readText(string prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    if (s.empty())
        throw invalid_argument("Text cannot be empty");
    return s;
}

double readNumber(string prompt, bool allowZero = false) {
    double x;
    cout << prompt;
    if (!(cin >> x) || x < 0 || (!allowZero && x == 0)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw invalid_argument("Invalid number");
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return x;
}

int readInt(string prompt) {
    int x;
    cout << prompt;
    if (!(cin >> x) || x < 1) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw invalid_argument("Enter a positive integer");
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return x;
}

// File handling
void saveCrops(const vector<unique_ptr<Crop>>& crops) {
    ofstream file("crops.txt");
    if (!file)
        throw runtime_error("Cannot open crops.txt");

    for (const auto& c : crops)
        c->save(file);

    cout << "Crops saved to crops.txt\n";
}

void loadCrops(vector<unique_ptr<Crop>>& crops) {
    ifstream file("crops.txt");
    if (!file) {
        cout << "No saved file found yet.\n";
        return;
    }

    vector<unique_ptr<Crop>> loaded;
    char type;
    string n, s;
    double a, y;

    while (file >> type >> quoted(n) >> a >> quoted(s) >> y) {
        try {
            if (type == 'C') {
                string f;
                if (file >> quoted(f))
                    loaded.push_back(make_unique<CerealCrop>(
                        n, a, s, y, f));
            } else if (type == 'V') {
                int i;
                if (file >> i)
                    loaded.push_back(make_unique<VegetableCrop>(
                        n, a, s, y, i));
            }
        } catch (const exception& e) {
            cout << "Skipped invalid crop: " << e.what() << "\n";
        }
    }

    crops = std::move(loaded);
    cout << crops.size() << " crops loaded.\n";
}

int main() {
    vector<unique_ptr<Crop>> crops;
    int choice = 0;

    do {
        cout << "\n===== SMART FARM MANAGEMENT =====\n"
             << "1. Add cereal crop\n"
             << "2. Add vegetable crop\n"
             << "3. Display all crops\n"
             << "4. Compare two crops\n"
             << "5. Get farming advice\n"
             << "6. Save crops\n"
             << "7. Load crops\n"
             << "8. Show crop count\n"
             << "9. Exit\n";

        try {
            choice = readInt("Enter choice: ");

            switch (choice) {
            case 1: {
                string n = readText("Crop name: ");
                double a = readNumber("Area in acres: ");
                string s = readText("Season: ");
                double y = readNumber("Yield per acre: ", true);
                string f = readText("Fertilizer: ");
                crops.push_back(make_unique<CerealCrop>(
                    n, a, s, y, f));
                cout << "Cereal crop added.\n";
                break;
            }
            case 2: {
                string n = readText("Crop name: ");
                double a = readNumber("Area in acres: ");
                string s = readText("Season: ");
                double y = readNumber("Yield per acre: ", true);
                int i = readInt("Irrigation times per week: ");
                crops.push_back(make_unique<VegetableCrop>(
                    n, a, s, y, i));
                cout << "Vegetable crop added.\n";
                break;
            }
            case 3:
                if (crops.empty())
                    cout << "No crops recorded.\n";
                for (const auto& c : crops)
                    c->display();
                break;

            case 4: {
                if (crops.size() < 2) {
                    cout << "Add at least two crops first.\n";
                    break;
                }
                for (size_t i = 0; i < crops.size(); ++i)
                    cout << i + 1 << ". Crop " << i + 1 << "\n";

                int x = readInt("First crop number: ");
                int y = readInt("Second crop number: ");

                if (x > static_cast<int>(crops.size()) ||
                    y > static_cast<int>(crops.size()) || x == y)
                    throw invalid_argument("Invalid crop selection");

                if (*crops[x - 1] > *crops[y - 1])
                    cout << "First selected crop has higher yield.\n";
                else if (*crops[y - 1] > *crops[x - 1])
                    cout << "Second selected crop has higher yield.\n";
                else
                    cout << "Both crops have equal yield.\n";
                break;
            }

            case 5: {
                if (crops.empty()) {
                    cout << "Add a crop first.\n";
                    break;
                }
                int x = readInt("Select crop number (1 to " +
                                to_string(crops.size()) + "): ");
                if (x > static_cast<int>(crops.size()))
                    throw invalid_argument("Invalid crop number");
                crops[x - 1]->advice();
                break;
            }

            case 6:
                saveCrops(crops);
                break;

            case 7:
                loadCrops(crops);
                break;

            case 8:
                cout << "Live crop objects: "
                     << Crop::getCount() << "\n";
                break;

            case 9:
                cout << "Thank you for using Smart Farm!\n";
                break;

            default:
                cout << "Choose an option from 1 to 9.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    } while (choice != 9);

    return 0;
}