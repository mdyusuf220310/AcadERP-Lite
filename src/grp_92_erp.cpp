/* Group 92 */

#include "grp_92_erp.h"
#include <fstream>

// ============================================================
// Global data
// ============================================================

Course courses[MAX_COURSES];
Student students[MAX_STUDENTS];

int numCourses = 0;
int numStudents = 0;

ProgramConfig program = {160, 24, 500, 24, 100};

static bool firstCommandSeen = false;
static bool programCommandSeen = false;

// ============================================================
// Student memory management
// ============================================================

Student::Student()
    : roll(""), name(""), regs(new Registration[MAX_REGS]),
      numRegs(0) {
    for (int i = 0; i < MAX_REGS; i++) {
        regs[i].course = -1;
        regs[i].term = 0;
        regs[i].grade = "";
        regs[i].thesis = NOT_THESIS;
    }
}

void Student::release() {
    delete[] regs;
    regs = 0;
    numRegs = 0;
}

Student::~Student() {
    release();
}

void cleanup() {
    for (int i = 0; i < MAX_STUDENTS; i++) {
        students[i].release();
    }
}

void initProgram() {
    program.totalCredits = 160;
    program.electiveCredits = 24;
    program.minGpaHundredths = 500;
    program.maxLoad = 24;
    program.thesisCredits = 100;

    firstCommandSeen = false;
    programCommandSeen = false;
}

// ============================================================
// General helpers
// ============================================================

int splitWords(const std::string& line, std::string words[],
               int maxWords) {
    int count = 0;
    std::string current;

    for (std::size_t i = 0; i <= line.size(); i++) {
        char ch = (i < line.size()) ? line[i] : ' ';

        if (ch == ' ' || ch == '\t' || ch == '\r') {
            if (!current.empty()) {
                if (count < maxWords) {
                    words[count] = current;
                }
                count++;
                current.clear();
            }
        } else {
            current += ch;
        }
    }

    return count;
}

static bool parseNonNegativeInt(const std::string& word, int* result) {
    if (word.empty() || result == 0) {
        return false;
    }

    int value = 0;

    for (std::size_t i = 0; i < word.size(); i++) {
        char ch = word[i];

        if (ch < '0' || ch > '9') {
            return false;
        }

        int digit = ch - '0';

        if (value > (2147483647 - digit) / 10) {
            return false;
        }

        value = value * 10 + digit;
    }

    *result = value;
    return true;
}

int findStudent(const std::string& roll) {
    for (int i = 0; i < numStudents; i++) {
        if (students[i].roll == roll) {
            return i;
        }
    }
    return -1;
}

int findCourse(const std::string& code) {
    for (int i = 0; i < numCourses; i++) {
        if (courses[i].code == code) {
            return i;
        }
    }
    return -1;
}

const char* errorName(ErrorCode error) {
    switch (error) {
        case OK: return "OK";
        case E_PARSE: return "E_PARSE";
        case E_FULL: return "E_FULL";
        case E_DUPLICATE: return "E_DUPLICATE";
        case E_UNKNOWN_STUDENT: return "E_UNKNOWN_STUDENT";
        case E_UNKNOWN_COURSE: return "E_UNKNOWN_COURSE";
        case E_BAD_COURSE: return "E_BAD_COURSE";
        case E_BAD_GRADE: return "E_BAD_GRADE";
        case E_GRADE_TYPE: return "E_GRADE_TYPE";
        case E_NOT_REGISTERED: return "E_NOT_REGISTERED";
        case E_ALREADY_GRADED: return "E_ALREADY_GRADED";
        case E_THESIS_STATE: return "E_THESIS_STATE";
        case E_ALREADY_PASSED: return "E_ALREADY_PASSED";
        case E_ALREADY_REGISTERED: return "E_ALREADY_REGISTERED";
        case E_PREREQ: return "E_PREREQ";
        case E_CREDIT_LIMIT: return "E_CREDIT_LIMIT";
        case E_THESIS_NOT_ALLOWED: return "E_THESIS_NOT_ALLOWED";
        case E_NO_GPA: return "E_NO_GPA";
    }

    return "E_PARSE";
}

// ============================================================
// Program configuration
// ============================================================

ErrorCode setProgram(const std::string& line) {
    std::string words[MAX_WORDS];
    int count = splitWords(line, words, MAX_WORDS);

    if (count != 6 || programCommandSeen || firstCommandSeen) {
        return E_PARSE;
    }

    int total;
    int elective;
    int maxLoad;
    int thesisCredits;

    if (!parseNonNegativeInt(words[1], &total)
        || !parseNonNegativeInt(words[2], &elective)
        || !parseNonNegativeInt(words[4], &maxLoad)
        || !parseNonNegativeInt(words[5], &thesisCredits)) {
        return E_PARSE;
    }

    // GPA must have exactly two digits after the decimal point.
    const std::string& gpa = words[3];
    std::size_t dot = gpa.find('.');

    if (dot == std::string::npos || dot == 0
        || dot != gpa.rfind('.')
        || gpa.size() != dot + 3) {
        return E_PARSE;
    }

    int whole = 0;

    for (std::size_t i = 0; i < dot; i++) {
        if (gpa[i] < '0' || gpa[i] > '9') {
            return E_PARSE;
        }

        int digit = gpa[i] - '0';

        if (whole > (2147483647 - digit) / 10) {
            return E_PARSE;
        }

        whole = whole * 10 + digit;
    }

    if (gpa[dot + 1] < '0' || gpa[dot + 1] > '9'
        || gpa[dot + 2] < '0' || gpa[dot + 2] > '9') {
        return E_PARSE;
    }

    int fraction = (gpa[dot + 1] - '0') * 10
                 + (gpa[dot + 2] - '0');

    if (whole > 10 || (whole == 10 && fraction != 0)) {
        return E_PARSE;
    }

    int minGpa = whole * 100 + fraction;

    if (total <= 0 || elective > total || maxLoad <= 0) {
        return E_PARSE;
    }

    program.totalCredits = total;
    program.electiveCredits = elective;
    program.minGpaHundredths = minGpa;
    program.maxLoad = maxLoad;
    program.thesisCredits = thesisCredits;

    programCommandSeen = true;
    firstCommandSeen = true;

    return OK;
}

// ============================================================
// Course and student creation
// ============================================================

ErrorCode addCourse(const std::string& line) {
    std::string words[MAX_WORDS];
    int count = splitWords(line, words, MAX_WORDS);

    if (count != 6) {
        return E_PARSE;
    }

    if (words[1].empty() || words[1].size() > MAX_CODE_LEN) {
        return E_PARSE;
    }

    int credits;

    if (!parseNonNegativeInt(words[3], &credits)) {
        return E_PARSE;
    }

    if (findCourse(words[1]) >= 0) {
        return E_DUPLICATE;
    }

    if (numCourses >= MAX_COURSES) {
        return E_FULL;
    }

    CourseType type;

    if (words[2] == "REGULAR") {
        type = REGULAR;
    } else if (words[2] == "SU") {
        type = SU;
    } else if (words[2] == "THESIS") {
        type = THESIS;
    } else {
        return E_BAD_COURSE;
    }

    if (credits > 24 || (credits == 0 && type != SU)) {
        return E_BAD_COURSE;
    }

    bool isCore;

    if (words[4] == "CORE") {
        isCore = true;
    } else if (words[4] == "ELECTIVE") {
        isCore = false;
    } else {
        return E_BAD_COURSE;
    }

    int prereq = -1;

    if (words[5] != "-") {
        if (words[5] == words[1]) {
            return E_BAD_COURSE;
        }

        prereq = findCourse(words[5]);

        if (prereq < 0) {
            return E_BAD_COURSE;
        }
    }

    courses[numCourses].code = words[1];
    courses[numCourses].type = type;
    courses[numCourses].credits = credits;
    courses[numCourses].isCore = isCore;
    courses[numCourses].prereq = prereq;

    numCourses++;

    return OK;
}

ErrorCode addStudent(const std::string& line) {
    std::string words[MAX_WORDS];
    int count = splitWords(line, words, MAX_WORDS);

    if (count != 3) {
        return E_PARSE;
    }

    if (words[1].empty() || words[1].size() > MAX_CODE_LEN
        || words[2].empty() || words[2].size() > MAX_NAME_LEN) {
        return E_PARSE;
    }

    if (findStudent(words[1]) >= 0) {
        return E_DUPLICATE;
    }

    if (numStudents >= MAX_STUDENTS) {
        return E_FULL;
    }

    students[numStudents].roll = words[1];
    students[numStudents].name = words[2];
    students[numStudents].numRegs = 0;

    numStudents++;

    return OK;
}

// ============================================================
// Grade helpers
// ============================================================

int gradePoints(const std::string& symbol) {
    if (symbol == "A") return 10;
    if (symbol == "A-") return 9;
    if (symbol == "B") return 8;
    if (symbol == "B-") return 7;
    if (symbol == "C") return 6;
    if (symbol == "C-") return 5;
    if (symbol == "D") return 4;
    if (symbol == "F") return 0;

    return -1;
}

static bool isLetterGrade(const std::string& symbol) {
    return gradePoints(symbol) >= 0;
}

static bool isFailGrade(const Registration& reg,
                        const Course& course) {
    if (course.type == SU) {
        return reg.grade == "X";
    }

    if (course.type == THESIS) {
        return reg.thesis == FAILED || reg.grade == "F";
    }

    return reg.grade == "F";
}

int countingReg(int s, int c) {
    if (s < 0 || s >= numStudents || c < 0 || c >= numCourses) {
        return -1;
    }

    for (int i = students[s].numRegs - 1; i >= 0; i--) {
        Registration& reg = students[s].regs[i];

        if (reg.course == c && !reg.grade.empty()) {
            return i;
        }
    }

    return -1;
}

bool isPassed(int s, int c) {
    int r = countingReg(s, c);

    if (r < 0) {
        return false;
    }

    Registration& reg = students[s].regs[r];
    Course& course = courses[c];

    if (course.type == SU) {
        return reg.grade == "S";
    }

    if (course.type == THESIS) {
        return reg.thesis == FINAL
            && isLetterGrade(reg.grade)
            && gradePoints(reg.grade) >= 4;
    }

    return isLetterGrade(reg.grade) && gradePoints(reg.grade) >= 4;
}

// ============================================================
// Registration and eligibility
// ============================================================

int termCredits(int s, int term) {
    if (s < 0 || s >= numStudents) {
        return 0;
    }

    int total = 0;

    for (int i = 0; i < students[s].numRegs; i++) {
        Registration& reg = students[s].regs[i];

        if (reg.term == term && reg.course >= 0
            && reg.course < numCourses) {
            total += courses[reg.course].credits;
        }
    }

    return total;
}

int earnedCredits(int s) {
    if (s < 0 || s >= numStudents) {
        return 0;
    }

    int total = 0;

    for (int c = 0; c < numCourses; c++) {
        int r = countingReg(s, c);

        if (r < 0 || !isPassed(s, c)) {
            continue;
        }

        total += courses[c].credits;
    }

    return total;
}

int checkEligibility(int s, int c, int term) {
    // E1: student and course exist.
    if (s < 0 || s >= numStudents || c < 0 || c >= numCourses) {
        return 1;
    }

    // E2: the course is not already passed.
    if (isPassed(s, c)) {
        return 2;
    }

    // E3: no ungraded registration in this course.
    for (int i = 0; i < students[s].numRegs; i++) {
        Registration& reg = students[s].regs[i];

        if (reg.course == c) {
            if (courses[c].type == THESIS) {
                if (reg.thesis == REGISTERED || reg.thesis == IN_PROGRESS) {
                    return 3;
                }
            } else if (reg.grade.empty()) {
                return 3;
            }
        }
    }

    // Repeating is permitted only after a failed latest graded attempt.
    int previous = countingReg(s, c);

    if (previous >= 0
        && !isFailGrade(students[s].regs[previous], courses[c])) {
        return 3;
    }

    // E4: prerequisite must be passed.
    if (courses[c].prereq >= 0
        && !isPassed(s, courses[c].prereq)) {
        return 4;
    }

    // E5: maximum credits in a term.
    if (termCredits(s, term) + courses[c].credits > program.maxLoad) {
        return 5;
    }

    // E6: thesis requires enough earned credits.
    if (courses[c].type == THESIS
        && earnedCredits(s) < program.thesisCredits) {
        return 6;
    }

    return 0;
}

ErrorCode registerCourse(int s, int c, int term) {
    if (s < 0 || s >= numStudents) {
        return E_UNKNOWN_STUDENT;
    }

    if (c < 0 || c >= numCourses) {
        return E_UNKNOWN_COURSE;
    }

    if (term < 1 || term > 12) {
        return E_PARSE;
    }

    int check = checkEligibility(s, c, term);

    switch (check) {
        case 1: return E_UNKNOWN_STUDENT;
        case 2: return E_ALREADY_PASSED;
        case 3: return E_ALREADY_REGISTERED;
        case 4: return E_PREREQ;
        case 5: return E_CREDIT_LIMIT;
        case 6: return E_THESIS_NOT_ALLOWED;
        default: break;
    }

    if (students[s].numRegs >= MAX_REGS) {
        return E_FULL;
    }

    int index = students[s].numRegs;

    students[s].regs[index].course = c;
    students[s].regs[index].term = term;
    students[s].regs[index].grade = "";
    students[s].regs[index].thesis =
        (courses[c].type == THESIS) ? REGISTERED : NOT_THESIS;

    students[s].numRegs++;

    return OK;
}

// ============================================================
// Grade submission and thesis conversion
// ============================================================

ErrorCode submitGrade(int s, int c, int term,
                      const std::string& symbol) {
    if (s < 0 || s >= numStudents) {
        return E_UNKNOWN_STUDENT;
    }

    if (c < 0 || c >= numCourses) {
        return E_UNKNOWN_COURSE;
    }

    int r = -1;

    for (int i = 0; i < students[s].numRegs; i++) {
        Registration& reg = students[s].regs[i];

        if (reg.course == c && reg.term == term) {
            r = i;
            break;
        }
    }

    if (r < 0) {
        return E_NOT_REGISTERED;
    }

    Registration& reg = students[s].regs[r];
    Course& course = courses[c];

    if (!reg.grade.empty()) {
        return E_ALREADY_GRADED;
    }

    if (course.type == REGULAR) {
        if (symbol == "S" || symbol == "X") {
            return E_GRADE_TYPE;
        }

        if (!isLetterGrade(symbol)) {
            return E_BAD_GRADE;
        }

        reg.grade = symbol;
        return OK;
    }

    if (course.type == SU) {
        if (isLetterGrade(symbol)) {
            return E_GRADE_TYPE;
        }

        if (symbol != "S" && symbol != "X") {
            return E_BAD_GRADE;
        }

        reg.grade = symbol;
        return OK;
    }

    if (symbol != "S" && symbol != "X") {
        if (isLetterGrade(symbol)) {
            return E_GRADE_TYPE;
        }

        return E_BAD_GRADE;
    }

    reg.grade = symbol;

    if (symbol == "S") {
        reg.thesis = IN_PROGRESS;
    } else {
        reg.thesis = FAILED;
    }

    return OK;
}

ErrorCode convertThesis(int s, int c, const std::string& letter) {
    if (s < 0 || s >= numStudents) {
        return E_UNKNOWN_STUDENT;
    }

    if (c < 0 || c >= numCourses) {
        return E_UNKNOWN_COURSE;
    }

    if (courses[c].type != THESIS) {
        return E_GRADE_TYPE;
    }

    int r = -1;

    for (int i = students[s].numRegs - 1; i >= 0; i--) {
        if (students[s].regs[i].course == c) {
            r = i;
            break;
        }
    }

    if (r < 0) {
        return E_NOT_REGISTERED;
    }

    Registration& reg = students[s].regs[r];

    if (reg.thesis == FINAL) {
        return E_ALREADY_GRADED;
    }

    if (reg.thesis != IN_PROGRESS) {
        return E_THESIS_STATE;
    }

    if (!isLetterGrade(letter)) {
        return E_BAD_GRADE;
    }

    reg.grade = letter;
    reg.thesis = FINAL;

    return OK;
}

// ============================================================
// GPA and average grade
// ============================================================

ErrorCode gpaHundredths(int s, int* result) {
    if (s < 0 || s >= numStudents || result == 0) {
        return E_NO_GPA;
    }

    long long numerator = 0;
    long long denominator = 0;

    for (int c = 0; c < numCourses; c++) {
        int r = countingReg(s, c);

        if (r < 0) {
            continue;
        }

        Registration& reg = students[s].regs[r];
        Course& course = courses[c];

        if (course.type == SU) {
            continue;
        }

        if (course.type == THESIS && reg.thesis != FINAL) {
            continue;
        }

        int points = gradePoints(reg.grade);

        if (points < 0) {
            continue;
        }

        numerator += static_cast<long long>(course.credits) * points;
        denominator += course.credits;
    }

    if (denominator == 0) {
        return E_NO_GPA;
    }

    // Round half-up to two decimal places using integer arithmetic.
    long long h = (200 * numerator + denominator)
                / (2 * denominator);

    *result = static_cast<int>(h);

    return OK;
}

char avgLetter(int h) {
    if (h >= 950) return 'A';
    if (h >= 850) return 'a'; // A-
    if (h >= 750) return 'B';
    if (h >= 650) return 'b'; // B-
    if (h >= 550) return 'C';
    if (h >= 450) return 'c'; // C-
    if (h >= 200) return 'D';

    return 'F';
}

// ============================================================
// Graduation audit
// ============================================================

ErrorCode graduationFailures(int s, std::string& out) {
    if (s < 0 || s >= numStudents) {
        return E_UNKNOWN_STUDENT;
    }

    out.clear();

    // R1: enough earned credits.
    if (earnedCredits(s) < program.totalCredits) {
        out += "R1";
    }

    // R2: every CORE course must be passed.
    bool corePassed = true;

    for (int c = 0; c < numCourses; c++) {
        if (courses[c].isCore && !isPassed(s, c)) {
            corePassed = false;
            break;
        }
    }

    if (!corePassed) {
        if (!out.empty()) out += ",";
        out += "R2";
    }

    // R3: enough passed ELECTIVE credits.
    int electiveCredits = 0;

    for (int c = 0; c < numCourses; c++) {
        if (!courses[c].isCore && isPassed(s, c)) {
            electiveCredits += courses[c].credits;
        }
    }

    if (electiveCredits < program.electiveCredits) {
        if (!out.empty()) out += ",";
        out += "R3";
    }

    // R4: at least one final thesis graded C or better.
    bool thesisRequirementMet = false;

    for (int i = 0; i < students[s].numRegs; i++) {
        Registration& reg = students[s].regs[i];

        if (reg.course < 0 || reg.course >= numCourses) {
            continue;
        }

        if (courses[reg.course].type == THESIS
            && reg.thesis == FINAL
            && gradePoints(reg.grade) >= 6) {
            thesisRequirementMet = true;
            break;
        }
    }

    if (!thesisRequirementMet) {
        if (!out.empty()) out += ",";
        out += "R4";
    }

    // R5: GPA exists and meets the minimum.
    int h = 0;
    ErrorCode gpaError = gpaHundredths(s, &h);

    if (gpaError != OK || h < program.minGpaHundredths) {
        if (!out.empty()) out += ",";
        out += "R5";
    }

    return OK;
}


// ============================================================
// Bonus: Student transcript
// ============================================================

ErrorCode printTranscript(int s) {
    if (s < 0 || s >= numStudents) return E_UNKNOWN_STUDENT;

    std::cout << "TRANSCRIPT " << students[s].roll << "\n";

    if (students[s].numRegs == 0) {
        std::cout << "NO REGISTRATIONS\n";
        return OK;
    }

    // Display registrations in ascending term order.
    bool printed[MAX_REGS] = {false};

    for (int term = 1; term <= 12; term++) {
        bool hasRegistration = false;

        for (int i = 0; i < students[s].numRegs; i++) {
            if (students[s].regs[i].term == term) {
                hasRegistration = true;
                break;
            }
        }

        if (!hasRegistration) continue;

        std::cout << "TERM " << term << "\n";

        for (int i = 0; i < students[s].numRegs; i++) {
            Registration& reg = students[s].regs[i];

            if (reg.term != term || printed[i]) continue;

            const Course& course = courses[reg.course];

            std::cout << course.code << " "
                      << course.credits << " ";

            if (course.type == THESIS) {
                if (reg.thesis == REGISTERED) {
                    std::cout << "- (REGISTERED)";
                } else if (reg.thesis == IN_PROGRESS) {
                    std::cout << "S (IN_PROGRESS)";
                } else if (reg.thesis == FAILED) {
                    std::cout << "X (FAILED)";
                } else if (reg.thesis == FINAL) {
                    std::cout << reg.grade;
                } else {
                    std::cout << "-";
                }
            } else if (reg.grade.empty()) {
                std::cout << "-";
            } else {
                std::cout << reg.grade;
            }

            std::cout << "\n";
            printed[i] = true;
        }
    }

    return OK;
}


// ============================================================
// Bonus: SAVE and LOAD
// ============================================================

static ErrorCode saveState(const std::string& filename) {
    std::ofstream file(filename.c_str());
    if (!file) return E_PARSE;

    file << "ACADERP_SAVE_V1\n";

    file << program.totalCredits << " "
         << program.electiveCredits << " "
         << program.minGpaHundredths << " "
         << program.maxLoad << " "
         << program.thesisCredits << "\n";

    file << numCourses << "\n";

    for (int i = 0; i < numCourses; i++) {
        file << courses[i].code << " "
             << static_cast<int>(courses[i].type) << " "
             << courses[i].credits << " "
             << (courses[i].isCore ? 1 : 0) << " "
             << courses[i].prereq << "\n";
    }

    file << numStudents << "\n";

    for (int i = 0; i < numStudents; i++) {
        const Student& student = students[i];

        file << student.roll << " "
             << student.name << " "
             << student.numRegs << "\n";

        for (int j = 0; j < student.numRegs; j++) {
            const Registration& reg = student.regs[j];

            file << reg.course << " "
                 << reg.term << " "
                 << static_cast<int>(reg.thesis) << " "
                 << (reg.grade.empty() ? "-" : reg.grade) << "\n";
        }
    }

    file.flush();
    if (!file.good()) return E_PARSE;

    file.close();
    if (file.fail()) return E_PARSE;

    return OK;
}


static ErrorCode loadState(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file) return E_PARSE;

    std::string magic;
    if (!(file >> magic) || magic != "ACADERP_SAVE_V1") {
        return E_PARSE;
    }

    ProgramConfig loadedProgram;

    if (!(file >> loadedProgram.totalCredits
              >> loadedProgram.electiveCredits
              >> loadedProgram.minGpaHundredths
              >> loadedProgram.maxLoad
              >> loadedProgram.thesisCredits)) {
        return E_PARSE;
    }

    if (loadedProgram.totalCredits < 0 ||
        loadedProgram.electiveCredits < 0 ||
        loadedProgram.minGpaHundredths < 0 ||
        loadedProgram.maxLoad < 0 ||
        loadedProgram.thesisCredits < 0) {
        return E_PARSE;
    }

    Course loadedCourses[MAX_COURSES];
    int loadedCourseCount;

    if (!(file >> loadedCourseCount) ||
        loadedCourseCount < 0 ||
        loadedCourseCount > MAX_COURSES) {
        return E_PARSE;
    }

    for (int i = 0; i < loadedCourseCount; i++) {
        int type, core;

        if (!(file >> loadedCourses[i].code
                  >> type
                  >> loadedCourses[i].credits
                  >> core
                  >> loadedCourses[i].prereq)) {
            return E_PARSE;
        }

        if (loadedCourses[i].code.empty() ||
            loadedCourses[i].code.length() > MAX_CODE_LEN ||
            type < REGULAR || type > THESIS ||
            loadedCourses[i].credits < 0 ||
            core < 0 || core > 1 ||
            loadedCourses[i].prereq < -1 ||
            loadedCourses[i].prereq >= loadedCourseCount ||
            loadedCourses[i].prereq == i) {
            return E_PARSE;
        }

        loadedCourses[i].type = static_cast<CourseType>(type);
        loadedCourses[i].isCore = (core == 1);

        for (int j = 0; j < i; j++) {
            if (loadedCourses[j].code == loadedCourses[i].code) {
                return E_PARSE;
            }
        }
    }

    int loadedStudentCount;

    if (!(file >> loadedStudentCount) ||
        loadedStudentCount < 0 ||
        loadedStudentCount > MAX_STUDENTS) {
        return E_PARSE;
    }

    // Temporary students own their registration arrays.
    // Their destructors release those arrays automatically.
    Student loadedStudents[MAX_STUDENTS];

    for (int i = 0; i < loadedStudentCount; i++) {
        int registrationCount;

        if (!(file >> loadedStudents[i].roll
                  >> loadedStudents[i].name
                  >> registrationCount)) {
            return E_PARSE;
        }

        if (loadedStudents[i].roll.empty() ||
            loadedStudents[i].roll.length() > MAX_CODE_LEN ||
            loadedStudents[i].name.empty() ||
            loadedStudents[i].name.length() > MAX_NAME_LEN ||
            registrationCount < 0 ||
            registrationCount > MAX_REGS) {
            return E_PARSE;
        }

        for (int j = 0; j < i; j++) {
            if (loadedStudents[j].roll == loadedStudents[i].roll) {
                return E_PARSE;
            }
        }

        loadedStudents[i].numRegs = registrationCount;

        for (int j = 0; j < registrationCount; j++) {
            int courseIndex, term, thesis;
            std::string grade;

            if (!(file >> courseIndex >> term >> thesis >> grade)) {
                return E_PARSE;
            }

            if (courseIndex < 0 ||
                courseIndex >= loadedCourseCount ||
                term < 1 || term > 12 ||
                thesis < NOT_THESIS || thesis > FAILED) {
                return E_PARSE;
            }

            if (grade == "-") grade = "-";

            loadedStudents[i].regs[j].course = courseIndex;
            loadedStudents[i].regs[j].term = term;
            loadedStudents[i].regs[j].thesis =
                static_cast<ThesisState>(thesis);
            loadedStudents[i].regs[j].grade =
                (grade == "-" ? "" : grade);

            const Course& course = loadedCourses[courseIndex];

            if (course.type == THESIS) {
                if (thesis == NOT_THESIS) return E_PARSE;
            } else {
                if (thesis != NOT_THESIS) return E_PARSE;
            }
        }
    }

    // Reject unexpected trailing data.
    std::string extra;
    if (file >> extra) return E_PARSE;

    // All input has been validated. Replace the current state now.
    for (int i = 0; i < MAX_STUDENTS; i++) {
        students[i].release();
        students[i].regs = new Registration[MAX_REGS];
        students[i].roll = "";
        students[i].name = "";
    }

    for (int i = 0; i < loadedCourseCount; i++) {
        courses[i] = loadedCourses[i];
    }

    numCourses = loadedCourseCount;

    for (int i = 0; i < loadedStudentCount; i++) {
        students[i].roll = loadedStudents[i].roll;
        students[i].name = loadedStudents[i].name;
        students[i].numRegs = loadedStudents[i].numRegs;

        for (int j = 0; j < loadedStudents[i].numRegs; j++) {
            students[i].regs[j].course =
                loadedStudents[i].regs[j].course;
            students[i].regs[j].term =
                loadedStudents[i].regs[j].term;
            students[i].regs[j].grade =
                loadedStudents[i].regs[j].grade;
            students[i].regs[j].thesis =
                loadedStudents[i].regs[j].thesis;
        }
    }

    numStudents = loadedStudentCount;
    program = loadedProgram;

    firstCommandSeen = true;
    programCommandSeen = true;

    return OK;
}

// ============================================================
// Command dispatcher
// ============================================================

ErrorCode processCommand(const std::string& line, int lineNumber) {
    (void)lineNumber;

    std::string words[MAX_WORDS];
    int count = splitWords(line, words, MAX_WORDS);

    if (count <= 0 || count > MAX_WORDS) {
        firstCommandSeen = true;
        return E_PARSE;
    }

    bool isProgram = words[0] == "PROGRAM";

    if (!isProgram) {
        firstCommandSeen = true;
    }

    if (isProgram) {
        return setProgram(line);
    }

    if (words[0] == "COURSE") {
        return addCourse(line);
    }

    if (words[0] == "STUDENT") {
        return addStudent(line);
    }

    if (words[0] == "REGISTER") {
        int term;

        if (count != 4 || !parseNonNegativeInt(words[3], &term)) {
            return E_PARSE;
        }

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        int c = findCourse(words[2]);
        if (c < 0) return E_UNKNOWN_COURSE;

        return registerCourse(s, c, term);
    }

    if (words[0] == "GRADE") {
        int term;

        if (count != 5 || !parseNonNegativeInt(words[3], &term)) {
            return E_PARSE;
        }

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        int c = findCourse(words[2]);
        if (c < 0) return E_UNKNOWN_COURSE;

        return submitGrade(s, c, term, words[4]);
    }

    if (words[0] == "CONVERT") {
        if (count != 4) return E_PARSE;

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        int c = findCourse(words[2]);
        if (c < 0) return E_UNKNOWN_COURSE;

        return convertThesis(s, c, words[3]);
    }

    if (words[0] == "SAVE") {
        if (count != 2) return E_PARSE;
        return saveState(words[1]);
    }

    if (words[0] == "LOAD") {
        if (count != 2) return E_PARSE;
        return loadState(words[1]);
    }

    if (words[0] == "TRANSCRIPT") {
        if (count != 2) return E_PARSE;

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        return printTranscript(s);
    }

    if (words[0] == "GPA") {
        if (count != 2) return E_PARSE;

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        int h;
        ErrorCode error = gpaHundredths(s, &h);
        if (error != OK) return error;

        std::cout << "GPA " << words[1] << " "
                  << h / 100 << ".";

        if (h % 100 < 10) std::cout << "0";

        std::cout << h % 100 << "\n";
        return OK;
    }

    if (words[0] == "AVG") {
        if (count != 2) return E_PARSE;

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        int h;
        ErrorCode error = gpaHundredths(s, &h);
        if (error != OK) return error;

        char average = avgLetter(h);

        std::cout << "AVG " << words[1] << " ";

        if (average == 'a') std::cout << "A-";
        else if (average == 'b') std::cout << "B-";
        else if (average == 'c') std::cout << "C-";
        else std::cout << average;

        std::cout << "\n";
        return OK;
    }

    if (words[0] == "GRAD") {
        if (count != 2) return E_PARSE;

        int s = findStudent(words[1]);
        if (s < 0) return E_UNKNOWN_STUDENT;

        std::string failures;
        ErrorCode error = graduationFailures(s, failures);
        if (error != OK) return error;

        std::cout << "GRAD " << words[1] << " ";

        if (failures.empty()) {
            std::cout << "YES\n";
        } else {
            std::cout << "NO " << failures << "\n";
        }

        return OK;
    }

    if (words[0] == "HELP") {
        if (count != 1) return E_PARSE;

        std::cout << "AcadERP-Lite Commands\n"
                  << "PROGRAM <total> <elective> <minGPA> <maxLoad> <thesisCredits>\n"
                  << "COURSE <code> <type> <credits> <category> <prereq>\n"
                  << "STUDENT <roll> <name>\n"
                  << "REGISTER <roll> <course> <term>\n"
                  << "GRADE <roll> <course> <term> <grade>\n"
                  << "CONVERT <roll> <thesis> <letter>\n"
                  << "GPA <roll>\n"
                  << "AVG <roll>\n"
                  << "GRAD <roll>\n"
                  << "ELIGIBLE <roll> <course> <term>\n"
                  << "TRANSCRIPT <roll>\n"
                  << "SAVE <filename>\n"
                  << "LOAD <filename>\n";

        return OK;
    }

    if (words[0] == "ELIGIBLE") {
        int term;

        if (count != 4 || !parseNonNegativeInt(words[3], &term)) {
            return E_PARSE;
        }

        int s = findStudent(words[1]);
        int c = findCourse(words[2]);

        std::cout << "ELIGIBLE " << words[1] << " "
                  << words[2] << " ";

        int check = checkEligibility(s, c, term);

        if (check == 0) {
            std::cout << "YES\n";
        } else {
            std::cout << "NO E" << check << "\n";
        }

        return OK;
    }

    return E_PARSE;
}