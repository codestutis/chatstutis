//
// defines a bounded buffer that supports the producer-consumer pattern
//
// the buffer elements are simply void* pointers
// note: the buffer does not make a copy of the data.
//       it simply stores a pointer to the data.
//
// the buffer is FIFO (first in, first out)
//

// create a bounded buffer
//   size: the number of elements in the buffer
//   returns: a handle to be used to operate on the buffer
void *createBuffer(unsigned int size);

// put a value in a buffer
//   handle: handle for bounded buffer
//   p: pointer to be put in the buffer
//   note: calling thread will block until there is space in the buffer
void putBuffer(void *handle, void *p);

// get a value from a buffer
//   handle: handle for bounded buffer
//   returns: pointer retrieved from buffer
//   note: calling thread will block until there is a value available
void *getBuffer(void *handle);

// get a value from a buffer if available
//   handle: handle for bounded buffer
//   returns: pointed retreived from buffer
//   note: calling thread will not block, will return NULL if no value available
void *tryGetBuffer(void *handle);

// delete a buffer
//   handle: handle for the bounded buffer to be deleted
void deleteBuffer(void *handle);

