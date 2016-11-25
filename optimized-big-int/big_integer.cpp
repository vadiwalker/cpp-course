#include <iostream>
#include <algorithm>
#include <vector>
#include "big_integer.h"
#include "vector.h"

//typedef uint64_t ui64;
//typedef uint32_t uint32_t;

big_integer::big_integer() : big_integer(0) {}

big_integer::big_integer(int value)
	: sign(0 <= value) {
	vec.push_back(std::abs((long long)value));
}

big_integer::big_integer(big_integer const& other)
	: sign(other.sign),
		vec(other.vec) {}

big_integer::~big_integer() {
	vec.clear();
}

big_integer::big_integer(std::string const& str) {
	vec.push_back(0);
	uint64_t base = 10;
	vector<uint64_t> demical;

	for (int i = (int) str.size()-1; i >= 0; --i) {
		if (str[i] != '-')
			demical.push_back(str[i] - '0');
	}

	uint64_t ptr = 0;
	while (demical.size() > 1 || demical.back())	{
		if (demical[0] & 1) {
			vec.back() |= (1UL << ptr);
		}
		++ptr;
		div_long_short(demical, base, 2);
		if (ptr == 32)	{
			ptr = 0;
			vec.push_back(0);
		}
	}
	sign = str[0] != '-';
	del_zeros(vec); verify_sign();
}

big_integer& big_integer::operator=(big_integer const& other)
{
	sign = other.sign;
	vec = other.vec;
	return *this;
}

big_integer& big_integer::operator+=(big_integer const& other) {
	if (sgn() == other.sgn()) {
		vec.resize(std::max(vec.size(), other.vec.size())+1);
		uint64_t carry = 0;
		for (size_t i = 0; i != vec.size(); ++i) {
			uint64_t value = carry + vec[i] + other.element(i);
			vec[i] = value % BASE;
			carry = value / BASE;
		}
		del_zeros(vec);

	} else {
		if (sgn()) {
			*this -= (-other);
		} else {
			*this = -(-*this - other);
		}
	}
	return *this;
}

bool big_integer::sgn() const {
	return sign;
}

big_integer& big_integer::operator-=(big_integer const& other)
{
	if (sgn() == other.sgn()) {

		int n = std::max(vec.size(), other.vec.size());
		vec.resize(n);

		int carry = 0;
		for (int i = 0; i < n; ++i) {
			int64_t val = (int64_t) element(i) - other.element(i) - carry;
			carry = (val < 0);
			vec[i] = (val + BASE) % BASE;
		}

		if (carry) {
			carry = 0;
			sign ^= 1;
			for (int i = 0; i < n; ++i) {
				int64_t val = -(int64_t) element(i) - carry;
				carry = (val < 0);
				vec[i] = (val + BASE) % BASE;
			}
		}

		del_zeros(vec); verify_sign();
	} else {
		if (sgn()) {
			*this += -other;
		} else {
			*this = -(-*this + other);
		}
	}
	return *this;
}




big_integer& big_integer::operator*=(big_integer const& other) {

	vector<uint32_t> buf(vec.size() + other.vec.size());
	for (size_t i = 0; i != vec.size(); ++i) {
		uint64_t carry = 0;
		for (size_t j = 0; j < other.vec.size() || carry; ++j) {
			uint64_t value = carry + buf[i+j] + vec[i] * other.element(j);
			buf[i+j] = value % BASE;
			carry = value / BASE;
		}
	}
	vec = buf;
	sign = !(sign ^ other.sign);
	del_zeros(vec); verify_sign();
	return *this;
}

big_integer& big_integer::operator/=(big_integer const& other) {

	int n = vec.size();
	int m = other.vec.size();

	vector<uint32_t> cur;
	vector<uint32_t> ans(n-m+1);

	for (int i = n-m; i >= 0; --i) {
		uint64_t l = 0;
		double a = other.element(m-1) + other.element(m-2) * 1.0 / BASE;
		double b = element(i+m-1) + element(i+m) * BASE;
		uint64_t r = b / a + 2;

		l = std::max(0, int(r - 3));

		while (l + 1 < r) {
			uint64_t m = (l+r) / 2;
			cur = other.vec;
			mul_long_short(cur, BASE, m);

			// fast check or ((m * other) << 32 * i) <= *this

			bool less_or_equal = true;
			if (cur.size() + i != vec.size()) {
				if (cur.size() + i > vec.size()) less_or_equal = false;
			} else {
				for (int j = cur.size()-1; j >= 0; --j) {
					if (vec[j+i] != cur[j]) {
						if (cur[j] > vec[j+i]) less_or_equal = false;
						break;
					}
				}
			}
			if (less_or_equal) {
				l = m;
			} else {
				r = m;
			}
		}

		// fast subtracts (cur << 32*i) from *this
		cur = other.vec;
		mul_long_short(cur, BASE, l);
		int64_t carry = 0;
		for (size_t j = 0; j != cur.size(); ++j) {
			int64_t val = (int64_t) vec[i+j] - cur[j] -carry;
			carry = (val < 0);
			vec[i+j] = (val + BASE) % BASE;
		}
		if (carry) {
			vec[cur.size()+i]--;
		}
		del_zeros(vec);

		ans[i] = l;
	}
	vec = ans;
	if (vec.empty()) {
		vec.push_back(0);
	}
	sign = !(sign ^ other.sign);
	del_zeros(vec); verify_sign();
	return *this;
}

big_integer& big_integer::operator%=(big_integer const& other) {
	big_integer a = *this;
	a /= other;
	a *= other;
	return *this -= a;
}

template <typename T>
T _and(T a, T b) {
	return a & b;
}

template <typename T>
T _or(T a, T b) {
	return a | b;
}

template <typename T>
T _xor(T a, T b) {
	return a ^ b;
}

void big_integer::apply_binary(big_integer const& other, uint64_t (*func)(uint64_t, uint64_t)) {
	big_integer b(other);
	uint64_t first = (sign ? 0 : BASE-1);
	uint64_t second = (b.sign ? 0 : BASE-1);
	to_byte();
	b.to_byte();
	size_t n = std::max(vec.size(), b.vec.size());
	for (size_t i = 0; i != n; ++i) {
		if (i == vec.size())
			vec.push_back(first);
		vec[i] = (*func)(vec[i], (i < b.vec.size() ? b.vec[i] : second));
	}
	from_byte();
	del_zeros(vec); verify_sign();
}

big_integer& big_integer::operator&=(big_integer const& other) {
	apply_binary(other, _and);
	return *this;
}

big_integer& big_integer::operator|=(big_integer const& other) {
	apply_binary(other, _or);
	return *this;
}

big_integer& big_integer::operator^=(big_integer const& other) {
	apply_binary(other, _xor);
	return *this;
}

big_integer big_integer::operator+() const {
	return *this;
}

big_integer big_integer::operator~() const {
	big_integer ret = *this;
	ret = -ret - 1;
	return ret;
}

big_integer big_integer::operator-() const {
	big_integer ret = *this;
	ret.sign ^= 1;
	ret.verify_sign();
	return ret;
}

big_integer& big_integer::operator++() {
	(*this) += 1;
	return *this;
}

big_integer big_integer::operator++(int) {
	big_integer ret = *this;
	++*this;
	return ret;
}

big_integer& big_integer::operator--() {
	(*this) -= 1;
	return *this;
}

big_integer big_integer::operator--(int) {
	big_integer ret = *this;
	--*this;
	return ret;
}

big_integer& big_integer::operator>>=(int shift) {
	int big_shift = shift / BASEPOW;
	int small_shift = shift % BASEPOW;
	uint64_t val = (sign ? 0 : BASE-1);
	to_byte();
	for (int i = 0; i + big_shift < (int) vec.size(); ++i) {
		vec[i] = vec[i + big_shift];
		if (big_shift) {
			vec[i + big_shift] = val;
		}
		if (i) {
			vec[i-1] %= (1UL << (32 - small_shift));
			vec[i-1] |= ((vec[i] % (1UL << small_shift)) << (32-small_shift));
		}
		vec[i] >>= small_shift;
		vec[i] |= (val << (32 - small_shift)) % BASE;
	}
	from_byte();
	del_zeros(vec);
	return *this;
}

big_integer& big_integer::operator<<=(int shift) {
	int big_shift = shift / BASEPOW;
	int small_shift = shift % BASEPOW;

	int n = vec.size() + big_shift + 1;
	vec.resize(n);

	for (int i = n-2; i >= big_shift; --i) {
		vec[i] = vec[i - big_shift];
		if (big_shift)
			vec[i - big_shift] = 0;
		vec[i + 1] |= (vec[i] >> (32 - small_shift));
		vec[i] = (vec[i] << small_shift)%BASE;
	}
	del_zeros(vec);
	return *this;
}

int compare(big_integer const& a, big_integer const& b) {
	big_integer ret = a - b;
	if (!ret.sign) {
		return -1;
	} else if (ret.sz() > 1 || ret.element(0)) {
		return 1;
	}
	return 0;
}

bool operator==(big_integer const& a, big_integer const& b) {
	return compare(a, b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b) {
	return compare(a, b) != 0;
}

bool operator<(big_integer const& a, big_integer const& b) {
	return compare(a, b) == -1;
}

bool operator>(big_integer const &a, big_integer const &b) {
	return compare(a, b) == 1;
}

bool operator<=(big_integer const& a, big_integer const& b) {
	return compare(a, b) != 1;
}

bool operator>=(big_integer const &a, big_integer const& b) {
	return compare(a, b) != -1;
}

big_integer operator+(big_integer a, big_integer const& b) {
	return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
	return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
	return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
	return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
	return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {

	return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
	return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
	return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
	return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
	return a >>= b;
}

std::string to_string(big_integer const& a) {
	uint64_t base = 10;
	vector<uint64_t> demical(1);
	demical[0] = 0;
	std::string ret;

	for (int i = a.sz()-1; i >= 0; --i) {
		mul_long_short(demical, base, BASE);
		add_long_short(demical, base, a.element(i));
	}

	int i = 0;
	int j = demical.size() - 1;
	while (i < j) {
		std::swap(demical[i], demical[j]), ++i, --j;
	}
	for (int i = 0; i < (int) demical.size(); ++i) {
		ret += std::to_string(demical[i]);
	}

	if (!a.sgn()) {
		ret = "-" + ret;
	}

	return ret;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}

uint64_t big_integer::element(size_t pos) const {
	return (pos < vec.size() ? vec[pos] : 0);
}

int big_integer::sz() const {
	return vec.size();
}

template <typename T>
void push(vector<T>& a, int pos)
{
	a[pos+1] += a[pos] / BASE;
	a[pos] %= BASE;
}

template <typename T>
void del_zeros(vector<T>& a)
{
	while (!a.back() && a.size() > 1)
		a.pop_back();
}

void big_integer::verify_sign() {
	if (vec.size() == 1 && !vec.back()) {
		sign = true;
	}
}

template <typename T>
void mul_long_short(vector<T>& a, uint64_t base, uint64_t b) {
	uint64_t carry = 0;
	for (int i = 0; i < (int) a.size() || carry; ++i) {

		if (i == (int) a.size()) {
			a.push_back(0);
		}

		uint64_t val = a[i] * b + carry;
		a[i] = val % base;
		carry = val / base;
	}
	del_zeros(a);
}

template <typename T>
void div_long_short(vector<T>& a, uint64_t base, uint64_t b) {
	uint64_t carry = 0;
	for (int i = a.size()-1; i >= 0; --i) {
		uint64_t val = a[i] + carry * base;

		a[i] = val / b;
		carry = val % b;
	}
	del_zeros(a);
}

template <typename T>
void add_long_short(vector<T>& a, uint64_t base, uint64_t b) {
	uint64_t carry = b;
	for (int i = 0; carry; ++i) {

		if (i == (int) a.size()) {
			a.push_back(0);
		}

		uint64_t val = a[i] + carry;
		a[i] = val % base;
		carry = val / base;
	}
	del_zeros(a);
}

void big_integer::to_byte() {
	if (!sign) {
        vec.push_back(0);
		for (int i = 0; i < (int) vec.size(); ++i) {
			vec[i] = uint32_t(~vec[i]);
		}
		sign = true;
		++*this;
	}
}

void big_integer::from_byte() {
	if (vec.back() & (1UL << 31)) {
		--*this;
		for (int i = 0; i < (int) vec.size(); ++i) {
			vec[i] = uint32_t(~vec[i]);
		}
		sign = false;
	} else {
		sign = true;
	}
}

