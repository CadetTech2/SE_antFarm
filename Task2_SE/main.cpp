#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <cassert>

using namespace std;

// Ant Types
enum class AntType
{
    DRONE,
    WARRIOR,
    QUEEN
};

// Species class
class Species
{
public:
    Species(const string &name, int strengthBonus, int efficiencyBonus, int harvestBonus)
        : speciesName(name), strengthBonus(strengthBonus), efficiencyBonus(efficiencyBonus), harvestBonus(harvestBonus) {}

    string getName() const { return speciesName; }
    int getStrengthBonus() const { return strengthBonus; }
    int getEfficiencyBonus() const { return efficiencyBonus; }
    int getHarvestBonus() const { return harvestBonus; }

private:
    string speciesName;
    int strengthBonus;
    int efficiencyBonus;
    int harvestBonus;
};

// Ant Base Class
class Ant
{
public:
    Ant(const string &name, const Species &species, AntType type)
        : name(name), species(species), type(type), energy(100), foodConsumption(10) {}

    virtual void act() = 0;

    void rest()
    {
        energy = min(100, energy + 20);
    }

    void work()
    {
        energy = max(0, energy - 10);
    }

    bool consumeFood(int &foodSupply)
    {
        if (foodSupply >= foodConsumption)
        {
            foodSupply -= foodConsumption;
            return true;
        }
        return false;
    }

    bool needsRest() const { return energy < 30; }

    AntType getType() const { return type; }

    virtual ~Ant() = default;

protected:
    string name;
    const Species &species;
    AntType type;
    int energy;
    int foodConsumption;
};

// Drone Ant
class DroneAnt : public Ant
{
public:
    DroneAnt(const string &name, const Species &species)
        : Ant(name, species, AntType::DRONE) {}

    void act() override
    {
        if (needsRest())
            rest();
        else
            work();
    }
};

// Warrior Ant
class WarriorAnt : public Ant
{
public:
    WarriorAnt(const string &name, const Species &species)
        : Ant(name, species, AntType::WARRIOR) {}

    void act() override
    {
        if (needsRest())
            rest();
        else
            work();
    }
};

// Queen Ant
class QueenAnt : public Ant
{
public:
    QueenAnt(const string &name, const Species &species)
        : Ant(name, species, AntType::QUEEN) {}

    void act() override
    {
        if (!needsRest())
            work();
    }
};

// Room Class
class Room
{
public:
    enum class RoomType
    {
        SPAWNING,
        RESTING,
        STORAGE,
        BATTLE
    };

    Room(const string &name, RoomType type, int capacity)
        : name(name), type(type), capacity(capacity) {}

    bool canAcceptMoreAnts() const
    {
        return ants.size() < capacity;
    }

    void addAnt(shared_ptr<Ant> ant)
    {
        if (canAcceptMoreAnts())
        {
            ants.push_back(ant);
        }
    }

    RoomType getType() const { return type; }

    const vector<shared_ptr<Ant>> &getAnts() const { return ants; }

private:
    string name;
    RoomType type;
    int capacity;
    vector<shared_ptr<Ant>> ants;
};

// Ant Farm Class
class AntFarm
{
public:
    AntFarm(const string &name, const Species &species)
        : name(name), species(species), foodSupply(1000), isActive(true) {}

    void addRoom(shared_ptr<Room> room)
    {
        rooms.push_back(room);
    }

    void setQueen(shared_ptr<QueenAnt> queen)
    {
        this->queen = queen;
        ants.push_back(queen);
    }

    void addAnt(shared_ptr<Ant> ant)
    {
        ants.push_back(ant);
    }

    void tick()
    {
        if (!isActive || !queen)
            return;

        for (auto &ant : ants)
        {
            ant->act();

            if (!ant->consumeFood(foodSupply))
            {
                isActive = false;
                return;
            }
        }
    }

    bool isActiveColony() const { return isActive && queen != nullptr; }

private:
    string name;
    const Species &species;
    shared_ptr<QueenAnt> queen;
    vector<shared_ptr<Ant>> ants;
    vector<shared_ptr<Room>> rooms;
    int foodSupply;
    bool isActive;
};

// Meadow Singleton
class Meadow
{
public:
    static Meadow &getInstance()
    {
        static Meadow instance;
        return instance;
    }

    shared_ptr<AntFarm> createAntFarm(const string &name, const Species &species)
    {
        auto farm = make_shared<AntFarm>(name, species);
        farms.push_back(farm);
        return farm;
    }

    bool simulationComplete() const
    {
        int activeColonies = count_if(farms.begin(), farms.end(), [](const auto &farm) {
            return farm->isActiveColony();
        });
        return activeColonies <= 1;
    }

    void tick()
    {
        for (auto &farm : farms)
        {
            farm->tick();
        }
    }

    const vector<Species> &getSpecies() const { return species; }

private:
    Meadow()
    {
        initializeSpecies();
    }

    void initializeSpecies()
    {
        int numSpecies = (38 % 6) + 10; // Example roll number

        for (int i = 0; i < numSpecies; ++i)
        {
            species.emplace_back("Species" + to_string(i), rand() % 5 + 5, rand() % 5 + 5, rand() % 5 + 5);
        }
    }

    vector<Species> species;
    vector<shared_ptr<AntFarm>> farms;
};

int main()
{
    srand(time(nullptr));

    Meadow &meadow = Meadow::getInstance();

    const auto &speciesList = meadow.getSpecies();

    auto farm1 = meadow.createAntFarm("Colony1", speciesList[0]);
    auto farm2 = meadow.createAntFarm("Colony2", speciesList[1]);

    auto queen1 = make_shared<QueenAnt>("Queen1", speciesList[0]);
    auto queen2 = make_shared<QueenAnt>("Queen2", speciesList[1]);

    farm1->setQueen(queen1);
    farm2->setQueen(queen2);

    farm1->addAnt(make_shared<DroneAnt>("Drone1", speciesList[0]));
    farm2->addAnt(make_shared<WarriorAnt>("Warrior1", speciesList[1]));

    int tickCount = 0;
    const int MAX_TICKS = 1000;

    while (!meadow.simulationComplete() && tickCount < MAX_TICKS)
    {
        meadow.tick();
        ++tickCount;
        cout << "Tick " << tickCount << " completed." << endl;
    }

    cout << "Simulation ended after " << tickCount << " ticks." << endl;

    return 0;
}
