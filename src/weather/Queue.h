#ifndef QUEUE_H
#define QUEUE_H

#define DEFAULT_LENGTH 10 // default circular array length

/**
   @brief Queue implemented with a circular array
*/
template <class T> // a generic type
class Queue {

   private:

   unsigned int length; // total length of circular array
   unsigned int size; // number of elements in the queue

   unsigned int front; // index of the front of the queue (oldest element)
   unsigned int back; // index of the back of the queue (newest element)

   T min; // minimum value in the queue
   T max; // maximum value in the queue

   T sum; // a running sum of the elements

   T* circularArr; // circular array to hold elements of type T

   void incrementFront();
   void incrementBack();

   void decrementFront();
   void decrementBack();
   
   void set(unsigned int index, const T element);

   public:

   Queue();

   Queue(unsigned int length);

   ~Queue();

   void push(const T element);

   bool isFull();

   bool isEmpty();

   T calculateAverage();

   T getMin();

   T getMax();

};


#endif /* QUEUE_H */