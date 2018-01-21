#include <iostream>

#include "MainForm.h"

int main()
{
	MainForm mainForm;

	mainForm.Init();

	mainForm.CreateGUI();

	mainForm.ShowModal();
}