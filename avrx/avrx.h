// cdpfw
// Copyright (C) 2022 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef AVRX_H_
#define AVRX_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
// #include <type_traits>
/*
template<class T, class... Rest>
inline constexpr bool are_same_port = (std::is_same_v<typename T::Impl::Port, typename
Rest::Impl::Port> && ...);
*/

#define ALWAYS_INLINE __attribute__((always_inline))

// TODO Masked register?

#define IOREGISTER8(reg)                                        \
  struct reg##Register {                                        \
    using Type = uint8_t;                                       \
    static volatile uint8_t *ptr() { return &reg; }             \
    static inline uint8_t Read() { return *ptr(); }             \
    static inline void Write(uint8_t value) { *ptr() = value; } \
    template <typename... Bits>                                 \
    static inline void SetBits()                                \
    {                                                           \
      *ptr() |= (... | Bits::Mask);                             \
    }                                                           \
    template <uint8_t... Bits>                                  \
    static inline void Write()                                  \
    {                                                           \
      *ptr() = (... | (_BV(Bits)));                             \
    }                                                           \
  }

#define IOREGISTER16(reg)                            \
  struct reg##Register {                             \
    using Type = uint16_t;                           \
    static volatile uint16_t *ptr() { return &reg; } \
  }

namespace avrx {

template <typename ptr_type>
ptr_type pgm_read_pointer(const void *ptr)
{
  return reinterpret_cast<ptr_type>(pgm_read_ptr(ptr));
}

template <typename I, typename O, typename M>
struct GpioPort {
  using InputRegister = I;
  using OutputRegister = O;
  using ModeRegister = M;
};

IOREGISTER8(PORTB);
IOREGISTER8(PINB);
IOREGISTER8(DDRB);
using PortB = GpioPort<PINBRegister, PORTBRegister, DDRBRegister>;

#ifdef PORTC
IOREGISTER8(PORTC);
IOREGISTER8(PINC);
IOREGISTER8(DDRC);
using PortC = GpioPort<PINCRegister, PORTCRegister, DDRCRegister>;
#endif

#ifdef PORTD
IOREGISTER8(PORTD);
IOREGISTER8(PIND);
IOREGISTER8(DDRD);
using PortD = GpioPort<PINDRegister, PORTDRegister, DDRDRegister>;
#endif

template <typename Register, uint8_t bit>
struct RegisterBit {
  using Type = typename Register::Type;
  static constexpr Type Mask = _BV(bit);

  static inline void reset() ALWAYS_INLINE { *Register::ptr() &= ~Mask; }
  static inline void set() ALWAYS_INLINE { *Register::ptr() |= Mask; }
  static inline void set(bool value)
  {
    if (value)
      set();
    else
      reset();
  }

  static inline Type value() { return (*Register::ptr() & Mask) ? 1 : 0; }
};

enum GPIO_MODE { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT };
enum GPIO_STATE { GPIO_RESET, GPIO_SET };

template <typename port, uint8_t bit>
struct GpioBase {
  using Port = port;
  using ModeBit = RegisterBit<typename Port::ModeRegister, bit>;
  using OutputBit = RegisterBit<typename Port::OutputRegister, bit>;
  using InputBit = RegisterBit<typename Port::InputRegister, bit>;

protected:
  static void SetMode(GPIO_MODE gpio_mode) { ModeBit::set(GPIO_MODE_INPUT != gpio_mode); }
  static void EnablePullup(bool enable) { OutputBit::set(enable); }

  static inline void set() ALWAYS_INLINE { OutputBit::set(); }
  static inline void reset() ALWAYS_INLINE { OutputBit::reset(); }

  static uint8_t value() ALWAYS_INLINE { return InputBit::value(); }
};

template <typename Port, uint8_t bit>
struct AFPin : public GpioBase<Port, bit> {
  using Impl = GpioBase<Port, bit>;

  static void OverridePullupEnable() { Impl::EnablePullup(true); }
};

template <typename Port, uint8_t bit, bool pullup = false>
struct InputPin : public GpioBase<Port, bit> {
  using Impl = GpioBase<Port, bit>;
  static inline void Init()
  {
    Impl::SetMode(GPIO_MODE_INPUT);
    Impl::EnablePullup(pullup);
  }

  static inline uint8_t is_high() { return Impl::value(); }
  static inline uint8_t is_low() { return !Impl::value(); }
};

template <typename Port, uint8_t bit, GPIO_STATE init_state>
struct OutputPin : public GpioBase<Port, bit> {
  using Impl = GpioBase<Port, bit>;
  static inline void Init()
  {
    Impl::SetMode(GPIO_MODE_OUTPUT);
    set(GPIO_SET == init_state);
  }

  static inline void set() ALWAYS_INLINE { Impl::set(); }
  static inline void reset() ALWAYS_INLINE { Impl::reset(); }
  static inline void set(bool value)
  {
    if (value)
      set();
    else
      reset();
  }
};

template <typename Port, uint8_t bit, bool enable_pullup = false>
struct IOPin : public GpioBase<Port, bit> {
  using Impl = GpioBase<Port, bit>;

  static inline void Init() ALWAYS_INLINE
  {
    Impl::SetMode(GPIO_MODE_INPUT);
    Impl::EnablePullup(enable_pullup);
  }

  static inline void SetInputMode(bool pullup) ALWAYS_INLINE
  {
    Impl::SetMode(GPIO_MODE_INPUT);
    Impl::EnablePullup(pullup);
  }

  static inline uint8_t value() ALWAYS_INLINE { return Impl::value(); }
  static inline void SetOutputMode() ALWAYS_INLINE { Impl::SetMode(GPIO_MODE_OUTPUT); }

  static inline void set() ALWAYS_INLINE { Impl::set(); }
  static inline void reset() ALWAYS_INLINE { Impl::reset(); }
};

#define GPIO(P, N) avrx::IOPin<avrx::Port##P, N>
#define GPIO_PU(P, N) avrx::IOPin<avrx::Port##P, N, true>
#define GPIO_OUT(P, N, S) avrx::OutputPin<avrx::Port##P, N, avrx::S>
#define GPIO_IN(P, N) avrx::InputPin<avrx::Port##P, N>
#define GPIO_IN_PU(P, N) avrx::InputPin<avrx::Port##P, N, true>
#define GPIO_AF(P, N) avrx::AFPin<avrx::Port##P, N>

template <typename Pin, GPIO_STATE state>
struct ScopedPulse {
  ScopedPulse() { Pin::set(state); }
  ~ScopedPulse() { Pin::set(!state); }
};

template <typename... Pins>
inline void InitPins()
{
  (Pins::Init(), ...);
}

}  // namespace avrx

#endif  // AVRX_H_
