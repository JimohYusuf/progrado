/*
**************************************
*************PROGRADO*****************
**************************************
RemoveCourseScreen header file
Authors: Ibrahim Suleiman, Yusuf Jimoh
Copyright 2019
***************************************
 */

#ifndef REMOVE_COURSE_H
#define REMOVE_COURSE_H

namespace Ui
{
  class RemoveCourseScreen;  
} 

class Ui::RemoveCourseScreen : public Ui::Screen{

public:
    RemoveCourseScreen();
    ~RemoveCourseScreen();
    void display()override;

private:
// private members go here




};

#endif