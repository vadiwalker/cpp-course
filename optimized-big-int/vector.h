#ifndef VECTOR_H
#define VECTOR_H
#include <iostream>
#include <memory.h>

const int SMALL = 7;

template <typename T>
struct hdd {
	int refs;
	T a[];
};

template<typename T>
struct big_data {
	hdd<T>* ptr;
	size_t _size;
	size_t volume;

	size_t size() const { return _size; }
	size_t capacity() const { return volume; }
	T& operator[](size_t i) { return ptr->a[i]; }
	T const& operator[](size_t i) const { return ptr->a[i]; }
	void pop_back() { --_size; }
	void push_back(T const& value) {
		ptr -> a[size()] = value;
		++_size;
	}
};

template<typename T>
struct small_data {
	char magic;
	T reg[SMALL];

	size_t size() const { return magic / 2; }
	size_t capacity() const { return SMALL; }
	T& operator[](size_t i) { return reg[i]; }
	T const& operator[](size_t i) const { return reg[i]; }
	void pop_back() { magic -= 2; }
	void push_back(T const& value) {
		reg[size()] = value;
		magic += 2;
	}
};

template <typename T>
struct vector {

	vector(size_t n = 0);
	vector(vector const& x);
	vector& operator=(vector const& other);
	~vector();

	bool is_small() const;
	size_t size() const;
	bool empty() const;
	void resize(size_t n);
	void pop_back();
	void push_back(T const& value);
	void clear();
	T& back();
	T& operator[](size_t i);
	T const& operator[](size_t i) const;

private:

	union {
		big_data<T> bd;
		small_data<T> sd;
	};

	void check_refs();
	void ensure_cap(size_t n);
	size_t capacity() const;
	void big_to_small();

};


template <typename T>
vector<T>::vector(size_t n) {
	sd.magic = 1;
	resize(n);
}

template <typename T>
vector<T>::vector (vector const& x) : vector(0) {
	*this = x;
}

template <typename T>
vector<T>& vector<T>::operator=(vector const& other) {
	clear();
	memcpy(this, &other, sizeof(vector));
	if (!is_small()) {
		bd.ptr->refs++;
	}
	return *this;
}

template <typename T>
vector<T>::~vector() {
	clear();
}

template <typename T>
bool vector<T>::is_small() const {
	return sd.magic & 1;
}

template <typename T>
size_t vector<T>::size() const {
	return is_small() ? sd.size() : bd.size();
}

template <typename T>
bool vector<T>::empty() const {
	return size() == 0;
}

template <typename T>
void vector<T>::resize(size_t n) {
	check_refs();
	if (n <= SMALL) {
		if (is_small()) {
			for (size_t i = size(); i < n; ++i) {
				(*this)[i] = 0;
			}
			sd.magic = 1 | (n << 1);
		} else {
			bd._size = n;
			big_to_small();
		}
	} else {
		ensure_cap(n);
		for (size_t i = size(); i < n; ++i) {
			(*this)[i] = 0;
		}
		bd._size = n;
	}
}

template <typename T>
void vector<T>::pop_back() {
	check_refs();
//	ensure_cap(size() - 1);
	is_small() ? sd.pop_back() : bd.pop_back();
}

template <typename T>
void vector<T>::push_back(T const& value) {
	check_refs();
	ensure_cap(size() + 1);
	is_small() ? sd.push_back(value) : bd.push_back(value);
}

template <typename T>
void vector<T>::clear() {
	if (!is_small()) {
		bd.ptr->refs--;
		if (!bd.ptr->refs)
			delete bd.ptr;
		sd.magic = 1;
	}
}

template <typename T>
T& vector<T>::back() {
	return (*this)[size()-1];
}

template <typename T>
T& vector<T>::operator[](size_t i) {
	check_refs();
	return is_small() ? sd[i] : bd[i];
}

template <typename T>
T const& vector<T>::operator[](size_t i) const {
	return is_small() ? sd[i] : bd[i];
}

template <typename T>
void vector<T>::check_refs() {
	if (!is_small() && bd.ptr->refs > 1) {
		bd.ptr->refs--;
		hdd<T>* tmp = bd.ptr;
		bd.ptr = (hdd<T>*) operator new (sizeof(hdd<T>) + bd.volume * sizeof(T));
		bd.ptr->refs = 1;
		for (int i = 0; i < (int) size(); ++i) {
			bd.ptr->a[i] = tmp->a[i];
		}
	}
}

template <typename T>
void vector<T>::ensure_cap(size_t n) {

	if (capacity() < n) {
		size_t sz = std::min(size(), n);
		T* buf = new T[sz];
		for (size_t i = 0; i != sz; ++i) {
			buf[i] = (*this)[i];
		}
		clear();
		bd.ptr = (hdd<T>*) operator new (sizeof(hdd<T>) + sizeof(T) * n * 2);
		bd.ptr->refs = 1;
		bd._size = sz;
		bd.volume = n * 2;
		for (size_t i = 0; i < sz; ++i) {
			(*this)[i] = buf[i];
		}
		delete[] buf;
	}
}

template <typename T>
size_t vector<T>::capacity() const {
	return is_small() ? sd.capacity() : bd.capacity();
}

template <typename T>
void vector<T>::big_to_small() {
	hdd<T>* tmp = bd.ptr;
	size_t sz = size();
	for (size_t i = 0; i != sz; ++i) {
		sd.reg[i] = tmp -> a[i];
	}
	sd.magic = (sz << 1) | 1;
	tmp->refs--;
	if (!tmp->refs) {
		delete tmp;
	}
}

#endif
