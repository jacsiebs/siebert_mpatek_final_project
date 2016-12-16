///////////////////////////////////////////////////////////////////////////////
// File Name:      Course.hpp
//
// Author:         Jacob Siebert, Matt Patek
// CS email:       siebert@cs.wisc.edu, mpatek@cs.wisc.edu
//
// Description:    Represents a Course at uw madison and contains basic info
//                 like name, listing, number, credits.
//                 
///////////////////////////////////////////////////////////////////////////////

#ifndef classnode_hpp
#define classnode_hpp

#include <string>
#include <vector>

class Course {

private:
        int credits;
        std::string name, courseNum;

public:
        Course() {
                this->credits = 0;
                this->name = "";
                this->courseNum = "";
        };

	Course(std::string courseNum) {
		this->courseNum = courseNum;
                // filler
 		this->credits = 0;
                this->name = "";
	}

        Course(int credits, std::string name, std::string courseNum) {
                this->credits = credits;
                this->name = name;
                this->courseNum = courseNum;
        };

        bool operator==(const Course &other) const {
		if(courseNum.compare(other.courseNum) == 0) {
			return true;
		}
		return false;
	}

	bool operator!=(const Course &other) const {
		return !(*this==other);
	} 

	void printId() {
		std::cout << courseNum << std::endl;
	}	

        void printData() {
		std::cout << name << " - " << courseNum << std::endl;
  		std::cout << "   Credits = " << credits << std::endl;
  	}
        int getCredits() {
                return credits;
        };

        std::string getName() {
                return name;
        };

        std::string getCourseNum() {
                return courseNum;
        }

        void setCredits(int credits) {
                this->credits = credits;
        };

	void setName(std::string name) {
                this->name = name;
        };

        void setCourseNum(std::string courseNum) {
                this->courseNum = courseNum;
        };
};
#endif


