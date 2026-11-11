<!-- group-92 -->

# AcadERP-Lite — Academic Records & Graduation Audit System

**Group ID:** 92  
**Member 1:** Md Yusuf — MT26176  
**Member 2:** Zainab Quadri — MT26202  
**Language:** C++  
**Standard:** C++11  
**Build System:** GNU Make  
**Testing:** Bash Test Runner and Diff

---

## 1. Project Location (WSL)

**Project directory:**

```bash
/mnt/c/Users/ymd22/OneDrive/Desktop/OOPD-Project
```

Open the project in WSL:

```bash
cd "/mnt/c/Users/ymd22/OneDrive/Desktop/OOPD-Project"
```

Verify the current directory:

```bash
pwd
```

List project files:

```bash
ls -la
```

---

## 2. Project Overview

AcadERP-Lite is a command-line Academic Records and Graduation Audit System developed using C++11.

The system manages student records, course catalogues, course registrations, academic grades, thesis progress, GPA calculations and graduation eligibility.

The project implements academic validation rules, prerequisite checking, credit limits, repeated course handling, thesis state transitions and automated graduation audits.

The implementation follows the restrictions specified in the assignment, including fixed-capacity arrays, manual memory management and error-code-based handling.

---

## 3. Project Directory Structure

```text
OOPD-Project/
|
|-- README.md
|-- run_tests.sh
|
|-- src/
|   |-- main.cpp
|   |-- erp.cpp
|   |-- erp.h
|   `-- Makefile
|
`-- tests/
    |-- T01.in ... T23.in
    |-- T01.out ... T23.out
    |
    |-- custom/
    |   `-- Additional test cases
    |
    `-- sample.in
```

### Source File Responsibilities

| File | Responsibility |
|---|---|
| `main.cpp` | Reads input commands and handles line numbers |
| `erp.h` | Constants, structures, enumerations and function declarations |
| `erp.cpp` | Core academic management implementation |
| `Makefile` | Compilation and cleanup instructions |
| `run_tests.sh` | Executes and compares automated test cases |

The `tests/` directory contains numbered input files and corresponding expected output files.

---

## 4. System Design and Implementation

### 4.1 Student Management

Maintains student information, including roll number, name and course registrations.

Each student has a maximum registration capacity of 40 courses.

### 4.2 Course Catalogue

Maintains course information, including:

- Course code
- Course type
- Credit value
- CORE or ELECTIVE category
- Prerequisite course

The supported course types are:

- REGULAR
- SU
- THESIS

### 4.3 Registration Management

Before accepting a registration, the system checks:

- Student and course existence
- Whether the course has already been passed
- Whether an ungraded registration already exists
- Prerequisite completion
- Maximum term credit load
- Thesis registration eligibility

### 4.4 Grade Management

Different course types have different grading rules.

| Course Type | Accepted Grades | GPA Contribution |
|---|---|---|
| REGULAR | A, A-, B, B-, C, C-, D, F | Yes |
| SU | S, X | No |
| THESIS | S, X followed by final letter conversion | Only after conversion |

### 4.5 Thesis Management

Thesis registrations follow a state-based process:

```text
REGISTERED
    |
    | Grade S
    v
IN_PROGRESS
    |
    | CONVERT
    v
FINAL

REGISTERED
    |
    | Grade X
    v
FAILED
```

A thesis can be converted into a final letter grade only after successful progress.

### 4.6 GPA Calculation

GPA is calculated using credit-weighted grade points from counting attempts.

Important rules:

- REGULAR courses contribute to GPA.
- SU courses do not contribute to GPA.
- Only converted FINAL thesis grades contribute to GPA.
- Failed regular attempts contribute to GPA.
- Repeated courses use the latest graded attempt for counting.
- GPA is rounded to two decimal places using integer half-up rounding.

If no GPA-contributing credits exist, the system reports `E_NO_GPA`.

### 4.7 Graduation Audit

The graduation audit evaluates five requirements:

| Rule | Description |
|---|---|
| R1 | Required total earned credits |
| R2 | All CORE courses must be passed |
| R3 | Required elective credits must be earned |
| R4 | A qualifying final thesis grade |
| R5 | Minimum required GPA |

The `GRAD` command checks all five requirements and reports the failed rules.

---

## 5. Requirements

The following tools are required to build and execute the project:

- C++ compiler supporting C++11
- GNU Make
- Linux or WSL environment
- Git for version control

Check installed versions:

```bash
g++ --version
make --version
git --version
```

---

## 6. Build the Project

Navigate to the project root:

```bash
cd "/mnt/c/Users/ymd22/OneDrive/Desktop/OOPD-Project"
```

Compile the project:

```bash
make -C src
```

The project uses the following compilation flags:

```text
-std=c++11 -Wall -Wextra -fno-exceptions
```

These flags enforce C++11 compatibility, enable compiler warnings and disable exception support.

### Clean Previous Build Files

```bash
make -C src clean
```

This removes generated object files and the executable.

To rebuild:

```bash
make -C src
```

---

## 7. Execute the Program

Run the application:

```bash
./src/acaderp
```

The program accepts commands through standard input.

To execute commands from an input file:

```bash
./src/acaderp < tests/T01.in
```

The system processes one command per line.

Blank lines and lines beginning with `#` are ignored.

---

## 8. Supported Commands

| Command | Purpose |
|---|---|
| `PROGRAM` | Configure academic program requirements |
| `COURSE` | Add a course |
| `STUDENT` | Add a student |
| `REGISTER` | Register a student for a course |
| `GRADE` | Submit a course grade |
| `CONVERT` | Convert a completed thesis grade |
| `GPA` | Calculate student GPA |
| `AVG` | Calculate average grade |
| `GRAD` | Check graduation eligibility |
| `ELIGIBLE` | Check course registration eligibility |
| `HELP` | Display all supported commands and their syntax |

### HELP Command

The `HELP` command displays the syntax of all supported commands,
including the TRANSCRIPT, SAVE and LOAD bonus commands.

- Format: `HELP`
- Supplying extra arguments returns `E_PARSE`.

### Command Format

```text
PROGRAM <total> <elective> <minGPA> <maxLoad> <thesisCredits>

COURSE <code> <type> <credits> <category> <prereq>

STUDENT <roll> <name>

REGISTER <roll> <course> <term>

GRADE <roll> <course> <term> <grade>

CONVERT <roll> <thesis> <letter>

GPA <roll>

AVG <roll>

GRAD <roll>

ELIGIBLE <roll> <course> <term>
```

Use `-` when a course does not have a prerequisite.

---

## 9. Error Handling

The system uses an enumerated `ErrorCode` type to communicate failures.

Errors follow this output format:

```text
ERROR <line_number> <ERROR_CODE>
```

Examples of supported error codes include:

- `E_PARSE`
- `E_FULL`
- `E_DUPLICATE`
- `E_UNKNOWN_STUDENT`
- `E_UNKNOWN_COURSE`
- `E_BAD_COURSE`
- `E_BAD_GRADE`
- `E_GRADE_TYPE`
- `E_NOT_REGISTERED`
- `E_ALREADY_GRADED`
- `E_THESIS_STATE`
- `E_ALREADY_PASSED`
- `E_ALREADY_REGISTERED`
- `E_PREREQ`
- `E_CREDIT_LIMIT`
- `E_THESIS_NOT_ALLOWED`
- `E_NO_GPA`

The system continues processing subsequent commands after an error.

Failed operations must not modify stored data.

---

## 10. Memory Management

The project uses dynamic memory allocation for student registration records.

Important implementation decisions:

- Registration arrays are dynamically allocated.
- Each student has a fixed maximum registration capacity.
- The `Student` destructor releases allocated registration memory.
- Copy construction and copy assignment are disabled for `Student` to prevent unsafe copying of owned pointers.
- Cleanup is performed before program termination.

---

## 11. Automated Testing

The project contains 23 numbered test cases.

Each test consists of:

```text
T01.in
T01.out
```

The `.in` file contains the input commands, while the `.out` file contains the expected output.

The test suite covers:

- Grade validation
- Invalid grade types
- Registration and grading errors
- Thesis state transitions
- Thesis conversion
- Repeated course attempts
- GPA calculation and rounding
- Average grade boundaries
- Graduation requirements
- Prerequisite checking
- Credit-load restrictions
- Thesis eligibility
- Maximum student and course capacity
- Input parsing
- Error recovery

### Run the Test Suite

From the project root:

```bash
chmod +x run_tests.sh
./run_tests.sh
```

The test runner executes all 23 test cases and compares actual program output against the corresponding expected output using `diff`.

Additional custom test inputs are preserved in `tests/custom/`.

The professor's sample input is available as `tests/sample.in`.

---

## 12. Git and GitHub Repository

The project is maintained collaboratively by both members of Group 92.

The intended workflow uses:

- One shared GitHub repository
- Individual GitHub accounts
- Separate development branches
- Independent commits
- Pull Requests and reviewed integration

### Check Git Configuration

```bash
git status
git branch --show-current
git remote -v
```

### Create a Development Branch

```bash
git checkout -b feature/yusuf
```

The second member should use a separate branch:

```bash
git checkout -b feature/zainab
```

### Stage Changes

```bash
git add -A
```

Review staged changes:

```bash
git status
git diff --cached
```

### Commit Changes

```bash
git commit -m "Describe implemented functionality"
```

### Push the Development Branch

```bash
git push -u origin feature/yusuf
```

Zainab should push her own branch using her own GitHub account.

Changes should be reviewed and integrated into the shared `main` branch through Pull Requests.

Do not force-push or overwrite another developer's work.

---

## 13. Final Verification Before Submission

Before preparing the final Group 92 ZIP:

1. Verify all required source files are present.
2. Check that the group-ID comment is present at the beginning of every applicable file.
3. Verify the required source filename convention.
4. Perform a clean compilation.
5. Run all 23 automated tests.
6. Review the README and source documentation.
7. Verify that both members' Git contributions are available in the shared repository.
8. Review Git commit history.
9. Exclude all executable and compiled object files.
10. Create one ZIP named `group-92.zip`.

### Clean Build and Test Sequence

```bash
make -C src clean
make -C src
./run_tests.sh
```

### Check the Final Git State

```bash
git status
git log --oneline -10
git branch
git remote -v
```

---

# Command Reference

## 1. Navigate to Project

```bash
cd "/mnt/c/Users/ymd22/OneDrive/Desktop/OOPD-Project"
pwd
ls -la
```

## 2. Build and Clean

```bash
make -C src
make -C src clean
```

## 3. Run the Program

```bash
./src/acaderp
```

Run with a test input:

```bash
./src/acaderp < tests/T01.in
```

## 4. Run Automated Tests

```bash
chmod +x run_tests.sh
./run_tests.sh
```

## 5. Inspect Project Files

```bash
find src tests -maxdepth 3 -type f -print
```

## 6. Git Verification

```bash
git status
git branch --show-current
git log --oneline -10
git remote -v
git diff --stat
```

## 7. Submission ZIP

The final archive must be named:

```text
group-92.zip
```

Do not include executables, object files or other compiled binaries.

---

**Project:** AcadERP-Lite  
**Course:** Object-Oriented Programming and Design  
**Group:** 92  
**Members:** Md Yusuf (MT26176) and Zainab Quadri (MT26202)  
**Programming Language:** C++11

## 11. Bonus Features

### TRANSCRIPT

Command: `TRANSCRIPT <roll>`

Displays all registrations for a student in ascending term order.

Each registration includes:
- Course code
- Course credits
- Grade or current thesis status

Output conventions:
- `TERM n` identifies the academic term.
- `-` represents an ungraded registration.
- `S (IN_PROGRESS)` represents a thesis in progress.
- `X (FAILED)` represents a failed thesis.
- `NO REGISTRATIONS` is printed for a student without registrations.

### SAVE and LOAD

Commands:
- `SAVE <filename>`
- `LOAD <filename>`

SAVE writes the complete academic database to a text file, including:
- Program configuration
- Course catalogue and prerequisites
- Student information
- Registration records
- Grades and thesis states

LOAD restores the saved database.

The save format uses the version identifier `ACADERP_SAVE_V1`.

### Assumptions

- Filenames must not contain whitespace.
- SAVE and LOAD produce no output when successful.
- File-access errors and invalid save-file contents return `E_PARSE`.
- LOAD validates the file before replacing the current database.
- Loading replaces the existing database rather than merging records.
- The exact TRANSCRIPT presentation format is an implementation choice because the assignment does not prescribe one.

### Bonus Testing

- `T24.in` / `T24.out`: TRANSCRIPT functionality.
- `T25.in` / `T25.out`: SAVE and LOAD restoration.
