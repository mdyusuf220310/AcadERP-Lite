/* Group 92 */

#include "grp_92_erp.h"

int main() {
    initProgram();

    std::string line;
    int lineNumber = 0;

    while (std::getline(std::cin, line)) {
        lineNumber++;

        // Ignore leading spaces, tabs, and carriage returns.
        std::string::size_type pos = line.find_first_not_of(" \t\r");

        // Blank lines and comment lines are ignored,
        // but still count toward lineNumber.
        if (pos == std::string::npos || line[pos] == '#') {
            continue;
        }

        ErrorCode result = processCommand(line, lineNumber);

        if (result != OK) {
            std::cout << "ERROR " << lineNumber << " "
                      << errorName(result) << "\n";
        }
    }

    cleanup();
    return 0;
}
