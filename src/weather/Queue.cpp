#include "Queue.h"
#include <climits>

template <typename T>
Queue<T>::Queue() : length(DEFAULT_LENGTH), size(0), front(0), back(0), sum(0), min(INT_MAX), max(INT_MIN) {
   circularArr = new T[DEFAULT_LENGTH]();
}

template <typename T>
Queue<T>::Queue(unsigned int length)
   : length(length),
     size(0),
     front(0),
     back(0),
     sum(0),
     min(INT_MAX),
     max(INT_MIN),
     circularArr(nullptr)
{
   if (length > 0) {
      circularArr = new T[length];
   } else {
      circularArr = new T[DEFAULT_LENGTH];
      this->length = DEFAULT_LENGTH;
   }
}

template <typename T>
Queue<T>::~Queue() {
   delete[] circularArr;
}

template <typename T>
void Queue<T>::set(unsigned int index, const T element) {
   circularArr[index] = element;
}

template <typename T>
void Queue<T>::push(const T element) {
   if (isEmpty()) {
      set(front, element);
      back = front;
      size = 1;
      sum = element;
   }
   else if (isFull()) {
      incrementFront();
      incrementBack();

      sum -= circularArr[back];
      sum += element;

      set(back, element);
   }
   else {
      incrementBack();

      sum += element;
      set(back, element);

      size++;
   }

   min = circularArr[front];
   max = circularArr[front];

   for (unsigned int i = 1; i < size; i++) {
      unsigned int index = (front + i) % length;

      if (circularArr[index] < min) {
         min = circularArr[index];
      }

      if (circularArr[index] > max) {
         max = circularArr[index];
      }
   }
}

template<typename T>
void Queue<T>::incrementFront() {
   if (front == length - 1) { // if front is at the end of the array
      front = 0; // wrap around
   } else {
      front++;
   }
}

template<typename T>
void Queue<T>::incrementBack() {
   if (back == length - 1) { // if back is at the end of the array
      back = 0; // wrap around
   } else {
      back++;
   }
}

template<typename T>
void Queue<T>::decrementFront() {
   if (front == 0) {
      front = length - 1;
   } else {
      front--;
   }
}

template<typename T>
void Queue<T>::decrementBack() {
   if (back == 0) {
      back = length - 1;
   } else {
      back--;
   }
}

template <typename T>
bool Queue<T>::isFull() {
   return size >= length;
}

template <typename T>
bool Queue<T>::isEmpty() {
   return size == 0;
}

template <typename T>
T Queue<T>::calculateAverage() {
   if (isEmpty()) {return 0;}
   return float(sum) / float(size);
}

template <typename T>
T Queue<T>::getMin() {
   if (isEmpty()) {
      return INT_MAX;
   }
   return min;
}

template <typename T>
T Queue<T>::getMax() {
   if (isEmpty()) {
      return INT_MIN;
   }
   return max;
}

template class Queue<int>;
template class Queue<float>;
template class Queue<double>;