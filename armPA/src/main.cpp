#include <armPA.ipp>
#include "unitTest.cpp"
#include "example.h"

typedef unsigned long timestamp_t;

int main()
{

    UNITTEST unitTest;
    //  Enter data types as template to run the test
    //  Example: To run test for data type : someDataType_t, execute
    //            unitTest.run<int, float, someDataType_t>();

    unitTest.run<int>();

    //  From example.h
    unitTest.run<Point, Base1, Base2, Derived, NoDefaultConstructor>();

    unitTest.run<float>();

    return 0;
}
