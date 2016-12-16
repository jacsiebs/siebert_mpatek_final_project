///////////////////////////////////////////////////////////////////////////////
// File Name:      course_guide_main.cpp
//
// Author:         Jacob Siebert, Matt Patek
// CS email:       siebert@cs.wisc.edu, mpatek@cs.wisc.edu
//
// Description:    A program that takes in a uw madison student's info, 
//                 including a list of passed classes. It contacts a database
//                 to determine the requirements for the student's major and
//                 prints which requirements have been completed and which
// 		   classes need to be taken or gives class options if there
// 		   is a choice. Accepts the student data as a command line
//		   argument containing a txt file name.
///////////////////////////////////////////////////////////////////////////////

// Note: Must use the following tags to compile:
//       -lmysqlcppconn -lmysqlcppconn-static

// include standard c++ libs
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cctype>
#include <map>
// include custom classes
#include "Student.hpp"
#include "Course.hpp"

// include mysql/c++ connector headers
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

// define database info
#define DB "course_guide_system"

// define funcitons
void printCourses(std::vector<Course> courses);
void printStringVector(const std::vector<std::string> print_me);
std::vector<std::string> concatCourseNumsAndListings(std::vector<std::string> results);
std::vector<Course> createCourses(std::vector<std::string> listings, std::vector<std::string> nums);
void populateCourseData(std::vector<Course> &courses, std::unique_ptr<sql::Connection> &con);
void updateRequired(std::vector<Course> &required, std::vector<Course> &completed);

int main(int argc, char* argv[]) {
  // connect to the datatbase (course_guide_system)
  sql::Driver *driver;
  std::unique_ptr< sql::Connection > con;
  try {
    driver = get_driver_instance();
    con.reset(driver->
       connect("tcp://127.0.0.1:3306", "root", "2yzlzhb2"));
  }
  catch(sql::SQLException &e) {
    // connection could not be estabolished
    std::cout << "# ERR: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
    /* what() (derived from std::runtime_error) fetches error message */
    std::cout << "# ERR: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
  }
  con->setSchema(DB);// open course_guide_system database

  // create student object from text file
  // the list of passed courses will be made into course objects and stored within s
  Student s;
  if(argc > 1) {
    s = Student(argv[1]);
  } else {
    // no student data inputted
    std::cout << "Please input a student txt file" << std::endl << "Usage ./course_guide <Student.txt>" << std::endl;
    return 1;
  }
  s.printStudentData();  
  std::cout << std::endl; 
  // find the student's major id from the name of the major
  // use a prepared statement to call the stored procedure
  std::unique_ptr< sql::PreparedStatement > p_stmt(con->
      prepareStatement("CALL get_major_id(?, @m_id)"));
  std::unique_ptr< sql::ResultSet > res;
  std::unique_ptr< sql::Statement > stmt(con->createStatement());  
  int major_id;

  p_stmt->setString(1, s.getMajor());
  p_stmt->execute();
  
  res.reset(stmt->executeQuery("SELECT @m_id"));

  if(res->next()) {
    major_id = res->getInt("@m_id");
  }

  // find the courses for this major absolutely required to graduate
  // intialize a vector to hold these courses
  std::vector<Course> requiredCourses;
  std::vector<std::string> results;// holds database results
  // use the get_abs_req_courses preocedure to get info from database
  // appends are used to construct the query statment which parameters
  std::string caller("CALL get_abs_req_courses(");
  caller.append(std::to_string(major_id));
  caller.append(")");
  stmt->execute(caller);

  // This statement will return the data from each column as 1 big list
  // the first half of the result set will be the course nums and second half is the listing
  do {
    res.reset(stmt->getResultSet());
    // get the course nums and the course listings
    while(res->next()) {
      std::string result = res->getString(1);
      results.push_back(result);
    }
  } while(stmt->getMoreResults());
  
  // results contains the first half course nums and second half course listings
  // concat the listing and course num to form an unique course id ( "CS" + 368 ->"CS368")
  // use this id to create a Course object for each course
  auto it_listing = results.begin() + results.size() / 2;
  for(auto it_num = results.begin(); it_num != (results.end() - results.size() / 2); ++it_num) {
    std::string concat = *it_listing;
    concat.append(*it_num);
    requiredCourses.push_back(Course(concat));
    ++it_listing;
  }

  // populate the required courses with the rest of the course data (credits, full name, and prereqs)
  populateCourseData(requiredCourses, con);
  // remove all courses from the required list which have already been completed
  std::vector<Course> completedCourses = s.getCompleted();
  updateRequired(requiredCourses, completedCourses);
  // print the required courses that still need to be taken
  std::cout << "Required Courses for " << s.getMajor() << ":" <<  std::endl;
  printCourses(requiredCourses);
  std::cout << std::endl;

  // get the elective classes with options
  std::map<std::string, int> optionClasses;
  caller.clear();
  caller = std::string("CALL getMajorOptions(");
  caller.append(std::to_string(major_id));
  caller.append(")");
  stmt->execute(caller);
  do {
    res.reset(stmt->getResultSet());
    // get option_class name and number of classes which need to be taken to fulfill the req
    while(res->next()) {
      std::string result = res->getString(1);
      optionClasses.insert(std::pair<std::string, int>(result, res->getInt(2)));
    }
  } while(stmt->getMoreResults());
   
  // if there is an electives options class - out it at the rear of vector
  // this is so courses are not used to fulfill electives beore the other reqs
  std::pair<std::string, int> electives;
  auto elective_ptr = optionClasses.find("Electives"); 
  bool hasElectives = false;
  if(elective_ptr != optionClasses.end()) {
    hasElectives = true;
    std::string n = elective_ptr->first;
    int a = elective_ptr->second;
    optionClasses.erase(elective_ptr);
    electives = std::pair<std::string, int>(n, a);
  }
  
  // iterate over the elective options classes and determine if they have been fulfilled
  for(auto it = optionClasses.begin(); it != optionClasses.end(); ++it) {
    // if the user has not fulfilled an option - present them with a list of choices
    std::vector<std::string> o_courseNums;
    std::vector<std::string> o_listings;
    std::vector<Course> o_courses;// create from the info in the above vectors
    // get the course info for this class from the database
    std::string caller("CALL getOptionalElectives('");
    caller.append(it->first);
    caller.append("', ");
    caller.append(std::to_string(major_id));
    caller.append(")");
    stmt->execute(caller);
  
    // get the results  
    do {
      res.reset(stmt->getResultSet());
      // get the course nums and the course listings
      while(res->next()) {
        std::string result = res->getString(1);
        o_courseNums.push_back(result);
        result = res->getString(2);
        o_listings.push_back(result);
      }
    } while(stmt->getMoreResults());
 
    o_courses = createCourses(o_listings, o_courseNums); 
    populateCourseData(o_courses, con);
    s.isFulfilled(it->first, o_courses, it->second);
    std::cout << std::endl;
  }

  // if a unique electives class exists - get the elective requriements
  if(hasElectives) {
    std::vector<std::string> o_courseNums;
    std::vector<std::string> o_listings;
    std::vector<Course> o_courses;// create from the info in the above vectors
    // any class above 400 counts as an elective
    // get the listing of this major
    std::string caller("CALL getListing(");
    caller.append(std::to_string(major_id));
    caller.append(", @lst)");
    stmt->execute(caller);
    res.reset(stmt->executeQuery("SELECT @lst"));
    std::string listing;
    if(res->next()) {
      listing = res->getString("@lst");
    }
    // call getElectives on this major listing
    caller.clear();
    caller = "CALL getElectives('";
    caller.append(listing);
    caller.append("')");
    stmt->execute(caller);
    // get the results  
    do {
      res.reset(stmt->getResultSet());
      // get the course nums and the course listings
      while(res->next()) {
        std::string result = res->getString(1);
        o_courseNums.push_back(result);
        result = res->getString(2);
        o_listings.push_back(result);
      }
    } while(stmt->getMoreResults());
    
    o_courses = createCourses(o_listings, o_courseNums); 
    populateCourseData(o_courses, con);   
    s.isFulfilled(electives.first, o_courses, electives.second);
  }  

  return 0;
}

// prints a vector of strings to cout
void printStringVector(const std::vector<std::string> print_me) {
  for(auto it = print_me.begin(); it != print_me.end(); ++it) {
    std::cout << *it << std::endl;
  }
}

// prints the data of each course in the vector
void printCourses(std::vector<Course> courses) {
  for(auto it = courses.begin(); it != courses.end(); ++it) {
    std::cout << "-";
    (*it).printData();
  }
}

/* Creates a vector of Courses from vectors of course numbers and listings
 * concatinates the class listing and course num into a string of the form "CS302"
 */
std::vector<Course> createCourses(std::vector<std::string> listings, std::vector<std::string> nums) {
  std::vector<Course> courses;
  auto it_listing = listings.begin();
  // halt once the course_num iterator is half way through the list as this is where the listings begin
  for(auto it_num = nums.begin(); it_num != nums.end(); ++it_num) {
    std::string concat = *it_listing;
    concat.append(*it_num);
    courses.push_back(Course(concat));
    ++it_listing;
  }
  return courses;
}

/*
 * Takes in a list of course objects with only the courseNum field non-null
 * Communicates with the database to get the number of credits, prereqs, and full name
 * and adds this info to the ocurse list.
 */
void populateCourseData(std::vector<Course> &courses, std::unique_ptr<sql::Connection> &con) {
  std::unique_ptr< sql::ResultSet > res;
  std::unique_ptr< sql::Statement > stmt(con->createStatement());
  // iterate over the courses
  for(auto it = courses.begin(); it != courses.end(); ++it) {
    // extract the course num and listing from the courseNum string
    // find where the number begins
    std::string courseNum = (*it).getCourseNum();
    int num;
    std::string listing;
    for(int i = 0; i < courseNum.length(); ++i) {
      if(std::isdigit(courseNum[i]) != 0) {
        listing = courseNum.substr(0, i-2);
        num = atoi(courseNum.substr(i-2, courseNum.length()).c_str());	
      } 
    } 
    // create a string for the exicute statement
    std::string exe("CALL getCourseData(");
    exe.append(std::to_string(num));
    exe.append(", '");
    exe.append(listing);
    exe.append("')");
    
    stmt->execute(exe);
    do {
      res.reset(stmt->getResultSet());
      // get the course name and credits
      while(res->next()) {
        std::string result = res->getString(1);
        (*it).setName(result);
        int result_i = res->getInt(2);
        (*it).setCredits(result_i);
      }
    } while(stmt->getMoreResults());

  }
}

// Removes the intersect of the 2 lists from the required vector
void updateRequired(std::vector<Course> &required, std::vector<Course> &completed) {
  for(auto it = required.begin(); it != required.end(); ++it) {
    for(auto tt = completed.begin(); tt != completed.end();  ++tt) {
      if((*it) == (*tt)) {
        required.erase(it);
 	--it;// decrement due to remove
      }
    }
  }
}
