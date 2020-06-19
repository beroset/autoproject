# [Simple C++ program to keep moving average of price](https://codereview.stackexchange.com/questions/221052)

The simple program listed below reads a text file with a specified format, update its time record of a stock price then calculates the time-weighted moving average of the price.

I am primarily concerned about performance.

```
#include <ios>
#include <set>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <utility>
#include <numeric>
#include <iostream>
#include <algorithm>

#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

struct State
{
    size_t time; char operation; size_t id; double value;

    bool validate () const
    { 
        if (time >= 0 && (operation == 'I' || operation == 'E') && id != 0 && value > 0)
            return true;
        return false; 
    }
};

class OrderBook
{
    vector<State> v_states_;
    multiset<double> highest_price_record;
    double current_time_weighted_price_ = 0.0;

    void UpdateTimeWeightedPrice() 
    {
        double delta_time;
        
        if (v_states_.size() > 1)
        {
            assert(v_states_.size() > 0 && !highest_price_record.empty());
            delta_time = v_states_.back().time - v_states_.end()[-2].time;
        }
        else 
            return;

        current_time_weighted_price_ += delta_time * *highest_price_record.rbegin();
    };

    void Insert(const State& in)
    {
        if(!in.validate())
            throw domain_error("Tried to insert invalid entry.");

        v_states_.push_back(in);

        UpdateTimeWeightedPrice();
        
        if (highest_price_record.empty())
        {
            highest_price_record.insert(in.value);
            return;
        }

        if (in.value >= *highest_price_record.rbegin())
            highest_price_record.insert(in.value);
    }

    void Erase(const State& in)
    {
        if (find_if(v_states_.begin(), v_states_.end(),
            [&](const State src){return (in.id == src.id);}) == v_states_.end())
            throw domain_error("Tried to erase non-existant entry.");
        else
        {
            v_states_.push_back(in);
            UpdateTimeWeightedPrice();
            
            const auto iter = highest_price_record.find(in.value);
            
            if (iter == highest_price_record.end()) 
                return;
            
            highest_price_record.erase(iter);
        }
    }

    public:

    double TimeWeightedAverage() const 
    {
        const double delta_time = v_states_.back().time - v_states_.begin()->time;
        assert(delta_time > 0.0);

        return current_time_weighted_price_ / delta_time; 
    }

    void ReadInFile(const fs::path& filepath)
    {
        if (!filepath.has_extension() || !fs::exists(filepath) 
                                      || !fs::is_regular_file(filepath))
            throw runtime_error("File path provided does not exist or is not a regular file.");

        ifstream file(filepath.string().c_str());

        string line;
		while (getline(file, line, '\n'))
		{
    		// Continue as long as line is not all whitespace characters
			if (any_of(line.cbegin(), line.cend(), 
                [](string::value_type character) {return !(isspace(character)); }))
			{
                State input;
                stringstream line_ss(line);

				line_ss >> skipws >> input.time 
                        >> skipws >> input.operation
                        >> skipws >> input.id;
                
                if (input.operation == 'I')
                {
                    line_ss >> skipws >> input.value;
                    Insert(input);
                }
                else
                    Erase(input);
            }
		}
    }
};

int main(int argc, char** argv)
{
    try
    {
        assert(argc == 2);
        const fs::path filename = argv[1];

        OrderBook book_1;
        book_1.ReadInFile(filename);
        cout << "Time weighted average: " << book_1.TimeWeightedAverage() << endl;
    }
    catch(exception& e)
    {
        cerr << "Soemthing unexpected went wrong: " << e.what() << endl;
    }
    catch(...)
    {
        cerr << "Soemthing unexpected went wrong." << endl;       
    }
    return 0;
}
```

Input format is:

    1. Time[double] 
    2. Insert/Erase[Char] 
    3. ID[int] 
    4. Value[Double]<only if Insert>

Sample:    
    
    1.      2.      3.      4.
    --------------------------
    0       I       1       20

    50      E       1   

    100     I       2       40

    120     I       3       45

    200     I       4       100

    250     E       4
