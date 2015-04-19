// GeneticXOR.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include "In_Neiron.h"
#include "Internal_Neiron.h"
#include "Out_Neiron.h"
#include <omp.h>
#define N 9
#define GEN 40
using namespace std;
// под хранение 1 веса отведено 1 бит на знак и по 8 бит на целую и дробную части (всго 17)
string toBinInt(int number)
{
	string s;
	char sign;	// 0 - если положительное число
	(number < 0) ? sign = '1' : sign = '0';

	while(number)
	{
		( number%2 == 0 ) ? s.push_back('0') : s.push_back('1');
		number /= 2;
	}

	// дописываем нули и знак
	while (s.length() < 8)
		s.push_back('0');
	s.push_back(sign);

	// переворот строки
	char c;
	int len = s.length();
	for (int i = 0; i < len/2; ++i)
	{
		c = s[i];
		s[i] = s[len-1 - i];
		s[len-1 - i] = c;
	}

	return s;
}

string toBinFloat(float number)
{
	number = fabs(number);
	string s = "00000000";
	for (int i = 0; i < 9; ++i)
	{
		if (number == 0)
			break;
		number *= 2;
		if (number >= 1) 
		{
			s[i] = '1';
			number -= 1;
		}
		else 
			s[i] = '0';
	}
	return s;
}

string toBin(float number)
{
	string result;
	int part1 = number;
	float part2 = number - part1;
	return result = toBinInt(part1) + toBinFloat(part2);
}

int toNumberInt(string str)
{
	int result = 0;
	for (int i = str.length()-1, j = 0; i > 0; --i, ++j)
		result += (str[i] - '0') * pow(2, j);

	if (str[0] == '1')
		result *= -1;
	return result;
}

float toNumberFloat(string str)
{
	float result = 0;
	for (int i = 1; i <= str.length(); ++i)
		result += (str[i-1] - '0') * pow(0.5, i);
	return result;
}

float toNumber(string strWes)
{
	float result = 0;
	string sInt = strWes.substr(0, 9);
	string sFloat = strWes.substr(9, 17);

	int partInt = toNumberInt(sInt);
	if ( partInt < 0 )
	{
		partInt *= -1;
		result = partInt + toNumberFloat(sFloat);
		result *= -1; 
	}
	else
		result = partInt + toNumberFloat(sFloat);
	
	return result;
}

void initialize(string *gens)
{
	srand(time(0));
	cout << "»нициализируем сеть случайными весами:\n";
	float wes[N];
	for (int k = 0; k < GEN; ++k)
	{
		gens[k] = "";
		for (int i = 0; i < N; i++) 
		{
			wes[i] = 128 * (float)(rand()%2000 - 1000)/1000;
			gens[k] += toBin(wes[i]);
		}	
	}
	
	/*for (int i = 0; i < N; i++)
		cout << wes[i] << ends;*/
}

// функци€ приспособленности
float calcError(string wesStr)
{
	float wes[N];
	string temp;

	for (int i = 0; i < N; i++)
	{
		temp = wesStr.substr(17*i, 17*(i+1));
		wes[i] = toNumber(temp);
	}

	auto error_func = [&](float input1, float input2, float expected)
	{
		In_Neiron N1(input1);
		In_Neiron N2(input2);
		In_Neiron N3(1.0);

		Internal_Neiron iN1;
		Internal_Neiron iN2;
		Internal_Neiron iN3;

		Out_Neiron oN;

		iN1.AddNeiron(wes[0], &N1);
		iN1.AddNeiron(wes[1], &N2);
		iN1.AddNeiron(wes[2], &N3);

		iN2.AddNeiron(wes[3], &N1);
		iN2.AddNeiron(wes[4], &N2);
		iN2.AddNeiron(wes[5], &N3);

		oN.AddNeiron(wes[6], &iN1);
		oN.AddNeiron(wes[7], &iN2);
		oN.AddNeiron(wes[8], &iN3);

		// пр€мой проход 
		iN1.activation();
		iN2.activation();
		float output = oN.activation();

		float result = pow(expected - output, 2.0);

		return result;
	};

	return (error_func(1.0, 1.0, 0.0) + error_func(1.0, 0.0, 1.0) + error_func(0.0, 1.0, 1.0) + error_func(0.0, 0.0, 0.0))/4;
}

void initRandomIndexs(int *arr, int size)
{
	for (int i = 0; i < size; ++i)
		arr[i] = -1;

	int randIdx;
	for (int number = 0; number < size; ++number)
	{
		while(true)
		{
			randIdx = rand()%size;
			if (arr[randIdx] == -1)
			{
				arr[randIdx] = number;
				break;
			}
		}
	}
}

void selection(string *input_gens, string *after_selection_gens)
{
	int *indexs = new int[GEN];

	initRandomIndexs(indexs, GEN);

	string gen1, gen2;
	for (int i = 0; i < GEN; i += 2)
	{
		gen1 = input_gens[indexs[i]];
		gen2 = input_gens[indexs[i+1]];
		if ( calcError(gen1) < calcError(gen2) )
			after_selection_gens[i/2] = gen1;
		else
			after_selection_gens[i/2] = gen2;
	}

	delete [] indexs;
}

void crossingOver(string *parents, string *children)
{
	int number = GEN/2;
	int *indexs = new int[number];

	initRandomIndexs(indexs, number);

	string parent1, parent2, temp = "";
	int locus;
	int genLength = N * 17;		// длина одной хромосомы, где 17 - длина одного веса в 2-ом виде
	for (int i = 0; i < number; i += 2)
	{
		parent1 = parents[indexs[i]];
		parent2 = parents[indexs[i+1]];

		locus =  rand()%(genLength-2) + 1;

		temp = parent1.substr(0, locus);
		temp = temp + parent2.substr(locus, genLength-1);
		children[i] = temp;

		temp = parent2.substr(0, locus);
		temp = temp + parent1.substr(locus, genLength-1);
		children[i+1] = temp;
	}

	delete [] indexs;
}

void mutation(string *population, int size)
{
	int beginMutation = 5;		// число, при выпадении которого происходит мутаци€ гена
	int idx;					// индекс мутирующего гена 
	int genLength = N * 17;		// длина одной хромосомы, где 17 - длина одного веса в 2-ом виде
	for (int i = 0; i < size; ++i)
	{
		if (rand()%10 == beginMutation)
		{
			idx = rand()%genLength;
			( population[i].at(idx) == '0' ) ? population[i].at(idx) = '1': population[i].at(idx) = '0';
			//cout << "произошла мутаци€ хромосомы No "<< i <<" в гене No " << idx << endl;
		}
	}
}

void createNewPopulation(string *gens, string *parents, string *childrens)
{
	for (int i = 0; i < GEN/2; ++i)
	{
		gens[i] = parents[i];
		gens[i+20] = childrens[i];
	}
}

void Neiron_Net(float *wes, float input1, float input2)
{
	In_Neiron N1(input1);
	In_Neiron N2(input2);
	In_Neiron N3(1.0);

	Internal_Neiron iN1;
	Internal_Neiron iN2;
	Internal_Neiron iN3;

	Out_Neiron oN;

	iN1.AddNeiron(wes[0], &N1);
	iN1.AddNeiron(wes[1], &N2);
	iN1.AddNeiron(wes[2], &N3);

	iN2.AddNeiron(wes[3], &N1);
	iN2.AddNeiron(wes[4], &N2);
	iN2.AddNeiron(wes[5], &N3);

	oN.AddNeiron(wes[6], &iN1);
	oN.AddNeiron(wes[7], &iN2);
	oN.AddNeiron(wes[8], &iN3);

	iN1.activation();
	iN2.activation();
	float output = oN.activation();

	cout << endl << input1 << " ^ " << input2 << " = " << output << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "rus");

	string gens[GEN];
	string parents[GEN/2];
	string childrens[GEN/2];
	bool termCondition = false;		// условие завершени€ генетического алгоритма
	int iter = 0;
	int number;						// номер первой хромосомы в попул€ции, дл€ которой выполнилось условие завершени€ алгоритма

	initialize(gens);				// начальна€ инициализаци€ попул€ции

	cout << endl << "«апуск генетического алгоритма..." << endl;
	while(!termCondition)
	{
		iter++;

		selection(gens, parents);						// селекци€ хромосом в попул€ции

		crossingOver(parents, childrens);				// скрещивание наилучших хромосом

		mutation(childrens, GEN/2);						// мутаци€ новых хромосом

		createNewPopulation(gens, parents, childrens);	// создание новой попул€ции		

		for (int i = 0; i < GEN; ++i)					// проверка услови€ выхода
			if (calcError(gens[i]) < 0.01)
			{
				termCondition = true;
				number = i;
				break;
			}
	}

	cout << endl << "јлгоритм завершен за " << iter << " итераций" << endl;
	
	string chromosome = gens[number];

	cout << endl << "ѕодход€ща€ хромосома имеет индекс <" << number << "> в попул€ции" << endl
		<< endl << "≈Є код = " << endl << chromosome << endl
		<< endl << "≈Є приспособленность = " << calcError(chromosome) << endl;

	float wes[N];
	for (int i = 0; i < N; i++)
		wes[i] = toNumber( chromosome.substr( 17*i, 17*(i+1) ) );

	cout << endl << "¬ыход обученной сети:";
	Neiron_Net(wes, 1.0, 1.0);
	Neiron_Net(wes, 1.0, 0.0);
	Neiron_Net(wes, 0.0, 1.0);
	Neiron_Net(wes, 0.0, 0.0);

	getchar();
	return 0;
}

