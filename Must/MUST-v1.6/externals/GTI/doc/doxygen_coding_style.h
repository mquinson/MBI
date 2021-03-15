/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @page CodingStyle Coding Style
 *
 * This chapter lists the coding style that should be used for GTI.
 * It is based on the one proposed at
 * http://geosoft.no/development/cppstyle.html, but it differs
 * in several aspects. The style is meant as a general pointer on
 * what the GTI code should look like, however exceptions may
 * be reasonable in some cases.
 *
 * @section CodingStyleGeneral General Naming Conventions
 * - Names representing types must be in mixed case starting with upper case.
 * @code
 Line, SavingsAccount
 @endcode
 * - Interfaces use a "I_" prefix.
 * @code
class I_LineInterface
{
    public:
        virtual float computeLength (void) = 0;
}
 @endcode
 * - Variable names must be in mixed case starting with lower case.
 * @code
 line, savingsAccount
 @endcode
 * - Named constants (including enumeration values) must be all uppercase using underscore to separate words.
 * @code
 MAX_ITERATIONS, COLOR_RED, PI
 @endcode
 * - Names representing methods or functions should be verbs and must be written in mixed case starting with lower case.
 * @code
 getName(), computeTotalWidth()
 @endcode
 * - Names representing namespaces are all lowercase.
 * @code
 must, gti
 @endcode
 * - Names representing template types should be a single uppercase letter.
 * @code
 template<class T> ...
 template<class C, class D> ...
 @endcode
 * - Abbreviations and acronyms must not be uppercase when used as name.
 * @code
 exportHtmlSource(); // NOT: exportHTMLSource();
 @endcode
 * - Global variables should always be referred to using the :: operator.
 * @code
 ::mainWindow.open()
 @endcode
 * - Private class variables have a "my" prefix.
 * @code
 class SomeClass
 {
	 private:
		 int myLength;
 }
 @endcode
 * - The name of the object is implicit, and should be avoided in a method name.
 * @code
 line.getLength(); // NOT: line.getLineLength();
 @endcode
 * - Plural form should be used on names representing a collection of objects.
 * @code
 vector<Point> points;
 int values[];
 @endcode
 * - Abbreviations in names should be avoided.
 * @code
 computeAverage(); // NOT: compAvg();
 @endcode
 * - Pointers should be prefixed with a "p".
 * @code
 Line* pLine; // NOT: Line* line;
              // NOT: Line* linePtr;
 @endcode
 * - Enumeration constants can be prefixed by a common type name.
 * @code
 enum Color
 {
	 COLOR_RED,
	 COLOR_GREEN,
	 COLOR_BLUE
 };
 @endcode
 * - C++ header files have the extension .h (preferred) or .hpp. Source files have the extension .cpp.
 * @code
 MyClass.cpp, MyClass.h
 @endcode
 * - A class should be declared in a header file and defined in a source file where the name of the files match the name of the class.
 * @code
 MyClass.h, MyClass.cpp
 @endcode
 * - Special characters like TAB and page break must be avoided.
 * - Header files must contain an include guard.
 * @code
 #ifndef CLASS_NAME_H
 #define CLASS_NAME_H
 ...
 #endif //CLASS_NAME_H
 @endcode
 *
 * @section CodingStyleVariables Variables
 * - Use of global variables should be minimized.
 * - Class variables should never be declared public, except for classes that represent data structures.
 * - Variables should be declared in the smallest scope possible.
 *
 * @section CodingStyleStructure Indentation and Structure
 * - The default TAB width is 4
 * - All code blocks that use "{" and "}" are structured and indented as follows:
 * @code
{
    statment1;
    statment2;
    ...
}
 @endcode
 *
 * @section CodingStyleMisc Miscellaneous
 * - The use of magic numbers in the code should be avoided. Numbers other than 0 and 1 should be considered declared as named constants instead.
 * - Goto should not be used.
 * - "NULL" should be used instead of "0" for pointers.
 * - The class declarations should have the following form:
 * @code
class SomeClass : public BaseClass
{
    public:
        ...

    protected:
        ...

    private:
        ...
}
 @endcode
 * - Document your code! Use doxygen tags!
 *
 */
