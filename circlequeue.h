#ifndef CIRCLEQUEUE_H
#define CIRCLEQUEUE_H

#include <QQueue>

//Кольцевой буфер на основе стандартной очереди
template <class T>
class CQueue : public QQueue <T> {
 //как наследоваться от стандартного контейнера
 //для любого типа данных T
 private:
  int count; //предельный размер
 public:
  inline CQueue (int cnt) : QQueue<T>(),count(cnt) {
   //конструктор
  }
  inline void enqueue (const T &t) {
   //перегрузка метода постановки в очередь
   if (count==QQueue<T>::count()) {
    QQueue<T>::dequeue();
    //удалить старейший элемент по заполнении
   }
   QQueue<T>::enqueue(t);
  }
};

#endif // CIRCLEQUEUE_H
