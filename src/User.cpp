/*
**************************************
*************PROGRADO*****************
**************************************
User header file
Authors: Ibrahim Suleiman, Yusuf Jimoh
Copyright 2019
***************************************
 */


#include"User.h"

Progrado::User::User():
m_firstName("Not set"),
m_lastName("Not set"),
m_yearInCollege("Not set"),
m_userName("Not set"),
m_major("Undecided"),
m_minor("none")
{
    // default constructor
}

Progrado::User::User(const std::string t_userArray[]):
m_firstName(t_userArray[0]),
m_lastName(t_userArray[1]),
m_yearInCollege(t_userArray[2]),
m_userName(t_userArray[3]),
m_major(t_userArray[4]),
m_minor(t_userArray[5]),
m_password(t_userArray[6])

{
    // multi-arg constructor

}

Progrado::User::~User()
{


}

inline std::string Progrado::User::get_lastName()
 {
     return m_lastName;
 }

inline std::string Progrado::User::get_firstName()
 {
     return m_firstName;
 }

inline std::string Progrado::User::get_yearInCollege()
{
    return m_yearInCollege;
}

inline std::string Progrado::User::get_userName()
{
    return m_userName;
}

inline std::string Progrado::User::get_password()
{
    return m_password;
}

inline std::string Progrado::User::get_major()
{
    return m_major;
}

inline std::string Progrado::User::get_minor()
{
    return m_minor;
}