/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file CProtSharedMemory.h
 *       A shared memory implementation of the communication protocol interface.
 *
 *  This implementation may be used to communicate between processes that share
 *  memory.
 *
 * @author Joachim Protze
 * @date 15.03.2012
 *
 */

#ifndef C_PROT_SHARED_MEMORY_H
#define C_PROT_SHARED_MEMORY_H

#include "I_CommProtocol.h"
#include "I_Module.h"
#include "ModuleBase.h"

#include <vector>
//#include <list>
//#include <map>
#include <queue>
#include <stack>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <sys/types.h>


extern "C" {
void __attribute__((weak)) AnnotateHappensAfter(const char *file, int line, const volatile void *cv){}
void __attribute__((weak)) AnnotateHappensBefore(const char *file, int line, const volatile void *cv){}
void __attribute__((weak)) AnnotateNewMemory(const char *file, int line, const volatile void *cv, size_t size){}
}
// This marker is used to define a happens-before arc. The race detector will
// infer an arc from the begin to the end when they share the same pointer
// argument.
# define TsanHappensBefore(cv) AnnotateHappensBefore(__FILE__, __LINE__, cv)
// This marker defines the destination of a happens-before arc.
# define TsanHappensAfter(cv) AnnotateHappensAfter(__FILE__, __LINE__, cv)
# define TsanNewMemory(addr, size) AnnotateNewMemory(__FILE__, __LINE__, addr, size)



namespace gti
{

	// Data structure to provide a threadsafe pool of reusable objects.
	// DataPool<Type of objects, Size of blockalloc>
	template <typename T, int N>
	struct DataPool {
	  std::mutex DPMutex;
	  std::stack<T *> DataPointer;
	  int total;

    typedef struct pooldata {T data; DataPool<T,N>* dp; bool allocated;} pooldata_type;

	  void newDatas(){
	    // prefix the Data with a pointer to 'this', allows to return memory to 'this',
	    // without explicitly knowing the source.
	    //
	    // To reduce lock contention, we use thread local DataPools, but Data objects move to other threads.
	    // The strategy is to get objects from local pool. Only if the object moved to another
	    // thread, we might see a penalty on release (returnData).
	    // For "single producer" pattern, a single thread creates tasks, these are executed by other threads.
	    // The master will have a high demand on TaskData, so return after use.
	    // We alloc without initialize the memory. We cannot call constructors. Therfore use malloc!
	    pooldata_type* datas = (pooldata_type*) malloc(sizeof(pooldata_type) * N);
	    datas[0].allocated=true;
	    for (int i = 0; i<N; i++) {
	      datas[i].dp = this;
	      DataPointer.push(&(datas[i].data));
	    }
	    total+=N;
	  }

	  T * getData() {
	    T * ret;
	    DPMutex.lock();
	    if (DataPointer.empty())
	      newDatas();
	    ret=DataPointer.top();
	    DataPointer.pop();
	    DPMutex.unlock();
	    return ret;
	  }

	  void returnData(T * data) {
	    DPMutex.lock();
	    DataPointer.push(data);
	    DPMutex.unlock();
	  }

	  void getDatas(int n, T** datas) {
	    DPMutex.lock();
	    for (int i=0; i<n; i++) {
	      if (DataPointer.empty())
		newDatas();
	      datas[i]=DataPointer.top();
	      DataPointer.pop();
	    }
	    DPMutex.unlock();
	  }

	  void returnDatas(int n, T** datas) {
	    DPMutex.lock();
	    for (int i=0; i<n; i++) {
	      DataPointer.push(datas[i]);
	    }
	    DPMutex.unlock();
	  }

	  DataPool() : DataPointer(), DPMutex(), total(0)
	  {}
	  ~DataPool()
	  {
      for (auto i : DataPointer)
      {
        if (i->allocated)
          free(i);
      }

	  }

	};

  // This function takes care to return the data to the originating DataPool
  // A pointer to the originating DataPool is stored just before the actual data.
  template <typename T, int N>
  static void retData(void * data) {
    ((typename DataPool<T,N>::pooldata_type*)data)->dp->returnData((T*)data);
  }


  struct SMRequest {
    static __thread DataPool<SMRequest, 8> *dp;
    uint64_t size;                 // size for send/ recv
    uint64_t inSize;              // actual received size
    unsigned int request;               // request for non-blocking
    uint64_t channel;              // channel for recv
    std::mutex SMRMutex;                // mutex for request, used for signaling
    std::condition_variable SMRCondVar; // conditional variable for waiting and signaling
    std::atomic<bool> finished;         // send is finished
    bool send;                          // this is a send request
    void * data;                        // pointer to data
    // send
    SMRequest(void * buf, uint64_t size, int request) : size(size), request(request), channel(0), SMRMutex(), SMRCondVar(), finished(false), send(true), data(buf) {}
    // recv
    SMRequest(void * buf, uint64_t size, uint64_t channel, int request) : size(size), request(request), channel(channel), SMRMutex(), SMRCondVar(), finished(false), send(false), data(buf) {}
    void * operator new(size_t size){
      void * ret = dp->getData();
      TsanNewMemory(ret, size);
      return ret;
    }
    void operator delete(void* p, size_t){
      retData<SMRequest, 2>(p);
    }
  };
  __thread DataPool<SMRequest, 8> *SMRequest::dp = nullptr;


/*  template <int N>
  struct SMRequestContainer : SMRequest {
    static __thread DataPool<SMRequestContainer<N>, 2> dp;
    char data[N];
    void * getData(){return data_no_copy ? data_no_copy : data;}
    // overload new/delete to use DataPool for memory management.
    void * operator new(size_t size){
      return SMRequestContainer<N>::dp->getData();
    }
    void operator delete(void* p, size_t){
      retData<SMRequestContainer<N>, 2>(p);
    }
  }*/

/*  static SMRequest* packageMessage(void * buf, size_t size, bool async)
  {
    SMRequest* ret;
    if (!async)
    {
      ret = new SMRequest(buf, size);
    } if (size < 128-sizeof(SMRequest)) {
      ret = new SMRequestContainer<128-sizeof(SMRequest)>();
    }
  }*/

/*  template <int N>
  __thread typename SMRequestContainer<N>::DataPool<SMRequestContainer<N>, 2> dp();*/

  struct SMQueue {
    std::atomic<int> size;
    std::queue<SMRequest*> MessageQueue;
    std::mutex SMQMutex;
    std::condition_variable SMQCondVar;
    uint64_t outChannel;
    uint64_t inChannel;
    SMQueue() : size(0), MessageQueue(), SMQMutex() {}
    bool empty(){return size==0;}
    SMRequest* try_pop(){
	    std::unique_lock<std::mutex> lock(SMQMutex);
	    if (MessageQueue.empty())
	      return nullptr;

	    SMRequest * ret=MessageQueue.front();
	    MessageQueue.pop();
	    size = MessageQueue.size();
	    return ret;
    }
    SMRequest* wait_pop(){
	    std::unique_lock<std::mutex> lock(SMQMutex);
	    while (MessageQueue.empty())
	      SMQCondVar.wait(lock);

	    SMRequest * ret=MessageQueue.front();
	    MessageQueue.pop();
	    size = MessageQueue.size();
	    return ret;
    }
    void push(SMRequest* p){
	    std::unique_lock<std::mutex> lock(SMQMutex);
	    MessageQueue.push(p);
	    size = MessageQueue.size();
	    SMQCondVar.notify_one();
    }
  };


  /* SMSyncPoint enforces P2P synchronization. Whoever reaches first blocks and waits for the partner to arrive */
  struct SMSyncPoint {
    bool isActive = false;
    std::mutex SMSPMutex;
    std::condition_variable SMSPCondVar;
    void visit(){
	    std::unique_lock<std::mutex> lock(SMSPMutex);
      if (isActive)
      {
        isActive = false;
        TsanHappensAfter(&isActive);
        TsanHappensBefore(&isActive);
        SMSPCondVar.notify_one();
        return;
      }
      else
      {
        isActive = true;
        TsanHappensBefore(&isActive);
        while (isActive)
          SMSPCondVar.wait(lock);
        TsanHappensAfter(&isActive);
        return;
      }
    }
  };

    /**
     * Class that describes the communication protocol interface.
     */
    class CommProtSharedMemory : public ModuleBase<CommProtSharedMemory, I_CommProtocol>
    {
    protected:
        bool initialized;
        bool finalized;
        bool isTop;
        bool myIsIntra;

        std::vector<SMQueue*> outQueues;    // out-going messages

        std::vector<SMQueue*> inQueues;     // in-comming messages
        std::vector<std::queue<SMRequest*>*> recvQueues;   // open non-blocking recv requests (accessed only locally)
        static SMSyncPoint entrySyncPoint;
        static SMSyncPoint exitSyncPoint;
        //std::queue<SMRequest*>* recvAnyQueue;

        static SMQueue* helloQueue;

        int numPartners;
        int maxNumPartners;

        uint64_t gtiOwnLevel;
        uint64_t remoteTierSize;
        uint64_t tierSize;
        int commId;
        GtiTbonNodeInLayerId myPlaceId;
        char commSide;
        int next_channel=0;

        std::map<int, SMRequest*> requestMap;
        unsigned int requestId;

        static void dummy(void){}
        static void initModule(); // called once
        void(*newClientCallback)(void) = dummy;

        /**
         * initialize connections
         */
        void connect();
        void reconnect();
        ssize_t recv_wrapper ( void* out_buf, size_t num_bytes, uint64_t channel, uint64_t* out_channel, int msgflg=0 );
        void handle_test( unsigned int request, int* out_completed, uint64_t* out_receive_length, uint64_t* out_channel, bool test);
    public:

        /**
         * Constructor.
         * @ref ModConf - The module configuration syntax
         * @param intanceName name of the module instance.
         */
        CommProtSharedMemory (const char* instanceName);

        /**
         * Destructor.
         */
        ~CommProtSharedMemory (void);

        /**
         * @see gti::I_CommProtocol::isConnected
         */
        bool isConnected (void);

        /**
         * @see gti::I_CommProtocol::isInitialized
         */
        bool isInitialized (void);

        /**
         * @see gti::I_CommProtocol::isFinalized
         */
        bool isFinalized (void);

        /**
         * @see gti::I_CommProtocol::getNumChannels
         */
        GTI_RETURN getNumChannels (uint64_t* out_numChannels);

        /**
         * @see gti::I_CommProtocol::getNumClients
         */
        GTI_RETURN getNumClients (uint64_t* out_numClients);

        /**
         * @see gti::I_CommProtocol::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see gti::I_CommProtocol::shutdown
         */
        GTI_RETURN shutdown (void);

        /**
         * @see gti::I_CommProtocol::removeOutstandingRequests
         */
        GTI_RETURN removeOutstandingRequests (void);

        GTI_RETURN registerNewClientCallback (
                void(*fun)(void),
                bool& isUsed)
        {isUsed=true; newClientCallback = fun; return GTI_SUCCESS;}
        /**
         * @see gti::I_CommProtocol::ssend
         */
        GTI_RETURN ssend (
                void* buf,
                uint64_t num_bytes,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::isend
         */
        GTI_RETURN isend (
                void* buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::recv
         */
        GTI_RETURN recv (
                void* out_buf,
                uint64_t num_bytes,
                uint64_t* out_length,
                uint64_t channel,
                uint64_t *out_channel
                );

        /**
         * @see gti::I_CommProtocol::irecv
         */
        GTI_RETURN irecv (
                void* out_buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::test_msg
         */
        GTI_RETURN test_msg (
                unsigned int request,
                int* out_completed,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

        /**
         * @see gti::I_CommProtocol::wait_msg
         */
        GTI_RETURN wait_msg (
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );
    }; /*class CommProtSharedMemory*/
} /*namespace gti*/

#endif /* C_PROT_SHARED_MEMORY_H */
