#include"init.h"
#include"create_account.h"
#include"retrieve_user_details.h" 
#include"login.h"
#include<iostream>
#include<fstream>
#include<stdexcept>

using namespace Progrado;
using namespace Progrado::UI;

init::~init() {}

bool init::is_new_user()
{
    /*this works (yay!), but it's a rough patch*/
    /*the following creates a file on inital start of progrado*/
    /*that it will eventually use to check if an acct exists*/
    std::ifstream infile(".acct_status.txt");
    if(infile.fail())
    {
        std::ofstream outfile(".acct_status.txt");
        if(outfile.fail())
            throw std::runtime_error("Can't verify acct status: init file creation failed");
        outfile << 0;
        outfile.close(); 

        infile.open(".acct_status.txt");
    }

    int x;
    infile >> x;

    infile.close();
    return x == 0;
}
bool init::execute()
{
    if(is_new_user())
    {
        std::cout << "Create an account to continue: enter any key or 0 to exit\n > ";
        std::string reply;
        std::cin >> reply;
        std::cin.ignore();

        if(reply == "0") return false;
        else
        {
            auto transfer_to_create_acct = std::make_unique<create_account>();
            transfer_to_create_acct->execute();
                /* then login*/
            auto transfer_to_login = std::make_unique<login>(); 
            transfer_to_login->prompt_user(); 

            int login_status = transfer_to_login->execute();
            /*check if login was successful*/
            while(login_status != 1)   
            {
                if(login_status == 0) return false; 
                else if(login_status == 2)   
                {   
                    /*allow user to change his details after answering a security question*/
                    auto transfer_to_retrieve_acct = std::make_unique<retrieve_user_details>();
                    transfer_to_retrieve_acct->print_instructions();
                    transfer_to_retrieve_acct->execute(); 
                    
                    /*take user to login after retrieving*/ 
                    transfer_to_login->prompt_user();
                    login_status = transfer_to_login->execute();  
                }
            }
            
            
            return true;  
        }
     
    }
    else
    {       std::cout << "\t\tLOGIN\n";
            std::cout << "Hit any key and enter to continue, or 0 to exit\n > "; 
            std::string choice;
            std::cin >> choice;
            std::cin.ignore();

            if(choice == "0") return false;
            else
            {
                auto transfer_to_login = std::make_unique<login>();
                transfer_to_login->prompt_user();

                int login_status = transfer_to_login->execute(); 

                while(login_status != 1)   //while login is unsuccessful
                {
                    if(login_status == 0) return false; 
                    else if(login_status == 2)   
                    {   
                        //allow user to change his details after answering a security question
                        auto transfer_to_retrieve_acct = std::make_unique<retrieve_user_details>();
                        transfer_to_retrieve_acct->print_instructions();
                        transfer_to_retrieve_acct->execute(); 
                        
                        //take user to login after retrieving 
                        transfer_to_login->prompt_user();
                        login_status = transfer_to_login->execute();  
                    }
                }
                return true;
            }
             
    }
    
}