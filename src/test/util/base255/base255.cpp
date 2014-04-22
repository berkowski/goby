// Copyright 2009-2014 Toby Schneider (https://launchpad.net/~tes)
//                     GobySoft, LLC (2013-)
//                     Massachusetts Institute of Technology (2007-2014)
//                     Goby Developers Team (https://launchpad.net/~goby-dev)
// 
//
// This file is part of the Goby Underwater Autonomy Project Binaries
// ("The Goby Binaries").
//
// The Goby Binaries are free software: you can redistribute them and/or modify
// them under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// The Goby Binaries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.

#include "goby/util/base_convert.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstdlib>

void intprint(const std::string& s)
{
    for(int i = s.size()-1, n = 0; i >= n; --i)
        std::cout << std::hex << int(s[i] & 0xFF) << " " << std::dec;
    std::cout << std::endl;
}

void test255(const std::string& in, bool output = true)
{
    if(output)
    {
        std::cout << "in: ";
        intprint(in);
    }
    
    std::string out;
    
    goby::util::base_convert(in, &out, 256, 255);
    if(output)
    {
        std::cout << "out: ";
        intprint(out);
    }
    
    std::string in2;
    
    goby::util::base_convert(out, &in2, 255, 256);
    if(output)
    {
        std::cout << "in2: ";
        intprint(in2);
    }
    
    assert(in == in2);

    std::cout << "Encoded string is " << out.size() - in.size() << " bytes larger than original string (" << in.size() << " bytes)" << std::endl;
}

std::string randstring(int size)
{
    std::string test(size, 0);
    for(int i = 0; i < size; ++i)
    {
        test[i] = rand() % 256;
    }
    return test;
}



int main()
{
    {
        
        char chin [10] = { 2, 1, 0, 0, 2, 2, 2, 0, 1, 1};    
        std::string in(chin, 10);
        char chout [5] = {5, 0, 8, 2, 4};
        std::string out;
        
        goby::util::base_convert(in, &out, 3, 9);
        
        intprint(in);
        intprint(out);
        
        assert(out == std::string(chout, 5));
    }
    
    test255("TOMcat");
    test255(std::string(4, 0xff));

    test255(randstring(125));
    test255(randstring(255));
    test255(randstring(1500));
    test255(randstring(15000), false);
    
    
    std::cout << "all tests passed" << std::endl;
    
    return 0;
}
