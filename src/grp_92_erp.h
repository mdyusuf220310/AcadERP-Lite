/* Group 92 */
#ifndef ERP_H
#define ERP_H

#include <iostream>
#include <string>

const int MAX_STUDENTS = 100;
const int MAX_COURSES  = 50;
const int MAX_REGS     = 40;
const int MAX_WORDS    = 8;    // max words stored per command line
const int MAX_CODE_LEN = 10;   // course code / roll number
const int MAX_NAME_LEN = 30;

enum ErrorCode {
    OK,
    E_PARSE,
    E_FULL,
    E_DUPLICATE,
    E_UNKNOWN_STUDENT,
    E_UNKNOWN_COURSE,
    E_BAD_COURSE,
    E_BAD_GRADE,
    E_GRADE_TYPE,
    E_NOT_REGISTERED,
    E_ALREADY_GRADED,
    E_THESIS_STATE,
    E_ALREADY_PASSED,
    E_ALREADY_REGISTERED,
    E_PREREQ,
    E_CREDIT_LIMIT,
    E_THESIS_NOT_ALLOWED,
    E_NO_GPA
};

enum CourseType { REGULAR, SU, THESIS };

enum ThesisState { NOT_THESIS, REGISTERED, IN_PROGRESS, FINAL, FAILED };

struct Course {
    std::string code;
    CourseType  type;
    int         credits;
    bool        isCore;     // false means ELECTIVE
    int         prereq;     // index into courses[], or -1
};

struct Registration {
    int         course;     // index into courses[]
    int         term;       // 1..12
    std::string grade;      // "" while ungraded, else "A", "B-", "S", ...
    ThesisState thesis;     // NOT_THESIS for non-thesis courses
};

struct Student {
    std::string   roll;
    std::string   name;
    Registration* regs;     // new Registration[MAX_REGS] in the constructor
    int           numRegs;

    Student();
    ~Student();
    void release();         // safe to call more than once

    Student(const Student&) = delete;
    Student& operator=(const Student&) = delete;
};

struct ProgramConfig {
    int totalCredits;
    int electiveCredits;
    int minGpaHundredths;   // 5.00 is stored as 500
    int maxLoad;
    int thesisCredits;
};

extern Course        courses[MAX_COURSES];
extern Student       students[MAX_STUDENTS];
extern int           numCourses;
extern int           numStudents;
extern ProgramConfig program;

// ---- setup / lookup ----
void initProgram();                              // defaults: 160 24 500 24 100
int  findStudent(const std::string& roll);       // -1 if not found
int  findCourse(const std::string& code);        // -1 if not found
int  splitWords(const std::string& line,
                std::string words[], int maxWords);  // returns word count

// ---- commands (each returns ErrorCode, caller must check) ----
ErrorCode setProgram(const std::string& line);
ErrorCode addCourse(const std::string& line);
ErrorCode addStudent(const std::string& line);
ErrorCode registerCourse(int s, int c, int term);
ErrorCode submitGrade(int s, int c, int term, const std::string& symbol);
ErrorCode convertThesis(int s, int c, const std::string& letter);

// ---- computations ----
int       gradePoints(const std::string& symbol);   // -1 if not a letter grade
ErrorCode gpaHundredths(int s, int* result);        // E_NO_GPA if d == 0
char      avgLetter(int h);                         // band from hundredths
int       checkEligibility(int s, int c, int term); // 0 if eligible, else k for Ek

// ---- helpers for rules ----
int  countingReg(int s, int c);                     // index of latest graded attempt, or -1
bool isPassed(int s, int c);                        // uses counting attempt only
int  termCredits(int s, int term);                  // credits registered in a term (E5)
int  earnedCredits(int s);                          // R1, E6
ErrorCode graduationFailures(int s, std::string& out);   // "R1,R4", or "" if graduated
ErrorCode printTranscript(int s);

// ---- driver ----
ErrorCode processCommand(const std::string& line, int lineNumber);
const char* errorName(ErrorCode error);
void cleanup();                                     // calls release() on every student

#endif