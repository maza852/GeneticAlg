#include "stdafx.h"
#include "Out_Neiron.h"
#include <cmath>
#include <iostream>
using namespace std;

Out_Neiron::Out_Neiron(void)
{
}


Out_Neiron::~Out_Neiron(void)
{
}

float Out_Neiron::sum()
{
	float result = 0;
	for (auto it = left_neirons.begin(); it != left_neirons.end(); it++)
	{
		result += it->first * it->second->get_value();
	}
	return result;
}

float Out_Neiron::activation()
{
	value = 1/(1 + exp(-sum() ));
	return value;
}