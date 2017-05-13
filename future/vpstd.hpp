#include <iostream>
#include <thread>
#include <functional>

namespace vpstd
{
    class future 
    {
        public:
        future() {};
        future(future&& that) = default;
        future(const future&) = delete;
        future& operator=(future&& that) = default;
        ~future() = default;
        friend future async(std::function<bool(int)> func, int arg);
        bool get() 
        {
            th.join();
            return result;
        }
   
        private:
        void wrapper(std::function<bool(int)> func, int arg) 
        {
            result = func(arg);
        }
    
        bool result;
        std::thread th;
    };
    
    future async(std::function<bool(int)> func, int arg)
    {
        future fut;
        fut.th = std::thread(&future::wrapper, &fut, func, arg);
        return fut;
    }
}
