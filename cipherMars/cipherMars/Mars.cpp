#include "stdafx.h"
#include "Mars.h"
#include <iostream>
#include <ctime>

using namespace std;

void Mars::S_to_S0_S1() {
	int i, j = 0;
	for (i = 0; i < 256; i++)
		S0[i] = S[i];
	i--;
	for (j = 0; j < 256; j++)
	{
		S1[j] = S[i];
		i++;
	}
}

void Mars::create_key()
{
	srand(time(NULL));
	int i, j;
	n = rand() % 10 + 4;
	k = new unsigned int[n];
	for (i = 0; i < n; i++)
	{
		k[i] = 0;
		for (j = 0; j < 32; j++)
			k[i] |= (rand() % 2 + 0) << j;
	}
	cout << "\nKey: " << endl;
	for (i = 0; i < n; i++)
		cout << hex << k[i];
}

void Mars::key_expansion()
{
	int i, j, low_b, w;
	int B[] = { 0xa4a8d57b, 0x5b5d193b, 0xc8a8309b, 0x73f9a978 };//фиксированная таблица из четырех слов
	int T[15]; //временный массив, состоящий из 15 слов
			   //1 этап
	for (i = 0; i < n; i++)
		T[i] = k[i];
	T[n] = n;
	for (i = n + 1; i < 15; i++)
		T[i] = 0;
	//2 этап
	for (j = 0; j <= 3; j++)// четыре итерации, вычисление 10 слов K[] в каждой
	{
		for (i = 0; i <= 14; i++)
			T[i] = T[i] ^ (((T[(i - 7) % 15] ^ T[(i - 2) % 15]) << 3) | ((T[(i - 7) % 15] ^ T[(i - 2) % 15]) >> 29)) ^ (4 * i + j);


		for (i = 0; i <= 14; i++)
		{
			low_b = (T[(i - 1) % 15]) & 0x1ff;//младшие 9 битов T[i - 1 mod 15]]
			T[i] = ((T[i] + S[low_b]) << 9) | ((T[i] + S[low_b]) >> 23);// Вращение влево на 9 бит
		}
		for (i = 0; i <= 9; i++)// сохранение следующих 10 слов в K[]
			K[10 * j + i] = T[(4 * i) % 15];
	}
	//3 этап
	int num = 0, count, c = 0, a, ii, b, p, mask = 0;
	int r;//. r =младшие пять битов K[i - 1] // число позиций вращения
		  // модификация ключей, используемых для умножения
	for (i = 5; i <= 35; i += 2)
	{
		j = K[i] & 0x3;
		w = K[i] | 0x3;
		for (count = 0; count < 31; count++)
		{
			a = b = 0;
			a = (w >> count) & 0x0001;
			b = (w >> (count + 1)) & 0x0001;
			if (a == b)
				num++;
			else {
				if (num > 9)
				{
					for (ii = count - num + 2; ii < count - 1; ii++)
						mask |= 1 << ii;

				}
				num = 0;
			}
		}
		r = K[i - 1] & 0x01f;
		p = (B[j] << r) | (B[j] >> (32 - r));
		// модифицировать K[i] с помощью p под контролем маски M
		K[i] = w ^ (p & mask);
		cout << "\nExtended key: " << endl;
		for (i = 0; i < 40; i++)
			cout << hex << K[i];
	}
}

void Mars::set_w(char *word, int len)
{
	char w1[16];

	int i, j = 0;

	for (i = 0; i < 16; i++)
		w1[i] = NULL;

	for (i = 0; i < len; i++)
		w1[i] = word[i];
	for (i = 0; i < 16; i += 4)
	{
		D[j] = w1[i + 3] + (w1[i + 2] << 8) + (w1[i + 1] << 16) + (w1[i] << 24);
		j++;
	}
	cout << "\nИсходный код блока: " << endl;

	for (i = 0; i < 4; i++)
		cout << hex << D[i];
	// Первое наложение ключа на данные
	for (i = 0; i < 4; i++)
		D[i] = D[i] + K[i];
}

void Mars::E_function(unsigned int *out, unsigned int in, unsigned int key1, unsigned int key2)
{
	int i, r;
	out[1] = in + key1;
	out[2] = (in << 13) | (in >> 19)&key2;
	i = out[2] & 0x1ff;//i = младшие 9 битов M
	out[0] = S[i];
	out[2] = (out[2] << 5) | (out[2] >> 27);
	r = out[2] & 0x1f;
	out[1] = (out[1] << r) | (out[1] >> (32 - r));
	out[0] = out[0] ^ out[2];
	out[2] = (out[2] << 5) | (out[2] >> 27);
	out[0] = out[0] ^ out[2];
	r = out[3] & 0x1f;
	out[0] = (out[0] << r) | (out[0] >> (32 - r));//вращение на r влево
}

void Mars::direct_transformation() {
	int i, j, tmp;
	unsigned int out[3];
	for (i = 0; i < 16; i++) {
		E_function(out, D[0], K[2 * i + 4], K[2 * i + 5]);
		D[0] = (D[0] << 13) | (D[0] >> 19);
		D[2] = D[2] + out[1];
		if (i < 8)
		{
			D[1] = D[1] + out[0];
			D[3] = D[3] ^ out[2];
		}
		else {
			D[3] = D[3] + out[0];
			D[1] = D[1] ^ out[2];
		}
		tmp = D[0];
		for (j = 0; j < 3; j++)
			D[j] = D[j + 1];
		D[3] = tmp;

	}
	cout << "\nBlock cipher: " << endl;
	for (int i = 0; i < 4; i++)
		cout << hex << D[i];
}

void Mars::encryption(char *text)
{
	char word[16];
	int len, i = 0, j = 0;
	len = strlen(text);
	//S_to_S0_S1();
	create_key();
	key_expansion();

	while (i < len)
	{
		word[j] = text[i];
		if (j == 15) {
			set_w(word, j + 1);
			direct_transformation();
			decryption();
			j = -1;
		}
		j++;
		i++;
	}
	if (j != 0)
	{
		set_w(word, j);
		direct_transformation();
		decryption();
	}


}
void Mars::decryption() {
	int i, j, tmp;
	unsigned int out[3];


	for (i = 15; i >= 0; i--)
	{
		//вращение D[] на одно слово влево
		tmp = D[3];
		for (j = 3; j > 0; j--)
			D[j] = D[j - 1];
		D[0] = tmp;
		//
		D[0] = (D[0] >> 13) | (D[0] << 19);
		E_function(out, D[0], K[2 * i + 4], K[2 * i + 5]);
		D[2] = D[2] - out[1];
		if (i < 8)
		{
			D[1] = D[1] - out[0];
			D[3] = D[3] ^ out[2];
		}
		else {
			D[3] = D[3] - out[0];
			D[1] = D[1] ^ out[2];
		}

	}
	for (i = 0; i < 4; i++)
		D[i] = D[i] - K[i];

	cout << "\nText of block: " << endl;

	for (i = 0; i < 4; i++)
		cout << hex << D[i];


}