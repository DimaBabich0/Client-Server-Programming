#pragma once
#include <iostream>
#include <string>
using namespace std;

class cDish
{
public:
	string szDishName;
	float fPrice;
	int dTime;

	cDish(string szDishName, float fPrice, int dTime)
	{
		this->szDishName = szDishName;
		this->fPrice = fPrice;
		this->dTime = dTime;
	}
};

void LoadDishes(vector <cDish>& aDishes)
{
    aDishes.push_back({ "Big Mac", 119.99, 20 });
    aDishes.push_back({ "Cheeseburger", 68.99, 8 });
    aDishes.push_back({ "Double Cheeseburger", 99.99, 12 });
    aDishes.push_back({ "Fries", 80.59, 6 });
    aDishes.push_back({ "Coca-Cola", 34.98, 3 });
    aDishes.push_back({ "Sprite", 34.98, 3 });
}

void PrintDishes(vector <cDish>& aDishes)
{
    cout << "\tMENU\n";
    for (int i = 0; i < aDishes.size(); i++)
    {
        cDish obj = aDishes[i];
        cout << obj.szDishName << ": " << obj.fPrice << " UAH; " << obj.dTime << " sec\n";
    }
    cout << "\n";
}