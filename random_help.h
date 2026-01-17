#if !defined(RANDOM_HELP_H)
#define RANDOM_HELP_H

#include <random>

template<typename T>
inline T random_real() {
    static std::uniform_real_distribution<T> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

template<typename T>
inline T random_real(T min, T max) {
	return min + (max-min)*random_real<T>();
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
	return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    // Return a random interger in [min,max].
    return int(random_double(min, max+1));
}
#endif
