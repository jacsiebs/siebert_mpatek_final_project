///////////////////////////////////////////////////////////////////////////////
// File Name:      Student.hpp
//
// Author:         Jacob Siebert, Matt Patek
// CS email:       siebert@cs.wisc.edu, mpatek@cs.wisc.edu
//
// Description:    Represents a Student at uw madison and contains basic info
//                 like name, id, year. Also contains a list of passed classes.
//                 A Student can be created through a txt file.
//                 See Student.txt as an example.
///////////////////////////////////////////////////////////////////////////////

#ifndef Student_hpp
#define Student_hpp

#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "Course.hpp"
#include <algorithm>
#include <fstream>

/* Represents a Student at uw madison and contains basic info like name, id, year
 * Also contains a list of passed classes. A Student can be created through a txt file.
 * See Student.txt as an example.
 */
class Student {

private:
        int year;// 1=freshman, 4=senior
        std::string id, name, major;
        std::vector<Course> completed;// courses that have been passed
        std::vector<Course> usedToFulfillOption;// classes that have been used to fulfill elective options 

public:
        Student() {
                this->id = "";
                this->year = 0;
                this->name = "";
                this->major = "";
        };

 	// populate student data with a text file
        Student(std::string filename) {
		std::ifstream inFile;	
		inFile.open(filename);
		processStudent(inFile);
	}

        Student(std::string id, int year, std::string name, std::string major,
        std::vector<Course> &completed) {
                this->id = id;
                this->year = year;
                this->name = name;
                this->major = major;
                this->completed = completed;
        };

        void printStudentData() {
		std::cout << "Name: " << name << std::endl;
 		std::cout << "Year: " << year << std::endl;
		std::cout << "Major: " << major << std::endl;
		std::cout << "ID: " << id << std::endl;
                std::cout << "Completed Classes: " << std::endl;
                for(auto it = completed.begin(); it != completed.end(); ++it) {
 			std::cout << " ";
			(*it).printId();
		}
	}

        std::string getId() {
                return id;
        };

        int getYear() {
                return year;
        };

        std::string getName() {
                return name;
        };

        std::string getMajor() {
                return major;
        };

        const std::vector<Course> getCompleted() {
                return completed;
        };

        void setId(std::string id) {
                this->id = id;
        };

	void setYear(int year) {
                this->year = year;
        };

        void setName(std::string name) {
                this->name = name;
        };

        void setMajor(std::string major) {
                this->major = major;
        };

        void setCompleted(std::vector<Course> completed) {
                this->completed = completed;
        };

        // Below are student processing functions
        std::vector<Course> populateClasses(std::string line) {
                std::vector<Course> classes;
                std::stringstream ss(line);
                std::string token;
                while(getline(ss, token, ',')) {
                        Course c(token);
                	
                        classes.push_back(c);
                };
                return classes;
        };

        void processStudent(std::ifstream &inFile) {
                std::string line;
                int num = 0;
                while(getline(inFile, line)) {
                        // extract info
                        switch(num) {
                                case 0: setId(line);
                                case 1: setYear(atoi(line.c_str()));
                                case 2: setName(line);
                                case 3: setMajor(line);
                                case 4: completed = populateClasses(line);
                        }
                        num++;
                };

        };

        /*
         * Determines if the student has fulfilled the elective requirement. 
	 * If yes -> Mark this class as having fulfilled this req
	 * If no -> print a list of courses that the student can choose from
	 */
	void isFulfilled(std::string option_name, std::vector<Course> class_choices, int numRequired) {
        	std::cout << option_name << ":" << std::endl;
		// determine if the any of the classes have been taken
 		for(auto it = class_choices.begin(); it != class_choices.end(); ++it) {
       			for(auto it_completed = completed.begin(); it_completed != completed.end(); ++it_completed) {
				// check if the student has already completed a course on the list
			 	if((*it).getCourseNum().compare((*it_completed).getCourseNum()) == 0) {
					// must confirm this class is not already being used to fulfill some other req
                                        bool alreadyUsed = false;
					for(auto itr = usedToFulfillOption.begin(); itr != usedToFulfillOption.end(); ++itr) {
						if((*it).getCourseNum().compare((*itr).getCourseNum()) == 0) {
 							alreadyUsed = true;
							break;
						}
					}
					// use this class to fulfill this requirement
					if(!alreadyUsed) {
						std::cout << " " << (*it).getCourseNum() << " can be used to fulfill this requirement." 
							<< std::endl;
                                        	usedToFulfillOption.push_back(*it);
						// remove this class form the options list
						class_choices.erase(std::find(class_choices.begin(), class_choices.end(),
								*it));
						--numRequired;
						//  this req has been satisfied if numRequired=0
						if(numRequired == 0) {
							std::cout << " You've completed the " << option_name << " requirement." << std::endl;
							return;
						}
					}
				}	
			}	
		}
		// print a list of the class options along with the name of the class and how mnay courses must be taken
                std::cout << " You must take " << numRequired;
                if(numRequired > 1) {
			std::cout << " more classes ";
 		} else {
			std::cout << " more class ";
		}
		std::cout << "from the " << option_name << " category." << std::endl;
		std::cout << " A list of your choices:" << std::endl;
		for(auto it = class_choices.begin(); it != class_choices.end(); ++it) {
  			std::cout << " -";
			(*it).printData();
		}
        }

};

#endif





