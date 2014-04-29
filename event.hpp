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

#ifndef EVENT_HPP
#define EVENT_HPP

// standard library
#include <functional>
#include <list>
#include <memory>
#include <set>

/*
    Events allow for multiple functions to be executed in response to an
    Event having been fired. Events can be fired at any time, causing all
    of their bound functions to immediatley execute. Functions can be bound
    and unbound at any time, even "during" the firing of an Event. Events are
    typed such that the arguments must match the arguments defined for the
    Event.
*/
template <typename... Args>
class Event
{
    public:
    
        typedef std::function<void(Args...)> Function;
        
    private:
    
        typedef std::list<std::shared_ptr<Function>> FunctionList;
        
        typedef std::list<std::weak_ptr<Function>> WeakFunctionList;

    public:
    
        /*
            An object that has ownership of the bind to an Event. When the Bind
            is destroyed it will automatically unbind the function that was
            bound. This is useful for automatically cleaning up Events with
            RAII.
        */
        class Bind
        {
            public:
            
                /*
                    Destructor
                =============================================================*/
                ~Bind()
                {
                    if (this->is_valid)
                    {
                        assert(
                            this->event.binds.find(this) !=
                            this->event.binds.end()
                        );
                        this->event.binds.erase(this);
                        this->event.bound_functions.erase(
                            this->bound_function_iterator
                        );
                    }
                }
            
            private:
            
                friend class Event<Args...>;
            
                /*
                    Constructor
                =============================================================*/
                Bind(
                    Event& event,
                    typename FunctionList::iterator bound_function_iterator
                ):
                    event(event),
                    bound_function_iterator(bound_function_iterator),
                    is_valid(true)
                {
                }
                
                void invalidate()
                {
                    assert(this->is_valid);
                    this->is_valid = false;
                }
                
                Event& event;
                
                typename FunctionList::iterator bound_function_iterator;
                
                bool is_valid;
        };
    
        /*
            Constructor
        =====================================================================*/
        Event()
        {
        }
        
        /*
            Destructor
        =====================================================================*/
        ~Event()
        {
            // Invalidate any remaining Binds.
            for(auto bind: this->binds)
            {
                bind->invalidate();
            }
        }
        
        /*
            permanent_bind
            
            Permanently binds a function to the Event. Useful for instances in
            which the bound function will never become invalid within the
            lifetime of the Event.
        =====================================================================*/
        void permanent_bind(const Function& function)
        {
            this->bound_functions.emplace(
                this->bound_functions.end(),
                std::make_shared<Function>(function)
            );
        }
        
        /*
            bind
            
            Binds a function to the Event for the duration of the Bind instance
            returned.
        =====================================================================*/
        std::shared_ptr<Bind> bind(const Function& function)
        {
            this->bound_functions.emplace(
                this->bound_functions.end(),
                std::make_shared<Function>(function)
            );
            auto bound_function_iterator = this->bound_functions.end();
            --bound_function_iterator;
            std::shared_ptr<Bind> bind(new Bind(
                *this,
                bound_function_iterator
            ));
            assert(this->binds.find(bind.get()) == this->binds.end());
            this->binds.insert(bind.get());
            return bind;
        }
        
        /*
            fire
            
            Executes all bound functions using the arguments provided.
        */
        void fire(Args... args)
        {
            WeakFunctionList weak_functions;
            for(auto& shared_ptr: this->bound_functions)
            {
                weak_functions.emplace(
                    weak_functions.end(),
                    shared_ptr
                );
            }
            for(auto& weak_ptr: weak_functions)
            {
                if (auto shared_ptr = weak_ptr.lock())
                {
                    (*shared_ptr)(args...);
                }
            }
        }
        
    private:
    
        friend class Bind;
    
        FunctionList bound_functions;
        
        std::set<Bind*> binds;
    
};

#endif