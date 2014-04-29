/*

The MIT License (MIT)

Copyright (c) 2012-2014 Erik Soma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// standard library
#include <assert.h>
#include <cstdlib>
// estd
#include "event.hpp"

static void test_basic_operations();
static void test_arguments();

/*
    This program tests the Event.
*/
int main(int argc, const char* argv[])
{
    test_basic_operations();
    test_arguments();
    return EXIT_SUCCESS;
}

static void test_basic_operations()
{
    Event<> event;
    event.fire();
    
    auto function_a_var = false;
    auto function_a_bind = event.bind([&]{
        assert(!function_a_var);
        function_a_var = true;
    });
    event.fire();
    assert(function_a_var);
    
    function_a_var = false;
    auto function_b_var = false;
    event.permanent_bind([&]{
        assert(!function_b_var);
        function_b_var = true;
    });
    event.fire();
    assert(function_a_var);
    assert(function_b_var);
    
    function_a_var = false;
    function_b_var = false;
    auto function_c_var = false;
    auto function_c_bind = event.bind([&]{
        assert(!function_c_var);
        function_c_var = true;
        function_a_bind = 0;
    });
    event.fire();
    assert(function_a_var);
    assert(function_b_var);
    assert(function_c_var);
    
    function_a_var = false;
    function_b_var = false;
    function_c_var = false;
    event.fire();
    assert(!function_a_var);
    assert(function_b_var);
    assert(function_c_var);
    
    function_b_var = false;
    function_c_var = false;
    auto function_d_var = false;
    auto function_d_bind = event.bind([&]{
        assert(!function_d_var);
        function_d_var = true;
        function_c_bind = 0;
    });
    function_c_bind = event.bind([&]{
        assert(false);
    });
    event.fire();
    assert(!function_a_var);
    assert(function_b_var);
    assert(!function_c_var);
    assert(function_d_var);
}

static void test_arguments()
{
    Event<int, int&, const int&> event;
    int a = 'a';
    int b = 'b';
    int c = 'c';
    event.fire(a, b, c);
    
    auto executed = false;
    event.permanent_bind([&](int pa, int& pb, const int& pc){
        assert(!executed);
        executed = true;
        
        assert(pa == a);
        assert(pb == b);
        assert(pc == c);
        
        a = 'z';
        assert(&pa != &a);
        assert(pa != a);
        
        b = 'y';
        assert(&pb == &b);
        assert(pb == b);
        
        assert(&pc == &c);
    });
    event.fire(a, b, c);
    assert(executed);
}