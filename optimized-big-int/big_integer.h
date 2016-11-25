#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include "vector.h"

//typedef uint64_t uint64_t;
//typedef uint32_t uint32_t;

const uint64_t BASE = (1ll << 32);
const int BASEPOW = 32;

struct big_integer {
public:
	big_integer();
	big_integer(big_integer const& other);
	big_integer(int);
	explicit big_integer(std::string const&);
	~big_integer();

	big_integer& operator=(big_integer const&);

	big_integer& operator+=(big_integer const&);
	big_integer& operator-=(big_integer const&);
	big_integer& operator*=(big_integer const&);
	big_integer& operator/=(big_integer const&);
	big_integer& operator%=(big_integer const&);

	big_integer& operator&=(big_integer const&);
	big_integer& operator|=(big_integer const&);
	big_integer& operator^=(big_integer const&);

	big_integer& operator<<=(int);
	big_integer& operator>>=(int);

	big_integer operator+() const;
	big_integer operator-() const;
	big_integer operator~() const;

	big_integer& operator++();
	big_integer operator++(int);

	big_integer& operator--();
	big_integer operator--(int);

	friend int compare(big_integer const&, big_integer const&);
	friend bool operator==(big_integer const&, big_integer const&);
	friend bool operator!=(big_integer const&, big_integer const&);
	friend bool operator<(big_integer const&, big_integer const&);
	friend bool operator>(big_integer const&, big_integer const&);
	friend bool operator<=(big_integer const&, big_integer const&);
	friend bool operator>=(big_integer const&, big_integer const&);

	uint64_t element(size_t pos) const;
	int sz() const;
	bool sgn() const;

private:
	bool sign;
	vector< uint32_t > vec;
	void set_sign(bool);
	void apply_binary(big_integer const&, uint64_t(*)(uint64_t, uint64_t));
	void verify_sign();
	void to_byte();
	void from_byte();
};

int compare(big_integer const&, big_integer const&);
bool operator==(big_integer const&, big_integer const&);
bool operator!=(big_integer const&, big_integer const&);
bool operator<(big_integer const&, big_integer const&);
bool operator>(big_integer const&, big_integer const&);
bool operator<=(big_integer const&, big_integer const&);
bool operator>=(big_integer const&, big_integer const&);

std::string to_string(big_integer const&);

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);
big_integer operator&(big_integer, big_integer const&);
big_integer operator|(big_integer, big_integer const&);
big_integer operator^(big_integer, big_integer const&);
big_integer operator>>(big_integer, int);
big_integer operator<<(big_integer, int);

std::ostream& operator<<(std::ostream& s, big_integer const& a);

template <typename T>
void push(vector<T>&a, int pos);
template <typename T>
void del_zeros(vector<T>&);

template <typename T>
void mul_long_short(vector<T>& a, uint64_t base, uint64_t b);
template <typename T>
void div_long_short(vector<T>& a, uint64_t base, uint64_t b);
template <typename T>
void add_long_short(vector<T>&a, uint64_t base, uint64_t b);
template <typename T>
T _and(T a, T b);
template <typename T>
T _or(T a, T b);
template <typename T>
T _xor(T a, T b);

#endif
