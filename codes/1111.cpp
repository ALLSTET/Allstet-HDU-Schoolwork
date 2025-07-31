#include <iostream>
#include <string>
using namespace std;

class Character
{
protected:
  string name;
  float healthpoints;
  virtual void Attack()
  {
    cout << "Character attacks!" << endl;
  }

public:
  Character(string a, float b) : name(a), healthpoints(b)
  {
    cout << "Name: " << name << '\t' << "HealthPoints: " << healthpoints << endl;
  }
  virtual ~Character() = default;
  void ShowInfo()
  {
    cout << "Name: " << name << '\t' << "HealthPoints: " << healthpoints << endl;
  }
  virtual void DoAttack()
  {
    Attack();
  }
};

class Warrior : virtual public Character
{
protected:
  void Attack() override
  {
    cout << "Sword Attack!" << endl;
  }

public:
  Warrior(string a) : Character(a, 30.0) {}
};

class Mage : virtual public Character
{
protected:
  void Attack() override
  {
    cout << "Magic Attack!" << endl;
  }

public:
  Mage(string a) : Character(a, 15.0) {}
};

class WarriorMage : public Warrior, public Mage
{
protected:
  void Attack() override final
  {
    cout << "Magic & Sword Attack!" << endl;
  }

public:
  WarriorMage(string a) : Character(a, 22.5), Warrior(a), Mage(a) {}
};

int main()
{
  cout << "Create Warrior:" << endl;
  Warrior w("Arthur");
  w.ShowInfo();
  w.DoAttack();

  cout << "\nCreate Mage:" << endl;
  Mage m("Merlin");
  m.ShowInfo();
  m.DoAttack();

  cout << "\nCreate WarriorMage:" << endl;
  WarriorMage wm("Gandalf");
  wm.ShowInfo();
  wm.DoAttack();

  return 0;
}