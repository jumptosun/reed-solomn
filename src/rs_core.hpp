#ifndef RS_CORE_HPP
#define RS_CORE_HPP

#include <stddef.h>
#include <assert.h>
#include <sys/types.h>
#include <pthread.h>


/////////////////// OPTION //////////////////////////

// whether turn on the thread safety
#undef RS_CONFIG_OPTION_THREAD_SAFETY
// change the log funtion if need
#define rs_log(msg, ...) printf(msg, ##__VA_ARGS__)

/////////////////////////////////////////////////////

#define rs_min(a, b) (a < b? a : b)
#define rs_max(a, b) (a > b? a : b)
#define rs_square(a) (a * a)

#define rs_freep(p) \
    if(p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0

#define rs_freepa(pa) \
    if(pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#define RsAutoFree(className, instance) \
impl__RsAutoFree<className> _auto_free_##instance(&instance, false)
#define RsAutoFreeA(className, instance) \
impl__RsAutoFree<className> _auto_free_array_##instance(&instance, true)
template<class T>
class impl__RsAutoFree
{
private:
    T** ptr;
    bool is_array;
public:
    /**
     * auto delete the ptr.
     */
    impl__RsAutoFree(T** p, bool array) {
        ptr = p;
        is_array = array;
    }

    virtual ~impl__RsAutoFree() {
        if (ptr == NULL || *ptr == NULL) {
            return;
        }

        if (is_array) {
            delete[] *ptr;
        } else {
            delete *ptr;
        }

        *ptr = NULL;
    }
};

class RsAutoLock
{
public:
    RsAutoLock(pthread_mutex_t *mutex)
    {
        _mutex = mutex;
        pthread_mutex_lock(_mutex);
    }

    ~RsAutoLock()
    {
        pthread_mutex_unlock(_mutex);
    }

private:
    pthread_mutex_t* _mutex;
};


#endif // RS_CORE_HPP
