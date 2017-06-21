/*
 * This file is part of readoutd.
 *
 * readoutd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * readoutd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with readoutd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright Sergey Negrashov 2014.
*/

#ifndef CORE_DATA_QUEUE_CLASS
#define CORE_DATA_QUEUE_CLASS

#include <string>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <deque>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace pland
{


typedef struct
{
    boost::posix_time::time_duration wait_durration;
    uint ops;
} Data_Queue_Stat;

template <class T>

/*! \brief Core Data Queue. This class is used for synchronization across the plugin threads.
*
* This class is thread safe. It is used in the program to pass data throughout the program.
*/

class DataQueue
{
public:

    /*! Default Constructor, creates an empty queue.
     *  \param maxsize the maximum depth of the queue. */
    DataQueue(uint maxsize = CORE_DATA_QUEUE_SIZE)
    {
        maxsize_ = maxsize;
        size_ = 0;
        profile_ = false;
        profile_stats_.ops = 0;
        profile_stats_.wait_durration = boost::posix_time::microseconds(0);
    }

    /*! Default Destructor, generally not used since the queues are persistent through out the execution of program*/
    ~DataQueue()
    {
    }

    /*! Enable or disable profiling.
    *   \param on profiling flag.
    */
    void setProfile(bool on)
    {
        boost::mutex::scoped_lock lock(mutex_);
        profile_ = on;
    }

    /*! Reset profiling counters.*/
    void resetCounters()
    {
        boost::mutex::scoped_lock lock(mutex_);
        profile_stats_.ops = 0;
        profile_stats_.wait_durration = boost::posix_time::microseconds(0);
    }

    Data_Queue_Stat getStatus()
    {
        return profile_stats_;
    }

    /*! Push data unit into the queue.
         * \param new_item data unit pushed into the queue.
         * \return false if the thread was interrupted by a signal in which case the thread will shut down. Otherwise true.
         * */
    bool push(T new_item)
    {
        boost::mutex::scoped_lock lock(mutex_);
        boost::posix_time::ptime start, stop;
        if(!profile_){}
        else
            start = boost::posix_time::microsec_clock::local_time();
        while(size_ >= maxsize_)
        {
            boost::system_time timeout=boost::get_system_time() + boost::posix_time::milliseconds(50);
            if(condition_full_.timed_wait(lock, timeout, list_not_full(size_, maxsize_)))
                break;
            boost::this_thread::interruption_point();
        }
        if(!profile_){}
        else
        {
            stop = boost::posix_time::microsec_clock::local_time();
            profile_stats_.ops++;
            profile_stats_.wait_durration += stop - start;
        }
        queue_.push_back(new_item);
        size_++;
        lock.unlock();
        condition_empty_.notify_one();
        return true;

    }

    /*! Push data unit into the queue, or timeout.
         * \param new_item data unit pushed into the queue.
         * \return false if the thread was interrupted by a signal . Otherwise true.
         * */
    bool push_timeout(T new_item, uint millisec)
    {
        boost::mutex::scoped_lock lock(mutex_);
        boost::posix_time::ptime start, stop;
        boost::system_time timeout=boost::get_system_time() + boost::posix_time::milliseconds(millisec);
        if(!profile_){}
        else
            start = boost::posix_time::microsec_clock::local_time();
        if(!condition_full_.timed_wait(lock, timeout, list_not_full(size_, maxsize_)))
            return false;
        if(!profile_){}
        else
        {
            stop = boost::posix_time::microsec_clock::local_time();
            profile_stats_.ops++;
            profile_stats_.wait_durration += stop - start;
        }
        queue_.push_back(new_item);
        size_++;
        lock.unlock();
        condition_empty_.notify_one();
        return true;

    }


    /*! Pop a data unit from the queue.
         * \return a data units.
         * */
    T pop()
    {
        T top;
        boost::mutex::scoped_lock lock(mutex_);
        boost::posix_time::ptime start, stop;
        if(!profile_){}
        else
            start = boost::posix_time::microsec_clock::local_time();
        while(true)
        {
            boost::system_time timeout=boost::get_system_time() + boost::posix_time::milliseconds(50);
            if(condition_empty_.timed_wait(lock, timeout, list_not_empty(size_)))
                break;
            boost::this_thread::interruption_point();
        }
        if(!profile_){}
        else
        {
            stop = boost::posix_time::microsec_clock::local_time();
            profile_stats_.ops++;
            profile_stats_.wait_durration += stop - start;
        }
        top = queue_.front();
        queue_.pop_front();
        size_--;
        lock.unlock();
        condition_full_.notify_one();
        return top;
    }

    /*! Pop a data unit from the queue or timeout.
         * \return a data units.
     * */
    T pop_timeout(uint millisec, bool &ok)
    {
        ok = true;
        T top;
        boost::mutex::scoped_lock lock(mutex_);
        boost::posix_time::ptime start, stop;
        if(!profile_){}
        else
            start = boost::posix_time::microsec_clock::local_time();
        boost::system_time timeout=boost::get_system_time() + boost::posix_time::milliseconds(millisec);
        if(!condition_empty_.timed_wait(lock, timeout, list_not_empty(size_)))
        {
            ok = false;
            return top;
        }
        if(!profile_){}
        else
        {
            stop = boost::posix_time::microsec_clock::local_time();
            profile_stats_.ops++;
            profile_stats_.wait_durration += stop - start;
        }
        top = queue_.front();
        queue_.pop_front();
        size_--;
        lock.unlock();
        condition_full_.notify_one();
        return top;

    }

    /**
     * @brief peek Returns the top of the queue.
     * @return top item in the queue, without removing it.
     */
    T peek()
    {
        T top;
        boost::mutex::scoped_lock lock(mutex_);
        while(true)
        {
            boost::system_time timeout=boost::get_system_time() + boost::posix_time::milliseconds(50);
            if(condition_empty_.timed_wait(lock, timeout, list_not_empty(size_)))
                break;
            boost::this_thread::interruption_point();
        }
        top = queue_.front();
        lock.unlock();
        return top;
    }

    /*! Flush the queue  deleting the contents and reseting the size.*/
    void clear()
    {
        mutex_.lock();
        queue_.clear();
        size_ = 0;
        condition_full_.notify_all();
        mutex_.unlock();
    }

    /*! Check if the queue is empty.
         * \return true if the queue is empty, false otherwise.
         * */
    bool is_empty()
    {
        return size_ == 0;
    }

    /*! Check if the queue is full.
         * \return false if the queue is empty, true otherwise.
         * */
    bool is_full()
    {
        return size_ != maxsize_;
    }

    /*! Returns the number of items in the queue.
         * \return number of items in the queue.
         */
    uint size()
    {
        return size_;
    }

    /*! Returns maximum queue size.
         * \return maximum queue size.
         */
    uint max_size()
    {
        return maxsize_;
    }

    static const int CORE_DATA_QUEUE_SIZE = 500;

private:

	struct list_not_empty
	{
		uint &size_;
		list_not_empty(uint &size): size_(size){}
		bool operator()() const
		{
			return size_ != 0;
		}
	};

	struct list_not_full
	{
		uint &size;
		uint & max;
		list_not_full(uint &size, uint &max_size):size(size), max(max_size){}
		bool operator()() const
		{
			return size <max;
		}
	};

    /*! Mutex for synchronization.*/
    mutable boost::mutex mutex_;

    /*! Conditinal variable for the readers*/
    boost::condition_variable condition_empty_;

    /*! Conditinal variable for the writers*/
    boost::condition_variable condition_full_;

    /*! Profiling flag */
    bool profile_;

    /*! stl list used for storage of data.*/
    std::deque <T> queue_;

    /*! size of the attached list, this way it does not have to be locked for reading.*/
    uint size_;

    /*! Maximum size of the queue, passed at initialization.*/
    uint        maxsize_;
    uint        waited_;

    /*! Profiling data structure */
    Data_Queue_Stat profile_stats_;
};
}
#endif /*coredataqueue.hpp*/
