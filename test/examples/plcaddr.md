# [Class representing digital PLC address](https://codereview.stackexchange.com/questions/207737)
### tags: ['c++', 'iterator', 'c++17']

I have a project where I work with digital addresses for a Programmable Logic Controller (PLC). The requirement is that the user gives a start address e.g. 1000.0 and then for example the next 20 things are numbered in increasing order.

In this example this would mean:

1000.0 <br>
1000.1 <br>
1000.2 <br>
1000.3 <br>
1000.4 <br>
1000.5 <br>
1000.6 <br>
1000.7 <br>
1001.0 <br>
1001.1 <br>
1001.2 <br>
1001.3 <br>
1001.4 <br>
1001.5 <br>
1001.6 <br>
1001.7 <br>
1002.0 <br>
1002.1 <br>
1002.2 <br>
1002.3 <br>

Also I want to prevent that invalid addresses can be created at the start like 1000.8

To accomplish this behaviour I created a class `Digital_plc_address`. I would like to know what can be improved. Are there any bad styles in it?

Is it good practice to make a class to report invalid_input errors when the class cannot be created?

I did some quick checks of the class in main. I know I could have done better with unit tests but I thought for this small class it's okay to check it like this.

<b> digital_plc_adress.h </b>

    #ifndef DIGITAL_PLC_ADDRESS_GUARD151120181614
    #define DIGITAL_PLC_ADDRESS_GUARD151120181614
    
    #include <string>
    
    namespace digital_plc_address {
    
    	class Digital_plc_address {
    		/*
    		Class representing a digital plc address.
    		Each address is represented by a byte address and the digits of the 
    		bytes from 0-7. e.g  100.6 , 5.2
    		Creating invalid addresses like 100.8 is not allowed by the class.
    		*/
    	public:
    		Digital_plc_address(int byte, char digit = 0);
    		explicit Digital_plc_address(const std::string& Digital_plc_address);
    
    		class Invalid_format {};
    
    		std::string as_string() const;
    
    		Digital_plc_address& operator++();
    	private:
    		int m_byte;
    		char m_digit;
    	};
    }
    
    #endif

<b> digital_plc_address.cpp </b>

    #include "digital_plc_address.h"
    
    #include <regex>
    #include <sstream>
    #include <cctype>
    #include <limits>
    
    namespace digital_plc_address {
    
    	Digital_plc_address::Digital_plc_address(int byte, char digit)
    		:m_byte{ byte }, m_digit{ digit }
    	{
    		if (m_byte < 0 || m_byte > std::numeric_limits<decltype(m_byte)>::max()
    			|| m_digit < 0 || m_digit > 7) {
    			throw Invalid_format{};
    		}
    	}
    
    	Digital_plc_address::Digital_plc_address(const std::string& Digital_plc_address)
    	{
    		std::regex valid_format(R"(\d+\.[0-7]{1})");
    
    		if (!std::regex_match(Digital_plc_address, valid_format)) {
    			throw Invalid_format{};
    		}
    
    		std::istringstream ist{ Digital_plc_address };
    
    		std::string byte_str;
    		std::getline(ist, byte_str, '.');
    
    		m_byte = std::stoi(byte_str);
    		m_digit = Digital_plc_address.back() - '0';
    	}
    
    	std::string Digital_plc_address::as_string() const
    	{
    		return std::to_string(m_byte) + '.' + std::to_string(m_digit);
    	}
    
    	Digital_plc_address& Digital_plc_address::operator++()
    	{
    		if (m_digit == 7) {
    			m_digit = 0;
    			++m_byte;
    		}
    		else {
    			++m_digit;
    		}
    		return *this;
    	}
    }


<b> main.cpp </b>

    #include "digital_plc_address.h"
    
    #include <iostream>
    
    
    void print_io(int a)
    {
    	try {
    		digital_plc_address::Digital_plc_address t{ a };
    		std::cout << t.as_string() << '\n';
    	}
    	catch (digital_plc_address::Digital_plc_address::Invalid_format) {
    		std::cout << "Invalid input: " << a << '\n';
    	}
    }
    
    void print_io(int a, char b)
    {
    	try {
    		digital_plc_address::Digital_plc_address t{ a,b };
    		std::cout << t.as_string() << '\n';
    	}
    	catch (digital_plc_address::Digital_plc_address::Invalid_format) {
    		std::cout << "Invalid input: " << a << '\t' << b <<'\n';
    	}
    }
    
    void print_io(const std::string& s)
    {
    	try {
    		digital_plc_address::Digital_plc_address t{ s };
    		std::cout << t.as_string() << '\n';
    	}
    	catch (digital_plc_address::Digital_plc_address::Invalid_format) {
    		std::cout << "Invalid input: " << s << '\n';
    	}
    }
    
    int main() 
    try{
    
    	digital_plc_address::Digital_plc_address inc{ 10 };
    
    	for (int i = 0; i < 20; ++i) {
    		++inc;
    		std::cout << inc.as_string() << '\n';
    	}
    
    	print_io(100);
    	print_io(100, 1);
    	print_io(50, 7);
    
    	print_io("100");
    	print_io("100.1");
    	print_io("50.7");
    
    	print_io("-50.7");
    
    	print_io("50.-7");
    
    	print_io("50.8");
    
    	std::getchar();
    }
    catch (digital_plc_address::Digital_plc_address::Invalid_format) {
    	std::cout << "error";
    	std::getchar();
    }

