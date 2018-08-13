#include <iostream>
#include <string>

#include <QString>

#include <KAuth>

static const QString TESTVALUE = "This is just a test";

int main() {

    KAuth::Action action(TESTVALUE);
    QString readValue = action.name();

    if (TESTVALUE == readValue) {
        std::cout << "Test OK" << std::endl;
        return 0;
    }
    std::cout << "Test FAILED" << std::endl;
    return 1;
}
