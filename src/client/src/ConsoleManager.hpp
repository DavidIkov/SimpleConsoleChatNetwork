namespace ConsoleManagerNS {
    //disables output of input in console
    //enables reading by specified amount of characters internally(with os api)
    //std::cin is now mostly broken, use ReadChar instead
    void EnableSettings();
    //when enter is pressed, '\r' will be written
    char ReadChar();
    //when enter is pressed, '\r' will be written
    void ReadChars(size_t* amount, char* str);
    struct ConsoleSizeS { int Colums, Rows; };
    ConsoleSizeS gConsoleSize();

};