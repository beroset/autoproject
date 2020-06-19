I am trying to design thread-safe data structure that I cna use as a buffer in my application. Can you please give me comments about this code and what can be improved:

    #include <deque>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    
    template <typename T>
    class Buffer
    {
    public:
        void add(T num)
        {
            while (true)
            {
                std::unique_lock<std::mutex> locker(mu);
                buffer.push_back(num);
                locker.unlock();
                cond.notify_all();
                return;
            }
        }
        T remove()
        {
            while (true)
            {
                std::unique_lock<std::mutex> locker(mu);
                cond.wait(locker, [this](){return buffer.size() > 0;});
                T back = buffer.back();
                buffer.pop_back();
                locker.unlock();
                cond.notify_all();
                return back;
            }
        }
        int size()
        {
            std::unique_lock<std::mutex> locker(mu);
            int s = buffer.size();
            locker.unlock();
            return s;
        }
    private:
        std::mutex mu;
        std::condition_variable cond;
    
        std::deque<T> buffer;
    };