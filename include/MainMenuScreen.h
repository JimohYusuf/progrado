/*
**************************************
*************PROGRADO*****************
**************************************
MainMenuScreen header file
Authors: Ibrahim Suleiman, Yusuf Jimoh
Copyright 2019
***************************************
 */

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

namespace Ui
{
  class MainMenuScreen;  
} 

class Ui::MainMenuScreen : public Ui::Screen{

public:
    MainMenuScreen();
    ~MainMenuScreen();
    void display()override;

private:
// private members go here




};

#endif