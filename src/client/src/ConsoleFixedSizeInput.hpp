class ConsoleFixedSizeInputC {
protected:
    //if exists settings for console wont be applied again
    static bool Exists;//false
public:
    //disables output of input in console
    //enables reading by specified amount of characters internally(with os api)
    //std::cin is now mostly broken, use ReadChar instead
    ConsoleFixedSizeInputC();
    //when enter is pressed, '\r' will be written
    static char ReadChar();
    //when enter is pressed, '\r' will be written
    static void ReadChars(size_t* amount, char* str);
};