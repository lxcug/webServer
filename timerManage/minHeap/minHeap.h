//
// Created by lx on 1/23/22.
//

#ifndef WEBSERVER_MINHEAP_H
#define WEBSERVER_MINHEAP_H
#include <vector>
#include <iostream>

using namespace std;

template<class T>
class minHeap {
public:
    minHeap();
    ~minHeap();
    void siftDown(int root);  // 建堆的辅助函数
    void siftUp(int k);  // 向堆内添加元素的辅助函数
    T getMaxElem();  // 取出最大元素并保证仍为最大堆
    T popMaxElem();  // 取出最大元素并保证仍为最大堆
    int size();
    void insertElem(T elem);  // 插入元素
    void adjust();
    void deleteElem(int i);
    inline T& operator[](int i) {
        return a[i];
    }


private:
    vector<T> a;
};


template<class T>
minHeap<T>::minHeap() {
    a.resize(0);
}

template<class T>
minHeap<T>::~minHeap() = default;

template<class T>
void minHeap<T>::siftDown(int root) {
    T temp = a[root];
    for(int i = 2*root+1; i < a.size(); i = 2*i+1) {  // 如果此级调整还需要循环调整下一级
        if(i+1 < a.size())  // 找到左右子树中较小值
            i = (a[i]->expire) < (a[i+1]->expire) ? i : i+1;
        if(temp->expire <= a[i]->expire)
            break;
        else {
            a[root] = a[i];  // 交换一次，如果用swap需要交换3次
            root = i;  // 上一级调整可能导致下一级的堆，继续对下一级进行调整
        }
    }
    a[root] = temp;
}

template<class T>
void minHeap<T>::siftUp(int k) {
    T temp = a[k];
    for(int i = (k-1)/2; i >= 0; i = (i-1)/2) {  // 注意i=0时，i = (i-1)/2 = int(-0.5) = 0
        if(a[i]->expire > temp->expire) {
            a[k] = a[i];
            k = i;
        }
        if(i == 0)  // i = 0时调整完break避免进入死循环
            break;
    }
    a[k] = temp;
}

template<class T>
T minHeap<T>::getMaxElem() {
    return a[0];
}

template<class T>
T minHeap<T>::popMaxElem() {
    T res = a[0];
    cout << "delete " << a[0]->m_user_data->sockfd << "'s timer ";
    swap(a[0], a[a.size()-1]);
    this->a.pop_back();
    siftDown(0);
    return res;
}

template<class T>
void minHeap<T>::insertElem(T elem) {
    this->a.emplace_back(elem);
    siftUp(a.size()-1);
}

template<class T>
void minHeap<T>::adjust() {
    for(int i = 0; i < a.size(); i++)
        siftDown(i);
}

template<class T>
void minHeap<T>::deleteElem(int i) {
    a.erase(a.begin()+i);
    adjust();
}

template<class T>
int minHeap<T>::size() {
    return a.size();
}


#endif //WEBSERVER_MINHEAP_H
