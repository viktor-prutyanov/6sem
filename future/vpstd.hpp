#include <iostream>
#include <thread>
#include <functional>

namespace vpstd
{
    template<typename _Res>
    class future 
    {
        public:
        future() {};
        future(future&& that) = default;
        future(const future&) = delete;
        future& operator=(future&& that) = default;
        ~future() = default;

        template<typename _Fn, typename... _Args>
        friend future<typename std::result_of<_Fn(_Args...)>::type> async(_Fn&&, _Args&&...);
        
        _Res get() 
        {
            th.join();
            return result;
        }
   
        private:
        template<typename _Fn, typename... _Args>
        void wrapper(_Fn&& f, _Args&&... args) 
        {
            result = f(args...);
        }
    
        _Res result;
        std::thread th;
    };
   
    template<typename _Fn, typename... _Args>
    future<typename std::result_of<_Fn(_Args...)>::type> async(_Fn&& f, _Args&&... args)
    {
        future<typename std::result_of<_Fn(_Args...)>::type> fut;
        fut.th = std::thread(&future<typename std::result_of<_Fn(_Args...)>::type>::wrapper<typename _Fn, typename... _Args>, &fut, f, args...);
        return fut;
    }
}
