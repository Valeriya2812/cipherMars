// cipherMars.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include "Mars.h"
#include <iostream>
#include <ctime>

using namespace std;
int main()
{

	char text[500];
	cout << "Enter text: " << endl;
	cin.getline(text, 500);
	Mars object;
	object.encryption(text);
	cout << endl;
	system("pause");
	return 0;
}

