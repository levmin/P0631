/*
   Include <math> or simulate it if it is not available
*/
#include <climits>
#if __has_include(<math>)
#include <math>
#else
#include <type_traits>
#include <limits>
template<class T> concept FloatingPoint = std::is_floating_point_v<T>;
namespace std {
   namespace math {

      template<typename> inline constexpr bool __always_false = false;

      template<typename T> inline constexpr T __invalidTemplate() {
         static_assert(__always_false<T>, "This template needs to be specialized."); return T();
      }

      template <typename T> inline constexpr T pi_v = __invalidTemplate<T>();
      template <FloatingPoint T> inline constexpr T pi_v<T> = 3.141592653589793238462643383279502884L;
      template double pi_v<double>;
      inline constexpr double pi = pi_v<double>;
   }
}

#endif
/*
   High precision FP type generalizing the internal representation of IEEE754 binary floating point numbers.
*/

template <typename N, typename D, typename E, typename F> class floating_t
{
   N m_numerator;
   D m_denominator;
   E m_exponent;
   static_assert(std::numeric_limits<N>::is_integer);
   static_assert(std::numeric_limits<D>::is_integer);
   static_assert(std::numeric_limits<E>::is_integer);
   static_assert(!std::numeric_limits<F>::is_integer);
   static_assert(std::numeric_limits<N>::is_signed);
   static_assert(!std::numeric_limits<D>::is_signed);
   static_assert(std::numeric_limits<E>::is_signed);
   static_assert(std::numeric_limits<N>::digits ==
      std::numeric_limits<D>::digits - 1);
   static constexpr unsigned exponent_length = CHAR_BIT * sizeof(F) -
      std::numeric_limits<F>::digits;
   static constexpr unsigned mantissa_length = std::numeric_limits<F>::digits - 1;
   static_assert(CHAR_BIT * sizeof(D) > mantissa_length);
   static_assert(std::numeric_limits<E>::digits >= exponent_length);
   static_assert(std::numeric_limits<N>::digits >
      std::numeric_limits<F>::digits);

   constexpr F getFloatingPointValue() const
   {
      F val = static_cast<F>(m_numerator) / static_cast<F>(m_denominator);
      for (E i = std::numeric_limits<F>::max_exponent; i <= m_exponent; i++)
         val *= 2;
      return val;
   }

public:

   constexpr floating_t(const N & num, const D & den, const E & e) :
      m_numerator(num), m_denominator(den), m_exponent(e) {}

   constexpr floating_t(F value) : m_numerator(0), m_denominator(1),
      m_exponent(std::numeric_limits<F>::max_exponent - 1)
   {
      m_denominator <<= mantissa_length;
      bool isNegative = false;
      if (value < 0)
      {
         isNegative = true;
         value = -value;
      }
      if (value < 1)
         do
            m_exponent--;
      while ((value = 2 * value) < 1);
      else if (value > 2)
         do
            m_exponent++;
      while ((value = value / 2) > 2);
      m_numerator = static_cast<N>(value * static_cast<F>(m_denominator));
      if (isNegative)
         m_numerator = -m_numerator;
   }

   constexpr operator F() const
   {
      return getFloatingPointValue();
   }

   constexpr bool validate(F value) const
   {//validate that the supposed value of the class is indeed equal to value
      return value == getFloatingPointValue();
   }

   /*
   Many other lines of code
   */

};

using myfptype = floating_t<signed long long, unsigned long long, short, double>;

constexpr myfptype dp_pi (std::math::pi);
static_assert(dp_pi.validate(std::math::pi));

static_assert(std::math::pi == (double)3.141'592'653'589'793'3L);
static_assert(std::math::pi == (double)3.141'592'653'589'793'0L);

template<> inline constexpr myfptype std::math::pi_v<myfptype>
{3'141'592'653'589'793'238L, 1'000'000'000'000'000'000L, 1};

//The internal precision of hp_pi is 19 decimal digits. 
constexpr myfptype hp_pi = std::math::pi_v<myfptype>;
constexpr myfptype almost_pi{ 3'141'592'653'589'793'237L, 1'000'000'000'000'000'000L, 1 };

static_assert(hp_pi.validate(std::math::pi));
static_assert(almost_pi.validate(std::math::pi));

/*
   Bignum FP type templated by its precision
*/

template <int PRECISION> class BigNumFPType
{
public:
   constexpr BigNumFPType (double value) :m_state(value) {};
   constexpr static BigNumFPType getPI() { return BigNumFPType(std::math::pi); }
   constexpr static BigNumFPType Pi = getPI();
   constexpr double roundToDouble() const { return m_state; }
private:
   typedef double InternalState; //could and should be much more sophisticated
   InternalState m_state;
};

//Explicit specialization of std::math::pi_v for BigNumFPType
template<int PRECISION> inline constexpr BigNumFPType<PRECISION> std::math::pi_v<BigNumFPType<PRECISION>> = BigNumFPType<PRECISION>::getPI();

constexpr auto v = std::math::pi_v<BigNumFPType<100>>;

static_assert(v.roundToDouble() == std::math::pi);

/*
   Bignum FP type with dynamic precision
*/

class DynamicBigNumFPType
{
public:
   constexpr DynamicBigNumFPType (double value, int precision) :m_state(value), m_precision(precision) {};
   void setPrecision(int precision) { m_precision = precision; }
   //notice that getPI() isn't constexpr or static, because pi is no longer deemed as a constant but rather as an algorithm running until a certain precision is achieved
   //the real implementation would calculate pi based upon m_precision and construct DynamicBigNumFPType from it  
   DynamicBigNumFPType getPI() { return DynamicBigNumFPType(std::math::pi, m_precision); }
   double roundToDouble() const { return m_state; }
private:
   typedef double InternalState; //could and should be much more sophisticated
   InternalState m_state;
   int m_precision;
};

//Explicit specialization of std::math::pi_v for BigNumFPType
template<> inline DynamicBigNumFPType std::math::pi_v<DynamicBigNumFPType> = DynamicBigNumFPType(std::math::pi, 0).getPI();

int main() {

   DynamicBigNumFPType pi{ std::math::pi_v<DynamicBigNumFPType> };
   for (int i = 0; i < 100; i++)
   {
      pi.setPrecision(i);
      //use pi in calculations, break if the right result is achieved
   }
}
