
#include"DBConnector.h"
#include"Course.h"
#include"User.h"
#include<cstring>
#include<stdexcept>
#include<exception>
#include"sqlite3.h"
#include"Progrado.h"

/*database handle: initialize to nullptr*/
sqlite3* DB::DBConnector::m_ptr_progradoDatabase = nullptr;

DB::DBConnector::DBConnector()
{
    // open DB connection
    sqlite3_initialize();
    int rc = sqlite3_open_v2(Progrado::DB_LOCATION.c_str(),
                             &m_ptr_progradoDatabase,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL
                            );

    if(rc != SQLITE_OK)
    {
        throw std::runtime_error("Couldn't open database; Progrado has to quit\n");
    }

    createCoursesTable();
}

DB::DBConnector::~DBConnector()
{
    sqlite3_close(m_ptr_progradoDatabase);
    sqlite3_shutdown(); // shut down sqlite3 engine
}


bool DB::DBConnector::verifyUserCredentials(const std::string& t_uname, const std::string& t_pword)
{
    sqlite3_stmt *verifystmt = nullptr;

    int rc_verify = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::VERIFY_USER.c_str(),
                                       -1,
                                       &verifystmt,
                                       nullptr);

    if (rc_verify != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::verifyUserCredentials():Database failed query\n");

    const char* uname_to_bind =  t_uname.c_str();
    const char* pword_to_bind = t_pword.c_str();

    // bind username and password from input
    sqlite3_bind_text(
        verifystmt,
        sqlite3_bind_parameter_index(verifystmt,":userName"),
        uname_to_bind,
        -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_text(
        verifystmt,
        sqlite3_bind_parameter_index(verifystmt,":password"),
        pword_to_bind,
        -1,
        SQLITE_TRANSIENT);

    if(sqlite3_step(verifystmt)!= SQLITE_ROW) {
        return false;
    }
    else return true;

}

bool DB::DBConnector::courseAlreadyExists(const Progrado::Course& t_course)
{
    sqlite3_stmt *verifystmt = nullptr;

    int rc_verify = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::VERIFY_COURSE.c_str(),
                                       -1,
                                       &verifystmt,
                                       nullptr);

    if (rc_verify != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::courseAlreadyExists:Database failed query\n");

    const char* stmt_to_bind =  t_course[Progrado::courseId].c_str();  // overloaded [] operator

    sqlite3_bind_text(
        verifystmt,
        sqlite3_bind_parameter_index(verifystmt,":courseId"),
        stmt_to_bind,
        -1,
        SQLITE_TRANSIENT);

    std::string l_courseId;

    int chk =  sqlite3_step(verifystmt);


    if(chk != SQLITE_ROW) {
        return false;
    }
    else {
        return true;
    }

}


void DB::DBConnector::addNewUser(const Progrado::User& t_user)
{
    sqlite3_stmt *addUserStmt = nullptr;

    // prepare SQL create statement
    int rc_create = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::NEW_USER_TABLE.c_str(),
                                       -1,
                                       &addUserStmt,
                                       nullptr);

    if(rc_create != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::addNewUser(): Database failed query\n");


    if(sqlite3_step(addUserStmt) != SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::addNewUser(): Database failed query\n");

    // prepare to insert new user details

    addUserStmt = nullptr; // reset addUserStmt to null
    int rc_insert = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::INSERT_NEW_USER.c_str(),
                                       -1,            // negative parameter makes sqlite calculate addUserStmt size automatically
                                       &addUserStmt,
                                       nullptr);

    if(rc_insert != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::addNewUser():Database failed query\n");


    // bind the values here

    // t_user.get_lastName returns a temporary and the results of c_str is
    // only valid throughout the lifetime of the temporary/
    // end of the full expression (i.e. semi colon)


    for(int i = 0; i < t_user.get_countUserDetails(); i++)
    {
        std::string str = t_user[i];  // overloaded [] operator
        const char* str_to_bind = str.c_str();

        std::string str_bind_parameter = t_user.get_BindParam(i);
        const char* bind_parameter = str_bind_parameter.c_str();

        sqlite3_bind_text
        (addUserStmt,
         sqlite3_bind_parameter_index(addUserStmt, bind_parameter),
         str_to_bind,
         -1,
         SQLITE_TRANSIENT  // tells SQLITE that str_to_bind won't be alive when the stmt is executed
        );

    }

    // execute statement
    if(sqlite3_step(addUserStmt) != SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::addNewUser(): Bind Failed");

    // finalize statement and release resources
    sqlite3_finalize(addUserStmt);


}

bool DB::DBConnector::coursesTableExists()
{
    // call this function to check if the courses table is empty

    sqlite3_stmt* coursesExists = nullptr;
    bool exists =  false;

    int rc_exists = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::CHECK_IF_COURSE_TABLE_EXISTS.c_str(),
                                       -1,
                                       &coursesExists,
                                       nullptr);

    // yes, table doesnot exits
    if(rc_exists != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::courseTableExists(): Database failed query\n");

    int rc_exec = sqlite3_step(coursesExists);

    if(rc_exec != SQLITE_DONE) {
        exists = false;
    }
    else {
        exists = true;
    }

    sqlite3_finalize(coursesExists);

    return exists;
}

void DB::DBConnector::createCoursesTable()
{
    sqlite3_stmt* createCoursesTable = nullptr;

    int rc_create = sqlite3_prepare_v2(  m_ptr_progradoDatabase,
                                         Progrado::CREATE_COURSE_TABLE.c_str(),
                                         -1,
                                         &createCoursesTable,
                                         nullptr );

    if(rc_create != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::createCoursesTable(): DB Failed query");

    int rc_exec = sqlite3_step(createCoursesTable);

    if(rc_exec != SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::createCoursesTable(): DB Failed query execution");

    sqlite3_finalize(createCoursesTable);

}

bool DB::DBConnector::addCourse(const Progrado::Course& t_course)
{

    if(courseAlreadyExists(t_course)) return false;
    // must be guaranteed that course table is not empty. Verify from UI class
    sqlite3_stmt* addCourseStmt = nullptr;

    int rc_insert = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::INSERT_NEW_COURSE.c_str(),
                                       -1,
                                       &addCourseStmt,
                                       nullptr);

    if(rc_insert != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::addCourse(): DB Failed query preparation");


    // binding the values

    // bind course name
    for (int i = 0; i < t_course.getCountCourseDetails(); i++)
    {
        std::string str =  t_course[i];  // overloaded [] operator
        const char* str_to_bind = str.c_str();

        std::string bind_str = t_course.getCourseBindParam(i);
        const char* bind_param = bind_str.c_str();


        sqlite3_bind_text
        (
            addCourseStmt,
            sqlite3_bind_parameter_index(addCourseStmt, bind_param),
            str_to_bind,
            -1,
            SQLITE_TRANSIENT
        );

    }

    sqlite3_bind_int
    (
        addCourseStmt,
        sqlite3_bind_parameter_index(addCourseStmt, ":numCredits"),
        t_course.getNumCredits()
    );


    int rc_exec = sqlite3_step(addCourseStmt);

    if(rc_exec != SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::addCourse(): DB Failed query execution");

    sqlite3_finalize(addCourseStmt);

    return true;
}

/// UPDATE A COURSE : CAUTION CALL WITH CARE
bool DB::DBConnector::updateCourse(const std::string& t_oldCourseName,
                                   const Progrado::Course& t_newCourse)
{
    // prevent duplicates
    if(courseAlreadyExists(t_newCourse)) return false;

    sqlite3_stmt* updateCourseStmt = nullptr;

    int rc_update = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::UPDATE_COURSE.c_str(),
                                       -1,
                                       &updateCourseStmt,
                                       nullptr);

    if(rc_update != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::updateCourse(): DB Failed query preparation");

    //oldcourse

    // bind course name

    for(int i = 0; i < t_newCourse.getCountCourseDetails(); ++i)
    {
        std::string str_new_course_detail = t_newCourse[i];  // overloaded [] operator
        const char* bind_new_course_detail = str_new_course_detail.c_str();

        std::string str_bind_param = t_newCourse.getCourseBindParam(i);
        const char* bind_param = str_bind_param.c_str();

        sqlite3_bind_text
        (
            updateCourseStmt,
            sqlite3_bind_parameter_index(updateCourseStmt, bind_param),
            bind_new_course_detail,
            -1,
            SQLITE_TRANSIENT
        );
    }


// bind num credits
    sqlite3_bind_int
    (
        updateCourseStmt,
        sqlite3_bind_parameter_index(updateCourseStmt, ":numCredits"),
        t_newCourse.getNumCredits()
    );

    // old course for WHERE clause
    const char* old_course_name = t_oldCourseName.c_str();

    sqlite3_bind_text
    (
        updateCourseStmt,
        sqlite3_bind_parameter_index(updateCourseStmt, ":OldCourseName"),
        old_course_name,
        -1,
        SQLITE_STATIC
    );

    //execute
    int rc_exec = sqlite3_step(updateCourseStmt);

    if(rc_exec!= SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::addCourse(): DB Failed query execution");

    sqlite3_finalize(updateCourseStmt);

    return true;

}

//CAUTION: CALL WITH CARE | DELETE A COURSE
void DB::DBConnector::removeCourse(const Progrado::Course& t_course)
{

    sqlite3_stmt* removeCourseStmt = nullptr;

    int rc_delete = sqlite3_prepare_v2(m_ptr_progradoDatabase,
                                       Progrado::REMOVE_COURSE.c_str(),
                                       -1,
                                       &removeCourseStmt,
                                       nullptr);

    if(rc_delete != SQLITE_OK)
        throw std::runtime_error("DB::DBConnector::removeCourse(): DB Failed query preparation");

    // bind courseid for WHERE clause
    std::string str_course_id =  t_course.getCourseId();
    const char* course_id = str_course_id.c_str();

    sqlite3_bind_text
    (
        removeCourseStmt,
        sqlite3_bind_parameter_index(removeCourseStmt, ":courseId"),
        course_id,
        -1,
        SQLITE_STATIC
    );

    // execute
    int rc_exec = sqlite3_step(removeCourseStmt);

    if(rc_exec != SQLITE_DONE)
        throw std::runtime_error("DB::DBConnector::addCourse(): DB Failed query execution");
}


std::vector< std::shared_ptr<Progrado::Course> >
DB::DBConnector::getCoursesMatching(const int t_flag)
{
    sqlite3_stmt* getCoursesStmt = nullptr;

    using namespace Progrado;


    switch (t_flag)
    {
    case(FRESHMAN_FALL):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_FRHMN_FALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(FRESHMAN_JTERM):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_FRHMN_JTERM_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case (FRESHMAN_SPRING):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_FRHMN_SPRG_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case (FRESHMAN_SUMMER):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_FRHMN_SUMMER_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;


    case(SOPH_FALL):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SOPH_FALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(SOPH_JTERM):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SOPH_JTERM_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(SOPH_SPRING):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SOPH_SPRG_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(SOPH_SUMMER):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SOPH_SUMMER_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(JUNIOR_FALL):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_JUN_FALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(JUNIOR_JTERM):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_JUN_JTERM_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(JUNIOR_SPRING):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_JUN_SPRG_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(JUNIOR_SUMMER):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_JUN_SUMMER_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    case(SENIOR_FALL):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SEN_FALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;


    case(SENIOR_SPRING):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_SEN_SPRG_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;


    case(ALL):
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_ALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    default:
        sqlite3_prepare_v2(
            m_ptr_progradoDatabase,
            GET_ALL_COURSES.c_str(),
            -1,
            &getCoursesStmt,
            nullptr);

        break;

    } // switch (t_flag)


    if(sqlite3_step(getCoursesStmt) != SQLITE_ROW)
        std::cout << "-------\n**absence of course in some semesters\n";
    // reset statement for execution anew
    sqlite3_reset(getCoursesStmt);

    std::vector< std::shared_ptr<Progrado::Course> > r_CoursesVector;

    while (sqlite3_step(getCoursesStmt) == SQLITE_ROW)
    {
        int ct = sqlite3_column_count(getCoursesStmt);


        std::vector<std::string> l_course;
        int l_numCredits(0);

        for( int i = 0; i < ct; i++ )
        {
            /* the following if statement assumes the last column is always Progrado::Course::numCredits
            sqlite3_column_decltype should be the appropriate method, but it doesn't work for
            some reason. Will try to fix this. */

            if(ct - 1 == i)
                l_numCredits = sqlite3_column_int(getCoursesStmt, i);
            else  /* cast const unsigned char* returned by sqlite3_column_text to const char* */
                l_course.push_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(getCoursesStmt, i))));

        } // end for

        r_CoursesVector.push_back(
            std::make_shared<Progrado::Course>(l_course, l_numCredits)
        );
    } // end while

    return r_CoursesVector;

} // getCoursesMatching

Progrado::Course DB::DBConnector::searchCourse(const std::string& t_search_term)
{
  sqlite3_stmt* search_stmt = nullptr;
  int rc = sqlite3_prepare_v2(m_ptr_progradoDatabase,
  Progrado::SEARCH_COURSE.c_str(),
  -1,
  &search_stmt,
  nullptr);

  if (rc != SQLITE_OK) throw std::runtime_error("DBSearch: failed query\n");

  sqlite3_bind_text(search_stmt,
  sqlite3_bind_parameter_index(search_stmt,":searchTerm"),
  t_search_term.c_str(),
  -1,
  SQLITE_TRANSIENT);

        int ct = sqlite3_column_count(search_stmt);


        std::vector<std::string> l_course;
        int l_numCredits(0);

  if(sqlite3_step(search_stmt)== SQLITE_ROW)
  {
         for( int i = 0; i < ct; i++ )
        {
            if(ct - 1 == i)
                l_numCredits = sqlite3_column_int(search_stmt, i);
            else  /* cast const unsigned char* returned by sqlite3_column_text to const char* */
                l_course.push_back(reinterpret_cast<const char*>(sqlite3_column_text(search_stmt, i)));

        } // end for
               
  }
  std::unique_ptr<Progrado::Course> crs_ptr(new Progrado::Course(l_course, l_numCredits));
  return *crs_ptr;

}



std::vector < std::vector< std::shared_ptr<Progrado::Course> > >
DB::DBConnector::getScheduleSummary()
{

    using namespace Progrado;
    std::vector < std::vector< std::shared_ptr<Course> > > r_TermVector;

    // this->getCoursesMatching(PARAMETER) is implicit here
    r_TermVector.push_back(getCoursesMatching(FRESHMAN_FALL));
    r_TermVector.push_back(getCoursesMatching(FRESHMAN_JTERM));
    r_TermVector.push_back(getCoursesMatching(FRESHMAN_SPRING));
    r_TermVector.push_back(getCoursesMatching(FRESHMAN_SUMMER));
    r_TermVector.push_back(getCoursesMatching(SOPH_FALL));
    r_TermVector.push_back(getCoursesMatching(SOPH_JTERM));
    r_TermVector.push_back(getCoursesMatching(SOPH_SPRING));
    r_TermVector.push_back(getCoursesMatching(SOPH_SUMMER));
    r_TermVector.push_back(getCoursesMatching(JUNIOR_FALL));
    r_TermVector.push_back(getCoursesMatching(JUNIOR_JTERM));
    r_TermVector.push_back(getCoursesMatching(JUNIOR_SPRING));
    r_TermVector.push_back(getCoursesMatching(JUNIOR_SUMMER));
    r_TermVector.push_back(getCoursesMatching(SENIOR_FALL));
    r_TermVector.push_back(getCoursesMatching(SENIOR_SPRING));

    return r_TermVector;
}

std::vector<std::string> DB::DBConnector::retrieveUserDetails()
{
    sqlite3_stmt* retrieveStmt = nullptr;
    int rc = sqlite3_prepare_v2(m_ptr_progradoDatabase,
    Progrado::RETRIEVE_USER.c_str(),
    -1,
    &retrieveStmt,
    nullptr);

    if(rc != SQLITE_OK)
         throw std::runtime_error("User table does not exist, or query preparation failed\n");

    std::vector<std::string> user_vector;

   

    while(sqlite3_step(retrieveStmt) == SQLITE_ROW)
    {
        for(int i = 0; i < Progrado::User::num_user_details; i++)
        {
            user_vector.push_back(reinterpret_cast<const char*>(sqlite3_column_text(retrieveStmt, i)));
        }
    }   

    return user_vector;  
}